#include "gamelib_core.h"
#include "hermite.h"
#include "fcurve.h"
#include "mesh.h"
#include "binary_file_stuff.h"

Mesh_Base::~Mesh_Base() {
    delete [] vertices;
    delete [] blend_info;
    delete [] material_info;  // XXX leak names
//    delete [] name;  // XXX name often not initialized -- fix this
	
    Foreach(&skeleton_node_names, name) { delete [] name; } Endeach;
}

const int STRIPPED_MESH_FILE_VERSION = 1;

void save_stripped_mesh(Triangle_Strip_Mesh *mesh, FILE *f) {
    put_u2b(STRIPPED_MESH_FILE_VERSION, f);

    put_u4b(mesh->num_vertices, f);
    put_u4b(mesh->num_faces, f);
    put_u4b(mesh->num_materials, f);
    put_u4b(mesh->num_triangle_strips, f);
    put_u4b(mesh->num_indices, f);

    int is_animatable = 0;
    if (mesh->blend_info) is_animatable = 1;

    put_u1b(is_animatable, f);

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
        Texture_Vertex *uv = &mesh->uvs[i];
        put_f32(uv->u, f);
        put_f32(uv->v, f);
    }

    for (i = 0; i < mesh->num_vertices; i++) {
        put_u4b(mesh->canonical_vertex_map[i], f);
    }

    for (i = 0; i < mesh->num_indices; i++) {
        put_u4b(mesh->indices[i], f);
    }

    for (i = 0; i < mesh->num_triangle_strips; i++) {
        Triangle_Strip_Mesh_Triangle_Strip_Info *info = &mesh->triangle_strip_info[i];
        put_u4b(info->material_index, f);
        put_u4b(info->num_vertices, f);
        put_u4b(info->start_of_strip, f);
    }

    if (is_animatable) {
        put_u4b(mesh->skeleton_node_names.items, f);

        char *name;
        Foreach(&mesh->skeleton_node_names, name) {
            put_string(name, f);
        } Endeach;

        for (i = 0; i < mesh->num_vertices; i++) {
            Vertex_Blend_Info *info = &mesh->blend_info[i];

            put_u2b(info->num_matrices, f);

            int j;
            for (j = 0; j < info->num_matrices; j++) {
                put_u2b(info->matrix_indices[j], f);
                put_vector3(&info->matrix_offsets[j], f);
                put_f32(info->matrix_weights[j], f);
            }
        }
    }
}

Triangle_Strip_Mesh *load_stripped_mesh(FILE *f) {
    bool error = false;

    int file_version;
    get_u2b(f, &file_version, &error);
    if (error) return NULL;

    assert(file_version == STRIPPED_MESH_FILE_VERSION);

    int num_vertices;
    int num_faces;
    int num_materials;
    int num_triangle_strips;
    int num_triangle_strip_indices;

    get_u4b(f, &num_vertices, &error);
    get_u4b(f, &num_faces, &error);
    get_u4b(f, &num_materials, &error);
    get_u4b(f, &num_triangle_strips, &error);
    get_u4b(f, &num_triangle_strip_indices, &error);

    int is_animatable;
    get_u1b(f, &is_animatable, &error);

    if (error) return NULL;


    Triangle_Strip_Mesh *mesh = new Triangle_Strip_Mesh();
    mesh->num_vertices = num_vertices;
    mesh->num_faces = num_faces;
    mesh->num_materials = num_materials;
    mesh->num_triangle_strips = num_triangle_strips;
    mesh->num_indices = num_triangle_strip_indices;

    // @Optimization: Allocate all this stuff in one block.
    mesh->vertices = new Vector3[mesh->num_vertices];
    mesh->uvs = new Texture_Vertex[mesh->num_vertices];
    mesh->canonical_vertex_map = new int[mesh->num_vertices];
    mesh->indices = new int[mesh->num_indices];
    mesh->triangle_strip_info = new Triangle_Strip_Mesh_Triangle_Strip_Info[mesh->num_triangle_strips];
    mesh->material_info = new Mesh_Material_Info[mesh->num_materials];

    mesh->name = NULL;
    mesh->user_data = NULL;

    if (is_animatable) {
        mesh->blend_info = new Vertex_Blend_Info[mesh->num_vertices];
    } else {
        mesh->blend_info = NULL;
    }

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
        Texture_Vertex *uv = &mesh->uvs[i];
        get_f32(f, &uv->u, &error);
        get_f32(f, &uv->v, &error);
    }

    if (error) return NULL; // Leak!

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

    for (i = 0; i < mesh->num_triangle_strips; i++) {
        Triangle_Strip_Mesh_Triangle_Strip_Info *info = &mesh->triangle_strip_info[i];
        get_u4b(f, &info->material_index, &error);
        get_u4b(f, &info->num_vertices, &error);
        get_u4b(f, &info->start_of_strip, &error);
    }

    if (error) return NULL; // Leak!

    if (is_animatable) {
        int num_node_names;
        get_u4b(f, &num_node_names, &error);

        if (error) return NULL; // Leak!

        int i;
        for (i = 0; i < num_node_names; i++) {
            char *name;
            get_string(f, &name, &error);

            if (error) return NULL; // Leak!

            mesh->skeleton_node_names.add(name);
        }

        for (i = 0; i < mesh->num_vertices; i++) {
            Vertex_Blend_Info *info = &mesh->blend_info[i];

            get_u2b(f, &info->num_matrices, &error);

            int j;
            for (j = 0; j < info->num_matrices; j++) {
                get_u2b(f, &info->matrix_indices[j], &error);
                get_vector3(f, &info->matrix_offsets[j], &error);
                get_f32(f, &info->matrix_weights[j], &error);
            }
        }

        if (error) return NULL; // Leak!
    }

    return mesh;  // We got it!
}

