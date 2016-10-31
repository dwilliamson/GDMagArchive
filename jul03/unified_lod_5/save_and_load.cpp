#include "framework.h"

#include "mesh.h"
#include "mesh/mesh_builder.h"
#include "make_world.h"
#include "bt_loader.h"
#include "binary_file_stuff.h"

#include "seam_database.h"
#include "mesh_seam.h"

#include <float.h>
#include <math.h>
#include <stdio.h> // @Refactor for world loading; remove?


/*
  A bunch of stuff to load and save!
*/

void save_seam(Mesh_Seam *seam, File_Handle *f) {
    assert(seam);

    int i;
    for (i = 0; i < 3; i++) {
        World_Block *block = seam->block_membership[i];
        if (block) {
            put_u4b(block->block_id, f);
        } else {
            put_u4b(0, f);
        }
    }

    put_u2b(seam->num_faces, f);


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


const int TRIANGLE_LIST_MESH_FILE_VERSION = 103;
void save_small_triangle_list_mesh(Triangle_List_Mesh *mesh, FILE *f) {
    put_u2b(TRIANGLE_LIST_MESH_FILE_VERSION, f);

    assert(mesh->num_vertices < 32768);

    put_u4b(mesh->num_vertices, f);
    put_u4b(mesh->num_faces, f);
    put_u4b(mesh->num_materials, f);
    put_u4b(mesh->num_triangle_lists, f);
    put_u4b(mesh->num_indices, f);

    int is_animatable = 0;
    put_u1b(is_animatable, f);

    int use_tangent_frames = mesh->tangent_frames ? 1 : 0;
    put_u1b(use_tangent_frames, f);

    int i;
    for (i = 0; i < mesh->num_materials; i++) {
        Mesh_Material_Info *info = &mesh->material_info[i];
        put_string(info->name, f);
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        Vector3 *pos = &mesh->vertices[i];
        put_vector3(pos, f);
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        Vector2 *uv = &mesh->uvs[i];
        put_f32(uv->x, f);
        put_f32(uv->y, f);
    }

    if (use_tangent_frames) {
        for (i = 0; i < mesh->num_vertices; i++) {
            Quaternion *frame = &mesh->tangent_frames[i];
            put_f32(frame->w, f);
            put_f32(frame->x, f);
            put_f32(frame->y, f);
            put_f32(frame->z, f);
        }
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        put_u2b(mesh->canonical_vertex_map[i], f);
    }

    for (i = 0; i < mesh->num_indices; i++) {
        put_u2b(mesh->indices[i], f);
    }

    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &mesh->triangle_list_info[i];
        put_u2b(info->material_index, f);
        put_u2b(info->num_vertices, f);
        put_u2b(info->start_of_list, f);
    }
}

void save_world_node(World_Block *block, 
                     File_Handle *f) {
    assert(block != NULL);

    put_u2b(block->leaf_distance, f);
    put_u4b(block->block_id, f);

    put_vector3(&block->position, f);
    put_vector3(&block->bounding_box_corner, f);
    put_vector3(&block->bounding_box_extents, f);
    put_f32(block->edge_length, f);
    put_f32(block->worldspace_error, f);

    if (block->mesh) {
        put_u1b(1, f);
        save_small_triangle_list_mesh(block->mesh, f);
    } else {
        put_u1b(0, f);
    }

    put_u1b(block->num_children, f);
    int i;
    for (i = 0; i < block->num_children; i++) {
        save_world_node(block->children[i], f);
    }
}

void save_seam_database(Seam_Database *database, File_Handle *f) {
    put_u4b(database->seams.live_items, f);

    Mesh_Seam *seam;
    Array_Foreach(&database->seams, seam) {
        save_seam(seam, f);
    } Endeach;
}

void save_world(World_Block *tree, 
                  Seam_Database *database, File_Handle *f) {
    assert(f != NULL);

    save_world_node(tree, f);
    save_seam_database(database, f);
}



