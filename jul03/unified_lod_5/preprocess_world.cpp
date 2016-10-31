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
#include "mesh_topology_handler.h"
#include "mesh_builder.h"
#include "error_quadric.h"
#include "covariance.h"

#include <math.h>  // For sqrtf

const int TRIANGLE_MAXIMUM = 6000;
    
void find_bounding_box(Triangle_List_Mesh *mesh, Vector3 *bbox_min, Vector3 *bbox_extents);


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
    void recompute_leaf_distances(World_Block *block);

  protected:
    void block_init_for_subdivide_step(World_Block *block, Triangle_List_Mesh *mesh);
    void localize_block(World_Block *block);
    void chop_leaves(World_Block *root);
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

void sanity_check(Mesh_Seam *seam) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        World_Block *block = seam->block_membership[index->which_mesh];
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}

void scale_mesh(Triangle_List_Mesh *mesh, float factor) {
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        mesh->vertices[i] *= factor;
    }
}

#include "quake3_bsp.h"

int get_num_triangles(BSP_BIQUADRATIC_PATCH *patch) {
    int t = patch->tesselation;
    return t*t*2;
}

void do_patch(Triangle_List_Mesh *mesh, int triangle_index, BSP_PATCH *patch) {

}

Triangle_List_Mesh *load_quake() {
    const int CURVE_TESSELATION = 8;

    BSP *bsp = new BSP();
//    bool success = bsp->load("data\\test1.bsp", CURVE_TESSELATION);
    bool success = bsp->load("hal9000_b_ta.bsp", CURVE_TESSELATION);
    if (!success) {
        printf("File load error.  Aborting!\n");
        exit(1);
    }

    assert(success);

    int num_faces = bsp->numPolygonFaces;
    int num_polyface_triangles = 0;
    int num_patch_triangles = 0;
    int num_mesh_face_triangles = 0;

    int i;
    for (i = 0; i < bsp->numPolygonFaces; i++) {
        int nvertices = bsp->polygonFaces[i].numVertices;
        assert(nvertices >= 3);
        num_polyface_triangles += nvertices - 2;
    }

    for (i = 0; i < bsp->numMeshFaces; i++) {
        BSP_MESH_FACE *face = &bsp->meshFaces[i];
        num_mesh_face_triangles += face->numMeshIndices / 3;
    }


    int j;
    for (i = 0; i < bsp->numPatches; i++) {
        BSP_PATCH *patch = &bsp->patches[i];

        for (j = 0; j < patch->numQuadraticPatches; j++) {
            num_patch_triangles += get_num_triangles(&patch->quadraticPatches[j]);
        }
    }

    int num_triangles = num_polyface_triangles + num_patch_triangles + num_mesh_face_triangles;

    Mesh_Builder builder(num_triangles * 3, num_triangles);
    for (i = 0; i < bsp->numTextures; i++) {
        Mesh_Material_Info *info = new Mesh_Material_Info;
        info->name = strdup(bsp->loadTextures[i].name);
        builder.add_material(info);
    }


    for (i = 0; i < bsp->numVertices; i++) {
        Vector3 position = bsp->vertices[i].position;
        float u = bsp->vertices[i].decalS;
        float v = bsp->vertices[i].decalT;
        
        builder.add_vertex(position, Vector2(u, v), Quaternion(0, 0, 0, 1));
    }

    for (i = 0; i < bsp->numPolygonFaces; i++) {
        int nvertices = bsp->polygonFaces[i].numVertices;
        int first_index = bsp->polygonFaces[i].firstVertexIndex;
        int material = bsp->polygonFaces[i].textureIndex;

        int j;
        for (j = 2; j < nvertices; j++) {
            int n0 = 0;
            int n1 = j-1;
            int n2 = j;

            // Flip the triangle's clockwiseness...

            builder.add_triangle(first_index + n0, first_index + n2,
                                 first_index + n1, material);
        }
    }


    for (i = 0; i < bsp->numPatches; i++) {
        BSP_PATCH *patch = &bsp->patches[i];
        int material = patch->textureIndex;

        for (j = 0; j < patch->numQuadraticPatches; j++) {
            BSP_BIQUADRATIC_PATCH *quadratic = &patch->quadraticPatches[j];
            
            int t = quadratic->tesselation;
            int num_vertices = (t+1)*(t+1);
            
            int vertex_offset = builder.num_vertices;

            int k;
            for (k = 0; k < num_vertices; k++) {
                BSP_VERTEX *vertex = &quadratic->vertices[k];
                builder.add_vertex(vertex->position,
                                   Vector2(vertex->decalS, vertex->decalT), Quaternion(0, 0, 0, 1));
            }


            int num_triangles_each_row = 2*t;

            int row;
            for (row = 0; row < t; row++) {
                int point;
                for (point = 0; point < t; point++) {
                    int n0 = row*(t+1)+point;
                    int n1 = (row+1)*(t+1)+point;
                    int n2 = n1 + 1;
                    int n3 = n0 + 1;

                    n0 += vertex_offset;
                    n1 += vertex_offset;
                    n2 += vertex_offset;
                    n3 += vertex_offset;

                    builder.add_triangle(n0, n1, n2, material);
                    builder.add_triangle(n0, n2, n3, material);
                }
            }
        }
    }



    for (i = 0; i < bsp->numMeshFaces; i++) {
        BSP_MESH_FACE *face = &bsp->meshFaces[i];
        int material = face->textureIndex;

        int k;
        for (k = 0; k < face->numMeshIndices; k += 3) {
            int n0 = bsp->meshIndices[face->firstMeshIndex + k];
            int n1 = bsp->meshIndices[face->firstMeshIndex + k + 1];
            int n2 = bsp->meshIndices[face->firstMeshIndex + k + 2];
            
            n0 += face->firstVertexIndex;
            n1 += face->firstVertexIndex;
            n2 += face->firstVertexIndex;

            builder.add_triangle(n0, n2, n1, material);
        }
    }


    printf("\nBuild mesh\n");

    Triangle_List_Mesh *mesh = builder.build_mesh();
    scale_mesh(mesh, 1.0f / 12.0f);

       
    Tangent_Frame_Maker maker;
    maker.compute_tangent_frames(mesh);
    delete [] mesh->tangent_frames;
    mesh->tangent_frames = maker.tangent_frames;
    maker.tangent_frames = NULL;

    return mesh;
}