void postprocess_normals(Mesh_Base *mesh, float *vertex_normals, int stride_in_floats, int *canonical_vertex_map) {
    int i;
    for (i = 0; i < mesh->num_vertices; i++) {
        int canonical = canonical_vertex_map[i];
        Vector3 *dest = (Vector3 *)(vertex_normals + i * stride_in_floats);
        if (canonical != i) {
            Vector3 *src = (Vector3 *)(vertex_normals + canonical * stride_in_floats);
            *dest = *src;
        } else {
            if (dest->length_squared()) dest->normalize();
        }
    }
}

void Triangle_Strip_Mesh::update_normals_slow(float *vertices, float *vertex_normals, int source_stride_in_bytes, int dest_stride_in_bytes) {
    if (source_stride_in_bytes == -1) source_stride_in_bytes = sizeof(Vector3);
    if (dest_stride_in_bytes == -1) dest_stride_in_bytes = sizeof(Vector3);

    assert(source_stride_in_bytes >= 3 * sizeof(float));
    assert(source_stride_in_bytes % sizeof(float) == 0);
    assert(dest_stride_in_bytes >= 3 * sizeof(float));
    assert(dest_stride_in_bytes % sizeof(float) == 0);

    int source_stride_in_floats = source_stride_in_bytes / sizeof(float);
    int dest_stride_in_floats = dest_stride_in_bytes / sizeof(float);

    int i;
    float *dest_cursor = vertex_normals;
    for (i = 0; i < num_vertices; i++) {
        dest_cursor[0] = 0;
        dest_cursor[1] = 0;
        dest_cursor[2] = 0;
        dest_cursor += dest_stride_in_floats;
    }

    int j;
    for (j = 0; j < num_triangle_strips; j++) {
        Triangle_Strip_Mesh_Triangle_Strip_Info *info = &triangle_strip_info[j];
        
        int i;
        for (i = 0; i < info->num_vertices - 2; i++) {
            int index0 = i;
            int index1, index2;

            if (i & 1) {
                index1 = i + 2;
                index2 = i + 1;
            } else {
                index1 = i + 1;
                index2 = i + 2;
            }

            int n0 = indices[info->start_of_strip + index0];
            int n1 = indices[info->start_of_strip + index1];
            int n2 = indices[info->start_of_strip + index2];

            // Look up the vertex coordinates, find the triangle normal.

            Vector3 *v0 = (Vector3 *)(vertices + n0 * source_stride_in_floats);
            Vector3 *v1 = (Vector3 *)(vertices + n1 * source_stride_in_floats);
            Vector3 *v2 = (Vector3 *)(vertices + n2 * source_stride_in_floats);

            Vector3 d0 = *v1 - *v0;
            Vector3 d1 = *v2 - *v0;

            Vector3 cross = cross_product(d0, d1);
            
            int nn0 = canonical_vertex_map[n0];
            int nn1 = canonical_vertex_map[n1];
            int nn2 = canonical_vertex_map[n2];

            Vector3 *dest0 = (Vector3 *)(vertex_normals + nn0 * dest_stride_in_floats);
            Vector3 *dest1 = (Vector3 *)(vertex_normals + nn1 * dest_stride_in_floats);
            Vector3 *dest2 = (Vector3 *)(vertex_normals + nn2 * dest_stride_in_floats);
            *dest0 = *dest0 + cross;
            *dest1 = *dest1 + cross;
            *dest2 = *dest2 + cross;
        }
    }

    for (i = 0; i < num_vertices; i++) {
        int canonical = canonical_vertex_map[i];
        Vector3 *dest = (Vector3 *)(vertex_normals + i * dest_stride_in_floats);
        if (canonical != i) {
            Vector3 *src = (Vector3 *)(vertex_normals + canonical * dest_stride_in_floats);
            *dest = *src;
        } else {
            if (dest->length_squared()) dest->normalize();
        }
    }
}