// @Refactor: DIGUSTING code here, terribly slow,
// please use a hash table or something.
World_Block *find_block_id(World_Block *world, int id) {
    if (world->block_id == (Block_Identifier)id) return world;

    int i;
    for (i = 0; i < world->num_children; i++) {
        World_Block *result = find_block_id(world->children[i], id);
        if (result) return result;
    }

    return NULL;
}

Mesh_Seam *load_seam(World_Block *world, File_Handle *f, bool *error) {
    int block_ids[3];

    int i;
    for (i = 0; i < 3; i++) {
        int block_id;
        get_u4b(f, &block_id, error);
        block_ids[i] = block_id;
    }

    if (*error) return NULL;
    
    int num_faces;
    get_u2b(f, &num_faces, error);
    if (*error) return NULL;


    Mesh_Seam *seam = new Mesh_Seam(num_faces);

    // xref the block ids, now that we have a place to store the blocks
    for (i = 0; i < 3; i++) {
        if (block_ids[i] == 0) {
            seam->block_membership[i] = NULL;
        } else {
            // @Refactor: DIGUSTING code here, terribly slow,
            // please use a hash table or something.
            World_Block *block = find_block_id(world, block_ids[i]);
            assert(block != NULL);
            seam->block_membership[i] = block;
        }
    }

    assert(seam->block_membership[0] != NULL);
    assert(seam->block_membership[1] != NULL);

    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];

        int which_mesh, vertex_index;
        float u, v;
        get_u1b(f, &which_mesh, error);
        get_u2b(f, &vertex_index, error);
        get_f32(f, &u, error);
        get_f32(f, &v, error);

        if (*error) return NULL;

        index->which_mesh = which_mesh;
        index->vertex_index = vertex_index;
        index->uv = Vector2(u, v);
    }

    return seam;
}


Triangle_List_Mesh *load_small_triangle_list_mesh(FILE *f) {
    bool error = false;

    int file_version;
    get_u2b(f, &file_version, &error);
    if (error) return NULL;

    int num_vertices;
    int num_faces;
    int num_materials;
    int num_triangle_lists;
    int num_indices;

    get_u4b(f, &num_vertices, &error);
    get_u4b(f, &num_faces, &error);
    get_u4b(f, &num_materials, &error);
    get_u4b(f, &num_triangle_lists, &error);
    get_u4b(f, &num_indices, &error);

    int is_animatable;
    int use_tangent_frames;
    get_u1b(f, &is_animatable, &error);
    get_u1b(f, &use_tangent_frames, &error);

    if (error) return NULL;

    // We're not loading animations this time, so we ignore the
    // value of 'is_animatable'.

    Triangle_List_Mesh *mesh = new Triangle_List_Mesh();

    // @Optimization: Allocate all this stuff in one block.
    assert(num_faces * 3 == num_indices);
    mesh->allocate_geometry(num_vertices, num_faces);
    mesh->allocate_materials(num_materials);

    mesh->num_triangle_lists = num_triangle_lists;

    mesh->triangle_list_info = new Triangle_List_Info[mesh->num_triangle_lists];

    if (!use_tangent_frames) {
        // @Speed: If we are loading a lot of tangent-frameless
        // meshes, then maybe we shouldn't allocate these in 
        // the first place.
        delete [] mesh->tangent_frames;
        mesh->tangent_frames = NULL;
    }

    mesh->name = NULL;
    mesh->user_data = NULL;

    int i;
    for (i = 0; i < mesh->num_materials; i++) {
        Mesh_Material_Info *info = &mesh->material_info[i];
        info->texture_index = -1;
        get_string(f, &info->name, &error);
        if (error) return NULL; // Leak!
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        Vector3 *pos = &mesh->vertices[i];
        get_vector3(f, pos, &error);
    }
    
    if (error) return NULL; // Leak!

    for (i = 0; i < mesh->num_vertices; i++) {
        Vector2 *uv = &mesh->uvs[i];
        get_f32(f, &uv->x, &error);
        get_f32(f, &uv->y, &error);
    }

    if (error) return NULL; // Leak!

    if (use_tangent_frames) {
        for (i = 0; i < mesh->num_vertices; i++) {
            Quaternion *frame = &mesh->tangent_frames[i];
            get_f32(f, &frame->w, &error);
            get_f32(f, &frame->x, &error);
            get_f32(f, &frame->y, &error);
            get_f32(f, &frame->z, &error);
        }
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        get_u2b(f, &mesh->canonical_vertex_map[i], &error);
    }

    if (error) return NULL; // Leak!

    for (i = 0; i < mesh->num_indices; i++) {
        int index;
        get_u2b(f, &index, &error);
        mesh->indices[i] = index;
    }

    if (error) return NULL; // Leak!

    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &mesh->triangle_list_info[i];
        get_u2b(f, &info->material_index, &error);
        get_u2b(f, &info->num_vertices, &error);
        get_u2b(f, &info->start_of_list, &error);
    }

    if (error) return NULL; // Leak!

    return mesh;  // We got it!
}

