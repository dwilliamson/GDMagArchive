#include "framework.h"

#include "mesh.h"
#include "mesh/mesh_builder.h"
#include "make_terrain.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include "tangent_frames.h"  // AAA for debugging  ... XXX eliminate?
#include "seam_database.h"

#include <float.h>
#include <math.h>
#include <stdio.h> // @Refactor for terrain loading; remove?

Elevation_Map *make_elevation_map();

bool is_power_of_two(unsigned int i) {
    if (i == 0) return false;
    if (i & (i - 1)) return false;
    return true;
}


// Find the bounding box of a mesh... we store this bounding box
// on the terrain block.

void find_bounding_box(Triangle_List_Mesh *mesh, Vector3 *bbox_min, Vector3 *bbox_extents) {
    Vector3 max;
    Vector3 *bbox_max = &max;

    *bbox_min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    *bbox_max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        float x = mesh->vertices[i].x;
        float y = mesh->vertices[i].y;
        float z = mesh->vertices[i].z;

        if (x < bbox_min->x) bbox_min->x = x;
        if (y < bbox_min->y) bbox_min->y = y;
        if (z < bbox_min->z) bbox_min->z = z;

        if (x > bbox_max->x) bbox_max->x = x;
        if (y > bbox_max->y) bbox_max->y = y;
        if (z > bbox_max->z) bbox_max->z = z;
    }

    *bbox_extents = *bbox_max - *bbox_min;
}


// 'fill_result' copies geometry from the mesh 'source' into the
// mesh 'result'.  This is a helper function used by 'merge_meshes'
// below.

void fill_result(Triangle_List_Mesh *result, Triangle_List_Mesh *source,
                 int vertex_cursor, int index_cursor, 
                 int triangle_list_cursor, Vector3 offset, Vector3 uv_offset,
                 int *index_map) {
    int i;
    for (i = 0; i < source->num_vertices; i++) {
        int di = i + vertex_cursor;

        result->vertices[di] = source->vertices[i] + offset;
        result->tangent_frames[di] = source->tangent_frames[i];
        result->uvs[di] = source->uvs[i];
        result->uvs[di].x += uv_offset.x;
        result->uvs[di].y += uv_offset.y;
        result->canonical_vertex_map[di] = source->canonical_vertex_map[i] + vertex_cursor;

        index_map[i] = di;
    }

    for (i = 0; i < source->num_indices; i++) {
        int di = i + index_cursor;
        result->indices[di] = source->indices[i] + vertex_cursor;
    }

    for (i = 0; i < source->num_triangle_lists; i++) {
        int di = i + triangle_list_cursor;
        Triangle_List_Info *src_info = &source->triangle_list_info[i];
        Triangle_List_Info *dest_info = &result->triangle_list_info[di];

        dest_info->material_index = src_info->material_index;
        dest_info->num_vertices = src_info->num_vertices;
        dest_info->start_of_list = src_info->start_of_list + index_cursor;
    }
}



// This version of 'fill_result' copies seam triangles into a mesh,
// instead of a mesh into a mesh as above.  You want to put seam
// triangles into the mesh when you are building your LOD hierarchy --
// to combine 4 blocks into 1, you want to include not only the geometry
// of those blocks but the seams between them as well.
void fill_result(Triangle_List_Mesh *result, Mesh_Seam *seam,
                 int index_cursor, int triangle_list_cursor, 
                 int **index_maps) {

    assert(result->num_triangle_lists > 0);  // So we can copy a material info out.  @Improvement eventually seams will have their own material info and we should use that instead of this hack


    int i;
    Triangle_List_Info *dest_info = &result->triangle_list_info[triangle_list_cursor];
    
    int num_indices = seam->num_faces * 3;

    dest_info->num_vertices = num_indices;
    dest_info->material_index = 0;  // XXX This is the hacky seam material to be replaced
    dest_info->start_of_list = index_cursor;

    for (i = 0; i < num_indices; i++) {
        int di = i + index_cursor;

        Seam_Index *seam_index = &seam->indices[i];

        int pre_mapped_index = seam_index->vertex_index;

        Static_Terrain_Block *block = seam->block_membership[seam_index->which_mesh];
        assert(block);
        int *index_map = index_maps[block->temporary_integer_storage];

        int mapped_index;
        assert(pre_mapped_index < block->mesh->num_vertices);
        mapped_index = index_map[pre_mapped_index];

        result->indices[di] = mapped_index;
    }
}