void World_Processor::block_init_for_subdivide_step(World_Block *block, Triangle_List_Mesh *mesh) {
    find_bounding_box(mesh,
                      &block->bounding_box_corner,
                      &block->bounding_box_extents);

    block->position = Vector3(0, 0, 0);

    block->mesh = mesh;
    block->block_id = (Block_Identifier)(next_block_id++);
}


void World_Processor::localize_block(World_Block *block) {
    Vector3 position = block->bounding_box_corner + block->bounding_box_extents * 0.5f;

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
    printf("Cleanup non_root\n");
    cleanup_non_root_blocks(root);

    printf("Delete seams\n");
    construction_database->delete_seams_that_touch_marked_blocks();
    unmark_blocks(root);

    // @Refactor: Assertion below... can be taken out when everything is working.
/*
    Mesh_Seam *seam;
    Array_Foreach(&construction_database->seams, seam) {
        int i;
        for (i = 0; i < 3; i++) {
            World_Block *block = seam->block_membership[i];
            if (block) assert(block->mesh);
        }
    } Endeach;
*/
}

void sanity_check(Triangle_List_Mesh *mesh) {
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        assert(fabs(mesh->vertices[i].x) < 100000.0f);
        assert(!_isnan(mesh->vertices[i].x));
        assert(_finite(mesh->vertices[i].x));
    }
}

