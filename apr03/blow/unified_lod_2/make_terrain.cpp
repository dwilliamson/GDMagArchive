#include "framework.h"

#include "mesh.h"
#include "mesh/mesh_builder.h"
#include "make_terrain.h"
#include "bt_loader.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"
#include "binary_file_stuff.h"

#include "tangent_frames.h"  // AAA for debugging  ... XXX eliminate?

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
                 Static_Terrain_Block *block_a,
                 Static_Terrain_Block *block_b,
                 int *index_map_a, int *index_map_b) {

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
        int *index_map;
        Static_Terrain_Block *block;

        if (seam_index->which_mesh == 0) {
            block = block_a;
            index_map = index_map_a;
        } else {
            block = block_b;
            index_map = index_map_b;
        }

        int mapped_index;
        assert(pre_mapped_index < block->mesh->num_vertices);
        mapped_index = index_map[pre_mapped_index];

        result->indices[di] = mapped_index;
    }
}


// 'merge_meshes' does the work of taking an array of meshes and seams
// plopping them together into one bigger mesh.
Triangle_List_Mesh *merge_meshes(int num_meshes, Triangle_List_Mesh **meshes, Vector3 *offsets, Vector3 *uv_offsets, int **index_maps, int num_seams, Mesh_Seam **seams, Static_Terrain_Block **seam_blocks_a, Static_Terrain_Block **seam_blocks_b) {
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

    for (i = 0; i < num_seams; i++) {
        Mesh_Seam *seam = seams[i];
        Static_Terrain_Block *block_a = seam_blocks_a[i];
        Static_Terrain_Block *block_b = seam_blocks_b[i];
        total_indices += seam->num_faces * 3;
        total_triangle_lists++;
        total_faces += seam->num_faces;
    }

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

    for (i = 0; i < num_seams; i++) {
        Mesh_Seam *seam = seams[i];
        Static_Terrain_Block *block_a = seam_blocks_a[i];
        Static_Terrain_Block *block_b = seam_blocks_b[i];
        
        int num_indices = seam->num_faces * 3;
        int na = block_a->temporary_integer_storage;
        int nb = block_b->temporary_integer_storage;

        fill_result(result, seam, indices_cursor, triangle_list_cursor, 
                    block_a, block_b, index_maps[na], index_maps[nb]);

        indices_cursor += num_indices;
        triangle_list_cursor++;
    }

    return result;
}

void xref_block_index_map(Static_Terrain_Block *block, Mesh_Reducer *reducer) {

    int i;
    for (i = 0; i < block->mesh->num_vertices; i++) {
        int old_index = block->index_map_into_parent[i];
        block->index_map_into_parent[i] = reducer->topology_handler->old_index_to_new_index[old_index];
    }
}

// 'add_seam' is a helper function used by 'combine_tree',
// it just puts a seam into an array of seams that we then
// pass to the 'merge_meshes' functions above.
void add_seam(Neighbor_Direction direction,
              int *num_seams_ret, Mesh_Seam **seams, 
              Static_Terrain_Block **seam_blocks_a,
              Static_Terrain_Block **seam_blocks_b,
              Static_Terrain_Block *block_a,
              Static_Terrain_Block *block_b) {

    if (block_a == NULL) return;
    if (block_b == NULL) return;

    Mesh_Seam *seam = block_a->seam_sets[direction].res_equal;
    if (!seam) return;

    int n = *num_seams_ret;

    seams[n] = seam;
    seam_blocks_a[n] = block_a;
    seam_blocks_b[n] = block_b;

    n++;
    *num_seams_ret = n;
}


