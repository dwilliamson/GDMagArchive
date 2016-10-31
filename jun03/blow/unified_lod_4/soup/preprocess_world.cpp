#include "framework.h"
#include "make_world.h"

#include <stdio.h>
#include <stdlib.h>

#include "seam_database.h"
#include "mesh.h"
#include "mesh_chopper.h"
#include "mesh_seam.h"
#include "tangent_frames.h"
#include <float.h>  // For FLT_MAX and FLT_MIN

#include "mesh_reducer.h"

#include <math.h>  // For debugging...


struct World_Processor {
    World_Processor();
    ~World_Processor();

    int next_block_id;
    Seam_Database *construction_database;
    World_Block *root;

    World_Block *chop_mesh(Triangle_List_Mesh *mesh,
                           Plane3 *planes, int num_planes);
    void finish_subdivide_phase();
    void do_merges();

  protected:
    void block_init_for_subdivide_step(World_Block *block, Triangle_List_Mesh *mesh);
    void localize_block(World_Block *block);
    void chop_leaves(World_Block *root, Plane3 *plane, int plane_id);
    void cleanup_non_root_blocks(World_Block *block);

    void do_single_rewrite(World_Block *root,
                           Mesh_Seam *seam, 
                           World_Block *block_a,
                           World_Block *block_b,
                           Chopped_Result *result_a,
                           Chopped_Result *result_b);
    void do_rewrite_rules(World_Block *root,
                          World_Block *block_a,
                          World_Block *block_b,
                          Chopped_Result *result_a,
                          Chopped_Result *result_b);

    void do_recursive_merge(World_Block *root);
    void unmark_blocks(World_Block *block);
};

void sane(Mesh_Seam *seam) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        World_Block *block = seam->block_membership[index->which_mesh];
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}

#define SMALL_BUNNY 0

// SMALL_BUNNY was a little hack I put in there so I could run
// faster (by processing a smaller data set) when running
// debug builds.
#if SMALL_BUNNY
Triangle_List_Mesh *load_small_triangle_list_mesh(FILE *f);
Triangle_List_Mesh *load_the_bunny() {
    FILE *f = fopen("data\\bunny5000.triangle_list_mesh", "rb");
    assert(f);

    Triangle_List_Mesh *mesh = load_small_triangle_list_mesh(f);
    fclose(f);
    return mesh;
}

#else

// Code that I used to load the bunny and make a Triangle_List_Mesh
// out of it.  Nto at all central to the algorithm!
Triangle_List_Mesh *load_the_bunny() {
    float scale = 30.0f;

    FILE *f = fopen("data\\bunny16000.txt", "rt");
    assert(f);

    int nvertices, nfaces;
    int success;

    success = fscanf(f, "%d\n", &nvertices);
    assert(success);
    fscanf(f, "%d\n", &nfaces);
    assert(success);

    Triangle_List_Mesh *mesh = new Triangle_List_Mesh();
    mesh->allocate_materials(1);
    Mesh_Material_Info *material_info = &mesh->material_info[0];
    material_info->name = app_strdup("bunny_texture");


    mesh->allocate_geometry(nvertices, nfaces);

    Vector3 mean(0, 0, 0);

    int i;
    for (i = 0; i < nvertices; i++) {
        float x, y, z, junk;
        success = fscanf(f, "%f %f %f %f %f", &x, &y, &z, &junk, &junk);
        mesh->vertices[i] = Vector3(x, y, z) * scale;
        mean += mesh->vertices[i];
        mesh->uvs[i].x = 0;
        mesh->uvs[i].y = 0;

        mesh->canonical_vertex_map[i] = i;

        assert(success == 5);
    }

    if (mesh->num_vertices) mean *= (1.0f / mesh->num_vertices);

    for (i = 0; i < nvertices; i++) {
        mesh->vertices[i] -= mean;
        mesh->vertices[i] += Vector3(0, 0, 3);
    }

    mesh->num_triangle_lists = 1;
    mesh->triangle_list_info = new Triangle_List_Info[1];

    Triangle_List_Info *info = &mesh->triangle_list_info[0];
    info->material_index = 0;
    info->num_vertices = nfaces * 3;
    info->start_of_list = 0;

    for (i = 0; i < nfaces; i++) {
        int junk, n0, n1, n2;
        success = fscanf(f, "%d %d %d %d", &junk, &n0, &n1, &n2);
        assert(success == 4);

        mesh->indices[i*3+0] = n0;
        mesh->indices[i*3+1] = n1;
        mesh->indices[i*3+2] = n2;
    }

    Tangent_Frame_Maker maker;
    maker.compute_tangent_frames(mesh);
    delete [] mesh->tangent_frames;
    mesh->tangent_frames = maker.tangent_frames;
    maker.tangent_frames = NULL;
    return mesh;
}