// 'merge_meshes' does the work of taking an array of meshes and seams
// plopping them together into one bigger mesh.
Triangle_List_Mesh *merge_meshes(int num_meshes, Triangle_List_Mesh **meshes, Vector3 *offsets, Vector3 *uv_offsets, int **index_maps,
                                 Auto_Array <Mesh_Seam *> *seams) {
    // XXX for now assumes identical material list
    // We will have to fix this before we're done, but for now
    // we leave it this way.

    int total_indices = 0;
    int total_vertices = 0;
    int total_triangle_lists = 0;
    int total_faces = 0;

    int i;
    for (i = 0; i < num_meshes; i++) {
        Triangle_List_Mesh *mesh = meshes[i];
        if (mesh) {
            total_vertices += mesh->num_vertices;
            total_indices += mesh->num_indices;
            total_triangle_lists += mesh->num_triangle_lists;
            total_faces += mesh->num_faces;
        }
    }

    Mesh_Seam *seam;
    Array_Foreach(seams, seam) {
        total_indices += seam->num_faces * 3;
        total_triangle_lists++;
        total_faces += seam->num_faces;
    } Endeach;

    Triangle_List_Mesh *result = new Triangle_List_Mesh();
    assert(meshes[0]); // AAA
    result->allocate_materials(meshes[0]->num_materials);
    
    result->allocate_geometry(total_vertices, total_faces);

    result->triangle_list_info = new Triangle_List_Info[total_triangle_lists];
    result->num_triangle_lists = total_triangle_lists;

    int vertices_cursor = 0;
    int indices_cursor = 0;
    int triangle_list_cursor = 0;

    for (i = 0; i < result->num_materials; i++) {
        copy_material_info(&result->material_info[i], &meshes[0]->material_info[i]);
    }

    for (i = 0; i < num_meshes; i++) {
        Triangle_List_Mesh *mesh = meshes[i];
        if (mesh == NULL) continue;

        int *index_map = new int[mesh->num_vertices];
        index_maps[i] = index_map;

        fill_result(result, mesh, vertices_cursor, indices_cursor, triangle_list_cursor, offsets[i], uv_offsets[i], index_map);

        vertices_cursor += mesh->num_vertices;
        indices_cursor += mesh->num_indices;
        triangle_list_cursor += mesh->num_triangle_lists;

        int j;
        for (j = 0; j < triangle_list_cursor; j++) {
            assert(result->triangle_list_info[j].material_index >= 0);
        }
    }

    Array_Foreach(seams, seam) {
        fill_result(result, seam, indices_cursor, triangle_list_cursor, 
                    index_maps);

        int num_indices = seam->num_faces * 3;
        indices_cursor += num_indices;
        triangle_list_cursor++;
    } Endeach;

    return result;
}

void xref_block_index_map(Static_Terrain_Block *block, Mesh_Reducer *reducer) {

    int i;
    for (i = 0; i < block->mesh->num_vertices; i++) {
        int old_index = block->index_map_into_parent[i];
        block->index_map_into_parent[i] = reducer->topology_handler->old_index_to_new_index[old_index];
    }
}