// 'combine_tree' is the entry point for all the merging stuff
// above.  Given a node that's not yet complete, look at its
// children, combine their geometry and stuff to fill out the node.
void combine_tree(Static_Terrain_Tree *node) {
    Static_Terrain_Tree **n = node->children;

    Triangle_List_Mesh *meshes[4];
    Static_Terrain_Block *blocks[4];

    Vector3 position_sum(0, 0, 0);
    int num_blocks_summed = 0;
    int num_faces_before_seams = 0;
    int num_triangle_lists_before_seams = 0;

    int i;
    for (i = 0; i < 4; i++) {
        Static_Terrain_Tree *child = node->children[i];
        if (child) {
            meshes[i] = child->block->mesh;
            blocks[i] = child->block;
            blocks[i]->temporary_integer_storage = i;
            position_sum += blocks[i]->position;
            num_faces_before_seams += meshes[i]->num_faces;
            num_triangle_lists_before_seams += meshes[i]->num_triangle_lists;
            num_blocks_summed++;
        } else {
            meshes[i] = NULL;
            blocks[i] = NULL;
        }
    }            

    assert(num_blocks_summed > 0);

    Static_Terrain_Block *block = node->block;
    block->position = position_sum * (1.0f / (float)num_blocks_summed);

    Vector3 offsets[4];
    for (i = 0; i < 4; i++) {
        if (blocks[i]) {
            offsets[i] = blocks[i]->position - block->position;
        } else {
            offsets[i].set(0, 0, 0);
        }
    }

    float uoff_scalar = (float)(node->leaf_distance);

    Vector3 uv_offsets[4];
    uv_offsets[0] = Vector3(0, 0, 0);
    uv_offsets[1] = Vector3(uoff_scalar, 0, 0);
    uv_offsets[2] = Vector3(0, uoff_scalar, 0);
    uv_offsets[3] = Vector3(uoff_scalar, uoff_scalar, 0);


    // Figure out which seams to combine into this block
    Mesh_Seam *seams[4];
    Static_Terrain_Block *seam_blocks_a[4];
    Static_Terrain_Block *seam_blocks_b[4];

    int num_seams = 0;
    add_seam(PLUS_X, &num_seams, seams, seam_blocks_a, seam_blocks_b,
             blocks[0], blocks[1]);
    add_seam(PLUS_X, &num_seams, seams, seam_blocks_a, seam_blocks_b,
             blocks[2], blocks[3]);
    add_seam(PLUS_Y, &num_seams, seams, seam_blocks_a, seam_blocks_b,
             blocks[0], blocks[2]);
    add_seam(PLUS_Y, &num_seams, seams, seam_blocks_a, seam_blocks_b,
             blocks[1], blocks[3]);

    int *index_maps[4];
    Triangle_List_Mesh *merged_mesh = merge_meshes(4, meshes, offsets, uv_offsets, 
                                                   index_maps,
                                                   num_seams,
                                                   seams, seam_blocks_a, seam_blocks_b);

    for (i = 0; i < 4; i++) {
        if (blocks[i]) blocks[i]->index_map_into_parent = index_maps[i];
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

    for (i = 0; i < 4; i++) {
        if (blocks[i]) xref_block_index_map(blocks[i], &reducer);
    }

    delete merged_mesh;


    Vector3 bbox_max;
    find_bounding_box(reduced, 
                      &block->bounding_box_corner,
                      &block->bounding_box_extents);
}


// Concatenate two seams together.
Mesh_Seam *concatenate_seams(Mesh_Seam *a, Mesh_Seam *b) {
    int num_faces = 0;
    if (a) num_faces += a->num_faces;
    if (b) num_faces += b->num_faces;
    Mesh_Seam *result = new Mesh_Seam(num_faces);

    int i;

    int dest_cursor = 0;

    if (a) {
        for (i = 0; i < a->num_faces * 3; i++) {
            result->indices[dest_cursor++] = a->indices[i];
        }
    }

    if (b) {
        for (i = 0; i < b->num_faces * 3; i++) {
            result->indices[dest_cursor++] = b->indices[i];
        }
    }
    
    return result;
}


// Given a block that has undergone some alteration
// (like merging), and a seam that was valid before the alteration,
// use the block's index map to convert the seam into something
// that is also valid after the alteration.
Mesh_Seam *xref_seam(Mesh_Seam *source, Static_Terrain_Block *block, int mesh_index) {
    if (source == NULL) return NULL;
    if (block == NULL) return NULL;

    int *index_map = block->index_map_into_parent;
    Mesh_Seam *dest = new Mesh_Seam(source->num_faces);

    int i;
    for (i = 0; i < source->num_faces * 3; i++) {
        dest->indices[i].which_mesh = source->indices[i].which_mesh;
        if (dest->indices[i].which_mesh == mesh_index) {
            dest->indices[i].vertex_index = index_map[source->indices[i].vertex_index];
        } else {
            dest->indices[i].vertex_index = source->indices[i].vertex_index;
        }
    }

    dest->remove_degenerate_faces();
    dest->compute_uv_coordinates(block->mesh);

    return dest;
}


inline Static_Terrain_Block *get_child_block(Static_Terrain_Tree *parent,
                                             int index) {
    if (parent->children[index]) return parent->children[index]->block;
    return NULL;
}


/* 
   The next functions build seams between blocks of varying LODs.
*/
void seam_build_upward(Static_Terrain_Tree *parent,
                       Static_Terrain_Tree *neighbor, Neighbor_Direction direction_type) {
    int direction, other_direction;
    if (direction_type == PLUS_X) {
        direction = 1;
        other_direction = 2;
    } else {
        direction = 2;
        other_direction = 1;
    }

    Static_Terrain_Block *block_alpha = NULL;
    Static_Terrain_Block *block_beta = NULL;
    if (parent->children[direction]) block_alpha = parent->children[direction]->block;
    if (parent->children[direction + other_direction]) block_beta = parent->children[direction + other_direction]->block;

    if (block_alpha == NULL) {
        assert(block_beta == NULL);
    }


    Static_Terrain_Block *block_alpha_neighbor = NULL;
    Mesh_Seam *seam_alpha = NULL;
    block_alpha_neighbor = neighbor->children[0]->block;
    seam_alpha = block_alpha->seam_sets[direction_type].res_equal;

    Static_Terrain_Block *block_beta_neighbor = NULL;
    Mesh_Seam *seam_beta = NULL;
    if (neighbor->children[other_direction]) {
        block_beta_neighbor = neighbor->children[other_direction]->block;
        seam_beta = block_beta->seam_sets[direction_type].res_equal;
    }

    assert(seam_alpha);
    
    Mesh_Seam *to_high_res_a = xref_seam(seam_alpha, block_alpha, 0);
    Mesh_Seam *to_high_res_b = xref_seam(seam_beta, block_beta, 0);

    Mesh_Seam *to_same_res_a = xref_seam(to_high_res_a, block_alpha_neighbor, 1);
    Mesh_Seam *to_same_res_b = xref_seam(to_high_res_b, block_beta_neighbor, 1);
    Mesh_Seam *to_same_res = concatenate_seams(to_same_res_a, to_same_res_b);
    delete to_same_res_a;
    delete to_same_res_b;

    parent->block->seam_sets[direction_type].res_equal = to_same_res;
    parent->block->seam_sets[direction_type].res_higher_a = to_high_res_a;
    parent->block->seam_sets[direction_type].res_higher_b = to_high_res_b;
}

void seam_build_downward(Static_Terrain_Tree *parent,
                         Static_Terrain_Tree *neighbor, int direction_type) {
    int direction, other_direction;

    if (direction_type == PLUS_X) {
        direction = 1;
        other_direction = 2;
    } else {
        assert(direction_type == PLUS_Y);

        direction = 2;
        other_direction = 1;
    }

    Static_Terrain_Block *block_alpha = get_child_block(neighbor, direction);
    Static_Terrain_Block *block_beta = get_child_block(neighbor, direction + other_direction);

    Mesh_Seam *seam_alpha = NULL;
    Mesh_Seam *seam_beta = NULL;

    if (block_alpha) seam_alpha = block_alpha->seam_sets[direction_type].res_equal;
    if (block_beta) seam_beta = block_beta->seam_sets[direction_type].res_equal;

    if (seam_alpha == NULL) assert(seam_beta == NULL);

    Mesh_Seam *to_low_res_alpha = xref_seam(seam_alpha,
                                            get_child_block(parent, 0), 1);
    Mesh_Seam *to_low_res_beta = xref_seam(seam_beta, 
                                           get_child_block(parent, 0 + other_direction), 1);

    if (block_alpha) block_alpha->seam_sets[direction_type].res_lower = to_low_res_alpha;
    if (block_beta) block_beta->seam_sets[direction_type].res_lower = to_low_res_beta;
}

void set_child(Static_Terrain_Tree *parent, int index, Static_Terrain_Tree *child) {
    parent->children[index] = child;
    if (child) {
        child->parent = parent;
        child->my_child_index = index;

        // We will overwrite parent->edge_length several times this
        // way, but should always be overwriting it with the same
        // value... basically we just need to find *some* child who
        // is not-NULL and derive our edge length from that child.
        // All children should have equal edge lengths.
        parent->block->edge_length = child->block->edge_length * 2;
    }
}

Static_Terrain_Tree *build_tree_upward(Static_Terrain_Tree *nodes,
                                       int num_blocks_x, int num_blocks_y) {

    if ((num_blocks_x == 1) && (num_blocks_y == 1)) return nodes;

    int num_parents_x = (num_blocks_x + 1) / 2;
    int num_parents_y = (num_blocks_y + 1) / 2;
    int num_parents = num_parents_x * num_parents_y;
    
    Static_Terrain_Tree *parents = new Static_Terrain_Tree[num_parents];
    
    int i, j;
    for (j = 0; j < num_parents_y; j++) {
        for (i = 0; i < num_parents_x; i++) {
            int parent_index = j * num_parents_x + i;

            Static_Terrain_Tree *parent = &parents[parent_index];
            parent->block = new Static_Terrain_Block;

            int ci0 = (j*2 + 0) * num_blocks_y + (i*2 + 0);
            int ci1 = (j*2 + 0) * num_blocks_y + (i*2 + 1);
            int ci2 = (j*2 + 1) * num_blocks_y + (i*2 + 0);
            int ci3 = (j*2 + 1) * num_blocks_y + (i*2 + 1);

            Static_Terrain_Tree *n0, *n1, *n2, *n3;
            n1 = NULL;
            n2 = NULL;
            n3 = NULL;

            n0 = &nodes[ci0]; 
            if (i*2 < num_blocks_x - 1) n1 = &nodes[ci1];
            if (j*2 < num_blocks_y - 1) n2 = &nodes[ci2];
            if ((i*2 < num_blocks_x - 1) && (j*2 < num_blocks_y - 1)) n3 = &nodes[ci3];

            set_child(parent, 0, n0);
            set_child(parent, 1, n1);
            set_child(parent, 2, n2);
            set_child(parent, 3, n3);

            parent->leaf_distance = n0->leaf_distance + 1;

            combine_tree(parent);
        }
    }

    // So the blocks are built; now build some of the seams between them.
    // This loop does seams for "same to same" and "same to higher" res
    // blocks on this level, and also "same to lower" for child levels.
    for (j = 0; j < num_parents_y; j++) {
        for (i = 0; i < num_parents_x; i++) {
            int parent_index = j * num_parents_x + i;
            int parent_index_plus_x = parent_index + 1;
            int parent_index_plus_y = parent_index + num_parents_x;

            Static_Terrain_Tree *parent = &parents[parent_index];

            if ((i < num_parents_x - 1) && (parent->children[1])) {
                Static_Terrain_Tree *parent_plus_x = &parents[parent_index_plus_x];
                seam_build_upward(parent, parent_plus_x, PLUS_X);
            }

            if (j < num_parents_y - 1) {
                Static_Terrain_Tree *parent_plus_y = &parents[parent_index_plus_y];
                seam_build_upward(parent, parent_plus_y, PLUS_Y);
            }

            if (i > 0) {
                Static_Terrain_Tree *parent_minus_x = &parents[parent_index - 1];
                seam_build_downward(parent, parent_minus_x, PLUS_X);
            }                

            if (j > 0) {
                Static_Terrain_Tree *parent_minus_y = &parents[parent_index - num_parents_x];
                seam_build_downward(parent, parent_minus_y, PLUS_Y);
            }                
        }
    }

    return build_tree_upward(parents, num_parents_x, num_parents_y);
}

Mesh_Seam *make_high_res_seam(int num_samples,
                              int starting_index_a, int stride_a,
                              int starting_index_b, int stride_b,
                              int *index_map_a, int *index_map_b) {
    int num_intervals = num_samples - 1;
    int max_faces = num_intervals * 2;

    Mesh_Seam *result = new Mesh_Seam(max_faces);
    result->num_faces = 0;

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

Mesh_Seam *make_high_res_seam_y(int num_samples_x, int num_samples_y, int *index_map_a, int *index_map_b) {
    Mesh_Seam *seam = make_high_res_seam(num_samples_x, 
                              (num_samples_y-1)*num_samples_x, 1,
                              0, 1,
                              index_map_a, index_map_b);

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

Mesh_Seam *make_high_res_seam_x(int num_samples_x, int num_samples_y, int *index_map_a, int *index_map_b) {
    return make_high_res_seam(num_samples_y, 
                              num_samples_x - 1, num_samples_x, 
                              0, num_samples_x,
                              index_map_a, index_map_b);
}

void reduce_seam(Mesh_Seam *seam, Triangle_List_Mesh *mesh_0, Mesh_Reducer *reducer) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        if (index->which_mesh == 1) {
            index->vertex_index = reducer->topology_handler->old_index_to_new_index[index->vertex_index];
        }
    }

    seam->remove_degenerate_faces();
    seam->compute_uv_coordinates(mesh_0);
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
Static_Terrain_Tree *make_static_terrain_tree(Elevation_Map *map,
                                              int block_samples) {
    int block_squares = block_samples - 1;
    int samples_x = block_samples;
    int samples_y = block_samples;

    int num_blocks_x = map->num_squares_x / block_squares;
    int num_blocks_y = map->num_squares_y / block_squares;
    int num_blocks = num_blocks_x * num_blocks_y;

    Static_Terrain_Tree *nodes = new Static_Terrain_Tree[num_blocks];
    int i, j;
    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;

            int si_x = block_squares * i;
            int si_y = block_squares * j;

            Static_Terrain_Block *block = make_terrain_block(map, samples_x, samples_y, si_x, si_y);

            nodes[block_index].leaf_distance = 0;
            nodes[block_index].block = block;
            block->mesh->material_info[0].name = app_strdup("3grass001"); // @Memory: Maybe don't do this strdup all the time.
        }
    }

    // Make seams between neighboring highest-level blocks
    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;
            int block_index_plus_y = block_index + num_blocks_x;
            int block_index_plus_x = block_index + 1;

            Static_Terrain_Block *block = nodes[block_index].block;
            
            Mesh_Seam *seam_x = NULL;
            Mesh_Seam *seam_y = NULL;

            if (i < num_blocks_x - 1) {
                Static_Terrain_Block *block_plus_x = nodes[block_index_plus_x].block;
                seam_x = make_high_res_seam_x(block_samples, block_samples, 
                                              block->index_map, block_plus_x->index_map);
            }

            if (j < num_blocks_y - 1) {
                Static_Terrain_Block *block_plus_y = nodes[block_index_plus_y].block;
                seam_y = make_high_res_seam_y(block_samples, block_samples, 
                                              block->index_map, block_plus_y->index_map);
            }

            block->seam_sets[PLUS_X].res_equal = seam_x;
            block->seam_sets[PLUS_Y].res_equal = seam_y;
        }
    }

    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;
            Static_Terrain_Block *block = nodes[block_index].block;
            delete [] block->index_map;
            block->index_map = NULL;
        }
    }

    Static_Terrain_Tree *root = build_tree_upward(nodes, num_blocks_x, num_blocks_y);


    // Delete the same-resolution seams on the lowest-level blocks...
    for (j = 0; j < num_blocks_y; j++) {
        for (i = 0; i < num_blocks_x; i++) {
            int block_index = j * num_blocks_x + i;
            Static_Terrain_Block *block = nodes[block_index].block;
            delete block->seam_sets[PLUS_X].res_equal;
            block->seam_sets[PLUS_X].res_equal = NULL;
            delete block->seam_sets[PLUS_Y].res_equal;
            block->seam_sets[PLUS_Y].res_equal = NULL;
        }
    }

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