#endif

void World_Processor::block_init_for_subdivide_step(World_Block *block, Triangle_List_Mesh *mesh) {
    Vector3 bbox_min(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 bbox_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        Vector3 *v = &mesh->vertices[i];
        float x = v->x;
        float y = v->y;
        float z = v->z;

        if (x < bbox_min.x) bbox_min.x = x;
        if (y < bbox_min.y) bbox_min.y = y;
        if (z < bbox_min.z) bbox_min.z = z;

        if (x > bbox_max.x) bbox_max.x = x;
        if (y > bbox_max.y) bbox_max.y = y;
        if (z > bbox_max.z) bbox_max.z = z;
    }

    if (mesh->num_vertices == 0) {
        bbox_min = bbox_max = Vector3(0, 0, 0);
    }

    block->bounding_box_corner = bbox_min;
    block->bounding_box_extents = bbox_max - bbox_min;
    block->position = Vector3(0, 0, 0);

    block->mesh = mesh;
    block->block_id = (Block_Identifier)(next_block_id++);
}


void World_Processor::localize_block(World_Block *block) {
    Vector3 position = block->bounding_box_corner + block->bounding_box_extents * 0.5f;
    assert(_finite(position.x));
    block->bounding_box_corner -= position;
    block->position = position;

    int i;
    for (i = 0; i < block->mesh->num_vertices; i++) {
        block->mesh->vertices[i] -= position;
    }
}


void get_bounding_points(World_Block *root, Vector3 results[8]) {
    Vector3 p0 = root->position + root->bounding_box_corner;
    Vector3 p1 = p0 + root->bounding_box_extents;

    results[0] = Vector3(p0.x, p0.y, p0.z);
    results[1] = Vector3(p1.x, p0.y, p0.z);
    results[2] = Vector3(p1.x, p1.y, p0.z);
    results[3] = Vector3(p0.x, p1.y, p0.z);
    results[4] = Vector3(p0.x, p0.y, p1.z);
    results[5] = Vector3(p1.x, p0.y, p1.z);
    results[6] = Vector3(p1.x, p1.y, p1.z);
    results[7] = Vector3(p0.x, p1.y, p1.z);
}

bool block_crosses_plane(World_Block *root, Plane3 *plane) {
    Vector3 bounding_points[8];
    get_bounding_points(root, bounding_points);

    float dot_min = FLT_MAX;
    float dot_max = FLT_MIN;

    int i;
    for (i = 0; i < 8; i++) {
        float dot = plane_dot(plane, &bounding_points[i]);
        if (dot < dot_min) dot_min = dot;
        if (dot > dot_max) dot_max = dot;
    }

    if ((dot_min < 0) && (dot_max > 0)) return true;
    return false;
}

void World_Processor::cleanup_non_root_blocks(World_Block *block) {
    if (block->num_children) {
        block->block_search_marker = 1;
        delete block->mesh;
        block->mesh = NULL;
        
        int i;
        for (i = 0; i < block->num_children; i++) {
            cleanup_non_root_blocks(block->children[i]);
        }
    } else {
        block->block_search_marker = 0;
        localize_block(block);
    }
}

