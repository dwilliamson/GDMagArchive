#include "../framework.h"
#include "tangent_frames.h"
#include "../mesh.h"
#include <math.h>


Tangent_Frame_Maker::Tangent_Frame_Maker() {
    num_vertices = 0;
    tangent_frames = NULL;

    normals = NULL;
    tangents = NULL;
    binormals = NULL;
}

Tangent_Frame_Maker::~Tangent_Frame_Maker() {
    delete [] normals;
    delete [] tangents;
    delete [] binormals;
}

void Tangent_Frame_Maker::begin_tangent_frames(int _num_vertices) {
    assert(num_vertices == 0);

    num_vertices = _num_vertices;
    normals = new Vector3[num_vertices];
    tangents = new Vector3[num_vertices];

    int i;
    for (i = 0; i < num_vertices; i++) {
        normals[i].set(0, 0, 0);
        tangents[i].set(0, 0, 0);
    }
}

void Tangent_Frame_Maker::complete_tangent_frames() {
    tangent_frames = new Quaternion[num_vertices];

    int i;
    for (i = 0; i < num_vertices; i++) {
        Vector3 *tangent = &tangents[i];
        Vector3 *normal = &normals[i];

        normal->normalize();
        tangent->normalize();

        Vector3 binormal = cross_product(*normal, *tangent);
        binormal.normalize();
        binormal.scale(-1.0f);  // @Confusion: Why?

        *tangent = cross_product(binormal, *normal);

        Rotation_Matrix rm;
        rm.set_columns(*tangent, binormal, *normal);
        Quaternion q;
        rm.get_orientation(&q);
        
        tangent_frames[i] = q;
    }
}

void make_matrix_from_points(Rotation_Matrix *matrix,
                             Vector3 points[3]) {
    Vector3 v1, v2, v3;
    v1 = points[1] - (points[0]);
    v2 = points[2] - (points[0]);
    v3 = cross_product(v1, v2);

    matrix->set_columns(v1, v2, v3);
}
    
Vector3 find_nonparallel_vector(Vector3 v) {
    Vector3 result;

    result.set(-v.y, v.x, v.z);
    if (fabs(dot_product(v, result)) > 0.5f) {
        result.set(v.z, v.y, -v.x);
        if (fabs(dot_product(v, result)) > 0.5f) {
            result.set(v.x, -v.z, v.y);
            assert(fabs(dot_product(v, result) < 0.5f));
        }
    }

    result.normalize();
    return result;
}

void make_space_from_0d(Vector3 *cross, Vector3 uv[3]) {
    if (cross->length_squared() < LAME_ASS_EPSILON) {
        uv[0].set(1, 0, 0);
        uv[1].set(0, 1, 0);
        uv[2].set(0, 0, 1);
        return;
    }

    uv[0] = *cross;
    uv[0].normalize();
    uv[1] = find_nonparallel_vector(uv[0]);

    uv[2] = cross_product(uv[0], uv[1]);
    uv[2].normalize();
    uv[1] = cross_product(uv[2], uv[0]);
}

void make_space_from_1d(Vector3 *cross, Vector3 uv[3],
                        Vector3 p0, Vector3 p1) {
    Vector3 diff1 = p1 - (p0);
    
    uv[0] = p0;
    uv[1] = p1;

    Vector3 diff2 = find_nonparallel_vector(diff1);
    if (diff2.length_squared() < LAME_ASS_EPSILON) {
        make_space_from_0d(cross, uv);
        return;
    }

    Vector3 diff3 = cross_product(diff1, diff2);
    if (diff3.length_squared() < LAME_ASS_EPSILON) {
        make_space_from_0d(cross, uv);
        return;
    }

    diff3.normalize();
    diff2 = cross_product(diff3, diff1);
    if (diff2.length_squared() < LAME_ASS_EPSILON) {
        make_space_from_0d(cross, uv);
        return;
    }

    uv[0] = p0;
    uv[1] = p0 + diff1;
    uv[2] = p0 + diff2;
}