Elevation_Map *load_elevation_map(char *filename) {
    File_Handle *file = fopen((char *)filename, "rb");

    if (!file) return NULL;

    Bt_Loader bt_loader;
    Elevation_Map *map = bt_loader.load(file);
    return map;
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

Static_Terrain_Block *make_terrain_block(Elevation_Map *map, int samples_i, int samples_j,
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

    Static_Terrain_Block *block = new Static_Terrain_Block;
    block->index_map = index_map;

    int max_samples = Max(samples_i, samples_j);
    float dimension_max = Max(map->square_size.x, map->square_size.y);
    block->edge_length = dimension_max * max_samples;

    init_block_for_mesh(block, mesh);
    return block;
}








Seam_Set::Seam_Set() {
    res_equal = NULL;
    res_lower = NULL;
    res_higher_a = NULL;
    res_higher_b = NULL;
}

Static_Terrain_Tree::Static_Terrain_Tree() {
    frame_index = 0;
    parent = NULL;

    children[0] = NULL;
    children[1] = NULL;
    children[2] = NULL;
    children[3] = NULL;

    block = NULL;

    my_child_index = -1;
}


Mesh_Seam::Mesh_Seam(int _num_faces) {
    num_faces = _num_faces;
    indices = new Seam_Index[num_faces * 3];
}

Mesh_Seam::~Mesh_Seam() {
    delete [] indices;
}

inline bool indices_match(Seam_Index *n0, Seam_Index *n1) {
    if (n0->which_mesh != n1->which_mesh) return false;
    if (n0->vertex_index != n1->vertex_index) return false;

    return true;
}

void Mesh_Seam::compute_uv_coordinates(Triangle_List_Mesh *mesh) {
    int num_indices = num_faces * 3;

    int last_anchor_point = -1;
    int num_anchor_points = 0;

    Vector3 p0, p1;
    Vector2 u0, u1;

    int i;
    for (i = 0; i < num_indices; i++) {
        if (indices[i].which_mesh == 0) {
            Vector2 uv = mesh->uvs[i];

            if (num_anchor_points == 0) {
                num_anchor_points++;

                last_anchor_point = i;
                p0 = mesh->vertices[i];
                u0 = uv;
            } else {
                if (indices[i].vertex_index == indices[last_anchor_point].vertex_index) continue;

                num_anchor_points++;

                p1 = mesh->vertices[i];
                u1 = uv;
            }
        }
    }

    if (num_anchor_points < 2) {
        Vector2 uv;

        if (num_anchor_points == 0) {
            uv.x = 0;
            uv.y = 0;
        } else {
            assert(last_anchor_point >= 0);
            int vertex_index = indices[last_anchor_point].vertex_index;
            uv = mesh->uvs[vertex_index];
        }

        int i;
        for (i = 0; i < num_indices; i++) {
            indices[i].uv = uv;
        }
    } else {
        // @Study: Seems like we have to break things into a direction
        // and length here, which is weird.  Can Clifford algebra help us
        // with this?
        Vector3 dp = p1 - p0;
        Vector2 du = u1 - u0;

        float dp_len = dp.length();
        Vector3 dp_dir;

        if (dp_len == 0) {
            dp_len = 1;
            dp_dir = Vector3(0, 0, 0);
        } else {
            dp_dir = dp;
            dp_dir.normalize();
        }

        int i;
        for (i = 0; i < num_indices; i++) {
            Vector3 pos = mesh->vertices[i];
            Vector3 relative = pos - p0;

            float proj_len = dot_product(relative, dp_dir);
            Vector2 uv = u0 + du * (proj_len / dp_len);

            indices[i].uv = uv;
        }            
    }
}

void sanity_seam(Mesh_Seam *seam) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        assert(index->vertex_index >= 0);
        assert(index->vertex_index < 4000);
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



/*
  A bunch of stuff to load and save!
*/

void save_seam(Mesh_Seam *seam, File_Handle *f) {
    if (seam == NULL) {
        put_u2b(0, f);
        return;
    }

    put_u2b(seam->num_faces, f);

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        put_u1b(index->which_mesh, f);
        put_u2b(index->vertex_index, f);

        // @Efficiency: Probably don't need full floating-point accuracy
        // for u and v, especially since this is a fricking seam.  We can
        // reduce the amount of file size we use, here.  But does it matter
        // (seam triangles should not be very common compared to main mesh
        // triangles).
        put_f32(index->uv.x, f);
        put_f32(index->uv.y, f);
    }
}