// 'combine_tree' is the entry point for all the merging stuff
// above.  Given a node that's not yet complete, look at its
// children, combine their geometry and stuff to fill out the node.
void combine_tree(Static_Terrain_Block *node) {
    Static_Terrain_Block **n = node->children;

    const int MAX_BLOCKS = 16;
    Triangle_List_Mesh *meshes[MAX_BLOCKS];
    Static_Terrain_Block *blocks[MAX_BLOCKS];
    Vector3 uv_offsets[MAX_BLOCKS];

    Vector3 position_sum(0, 0, 0);
    int num_faces_before_seams = 0;
    int num_triangle_lists_before_seams = 0;

    float uoff_scalar = (float)(node->leaf_distance);

    int i;
    int num_blocks = 0;
    for (i = 0; i < node->num_children; i++) {
        assert(num_blocks < MAX_BLOCKS);

        Static_Terrain_Block *child = node->children[i];
        assert(child);

        meshes[num_blocks] = child->mesh;
        blocks[num_blocks] = child;
        blocks[num_blocks]->temporary_integer_storage = i;

        uv_offsets[num_blocks] = Vector3(uoff_scalar * (i & 1),
                                         uoff_scalar * ((i & 2) >> 1), 0);

        position_sum += blocks[num_blocks]->position;

        num_faces_before_seams += child->mesh->num_faces;
        num_triangle_lists_before_seams += child->mesh->num_triangle_lists;

        num_blocks++;
    }            

    assert(num_blocks > 0);

    Static_Terrain_Block *block = node;
    block->position = position_sum * (1.0f / (float)num_blocks);

    Vector3 offsets[MAX_BLOCKS];
    for (i = 0; i < num_blocks; i++) {
        offsets[i] = blocks[i]->position - block->position;
    }


    // Figure out which seams to combine into this block
    Auto_Array <Mesh_Seam *> seams;
    seam_database->find_seams_containing_these_blocks(blocks, num_blocks, &seams);

    int *index_maps[MAX_BLOCKS];
    Triangle_List_Mesh *merged_mesh = merge_meshes(num_blocks, meshes, offsets, uv_offsets, 
                                                   index_maps,
                                                   &seams);

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

    for (i = 0; i < num_blocks; i++) {
        xref_block_index_map(blocks[i], &reducer);
    }

    delete merged_mesh;


    Vector3 bbox_max;
    find_bounding_box(reduced, 
                      &block->bounding_box_corner,
                      &block->bounding_box_extents);
}

void add_child(Static_Terrain_Block *parent, Static_Terrain_Block *child) {
    if (!child) return;
    if (parent->num_children == 0) {
        parent->edge_length = child->edge_length * 2;
        parent->leaf_distance = child->leaf_distance + 1;
    }
        
    assert(parent->num_children < MAX_CHILDREN);
    parent->children[parent->num_children++] = child;
}

void add_rewrite_rule(Auto_Array <Rewrite_Rule *> *rules,
                      Static_Terrain_Block *source_block,
                      Static_Terrain_Block *dest_block) {

    if (source_block == NULL) return;
    assert(dest_block != NULL);

    Rewrite_Rule *rule = new Rewrite_Rule;
    rule->source_block = source_block;
    rule->dest_block = dest_block;
    rule->index_map = source_block->index_map_into_parent;
    rules->add(rule);
}

void delete_rewrite_rules(Auto_Array <Rewrite_Rule *> *rules) {
    Rewrite_Rule *rule;
    Array_Foreach(rules, rule) {
        delete rule;
    } Endeach;
}

Static_Terrain_Block *build_tree_upward(Static_Terrain_Block *nodes,
                                        int num_blocks_x, int num_blocks_y,
                                        int block_id_offset) {

    if ((num_blocks_x == 1) && (num_blocks_y == 1)) return nodes;

    int num_parents_x = (num_blocks_x + 1) / 2;
    int num_parents_y = (num_blocks_y + 1) / 2;
    int num_parents = num_parents_x * num_parents_y;
    
    Static_Terrain_Block *parents = new Static_Terrain_Block[num_parents];
    
    Auto_Array <Rewrite_Rule *> rewrite_rules;

    int i, j;
    for (j = 0; j < num_parents_y; j++) {
        for (i = 0; i < num_parents_x; i++) {
            int parent_index = j * num_parents_x + i;

            int i_block_id = block_id_offset + parent_index;

            Static_Terrain_Block *parent = &parents[parent_index];
            parent->block_id = (Block_Identifier)i_block_id;

            int ci0 = (j*2 + 0) * num_blocks_y + (i*2 + 0);
            int ci1 = (j*2 + 0) * num_blocks_y + (i*2 + 1);
            int ci2 = (j*2 + 1) * num_blocks_y + (i*2 + 0);
            int ci3 = (j*2 + 1) * num_blocks_y + (i*2 + 1);

            Static_Terrain_Block *n0, *n1, *n2, *n3;
            n1 = NULL;
            n2 = NULL;
            n3 = NULL;

            n0 = &nodes[ci0]; 
            if (i*2 < num_blocks_x - 1) n1 = &nodes[ci1];
            if (j*2 < num_blocks_y - 1) n2 = &nodes[ci2];
            if ((i*2 < num_blocks_x - 1) && (j*2 < num_blocks_y - 1)) n3 = &nodes[ci3];

            add_child(parent, n0);
            add_child(parent, n1);
            add_child(parent, n2);
            add_child(parent, n3);

            combine_tree(parent);
            
            add_rewrite_rule(&rewrite_rules, n0, parent);
            add_rewrite_rule(&rewrite_rules, n1, parent);
            add_rewrite_rule(&rewrite_rules, n2, parent);
            add_rewrite_rule(&rewrite_rules, n3, parent);
        }
    }

    seam_database->perform_rewrite_rules(&rewrite_rules, 1);
    delete_rewrite_rules(&rewrite_rules);

    return build_tree_upward(parents, num_parents_x, num_parents_y,
                             block_id_offset + num_parents_x * num_parents_y);
}

