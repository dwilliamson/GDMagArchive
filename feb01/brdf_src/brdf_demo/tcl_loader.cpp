#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "geometry.h"
#include "Brdf_Map.h"
#include "triangle_model.h"
#include "tcl_loader.h"

Tcl_Loader::Tcl_Loader() {
}

Tcl_Loader::~Tcl_Loader() {
}

char *Tcl_Loader::get_command_name(char *s, char **result) {
    while (isspace(*s)) s++;
    char *t = s;
    while (isalnum(*t)) t++;
    *t = '\0';
    t++;
    *result = t;
    return s;
}

char *Tcl_Loader::get_next_line() {
    char *s = fgets(line_data, BUFSIZ, current_file);
    if (s == NULL) return NULL;
    while (isspace(*s)) s++;

    if (*s && (*s != '#')) return s;

    return get_next_line();
}

Vector get_vector_3d(char *s) {
    double x, y, z;
    int terms = sscanf(s, "%lf %lf %lf", &x, &y, &z);
    assert(terms == 3);
    return Vector(x, y, z);
}

Vector get_vector_2d(char *s) {
    double x, y;
    int terms = sscanf(s, "%lf %lf", &x, &y);
    assert(terms == 2);
    return Vector(x, y, 0);
}

bool vectors_match(Vector v1, Vector v2) {
    if (v1.x != v2.x) return false;
    if (v1.y != v2.y) return false;
    if (v1.z != v2.z) return false;
    return true;
}

void Tcl_Loader::finish_current_polygon() {
  //    assert(vertices_in_this_polygon > 2);
    vertices_in_this_polygon = 0;
    current_vector_index[0] = 0;
    current_vector_index[1] = 0;
    current_vector_index[2] = 0;
}

int Tcl_Loader::find_vector_index(Vector v) {
    int i;
    for (i = 0; i < nvertices; i++) {
        if (vectors_match(vertices[i], v)) return i;
    }

    assert(nvertices < MAX_VERTICES);
    int result = nvertices;
    vertices[nvertices++] = v;

    return result;
}

void Tcl_Loader::emit_a_triangle() {
    assert(vertices_in_this_polygon > 2);

    bool odd;
    if (vertices_in_this_polygon & 0x1) {
        odd = true;
    } else {
        odd = false;
    }

    int k0, k1, k2;
    k0 = 0;
    
    if (odd) {
        k1 = 1;
	k2 = 2;
    } else {
        k1 = 1;
	k2 = 2;
    }

    assert(nfaces < MAX_FACES);
    Face_Data *face = &faces[nfaces++];

    face->normals[k0] = current_normal[0];
    face->normals[k1] = current_normal[1];
    face->normals[k2] = current_normal[2];

    face->tangents[k0] = current_frame_vector[0];
    face->tangents[k1] = current_frame_vector[1];
    face->tangents[k2] = current_frame_vector[2];

    face->cross_tangents[0] = cross_product(face->normals[0],
					    face->tangents[0]);
    face->cross_tangents[1] = cross_product(face->normals[1],
					    face->tangents[1]);
    face->cross_tangents[2] = cross_product(face->normals[2],
					    face->tangents[2]);

    int n0 = current_vector_index[k0];
    int n1 = current_vector_index[k1];
    int n2 = current_vector_index[k2];

    face->vertex_indices[0] = n0;
    face->vertex_indices[1] = n1;
    face->vertex_indices[2] = n2;

    face->texture_vertices[k0] = current_texture_coords[0];
    face->texture_vertices[k1] = current_texture_coords[1];
    face->texture_vertices[k2] = current_texture_coords[2];

    int d = 1;
    if (odd) d = 0;

    current_normal[d] = current_normal[2];
    current_frame_vector[d] = current_frame_vector[2];
    current_vector_index[d] = current_vector_index[2];
    current_texture_coords[d] = current_texture_coords[2];
}

Triangle_Model *Tcl_Loader::load(char *filename) {
    FILE *f = fopen(filename, "rt");
    if (f == NULL) return NULL;

    current_file = f;

    nvertices = 0;
    nfaces = 0;
    vertices_in_this_polygon = 0;

    while (1) {
        char *s = get_next_line();
	if (s == NULL) break;

	char *residual;
	char *comm = get_command_name(s, &residual);
	
	int temp_index;
	if (vertices_in_this_polygon > 1) {
	    temp_index = 2;
	} else {
	    temp_index = vertices_in_this_polygon;
	}

	if (strcmp(s, "pointlight") == 0) {
	} else if (strcmp(s, "brdfmap") == 0) {
	} else if (strcmp(s, "polygon") == 0) {
	    finish_current_polygon();
	} else if (strcmp(s, "framev") == 0) {
	    current_frame_vector[temp_index] = get_vector_3d(residual);
	} else if (strcmp(s, "normal") == 0) {
	    current_normal[temp_index] = get_vector_3d(residual);
	} else if (strcmp(s, "texture") == 0) {
	    current_texture_coords[temp_index] = get_vector_2d(residual);
	} else if (strcmp(s, "vector") == 0) {
	    Vector v = get_vector_3d(residual);
	    int index = find_vector_index(v);
	    current_vector_index[temp_index] = index;
	    vertices_in_this_polygon++;

	    if (temp_index == 2) emit_a_triangle();
	} else {
	  //	    assert(0);
	}
    }

    fclose(f);
    finish_current_polygon();

    Triangle_Model *model = new Triangle_Model(nvertices, nfaces, 0);
    memcpy(model->faces, faces, nfaces * sizeof(Face_Data));
    memcpy(model->vertices, vertices, nvertices * sizeof(Vector));
    model->vertex_normals = NULL;

    int i;
    for (i = 0; i < model->nfaces; i++) {
        Face_Data *face = &faces[i];
	model->face_vertices[i * 3 + 0] = vertices[face->vertex_indices[0]];
	model->face_vertices[i * 3 + 1] = vertices[face->vertex_indices[1]];
	model->face_vertices[i * 3 + 2] = vertices[face->vertex_indices[2]];
    }

    model->position = Vector(0, 0, 0);
    model->orientation = Quaternion(0, 0, 0, 1);
    
    model->condense();

    return model;
}