void World_Processor::unmark_blocks(World_Block *block) {
    block->block_search_marker = 0;
    int i;
    for (i = 0; i < block->num_children; i++) unmark_blocks(block->children[i]);
}

void World_Processor::finish_subdivide_phase() {
    cleanup_non_root_blocks(root);
    construction_database->delete_seams_that_touch_marked_blocks();
    unmark_blocks(root);

    // @Refactor: Assertion below... can be taken out when everything is working.
    Mesh_Seam *seam;
    Array_Foreach(&construction_database->seams, seam) {
        int i;
        for (i = 0; i < 3; i++) {
            World_Block *block = seam->block_membership[i];
            if (block) assert(block->mesh);
        }
    } Endeach;
}

void sane(Triangle_List_Mesh *mesh) {
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        assert(fabs(mesh->vertices[i].x) < 100000.0f);
        assert(!_isnan(mesh->vertices[i].x));
        assert(_finite(mesh->vertices[i].x));
    }
}

void World_Processor::do_recursive_merge(World_Block *root) {
    if (root->num_children == 0) {
        assert(_finite(root->position.x));
        return;
    }

    // Make sure all the children are merged...

    int i;
    for (i = 0; i < root->num_children; i++) do_recursive_merge(root->children[i]);

    // Now let's compute our mesh!
    World_Block *node = root;
    assert(node->mesh == NULL);
    

    const int MAX_BLOCKS = 16;
    Triangle_List_Mesh *meshes[MAX_BLOCKS];
    World_Block *blocks[MAX_BLOCKS];

    Vector3 position_sum(0, 0, 0);
    int num_faces_before_seams = 0;
    int num_triangle_lists_before_seams = 0;

    int num_blocks = 0;
    for (i = 0; i < node->num_children; i++) {
        assert(num_blocks < MAX_BLOCKS);

        World_Block *child = node->children[i];
        assert(child);

        meshes[num_blocks] = child->mesh;
        blocks[num_blocks] = child;
        blocks[num_blocks]->temporary_integer_storage = i;

        assert(_finite(blocks[num_blocks]->position.x));
        position_sum += blocks[num_blocks]->position;

        num_faces_before_seams += child->mesh->num_faces;
        num_triangle_lists_before_seams += child->mesh->num_triangle_lists;

        num_blocks++;
    }            

    assert(num_blocks > 0);

    World_Block *block = node;
    block->position = position_sum * (1.0f / (float)num_blocks);
    assert(_finite(block->position.x));

    // @Robustness: Maybe we should first merge, then compute the
    // bounding box, then figure out the new position and recenter
    // the box and mesh etc.
    Vector3 offsets[MAX_BLOCKS];
    for (i = 0; i < num_blocks; i++) {
        offsets[i] = blocks[i]->position - block->position;
    }


    // Figure out which seams to combine into this block
    Auto_Array <Mesh_Seam *> seams;
    construction_database->find_seams_containing_these_blocks(blocks, num_blocks, &seams);

    int *index_maps[MAX_BLOCKS];
    Triangle_List_Mesh *merged_mesh = merge_meshes(num_blocks, meshes, offsets,
                                                   index_maps, &seams);
    sane(merged_mesh);

    for (i = 0; i < num_blocks; i++) {
        blocks[i]->index_map_into_parent = index_maps[i];
    }

    Triangle_List_Mesh *reduced;
    Mesh_Reducer reducer;
    reducer.init(merged_mesh);

    int seam_face_index = num_faces_before_seams;
    for (i = num_triangle_lists_before_seams; i < merged_mesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &merged_mesh->triangle_list_info[i];

        int j;
        assert(info->num_vertices % 3 == 0);
        for (j = 0; j < info->num_vertices / 3; j++) {
            reducer.mark_face_as_seam(seam_face_index);
            seam_face_index++;
        }
    }

    reducer.collapse_similar_vertices(0.001);
    reducer.reduce(num_faces_before_seams / 4);
    reducer.get_result(&reduced);
    block->mesh = reduced;

    sane(reduced);

    void xref_block_index_map(World_Block *block, Mesh_Reducer *reducer);
    for (i = 0; i < num_blocks; i++) {
        xref_block_index_map(blocks[i], &reducer);
    }

    delete merged_mesh;

    // XXXXX Redundant with other bounding box code in this file?
    void find_bounding_box(Triangle_List_Mesh *mesh, Vector3 *bbox_min, Vector3 *bbox_extents);

    Vector3 bbox_max;
    find_bounding_box(reduced, 
                      &block->bounding_box_corner,
                      &block->bounding_box_extents);

    for (i = 0; i < num_blocks; i++) {
        Rewrite_Rule rule;
        rule.source_block = blocks[i];
        rule.dest_block = block;
        rule.index_map = blocks[i]->index_map_into_parent;
        construction_database->perform_rewrite_rule(&rule, 100);
    }
    
}