void Triangle_List_Mesh::update_normals_slow(float *vertices, float *vertex_normals, int source_stride_in_bytes, int dest_stride_in_bytes) {
    if (source_stride_in_bytes == -1) source_stride_in_bytes = sizeof(Vector3);
    if (dest_stride_in_bytes == -1) dest_stride_in_bytes = sizeof(Vector3);

    assert(source_stride_in_bytes >= 3 * sizeof(float));
    assert(source_stride_in_bytes % sizeof(float) == 0);
    assert(dest_stride_in_bytes >= 3 * sizeof(float));
    assert(dest_stride_in_bytes % sizeof(float) == 0);

    int source_stride_in_floats = source_stride_in_bytes / sizeof(float);
    int dest_stride_in_floats = dest_stride_in_bytes / sizeof(float);

    int i;
    float *dest_cursor = vertex_normals;
    for (i = 0; i < num_vertices; i++) {
        dest_cursor[0] = 0;
        dest_cursor[1] = 0;
        dest_cursor[2] = 0;
        dest_cursor += dest_stride_in_floats;
    }

    for (i = 0; i < num_faces; i++) {
        int n0 = indices[i*3 + 0];
        int n1 = indices[i*3 + 1];
        int n2 = indices[i*3 + 2];

        // Look up the vertex coordinates, find the triangle normal.

        Vector3 *v0 = (Vector3 *)(vertices + n0 * source_stride_in_floats);
        Vector3 *v1 = (Vector3 *)(vertices + n1 * source_stride_in_floats);
        Vector3 *v2 = (Vector3 *)(vertices + n2 * source_stride_in_floats);

        Vector3 d0 = *v1 - *v0;
        Vector3 d1 = *v2 - *v0;

        
        Vector3 cross = cross_product(d0, d1);
            
        int nn0 = canonical_vertex_map[n0];
        int nn1 = canonical_vertex_map[n1];
        int nn2 = canonical_vertex_map[n2];

        Vector3 *dest0 = (Vector3 *)(vertex_normals + nn0 * dest_stride_in_floats);
        Vector3 *dest1 = (Vector3 *)(vertex_normals + nn1 * dest_stride_in_floats);
        Vector3 *dest2 = (Vector3 *)(vertex_normals + nn2 * dest_stride_in_floats);
        assert(nn0 < num_vertices);
        assert(nn1 < num_vertices);
        assert(nn2 < num_vertices);

        *dest0 = *dest0 + cross;
        *dest1 = *dest1 + cross;
        *dest2 = *dest2 + cross;
    }

    postprocess_normals(this, vertex_normals, dest_stride_in_floats, canonical_vertex_map);
}


void Triangle_Strip_Iterator::init(Triangle_Strip_Mesh_Triangle_Strip_Info *_strip, Triangle_Strip_Mesh *_mesh) {
    done = false;
    strip = _strip;
    mesh = _mesh;

    cursor = -1;
    next();
}

void Triangle_Strip_Iterator::next() {
    while (cursor < strip->num_vertices - 3) {
        cursor++;
        bool success = update_indices();
        if (success) return;
    }

    done = true;
    return;
}

bool Triangle_Strip_Iterator::update_indices() {
    assert(cursor >= 0);
    assert(cursor <= strip->num_vertices - 3);

    int *strip_data = mesh->indices + strip->start_of_strip;

    int n0, n1, n2;
    n0 = strip_data[cursor+0];
    n1 = strip_data[cursor+1];
    n2 = strip_data[cursor+2];

    if (n0 == n1) return false;
    if (n0 == n2) return false;
    if (n1 == n2) return false;

    indices[0] = n0;
    if (cursor & 1) {
        indices[1] = n2;
        indices[2] = n1;
    } else {
        indices[1] = n1;
        indices[2] = n2;
    }

    assert(indices[0] != indices[1]);
    assert(indices[0] != indices[2]);
    assert(indices[1] != indices[2]);

    return true;
}


void copy_material_info(Mesh_Material_Info *dest, Mesh_Material_Info *src) {
    // XXX If we are to become careful about allocations here, this will have to change.
    dest->name = gamelib_copy_string(src->name);
    dest->texture_index = src->texture_index;
}



void Mesh_Base::allocate_materials(int n) {
    num_materials = n;
    material_info = new Mesh_Material_Info[n];
}

