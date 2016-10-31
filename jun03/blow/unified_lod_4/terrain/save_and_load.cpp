#include "framework.h"

#include "mesh.h"
#include "mesh/mesh_builder.h"
#include "make_terrain.h"
#include "bt_loader.h"
#include "binary_file_stuff.h"

#include "seam_database.h"

#include <float.h>
#include <math.h>
#include <stdio.h> // @Refactor for terrain loading; remove?


/*
  A bunch of stuff to load and save!
*/

void save_seam(Mesh_Seam *seam, File_Handle *f) {
    assert(seam);

    int i;
    for (i = 0; i < 3; i++) {
        Static_Terrain_Block *block = seam->block_membership[i];
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
    }
}

void save_terrain_node(Static_Terrain_Block *block, 
                       File_Handle *f) {
    assert(block != NULL);

    put_u2b(block->my_child_index, f);
    put_u2b(block->leaf_distance, f);
    put_u4b(block->block_id, f);

    put_vector3(&block->position, f);
    put_vector3(&block->bounding_box_corner, f);
    put_vector3(&block->bounding_box_extents, f);
    put_f32(block->edge_length, f);

    save_triangle_list_mesh(block->mesh, f);

    put_u1b(block->num_children, f);
    int i;
    for (i = 0; i < block->num_children; i++) {
        save_terrain_node(block->children[i], f);
    }
}

void save_seam_database(Seam_Database *database, File_Handle *f) {
    put_u4b(database->seams.live_items, f);

    Mesh_Seam *seam;
    Array_Foreach(&database->seams, seam) {
        save_seam(seam, f);
    } Endeach;
}

void save_terrain(Static_Terrain_Block *tree, 
                  Seam_Database *database, File_Handle *f) {
    assert(f != NULL);

    save_terrain_node(tree, f);
    save_seam_database(database, f);
}



// @Refactor: DIGUSTING code here, terribly slow,
// please use a hash table or something.
Static_Terrain_Block *find_block_id(Static_Terrain_Block *terrain, int id) {
    if (terrain->block_id == (Block_Identifier)id) return terrain;

    int i;
    for (i = 0; i < terrain->num_children; i++) {
        Static_Terrain_Block *result = find_block_id(terrain->children[i], id);
        if (result) return result;
    }

    return NULL;
}

Mesh_Seam *load_seam(Static_Terrain_Block *terrain, File_Handle *f, bool *error) {
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

    if (num_faces == 0) return NULL;


    Mesh_Seam *seam = new Mesh_Seam(num_faces);

    // xref the block ids, now that we have a place to store the blocks
    for (i = 0; i < 3; i++) {
        if (block_ids[i] == 0) {
            seam->block_membership[i] = NULL;
        } else {
            // @Refactor: DIGUSTING code here, terribly slow,
            // please use a hash table or something.
            Static_Terrain_Block *block = find_block_id(terrain, block_ids[i]);
            assert(block != NULL);
            seam->block_membership[i] = block;
        }
    }

    assert(seam->block_membership[0] != NULL);
    assert(seam->block_membership[1] != NULL);

    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];

        int which_mesh, vertex_index;
        get_u1b(f, &which_mesh, error);
        get_u2b(f, &vertex_index, error);

        if (*error) return NULL;

        index->which_mesh = which_mesh;
        index->vertex_index = vertex_index;
    }

    return seam;
}


Static_Terrain_Block *load_terrain_node(File_Handle *f, bool *error) {
    int child_index, leaf_distance;
    Block_Identifier block_id;

    get_u2b(f, &child_index, error);
    get_u2b(f, &leaf_distance, error);
    get_u4b(f, (int *)&block_id, error);
    if (*error) return NULL;

    Static_Terrain_Block *block = new Static_Terrain_Block;
    block->my_child_index = child_index;
    block->leaf_distance = leaf_distance;
    block->block_id = block_id;




    block->mesh = NULL;    // Hopefully will get overwritten with successful mesh.


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

    block->position = position;
    block->bounding_box_corner = corner;
    block->bounding_box_extents = extents;
    block->edge_length = edge_length;

    block->mesh = mesh;



    int num_children;
    get_u1b(f, &num_children, error);
    if (*error) return NULL;
    block->num_children = num_children;

    int i;
    for (i = 0; i < num_children; i++) {
        block->children[i] = load_terrain_node(f, error);
        if (*error) return NULL;
        if (block->children[i] == NULL) return NULL;
        
        block->children[i]->parent = block;  // XXX Needed?
    }

    return block;
}

void load_seam_database(Seam_Database *database, Static_Terrain_Block *terrain, File_Handle *f, bool *error) {
    int num_seams;
    get_u4b(f, &num_seams, error);
    if (*error) return;
    assert(num_seams >= 0);

    int i;
    for (i = 0; i < num_seams; i++) {
        Mesh_Seam *seam = load_seam(terrain, f, error);
        database->add(seam);
    }

    return;
}

Static_Terrain_Block *load_terrain(File_Handle *f, Seam_Database **database_return) {
    assert(f != NULL);

    bool error = false;
    Static_Terrain_Block *root = load_terrain_node(f, &error);

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