void save_seam_set(Seam_Set *set, File_Handle *f) {
    save_seam(set->res_lower, f);
    save_seam(set->res_equal, f);
    save_seam(set->res_higher_a, f);
    save_seam(set->res_higher_b, f);
}

void save_terrain_block(Static_Terrain_Block *block, File_Handle *f) {
    put_vector3(&block->position, f);
    put_vector3(&block->bounding_box_corner, f);
    put_vector3(&block->bounding_box_extents, f);
    put_f32(block->edge_length, f);
    save_triangle_list_mesh(block->mesh, f);

    int i;
    for (i = 0; i < NUM_NEIGHBOR_DIRECTIONS; i++) {
        save_seam_set(&block->seam_sets[i], f);
    }
}

void save_terrain_node(Static_Terrain_Tree *tree, File_Handle *f) {
    if (tree == NULL) {
        put_u1b(0, f);
        return;
    } else {
        put_u1b(1, f);
    }

    put_u2b(tree->my_child_index, f);
    put_u2b(tree->leaf_distance, f);

    if (tree->children[0]) {
        put_u1b(1, f);
    } else {
        put_u1b(0, f);
    }

    save_terrain_block(tree->block, f);

    if (tree->children[0]) {
        save_terrain_node(tree->children[0], f);
        save_terrain_node(tree->children[1], f);
        save_terrain_node(tree->children[2], f);
        save_terrain_node(tree->children[3], f);
    }
}