Mesh_Seam *make_high_res_seam(int num_samples,
                              int starting_index_a, int stride_a,
                              int starting_index_b, int stride_b,
                              Static_Terrain_Block *block_a,
                              Static_Terrain_Block *block_b) {
    int num_intervals = num_samples - 1;
    int max_faces = num_intervals * 2;

    assert(block_a->block_id < 0x10000000);
    assert(block_b->block_id < 0x10000000);

    int *index_map_a = block_a->index_map;
    int *index_map_b = block_b->index_map;

    Mesh_Seam *result = new Mesh_Seam(max_faces);
    result->num_faces = 0;
    result->is_high_res_seam = true;
    result->block_membership[0] = block_a;
    result->block_membership[1] = block_b;
    result->block_membership[2] = NULL;
    result->simplify_block_membership();

    int j;
    for (j = 0; j < num_intervals; j++) {
        int b_index = starting_index_b + j * stride_b;
        int a_index = starting_index_a + j * stride_a;

        Seam_Index i0, i1, i2, i3;
        i0.which_mesh = 0;
        i1.which_mesh = 1;
        i2.which_mesh = 1;
        i3.which_mesh = 0;

        i0.vertex_index = index_map_a[a_index];
        i1.vertex_index = index_map_b[b_index];
        i2.vertex_index = index_map_b[b_index + stride_b];
        i3.vertex_index = index_map_a[a_index + stride_a];

        result->indices[result->num_faces * 3 + 0] = i0;
        result->indices[result->num_faces * 3 + 1] = i1;
        result->indices[result->num_faces * 3 + 2] = i2;
        result->num_faces++;

        result->indices[result->num_faces * 3 + 0] = i0;
        result->indices[result->num_faces * 3 + 1] = i2;
        result->indices[result->num_faces * 3 + 2] = i3;
        result->num_faces++;
    }

    return result;
}

Mesh_Seam *make_high_res_seam_y(int num_samples_x, int num_samples_y, 
                                Static_Terrain_Block *block_a,
                                Static_Terrain_Block *block_b) {
    Mesh_Seam *seam = make_high_res_seam(num_samples_x, 
                                         (num_samples_y-1)*num_samples_x, 1,
                                         0, 1, block_a, block_b);
                                         

    // AAA
    int i;
    for (i = 0; i < seam->num_faces; i++) {
        Seam_Index i1 = seam->indices[i*3+1];
        Seam_Index i2 = seam->indices[i*3+2];

        seam->indices[i*3+2] = i1;
        seam->indices[i*3+1] = i2;
    }

    return seam;
}

Mesh_Seam *make_high_res_seam_x(int num_samples_x, int num_samples_y, 
                                Static_Terrain_Block *block_a,
                                Static_Terrain_Block *block_b) {
    return make_high_res_seam(num_samples_y, 
                              num_samples_x - 1, num_samples_x, 
                              0, num_samples_x,
                              block_a, block_b);
}


