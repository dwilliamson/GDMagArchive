#include "framework.h"

#include "mesh.h"
#include "mesh/mesh_builder.h"
#include "make_world.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include "tangent_frames.h"
#include "seam_database.h"
#include "mesh_seam.h"

#include <float.h>
#include <math.h>
#include <stdio.h> // @Refactor for terrain loading; remove?

Elevation_Map *make_elevation_map();

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
                 int triangle_list_cursor, Vector3 offset,
                 int *index_map) {
    int i;
    for (i = 0; i < source->num_vertices; i++) {
        int di = i + vertex_cursor;

        result->vertices[di] = source->vertices[i] + offset;
        result->tangent_frames[di] = source->tangent_frames[i];
        result->uvs[di] = source->uvs[i];
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

    assert(result->num_triangle_lists > 0);  // So we can copy a material info out.  @Improvement: Eventually seams will have their own material info and we should use that instead of this hack


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

        World_Block *block = seam->block_membership[seam_index->which_mesh];
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
Triangle_List_Mesh *merge_meshes(int num_meshes, Triangle_List_Mesh **meshes, 
                                 Vector3 *offsets, int **index_maps,
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

        fill_result(result, mesh, vertices_cursor, indices_cursor, triangle_list_cursor, offsets[i], index_map);

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

void xref_block_index_map(World_Block *block, Mesh_Reducer *reducer) {

    int i;
    for (i = 0; i < block->mesh->num_vertices; i++) {
        int old_index = block->index_map_into_parent[i];
        block->index_map_into_parent[i] = reducer->topology_handler->old_index_to_new_index[old_index];
    }
}

Vector3 get_position(Mesh_Seam *seam, 
                     World_Block *b0, World_Block *b1,
                     Seam_Index *index) {
    if (index->which_mesh == 0) return b0->mesh->vertices[index->vertex_index] + b0->position;
    return b1->mesh->vertices[index->vertex_index] + b1->position;
}

void validate_seam(Mesh_Seam *seam, World_Block *b0, World_Block *b1) {
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


World_Block::World_Block() {
    frame_index = 0;
    parent = NULL;

    children[0] = NULL;
    children[1] = NULL;
    children[2] = NULL;
    children[3] = NULL;

    num_children = 0;
    block_search_marker = 0;
    leaf_distance = 0;

    worldspace_error = 0;
}

World_Block::~World_Block() {
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

void sanity_seam(Mesh_Seam *seam, World_Block *block_a,
                 World_Block *block_b) {
    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        assert((index->which_mesh == 0) || (index->which_mesh == 1));
        World_Block *block = block_a;
        if (index->which_mesh == 1) block = block_b;

        assert(index->vertex_index >= 0);
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}


void Mesh_Seam::remove_degenerate_faces() {

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
    World_Block *block1 = *(World_Block **)b1;
    World_Block *block2 = *(World_Block **)b2;

    if (block1->block_id < block2->block_id) return -1;
    if (block1->block_id > block2->block_id) return +1;
    return 0;
}

int find_remapping(World_Block *old_block,
                   World_Block **new_blocks, int num_blocks) {
    int i;
    for (i = 0; i < num_blocks; i++) {
        if (new_blocks[i] == old_block) return i;
    }

    return -1;
}

void Mesh_Seam::simplify_block_membership() {
    // Copy block membership

    World_Block *new_membership[3];
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
        World_Block *block = block_membership[index->which_mesh];
        assert(block);
        assert(index->vertex_index < block->mesh->num_vertices);
    }
}