void save_terrain(Static_Terrain_Tree *tree, File_Handle *f) {
    assert(f != NULL);

    save_terrain_node(tree, f);
}



void load_seam(File_Handle *f, Mesh_Seam **seam_return, bool *error) {
    if (*error) return;
    
    int num_faces;
    get_u2b(f, &num_faces, error);
    if (*error) return;

    if (num_faces == 0) {
        *seam_return = NULL;
        return;
    }

    Mesh_Seam *seam = new Mesh_Seam(num_faces);
    *seam_return = seam;

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];

        int which_mesh, vertex_index;
        float u, v;
        get_u1b(f, &which_mesh, error);
        get_u2b(f, &vertex_index, error);
        get_f32(f, &u, error);
        get_f32(f, &v, error);

        if (*error) return;

        index->which_mesh = which_mesh;
        index->vertex_index = vertex_index;
        index->uv = Vector2(u, v);
    }
}

void load_seam_set(File_Handle *f, Seam_Set *set, bool *error) {
    load_seam(f, &set->res_lower, error);
    load_seam(f, &set->res_equal, error);
    load_seam(f, &set->res_higher_a, error);
    load_seam(f, &set->res_higher_b, error);
}

Static_Terrain_Block *load_terrain_block(File_Handle *f, bool *error) {
    Vector3 position, corner, extents;
    float edge_length;

    get_vector3(f, &position, error);
    get_vector3(f, &corner, error);
    get_vector3(f, &extents, error);
    get_f32(f, &edge_length, error);

    if (*error) return NULL;

    Triangle_List_Mesh *mesh;
    mesh = load_triangle_list_mesh(f);
    if (mesh == NULL) return NULL;

    Static_Terrain_Block *block = new Static_Terrain_Block();

    block->position = position;
    block->bounding_box_corner = corner;
    block->bounding_box_extents = extents;
    block->edge_length = edge_length;

    block->mesh = mesh;

    int i;
    for (i = 0; i < NUM_NEIGHBOR_DIRECTIONS; i++) {
        load_seam_set(f, &block->seam_sets[i], error);
        if (*error) return NULL;
    }

    return block;
}