Vector3 get_position(Mesh_Seam *seam, 
                     Static_Terrain_Block *b0, Static_Terrain_Block *b1,
                     Seam_Index *index) {
    if (index->which_mesh == 0) return b0->mesh->vertices[index->vertex_index] + b0->position;
    return b1->mesh->vertices[index->vertex_index] + b1->position;
}

void validate_seam(Mesh_Seam *seam, Static_Terrain_Block *b0, Static_Terrain_Block *b1) {
    int i;
    for (i = 0; i < seam->num_faces; i++) {
        Seam_Index *index0 = &seam->indices[i*3+0];
        Seam_Index *index1 = &seam->indices[i*3+1];
        Seam_Index *index2 = &seam->indices[i*3+2];

        Vector3 v0, v1, v2;
        v0 = get_position(seam, b0, b1, index0);
        v1 = get_position(seam, b0, b1, index1);
        v2 = get_position(seam, b0, b1, index2);

        float d0 = distance(v0, v1);
        float d1 = distance(v1, v2);
        float d2 = distance(v0, v2);

        assert(d0 < 10.0f);
        assert(d1 < 10.0f);
        assert(d2 < 10.0f);
    }
}


/*
  Entry point to make an LOD'd terrain tree out of an elevation map.
*/
Static_Terrain_Block *make_static_terrain_block(Elevation_Map *map,
                                                int block_samples) {
    int block_squares = block_samples - 1;
    int samples_x = block_samples;
    int samples_y = block_samples;

    int num_blocks_x = map->num_squares_x / block_squares;
    int num_blocks_y = map->num_squares_y / block_squares;
    int num_blocks = num_blocks_x * num_blocks_y;

    int block_id_offset = 1;  // We start at 1 since 0 is reserved!

    Static_Terrain_Block *nodes = new Static_Terrain_Block[num_blocks];
    int i, j;
    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;

            int si_x = block_squares * i;
            int si_y = block_squares * j;

            Static_Terrain_Block *block = &nodes[block_index];
            make_terrain_block(map, block, samples_x, samples_y, si_x, si_y);
            block->block_id = (Block_Identifier)(block_index + block_id_offset);

            block->leaf_distance = 0;
            block->mesh->material_info[0].name = app_strdup("3grass001"); // @Memory: Maybe don't do this strdup all the time.
        }
    }

    // Make seams between neighboring highest-level blocks
    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;
            int block_index_plus_y = block_index + num_blocks_x;
            int block_index_plus_x = block_index + 1;

            Static_Terrain_Block *block = &nodes[block_index];
            
            if (i < num_blocks_x - 1) {
                Static_Terrain_Block *block_plus_x = &nodes[block_index_plus_x];
                Mesh_Seam *seam = make_high_res_seam_x(block_samples, block_samples, 
                                                       block, block_plus_x);
                seam_database->add(seam);
            }

            if (j < num_blocks_y - 1) {
                Static_Terrain_Block *block_plus_y = &nodes[block_index_plus_y];
                Mesh_Seam *seam = make_high_res_seam_y(block_samples, block_samples, 
                                                       block, block_plus_y);
                seam_database->add(seam);
            }
        }
    }

    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;
            Static_Terrain_Block *block = &nodes[block_index];

            delete [] block->index_map;
            block->index_map = NULL;
        }
    }

    Static_Terrain_Block *root = build_tree_upward(nodes, num_blocks_x, num_blocks_y, block_id_offset + num_blocks_x * num_blocks_y);


    // Delete the same-resolution seams on the lowest-level blocks...
    seam_database->delete_high_resolution_seams();

    return root;
}



inline int vertex_index(int i, int j, int verts_per_ring, int num_rings) {
    return j * verts_per_ring + (i % verts_per_ring);
}

Elevation_Map::Elevation_Map(int _num_samples_x, int _num_samples_y) {
    num_samples_x = _num_samples_x;
    num_samples_y = _num_samples_y;

    num_squares_x = num_samples_x - 1;
    num_squares_y = num_samples_y - 1;

    samples = new float[num_samples_x * num_samples_y];
    corner.set(0, 0, 0);

    tangent_frames = NULL;
}