void fixup_uv_coordinates(Vector3 *cross,
                          Rotation_Matrix *uv_columns,
                          Vector3 uv[3]) {
    // If we reach this function, it means that the uv coordinates
    // in the triangle we're processing do not actually span a
    // 2-dimensional space, yet we want to make tangent frames.
    // First we see if the coordinates define a 1-dimensional space.
    // If so, we use that and the triangle's geometry-space normal 
    // (represented by the variable 'cross') to define a full 
    // tangent space.  If uv represents only a 0-dimensional space (all
    // 3 uv coordinates on the triangle are the same), we will just
    // make something up.

    Vector3 v1 = uv[1] - (uv[0]);
    Vector3 v2 = uv[2] - (uv[0]);
    Vector3 v3 = uv[2] - (uv[1]);
    
    float length_squared1 = v1.length_squared();
    float length_squared2 = v2.length_squared();
    float length_squared3 = v3.length_squared();

    // Don't change the following uv[0] etc to pass by reference!
    // Things will break (unless we make make_space_fromEtc use
    // its own temporary storage).
    if (length_squared1) {
        make_space_from_1d(cross, uv, uv[0], uv[1]);
    } else if (length_squared2) {
        make_space_from_1d(cross, uv, uv[0], uv[2]);
    } else if (length_squared3) {
        make_space_from_1d(cross, uv, uv[1], uv[2]);
    } else {
        make_space_from_0d(cross, uv);
    }

    make_matrix_from_points(uv_columns, uv);
}

void Tangent_Frame_Maker::accumulate_triangle(int indices[3], Vector3 p[3], 
                                              Vector3 uv[3]) {
    int n0 = indices[0];
    int n1 = indices[1];
    int n2 = indices[2];


    Vector3 v1, v2;
    v1 = p[1] - p[0];
    v2 = p[2] - p[1];

    Vector3 cross = cross_product(v1, v2);
    normals[n0] += cross;
    normals[n1] += cross;
    normals[n2] += cross;
            
            // We are not normalizing the cross product yet because
            // we are weighting this triangle by its area when summing
            // the normals.  We will normalize the normals in a 
            // postpass.

    float weight = cross.length();

    // Now let's compute the tangent vectors, while we're at it.

    Rotation_Matrix xyz_columns;
    Rotation_Matrix uv_columns;
    make_matrix_from_points(&xyz_columns, p);
    make_matrix_from_points(&uv_columns, uv);

    float det = uv_columns.invert();
    if (det == 0) {
        fixup_uv_coordinates(&cross, &uv_columns, uv);
        det = uv_columns.invert();
        assert(det != 0);
    }

    Rotation_Matrix uv_to_xyz;
    rotation_multiply(&xyz_columns, &uv_columns, &uv_to_xyz);

    Vector3 tangent;
    Vector3 uv_direction(1, 0, 0);
    tangent = uv_to_xyz * uv_direction;

    tangent.normalize();
    tangent.scale(weight);

    tangents[n0] += tangent;
    tangents[n1] += tangent;
    tangents[n2] += tangent;
}


void Tangent_Frame_Maker::compute_tangent_frames(Triangle_List_Mesh *mesh) {

    begin_tangent_frames(mesh->num_vertices);


    // Compute tangent vectors.
    int i;
    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *info = &mesh->triangle_list_info[i];

        int j;
        for (j = 0; j < info->num_vertices / 3; j++) {
            int n0 = mesh->indices[j*3+0 + info->start_of_list];
            int n1 = mesh->indices[j*3+1 + info->start_of_list];
            int n2 = mesh->indices[j*3+2 + info->start_of_list];

            Vector3 p[3];
            Vector3 uv[3];
            int indices[3];

            indices[0] = n0;
            indices[1] = n1;
            indices[2] = n2;

            p[0] = mesh->vertices[n0];
            p[1] = mesh->vertices[n1];
            p[2] = mesh->vertices[n2];

            Vector2 u;
            u = mesh->uvs[n0];
            uv[0] = Vector3(u.x, u.y, 0);
            u = mesh->uvs[n1];
            uv[1] = Vector3(u.x, u.y, 0);
            u = mesh->uvs[n2];
            uv[2] = Vector3(u.x, u.y, 0);

            accumulate_triangle(indices, p, uv);
        }
    }

    complete_tangent_frames();
}