void World_Processor::do_merges() {
    do_recursive_merge(root);
}

void maybe_add(Seam_Database *database, Mesh_Seam *seam) {
    if (seam->num_faces == 0) {
        delete seam;
        return;
    }

    database->add(seam);
}

void rewrite_block_membership(Mesh_Seam *src_seam, Mesh_Seam *dest_seam,
                              World_Block *src_block, World_Block *dest_block) {
    int i;
    for (i = 0; i < 3; i++) {
        World_Block *block = src_seam->block_membership[i];
        if (block == src_block) block = dest_block;
        dest_seam->block_membership[i] = block;
    }
}


void add_face(Mesh_Seam *seam, Seam_Index *indices) {
    int i = seam->num_faces++;
    seam->indices[i*3+0] = indices[0];
    seam->indices[i*3+1] = indices[1];
    seam->indices[i*3+2] = indices[2];
}

int find_output_vertex(Seam_Index *index, Chopped_Result *result) {
    int pre_xref = index->vertex_index;
    
    Vertex_Specifier spec(pre_xref, pre_xref, 0);
    Vertex_Specifier *found = (Vertex_Specifier *)result->vertex_hash_table->find(&spec);
    if (!found) return -1;
    return found->output_index;
}

int find_clip_vertex(Vertex_Specifier *spec, Chopped_Result *result) {
    Vertex_Specifier *found = (Vertex_Specifier *)result->vertex_hash_table->find(spec);
    if (!found) return -1;
    return found->output_index;
}

void rectify(Vertex_Specifier *spec) {
    if (spec->input_n0 > spec->input_n1) {
        int tmp = spec->input_n0;
        spec->input_n0 = spec->input_n1;
        spec->input_n1 = tmp;
    }
}