Elevation_Map::~Elevation_Map() {
    delete [] samples;
}

Elevation_Map *make_dummy_elevation_map(float units_per_sample, int elevation_map_samples,
                                        float bump_f1, float bump_f2, float bump_height) {
    float s = units_per_sample;

    int samples = elevation_map_samples;
    Elevation_Map *map = new Elevation_Map(samples, samples);

    map->square_size.set(s, s, 0);
    map->map_size.set(s * map->num_squares_x, s*map->num_squares_y, 0);
    map->corner.set(0, 0, 0);

    float K = 4;
    int i, j;
    for (j = 0; j < map->num_samples_y; j++) {
        for (i = 0; i < map->num_samples_x; i++) {
            float x = map->corner.x + map->square_size.x * i;
            float y = map->corner.y + map->square_size.y * j;

            float f1 = cos((i / (float)(map->num_samples_x)) * bump_f1*K * (2*M_PI));
            float f2 = cos((j / (float)(map->num_samples_y)) * bump_f2*K * (2*M_PI));
            float z = f1 * f2 * bump_height;

            int index = map->get_index(x, y);
            map->samples[index] = z;
        }
    }

    return map;
}


void init_block_for_mesh(Static_Terrain_Block *block, Triangle_List_Mesh *mesh) {
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

    Vector3 position = (bbox_min + bbox_max) * 0.5f;
    position.z = 0;

    bbox_min = bbox_min - position;
    bbox_max = bbox_max - position;
    for (i = 0; i < mesh->num_vertices; i++) {
        mesh->vertices[i] = mesh->vertices[i] - position;
    }

    block->bounding_box_corner = bbox_min;
    block->bounding_box_extents = bbox_max - bbox_min;
    block->position = position;

    block->mesh = mesh;
}

Vector3 get_position(Elevation_Map *map, int i, int j) {
    float x = map->corner.x + map->square_size.x * i;
    float y = map->corner.y + map->square_size.y * j;

    int sample_index = map->get_index(i, j);
    float z = map->samples[sample_index];

    return Vector3(x, y, z);
}

Vector3 get_uv(Elevation_Map *map, int i, int j) {
    float u = (float)i;
    float v = (float)j;
    float w = 0;

    return Vector3(u, v, w);
}

Vector3 get_position(Elevation_Map *map, int index) {
    int i = index % map->num_samples_x;
    int j = index / map->num_samples_x;

    return get_position(map, i, j);
}

Vector3 get_uv(Elevation_Map *map, int index) {
    int i = index % map->num_samples_x;
    int j = index / map->num_samples_x;

    return get_uv(map, i, j);
}


int *identity_index_map(int num_vertices) {
    int *result = new int[num_vertices];

    int i;
    for (i = 0; i < num_vertices; i++) result[i] = i;

    return result;
}

