#include "../framework.h"
#include "../mesh.h"
#include "mesh_builder.h"

#include <float.h>
#include <math.h>

Mesh_Builder::Mesh_Builder(int _max_vertices, int _max_faces) {
    max_vertices = _max_vertices;
    max_faces = _max_faces;

    num_vertices = 0;
    num_faces = 0;

    vertex_positions = new Vector3[max_vertices];
    vertex_uvs = new Vector2[max_vertices];
    tangent_frames = new Quaternion[max_vertices];
    faces = new Face[max_faces];
}

void Mesh_Builder::add_material(Mesh_Material_Info *info) {
    Mesh_Material_Info *new_info = new Mesh_Material_Info;
    copy_material_info(new_info, info);
    materials.add(new_info);
}

Mesh_Builder::~Mesh_Builder() {
    delete [] vertex_positions;
    delete [] vertex_uvs;
    delete [] faces;

    Mesh_Material_Info *info;
    Foreach(&materials, info) {
        delete info;
    } Endeach;
}

void Mesh_Builder::add_triangle(int n0, int n1, int n2, int material_index) {
    Face *face = &faces[num_faces++];
    face->n0 = n0;
    face->n1 = n1;
    face->n2 = n2;
    face->material_index = material_index;
}

int Mesh_Builder::add_vertex(Vector3 position, Vector2 uv, Quaternion tangent_frame) {
    vertex_positions[num_vertices] = position;
    vertex_uvs[num_vertices] = uv;
    tangent_frames[num_vertices] = tangent_frame;

    int result = num_vertices;
    num_vertices++;
    return result;
}

int Mesh_Builder::find_end_of_matching_materials(int cursor) {
    int material_to_match = faces[cursor].material_index;

    cursor++;
    while (cursor < num_faces) {
        if (faces[cursor].material_index != material_to_match) break;
        cursor++;
    }

    return cursor;
}

int Mesh_Builder::count_triangle_lists() {
    int cursor = 0;
    int num_lists = 0;

    while (cursor < num_faces) {
        cursor = find_end_of_matching_materials(cursor);
        num_lists++;
    }

    return num_lists;
}

int Mesh_Builder::do_one_triangle_list(int cursor, int list_index,
                                               Triangle_List_Mesh *result) {
    assert(list_index < result->num_triangle_lists);

    int cursor_end = find_end_of_matching_materials(cursor);
    int num_faces = cursor_end - cursor;

    Triangle_List_Info *info = &result->triangle_list_info[list_index];
    info->material_index = faces[cursor].material_index;
    info->num_vertices = num_faces * 3;
    info->start_of_list = cursor * 3;

    while (cursor < cursor_end) {
        Face *face = &faces[cursor];

        assert(face->n0 < num_vertices);
        assert(face->n1 < num_vertices);
        assert(face->n2 < num_vertices);

        result->indices[cursor*3 + 0] = face->n0;
        result->indices[cursor*3 + 1] = face->n1;
        result->indices[cursor*3 + 2] = face->n2;

        cursor++;
    }

    return cursor_end;
}

int compare_face_materials(const void *fp1, const void *fp2) {
    Mesh_Builder::Face *face1 = (Mesh_Builder::Face *)fp1;
    Mesh_Builder::Face *face2 = (Mesh_Builder::Face *)fp2;

    return face1->material_index - face2->material_index;
}

Triangle_List_Mesh *Mesh_Builder::build_mesh() {
    Triangle_List_Mesh *result = new Triangle_List_Mesh();
    result->allocate_geometry(num_vertices, num_faces);

    memcpy(result->vertices, vertex_positions, sizeof(Vector3) * num_vertices);
    memcpy(result->uvs, vertex_uvs, sizeof(Vector2) * num_vertices);
    memcpy(result->tangent_frames, tangent_frames, sizeof(Quaternion) * num_vertices);
    
    qsort(faces, num_faces, sizeof(Face), compare_face_materials);

    result->allocate_materials(materials.items);
    Mesh_Material_Info *src_material;
    int material_cursor = 0;
    Foreach(&materials, src_material) {
        copy_material_info(&result->material_info[material_cursor], src_material);
        material_cursor++;
    } Endeach;

    int num_triangle_lists = count_triangle_lists();
    result->num_triangle_lists = num_triangle_lists;
    result->triangle_list_info = new Triangle_List_Info[num_triangle_lists];


    int face_cursor = 0;
    int list_index = 0;
    while (face_cursor < num_faces) {
        face_cursor = do_one_triangle_list(face_cursor, list_index, result);
        list_index++;
    }

    int i;
    for (i = 0; i < result->num_vertices; i++) result->canonical_vertex_map[i] = i;

    return result;
}