void World_Processor::do_single_rewrite(World_Block *root,
                                        Mesh_Seam *seam, 
                                        World_Block *block_a,
                                        World_Block *block_b,
                                        Chopped_Result *result_a,
                                        Chopped_Result *result_b) {

    // @Memory: We are allocating a lot of extra memory that gets unused
    // for these seams, potentially.  Of course it'll get freed when we
    // delete them or save the database.  But still, maybe we want to
    // think about shrinking them or being more careful somehow.
    Mesh_Seam *result_seam_a = new Mesh_Seam(seam->num_faces * 2);
    Mesh_Seam *result_seam_b = new Mesh_Seam(seam->num_faces * 2);
    result_seam_a->num_faces = 0;
    result_seam_b->num_faces = 0;

    rewrite_block_membership(seam, result_seam_a, root, block_a);
    rewrite_block_membership(seam, result_seam_b, root, block_b);

    int i;
    for (i = 0; i < seam->num_faces; i++) {

        int num_a_indices = 0;
        int num_b_indices = 0;

        Seam_Index tmp_indices[3];
        int orig_index[3];
        World_Block *blocks[3];
        Chopped_Result *results[3];
        Mesh_Seam *dest_seams[3];

        int n0, n1, n2;

        int j;
        for (j = 0; j < 3; j++) {
            Seam_Index *index = &seam->indices[i*3+j];
            orig_index[j] = index->vertex_index;

            World_Block *block = seam->block_membership[index->which_mesh];

            if (block == root) {
                int xref;
                xref = find_output_vertex(index, result_a);
                if (xref != -1) {
                    block = block_a;
                    results[j] = result_a;

                    tmp_indices[j] = *index;
                    tmp_indices[j].vertex_index = xref;
                    
                    dest_seams[j] = result_seam_a;
                    num_a_indices++;
                } else {
                    block = block_b;
                    results[j] = result_b;

                    xref = find_output_vertex(index, result_b);
                    assert(xref != -1);
                    tmp_indices[j] = *index;
                    tmp_indices[j].vertex_index = xref;

                    dest_seams[j] = result_seam_b;
                    num_b_indices++;
                }
            } else {
                tmp_indices[j] = *index;
                results[j] = NULL;
                dest_seams[j] = NULL;

                n0 = j;
                n1 = (j + 1) % 3;
                n2 = (j + 2) % 3;
            }

            blocks[j] = block;
        }

        if (num_a_indices == 0) {
            add_face(result_seam_b, tmp_indices);
        } else if (num_b_indices == 0) {
            add_face(result_seam_a, tmp_indices);
        } else {
            // This face spans 3 blocks now... gotta clip it!
            Vertex_Specifier spec(orig_index[n1], orig_index[n2], 0);
            rectify(&spec);

            Seam_Index new_face[3];
            new_face[0] = tmp_indices[n0];

            int xref_1 = find_clip_vertex(&spec, results[n1]);
            assert(xref_1 != -1);

            if (xref_1 != -1) {
                new_face[1] = tmp_indices[n1];
                new_face[2] = tmp_indices[n1];
                new_face[2].vertex_index = xref_1;

                add_face(dest_seams[n1], new_face);
            }

            int xref_2 = find_clip_vertex(&spec, results[n2]);
            assert(xref_2 != -1);

            if (xref_2 != -1) {
                new_face[1] = tmp_indices[n2];
                new_face[1].vertex_index = xref_2;
                new_face[2] = tmp_indices[n2];
                add_face(dest_seams[n2], new_face);
            }


            Mesh_Seam *new_seam = new Mesh_Seam(1); // XXX spammy!
            new_seam->num_faces = 0;
            new_seam->block_membership[0] = blocks[n0];
            new_seam->block_membership[1] = blocks[n1];
            new_seam->block_membership[2] = blocks[n2];

            // new_face[0].vertex_index stays the same from above.
            new_face[0].which_mesh = 0;
            new_face[1].which_mesh = 1;
            new_face[2].which_mesh = 2;

            new_face[1].vertex_index = xref_1;
            new_face[2].vertex_index = xref_2;

            add_face(new_seam, new_face);
            maybe_add(construction_database, new_seam);
        }
    }

    maybe_add(construction_database, result_seam_a);
    maybe_add(construction_database, result_seam_b);
}

void World_Processor::do_rewrite_rules(World_Block *root,
                                       World_Block *block_a,
                                       World_Block *block_b,
                                       Chopped_Result *result_a,
                                       Chopped_Result *result_b) {
    Auto_Array <Mesh_Seam *> seams;
    construction_database->find_seams_containing_at_least_this_block(root, &seams);

    Mesh_Seam *seam;
    Array_Foreach(&seams, seam) {
        do_single_rewrite(root, seam, block_a, block_b, result_a, result_b);
    } Endeach;
}