World_Block *load_world_node(File_Handle *f, bool *error) {
    int leaf_distance;
    Block_Identifier block_id;

    get_u2b(f, &leaf_distance, error);
    get_u4b(f, (int *)&block_id, error);
    if (*error) return NULL;

    World_Block *block = new World_Block;
    block->leaf_distance = leaf_distance;
    block->block_id = block_id;




    block->mesh = NULL;    // Hopefully will get overwritten with successful mesh.


    Vector3 position, corner, extents;
    float edge_length;
    float worldspace_error;

    get_vector3(f, &position, error);
    get_vector3(f, &corner, error);
    get_vector3(f, &extents, error);
    get_f32(f, &edge_length, error);
    get_f32(f, &worldspace_error, error);

    if (*error) return NULL;

    Triangle_List_Mesh *mesh;
    int mesh_exists = 0;
    get_u1b(f, &mesh_exists, error);
    if (mesh_exists) {
        assert(mesh_exists == 1);
        mesh = load_small_triangle_list_mesh(f);
        if (mesh == NULL) return NULL;
    } else {
        mesh = NULL;
    }

    block->position = position;
    block->bounding_box_corner = corner;
    block->bounding_box_extents = extents;
    block->edge_length = edge_length;
    block->worldspace_error = worldspace_error;
    block->mesh = mesh;



    int num_children;
    get_u1b(f, &num_children, error);
    if (*error) return NULL;
    block->num_children = num_children;

    int i;
    for (i = 0; i < num_children; i++) {
        block->children[i] = load_world_node(f, error);
        if (*error) return NULL;
        if (block->children[i] == NULL) return NULL;
        
        block->children[i]->parent = block;
    }

    return block;
}

void load_seam_database(Seam_Database *database, World_Block *world, File_Handle *f, bool *error) {
    int num_seams;
    get_u4b(f, &num_seams, error);
    if (*error) return;
    assert(num_seams >= 0);

    int i;
    for (i = 0; i < num_seams; i++) {
        Mesh_Seam *seam = load_seam(world, f, error);
        if (seam) database->add(seam);
    }

    return;
}

World_Block *load_world(File_Handle *f, Seam_Database **database_return) {
    assert(f != NULL);

    bool error = false;
    World_Block *root = load_world_node(f, &error);

    Seam_Database *database = new Seam_Database();
    load_seam_database(database, root, f, &error);

    *database_return = database;

    return root;
}






Elevation_Map *load_elevation_map(char *filename) {
    File_Handle *file = fopen((char *)filename, "rb");

    if (!file) return NULL;

    Bt_Loader bt_loader;
    Elevation_Map *map = bt_loader.load(file);
    fclose(file);

    return map;
}

