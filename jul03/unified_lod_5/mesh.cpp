#include "framework.h"
#include "mesh.h"
#include "binary_file_stuff.h"

/*
  This file contains support functions for the Triangle_List_Mesh:
  initialization, loading/saving, etc.
*/
const int TRIANGLE_LIST_MESH_FILE_VERSION = 2;

Triangle_List_Mesh::Triangle_List_Mesh() {
    vertices = NULL;
    uvs = NULL;
    tangent_frames = NULL;
    indices = NULL;
    canonical_vertex_map = NULL;
    triangle_list_info = NULL;
    material_info = NULL;
    name = NULL;
    user_data = NULL;
    normals = NULL;

    index_buffer = 0;
    vertex_buffer = 0;
}

Triangle_List_Mesh::~Triangle_List_Mesh() {
    delete [] vertices;
    delete [] material_info;  // XXX Names are currently leaked, fix this for a real app!
    delete [] name;
}


void copy_material_info(Mesh_Material_Info *dest, Mesh_Material_Info *src) {
    // XXX If we are to become careful about allocations here, this will have to change.
    dest->name = app_strdup(src->name);
    dest->texture_index = src->texture_index;
}



void Triangle_List_Mesh::allocate_materials(int n) {
    num_materials = n;
    material_info = new Mesh_Material_Info[n];
    int i;
    for (i = 0; i < num_materials; i++) material_info[i].texture_index = -1;
}


void Triangle_List_Mesh::allocate_geometry(int _num_vertices, int _num_faces) {
    num_vertices = _num_vertices;
    num_faces = _num_faces;
    
    vertices = new Vector3[num_vertices];
    uvs = new Vector2[num_vertices];
    tangent_frames = new Quaternion[num_vertices];
    canonical_vertex_map = new int[num_vertices];

    num_indices = num_faces * 3;
    indices = new int[num_indices];
}




Triangle_List_Mesh *load_triangle_list_mesh(FILE *f) {
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
        get_u4b(f, &mesh->canonical_vertex_map[i], &error);
    }

    if (error) return NULL; // Leak!

    for (i = 0; i < mesh->num_indices; i++) {
        int index;
        get_u4b(f, &index, &error);
        mesh->indices[i] = index;
    }

    if (error) return NULL; // Leak!

    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &mesh->triangle_list_info[i];
        get_u4b(f, &info->material_index, &error);
        get_u4b(f, &info->num_vertices, &error);
        get_u4b(f, &info->start_of_list, &error);
    }

    if (error) return NULL; // Leak!

    return mesh;  // We got it!
}