void World_Processor::chop_leaves(World_Block *root, Plane3 *plane,
                                  int plane_id) {
    if (!block_crosses_plane(root, plane)) return;

    if (root->num_children) {
        int i;
        for (i = 0; i < root->num_children; i++) {
            chop_leaves(root->children[i], plane, plane_id);
        }

        return;
    }

    Mesh_Chopper chopper;
    Chopped_Result result_a;
    Chopped_Result result_b;

    Mesh_Seam *seam_a_to_b;
    chopper.chop_mesh(root->mesh, plane, plane_id,
                      &result_a, &result_b, &seam_a_to_b);

    World_Block *block_a = new World_Block();
    block_init_for_subdivide_step(block_a, result_a.result_mesh);

    World_Block *block_b = new World_Block();
    block_init_for_subdivide_step(block_b, result_b.result_mesh);

    seam_a_to_b->block_membership[0] = block_a;
    seam_a_to_b->block_membership[1] = block_b;

    block_a->leaf_distance = root->leaf_distance - 1;
    block_b->leaf_distance = root->leaf_distance - 1;
    if (seam_a_to_b->num_faces) construction_database->add(seam_a_to_b);

    root->children[0] = block_a;
    root->children[1] = block_b;
    root->num_children = 2;

    do_rewrite_rules(root, block_a, block_b, &result_a, &result_b);
    delete result_a.vertex_hash_table;
    delete result_b.vertex_hash_table;
}

World_Processor::World_Processor() {
    next_block_id = 1;  // 0 is reserved!
    construction_database = new Seam_Database();
    root = NULL;
}

World_Processor::~World_Processor() {
    delete construction_database;
}

World_Block *World_Processor::chop_mesh(Triangle_List_Mesh *mesh,
                                        Plane3 *planes, int num_planes) {
    World_Block *block = new World_Block();
    block_init_for_subdivide_step(block, mesh);
    block->leaf_distance = 500;

    int i;
    for (i = 0; i < num_planes; i++) {
        chop_leaves(block, &planes[i], 0);  // All planes are id #0!
    }

    root = block;
    return block;
}

void main(int argc, char **argv) {
    Elevation_Map *map;
    char *filename = NULL;

    if (argc > 1) {
        filename = argv[1];
    }

/*
    if (filename == NULL) {
        fprintf(stderr, "Please specify a file to load!\n");
        exit(1);
    }

    if (map == NULL) {
        assert(filename);
        fprintf(stderr, "Unable to open map file '%s'!\n", filename);
        exit(1);
    }

*/

    Triangle_List_Mesh *mesh = load_the_bunny();
    if (mesh == NULL) {
        fprintf(stderr, "Could not load the bunny!\n");
        exit(1);
    }

/*
    // This code was here just to help me build the 5000 triangle bunny;
    // it's not meant for general use, but I left it here just in case.
    Mesh_Reducer reducer;
    reducer.init(mesh);
    reducer.collapse_similar_vertices(0.001);
    reducer.reduce(5000);
    Triangle_List_Mesh *reduced;
    reducer.get_result(&reduced);
    FILE *g = fopen("bunny5000.triangle_list_mesh", "wb");
    void save_small_triangle_list_mesh(Triangle_List_Mesh *mesh, FILE *f);
    save_small_triangle_list_mesh(reduced, g);
    fclose(g);
*/

    char *output_filename = "output.world_lod";
    FILE *f = fopen(output_filename, "wb");
    if (f == NULL) {
        fprintf(stderr, "Unable to open output file '%s' for writing!\n",
                output_filename);
        exit(1);
    }

    // Make some splitting planes
    
    const int MAX_PLANES = 20;
    Plane3 planes[MAX_PLANES];
    int num_planes = 0;
    const float k = 1.0f;

    planes[num_planes++] = Plane3(Vector3(0, 0, 0), Vector3(1, 0, 0));
    planes[num_planes++] = Plane3(Vector3(0, 0, 3), Vector3(0, 0, 1));

    Vector3 weird1(1, 0, 1);
    weird1.normalize();
    Vector3 weird2(.11, -.4, 1);
    weird2.normalize();
    planes[num_planes++] = Plane3(Vector3(0.5, 0, 3), weird1);
    planes[num_planes++] = Plane3(Vector3(0, 0, 2.6), weird2);

    World_Processor processor;
    processor.chop_mesh(mesh, planes, num_planes);
    processor.finish_subdivide_phase();

    processor.do_merges();

    save_world(processor.root, processor.construction_database, f);
    fclose(f);
    printf("Saved result as '%s'.\n", output_filename);

    exit(0);
}