Static_Terrain_Tree *load_terrain_node(File_Handle *f, bool *error) {
    int node_exists;
    get_u1b(f, &node_exists, error);
    if (*error) return NULL;
    if (!node_exists) return NULL;

    int child_index, leaf_distance, has_children;

    get_u2b(f, &child_index, error);
    get_u2b(f, &leaf_distance, error);
    get_u1b(f, &has_children, error);
    if (*error) return NULL;

    Static_Terrain_Tree *tree = new Static_Terrain_Tree;
    tree->my_child_index = child_index;
    tree->leaf_distance = leaf_distance;

    tree->block = load_terrain_block(f, error);
    if (*error) return NULL;

    if (has_children) {
        tree->children[0] = load_terrain_node(f, error);
        if (*error) return NULL;
        tree->children[1] = load_terrain_node(f, error);
        if (*error) return NULL;
        tree->children[2] = load_terrain_node(f, error);
        if (*error) return NULL;
        tree->children[3] = load_terrain_node(f, error);
        if (*error) return NULL;

        int i;
        for (i = 0; i < 4; i++) {
            if (tree->children[i]) {
                tree->children[i]->parent = tree;
            }
        }
    }

    return tree;
}

Static_Terrain_Tree *load_terrain(File_Handle *f) {
    assert(f != NULL);

    bool error = false;
    Static_Terrain_Tree *root = load_terrain_node(f, &error);
    return root;
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