void World_Processor::recompute_leaf_distances(World_Block *block) {
    if (block->num_children == 0) {
        block->leaf_distance = 0;
        return;
    }

    int leaf_distance_min = 99999;   // @Refactor: Icky hack-type thing
    int i;
    for (i = 0; i < block->num_children; i++) {
        recompute_leaf_distances(block->children[i]);
        int dist = block->children[i]->leaf_distance;
        if (dist < leaf_distance_min) leaf_distance_min = dist;
    }

    block->leaf_distance = leaf_distance_min + 1;
}

float get_squared_error(Mesh_Reducer *reducer, int index) {
    Error_Quadric *quadric = reducer->get_quadric(index);

    float xyzuv[5];
    reducer->fill_point(xyzuv, index, -1);
    float error = quadric->evaluate_error(xyzuv);

    // The error we return takes into account textural error,
    // in addition to position error.  Maybe this is not strictly
    // what you want but for most applications it is
    // probably fine.  (ATTENTION: This differs from what was
    // originally written in the companion article; what was
    // written there was actually an incorrect solution to
    // the problem).

    if (error < 0) error = 0; // Takes care of floating-point rounding issue
    return sqrtf(error);
}

float get_root_mean_squared_error(Mesh_Reducer *reducer, Triangle_List_Mesh *orig_mesh) {
    float sum_of_squares = 0;

    Mesh_Topology_Handler *handler = reducer->topology_handler;

    int i;
    for (i = 0; i < reducer->initial_num_vertices; i++) {
        if (!(handler->vertex_flags[i] & VERTEX_IS_LIVE)) continue;
        if (orig_mesh->canonical_vertex_map[i] != i) continue;

        float square = get_squared_error(reducer, i);
        sum_of_squares += square;
    }

    return sqrt(sum_of_squares);
}

void World_Processor::do_recursive_merge(World_Block *root) {
    if (root->num_children == 0) return;

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

    printf("merge %d\n", node->leaf_distance);
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
    for (i = 0; i < num_blocks; i++) {
        blocks[i]->index_map_into_parent = index_maps[i];
    }

    Triangle_List_Mesh *reduced;
    Mesh_Reducer reducer;
    reducer.tuning.texture_space_importance_factor = 20.0f;
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
    reducer.reduce(TRIANGLE_MAXIMUM);
    reducer.get_result(&reduced);
    block->mesh = reduced;
    block->worldspace_error = get_root_mean_squared_error(&reducer, merged_mesh);

    for (i = 0; i < num_blocks; i++) {
        World_Block *child_block = blocks[i];
        if (block->worldspace_error < child_block->worldspace_error) 
            block->worldspace_error = child_block->worldspace_error;
    }

    void xref_block_index_map(World_Block *block, Mesh_Reducer *reducer);
    for (i = 0; i < num_blocks; i++) {
        xref_block_index_map(blocks[i], &reducer);
    }

    delete merged_mesh;

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

//    sanity_check(seam);
}

Vertex_Specifier *get_spec(Seam_Index *index, Chopped_Result *result) {
    int pre_xref = index->vertex_index;
    Vertex_Specifier spec(pre_xref, pre_xref);
    Vertex_Specifier *found = (Vertex_Specifier *)result->vertex_hash_table->find(&spec);
    return found;
}

int find_output_vertex(Seam_Index *index, Chopped_Result *result) {
    int pre_xref = index->vertex_index;
    
    Vertex_Specifier spec(pre_xref, pre_xref);
    Vertex_Specifier *found = (Vertex_Specifier *)result->vertex_hash_table->find(&spec);
    if (!found) return -1;
    return found->output_index;
}

int find_clip_vertex(Vertex_Specifier *spec, Chopped_Result *result) {
    Vertex_Specifier *found = (Vertex_Specifier *)result->vertex_hash_table->find(spec);
    if (!found) return -1;

    assert(found->output_index != -1);
    return found->output_index;
}