void make_terrain_block(Elevation_Map *map, Static_Terrain_Block *block,
                        int samples_i, int samples_j,
                        int offset_i, int offset_j) {
    assert(samples_i + offset_i <= map->num_samples_x);
    assert(samples_j + offset_j <= map->num_samples_y);

    int mesh_samples_x = samples_i;
    int mesh_samples_y = samples_j;

    int num_vertices = mesh_samples_x * mesh_samples_y;
    int num_faces = 2 * (mesh_samples_x - 1) * (mesh_samples_y - 1);

    Mesh_Builder builder(num_vertices, num_faces);
    Mesh_Material_Info info;
    info.name = app_strdup("material");
    info.texture_index = 0;
    builder.add_material(&info);


    // Do vertices (position, uv coordinates and tangent frames)
    float du = 1.0 / (float)(mesh_samples_x - 1);
    float dv = 1.0 / (float)(mesh_samples_y - 1);

    Vector2 uv;

    int i, j;
    for (j = 0; j < mesh_samples_y; j++) {
        uv.y = j * dv;
        int jj = j + offset_j;

        for (i = 0; i < mesh_samples_x; i++) {
            int ii = i + offset_i;

            uv.x = i * du;

            Vector3 position = get_position(map, ii, jj);
            Quaternion frame = map->tangent_frames[map->get_index(ii, jj)];

            builder.add_vertex(position, uv, frame);
        }
    }

    //
    // Do the surface topology...
    // 

    for (j = 0; j < mesh_samples_y - 1; j++) {
        int jj = j + offset_j;

        int i;
        for (i = 0; i < mesh_samples_x - 1; i++) {
            int ii = i + offset_i;

            int face_type = (i ^ j) & 1;

            int n0, n1, n2, n3;
            n0 = vertex_index(i+0, j+0, mesh_samples_x, mesh_samples_y);
            n1 = vertex_index(i+1, j+0, mesh_samples_x, mesh_samples_y);
            n2 = vertex_index(i+1, j+1, mesh_samples_x, mesh_samples_y);
            n3 = vertex_index(i+0, j+1, mesh_samples_x, mesh_samples_y);

            if (face_type == 0) {
                builder.add_triangle(n0, n1, n2, 0);
                builder.add_triangle(n0, n2, n3, 0);
            } else {
                builder.add_triangle(n0, n1, n3, 0);
                builder.add_triangle(n1, n2, n3, 0);
            }
        }
    }

    int *index_map = identity_index_map(num_vertices);
    Triangle_List_Mesh *mesh = builder.build_mesh();

    block->index_map = index_map;

    int max_samples = Max(samples_i, samples_j);
    float dimension_max = Max(map->square_size.x, map->square_size.y);
    block->edge_length = dimension_max * max_samples;

    init_block_for_mesh(block, mesh);
    return;
}






Static_Terrain_Block::Static_Terrain_Block() {
    frame_index = 0;
    parent = NULL;

    children[0] = NULL;
    children[1] = NULL;
    children[2] = NULL;
    children[3] = NULL;

    my_child_index = -1;
    num_children = 0;
    block_search_marker = 0;
}

Static_Terrain_Block::~Static_Terrain_Block() {
    delete mesh;
}


Mesh_Seam::Mesh_Seam(int _num_faces) {
    num_faces = _num_faces;
    indices = new Seam_Index[num_faces * 3];
    is_high_res_seam = false;
    block_membership[0] = NULL;
    block_membership[1] = NULL;
    block_membership[2] = NULL;
}

Mesh_Seam::~Mesh_Seam() {
    delete [] indices;
}

inline bool indices_match(Seam_Index *n0, Seam_Index *n1) {
    if (n0->which_mesh != n1->which_mesh) return false;
    if (n0->vertex_index != n1->vertex_index) return false;

    return true;
}

void sanity_seam(Mesh_Seam *seam) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        assert(index->vertex_index >= 0);
        assert(index->vertex_index < 40000);
        assert((index->which_mesh == 0) || (index->which_mesh == 1));
    }
}

void sanity_seam(Mesh_Seam *seam, Static_Terrain_Block *block_a,
                 Static_Terrain_Block *block_b) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        assert((index->which_mesh == 0) || (index->which_mesh == 1));
        Static_Terrain_Block *block = block_a;
        if (index->which_mesh == 1) block = block_b;

        assert(index->vertex_index >= 0);
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}


void Mesh_Seam::remove_degenerate_faces() {
    sanity_seam(this);

    int i;
    for (i = 0; i < num_faces; i++) {
        Seam_Index *n0 = &indices[i*3+0];
        Seam_Index *n1 = &indices[i*3+1];
        Seam_Index *n2 = &indices[i*3+2];

        if (indices_match(n0, n1) || indices_match(n0, n2) || indices_match(n1, n2)) {
            // Actually copy the rest of the faces in the seam downward
            // I do this inefficient thing, rather than just copying the
            // guy out from the end, because I want the colored stripeys
            // to come out looking right in the "seam demo" and debug
            // visualization.

            // For a shipping tool where this is not a concern,
            // go ahead and just copy the end face into this slot.
            // (This is the code that's currently commented out).
/*
            int k = num_faces - 1;
            indices[i*3+0] = indices[k*3+0];
            indices[i*3+1] = indices[k*3+1];
            indices[i*3+2] = indices[k*3+2];
            num_faces--;
            i--;
*/
            int src_face = i + 1;
            while (src_face < num_faces) {
                int dest_face = src_face - 1;
                indices[dest_face*3+0] = indices[src_face*3+0];
                indices[dest_face*3+1] = indices[src_face*3+1];
                indices[dest_face*3+2] = indices[src_face*3+2];
                src_face++;
            }

            num_faces--;

            i--;

        }
    }
}



void do_face(Tangent_Frame_Maker *maker, Elevation_Map *map, int indices[3]) {
    Vector3 pos[3];
    Vector3 uv[3];

    pos[0] = get_position(map, indices[0]);
    pos[1] = get_position(map, indices[1]);
    pos[2] = get_position(map, indices[2]);

    uv[0] = get_uv(map, indices[0]);
    uv[1] = get_uv(map, indices[1]);
    uv[2] = get_uv(map, indices[2]);
    
    maker->accumulate_triangle(indices, pos, uv);
}


void build_tangent_frames(Elevation_Map *map) {
    Tangent_Frame_Maker maker;
    maker.begin_tangent_frames(map->num_samples_x * map->num_samples_y);
    int i, j;
    for (j = 0; j < map->num_samples_y-1; j++) {
        for (i = 0; i < map->num_samples_x-1; i++) {
            int face_type = (i ^ j) & 1;
            int n0 = map->get_index(i+0, j+0);
            int n1 = map->get_index(i+1, j+0);
            int n2 = map->get_index(i+1, j+1);
            int n3 = map->get_index(i+0, j+1);

            int indices[3];


            indices[0] = n0;
            indices[1] = n1;

            if (face_type == 0) {
                indices[2] = n2;
            } else {
                indices[2] = n3;
            }

            do_face(&maker, map, indices);

            if (face_type == 0) {
                indices[0] = n0;
            } else {
                indices[0] = n1;
            }

            indices[1] = n2;
            indices[2] = n3;

            do_face(&maker, map, indices);
        }
    }

    maker.complete_tangent_frames();
    map->tangent_frames = maker.tangent_frames;
}

// Helper function called by qsort in simplify_block_membership()
int compare_block_membership(const void *b1, const void *b2) {
    Static_Terrain_Block *block1 = *(Static_Terrain_Block **)b1;
    Static_Terrain_Block *block2 = *(Static_Terrain_Block **)b2;

    if (block1->block_id < block2->block_id) return -1;
    if (block1->block_id > block2->block_id) return +1;
    return 0;
}

int find_remapping(Static_Terrain_Block *old_block,
                   Static_Terrain_Block **new_blocks, int num_blocks) {
    int i;
    for (i = 0; i < num_blocks; i++) {
        if (new_blocks[i] == old_block) return i;
    }

    return -1;
}

void Mesh_Seam::simplify_block_membership() {
    // Copy block membership

    Static_Terrain_Block *new_membership[3];
    int num_blocks = 0;
    int i;
    for (i = 0; i < 3; i++) {
        new_membership[i] = block_membership[i];
        if (new_membership[i]) num_blocks++;
    }

    // Sort it

    qsort(new_membership, num_blocks, sizeof(new_membership[0]), compare_block_membership);

    // Check for duplicates

    if (new_membership[1] == new_membership[2]) {
        new_membership[2] = NULL;
    }

    if (new_membership[0] == new_membership[1]) {
        if (new_membership[2]) {
            new_membership[1] = new_membership[2];
            new_membership[2] = NULL;
        } else {
            new_membership[1] = NULL;
        }
    }

    // Figure out the index cross-reference for remapping.

    int remap[3];
    for (i = 0; i < num_blocks; i++) {
        int index = find_remapping(block_membership[i], new_membership, 3);
        assert(index != -1);
        remap[i] = index;
    }

    // Update the block membership array.

    for (i = 0; i < 3; i++) {
        block_membership[i] = new_membership[i];
    }

    // Actually remap the seam.

    for (i = 0; i < num_faces * 3; i++) {
        Seam_Index *index = &indices[i];
        index->which_mesh = remap[index->which_mesh];

        // Verify...
        Static_Terrain_Block *block = block_membership[index->which_mesh];
        assert(block);
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}