void rectify(Vertex_Specifier *spec) {
    if (spec->input_n0 > spec->input_n1) {
        int tmp = spec->input_n0;
        spec->input_n0 = spec->input_n1;
        spec->input_n1 = tmp;
    }
}


const int MAX_HIERARCHY_DISTANCE = 999;
void World_Processor::do_single_rewrite(World_Block *root,
                                        Mesh_Seam *seam, 
                                        World_Block *block_a,
                                        World_Block *block_b,
                                        Chopped_Result *result_a,
                                        Chopped_Result *result_b) {

    // Check to see if the resulting seam will be within the
    // proper hierarchy distance.  If it won't, we bail!
    int max_hierarchy_distance = MAX_HIERARCHY_DISTANCE;
    int leaf_distance_min, leaf_distance_max;
    leaf_distance_min = leaf_distance_max = block_a->leaf_distance;
    
    int i;
    for (i = 0; i < 3; i++) {
        World_Block *other = seam->block_membership[i];
        if (!other) break;
        if (other == root) continue;
        if (other->leaf_distance < leaf_distance_min) leaf_distance_min = other->leaf_distance;
        if (other->leaf_distance > leaf_distance_max) leaf_distance_max = other->leaf_distance;
    }

    int stride = leaf_distance_max - leaf_distance_min;
    if (stride > max_hierarchy_distance) return;


    // Okay let's actually do it...


//    sanity_check(seam);

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

    for (i = 0; i < seam->num_faces; i++) {

        int num_a_indices = 0;
        int num_b_indices = 0;
        int num_c_indices = 0;

        Seam_Index tmp_indices_a[3];
        Seam_Index tmp_indices_b[3];
        Seam_Index tmp_indices[3];
        int orig_index[3];
        World_Block *blocks[3];
        Chopped_Result *results[3];
        Mesh_Seam *dest_seams[3];

        Vertex_Specifier *spec_a[3], *spec_b[3], *spec_c[3];

        int k;
        for (k = 0; k < 3; k++) {
            spec_a[k] = spec_b[k] = spec_c[k] = NULL;
        }

        int n0, n1, n2;
        n0 = -1;

        int j;
        for (j = 0; j < 3; j++) {
            Seam_Index *index = &seam->indices[i*3+j];
            orig_index[j] = index->vertex_index;

            World_Block *block = seam->block_membership[index->which_mesh];

            if (block == root) {
                spec_a[j] = get_spec(index, result_a);
                spec_b[j] = get_spec(index, result_b);

                int xref_a, xref_b;
                xref_a = find_output_vertex(index, result_a);
                xref_b = find_output_vertex(index, result_b);
                if (xref_a != -1) {
                    block = block_a;
                    results[j] = result_a;

                    tmp_indices[j] = *index;
                    tmp_indices[j].vertex_index = xref_a;
                    tmp_indices_a[j] = tmp_indices[j];

                    dest_seams[j] = result_seam_a;
                    num_a_indices++;
                }

                if (xref_b != -1) {
                    block = block_b;
                    results[j] = result_b;

                    tmp_indices[j] = *index;
                    tmp_indices[j].vertex_index = xref_b;
                    tmp_indices_b[j] = tmp_indices[j];

                    dest_seams[j] = result_seam_b;
                    num_b_indices++;
                }
            } else {
                tmp_indices[j] = *index;
                tmp_indices_a[j] = *index;
                tmp_indices_b[j] = *index;
                results[j] = NULL;
                dest_seams[j] = NULL;

                n0 = j;
                n1 = (j + 1) % 3;
                n2 = (j + 2) % 3;

                num_c_indices++;
            }

            blocks[j] = block;
        }

        if ((num_c_indices == 2) && (num_a_indices + num_b_indices > 1)) {
            // XXX Fucking fuck it for now
        }

        if (num_a_indices + num_c_indices == 3) {
            add_face(result_seam_a, tmp_indices_a);
        } 

        if (num_b_indices + num_c_indices == 3) {
            add_face(result_seam_b, tmp_indices_b);
        }

        if ((num_a_indices == 1) && (num_b_indices == 1) && (num_c_indices == 1)) {
            // This face spans 3 blocks now... gotta clip it!

            // n0 is the vertex that is not on the block being
            // split... which must exist, so n0 != -1.
            assert(n0 != -1);


            Vertex_Specifier spec(orig_index[n1], orig_index[n2]);
            rectify(&spec);

            Seam_Index new_face[3];
            new_face[0] = tmp_indices[n0];

            int xref_1 = find_clip_vertex(&spec, results[n1]);
            if (xref_1 == -1) continue;  // XXX Think harder about this

            new_face[1] = tmp_indices[n1];
            new_face[2] = tmp_indices[n1];
            new_face[2].vertex_index = xref_1;

            add_face(dest_seams[n1], new_face);


            int xref_2 = find_clip_vertex(&spec, results[n2]);
            if (xref_2 == -1) continue;  // XXX Think harder about this

            new_face[1] = tmp_indices[n2];
            new_face[1].vertex_index = xref_2;
            new_face[2] = tmp_indices[n2];
            add_face(dest_seams[n2], new_face);



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

//    sanity_check(result_seam_a);
//    sanity_check(result_seam_b);

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

Plane3 get_plane_from_mesh(Triangle_List_Mesh *mesh) {
    assert(mesh->num_vertices > 0);

    Vector3 mean(0, 0, 0);
    Covariance3 cov;
    cov.reset();

    // XXXXXXXXXXXXXXXXX do barycenter here, not mesh->vertex
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        mean += mesh->vertices[i];
        cov.accumulate(mesh->vertices[i]);
    }

    float factor = (1.0f / (float)mesh->num_vertices);
    mean *= factor;
    cov.scale(factor);

    Covariance3 local;
    cov.move_to_local_coordinates(&local, mean);
    
    float eigenvalues[3];
    Vector3 eigenvectors[3];
    local.find_eigenvectors(eigenvalues, eigenvectors);

//    mean = Vector3(0, 0, 0);
//    eigenvectors[0] = Vector3(1, 0, 0);

    Vector3 normal = eigenvectors[0];
/*
    if ((fabs(normal.x) > fabs(normal.y)) && (fabs(normal.x) > fabs(normal.z))) {
        normal.set(1, 0, 0);
    } else if ((fabs(normal.y) > fabs(normal.x)) && (fabs(normal.y) > fabs(normal.z))) {
        normal.set(0, 1, 0);
    } else {
        normal.set(0, 0, 1);
    }
*/
    return Plane3(mean, normal);
}

void World_Processor::chop_leaves(World_Block *root) {
    Triangle_List_Mesh *mesh = root->mesh;
    if (!mesh) return;  // Shouldn't happen??

    if (mesh->num_faces <= TRIANGLE_MAXIMUM) return;


    printf("Split %d (%d)\n", root->leaf_distance, mesh->num_faces);

    // Split mesh...


    Mesh_Chopper chopper;
    Chopped_Result result_a;
    Chopped_Result result_b;

    
    Plane3 plane = get_plane_from_mesh(mesh);

    int plane_id = 0;
    Mesh_Seam *seam_a_to_b;
    chopper.chop_mesh(mesh, &plane, plane_id,
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


    // Get rid of it!

    delete mesh;
    root->mesh = NULL;

    if (root->num_children) {
        int i;
        for (i = 0; i < root->num_children; i++) {
            chop_leaves(root->children[i]);
        }
    }
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
        chop_leaves(block);
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

    Triangle_List_Mesh *mesh = load_quake();

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

    printf("Leaf distances.\n");
    processor.recompute_leaf_distances(processor.root);

    save_world(processor.root, processor.construction_database, f);
    fclose(f);
    printf("Saved result as '%s'.\n", output_filename);

    exit(0);
}
