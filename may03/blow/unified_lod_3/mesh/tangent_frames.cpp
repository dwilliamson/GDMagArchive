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

const double EPSILON = 0.00000001;
Quaternion simple_rotation(const Vector3 &a, const Vector3 &b) {
    Vector3 axis = cross_product(a, b);
    double dot = dot_product((Vector3 &)a, (Vector3 &)b);
    
    if (dot >= 1 - EPSILON) return Quaternion(0, 0, 0, 1);

    double theta = acos(dot);

    axis.safe_normalize();
    Quaternion result;
    result.set_from_axis_and_angle(axis.x, axis.y, axis.z, theta);

    return result;
}

/*
  I used to need this function but I don't any more!!!
  However, I like this function a lot os I am leaving it
  in the source code!

float get_angle_between_vectors(Vector3 v1, Vector3 v2, Vector3 normal) {
    // We assume v1 and v2 are unit length.

    float dot = dot_product(v1, v2);
    Vector3 cross = cross_product(v1, v2);
    float perp_dot = dot_product(cross, normal);

    // This seems a lot like it wants to be Geometric Algebra.
    // 'perp_dot' is the signed magnitude of the bivector (v1 ^ v2).

    Clamp(dot, -1, 1);
    Clamp(perp_dot, -1, 1);

    return atan2(perp_dot, dot);
}
*/

void Tangent_Frame_Maker::complete_tangent_frames() {
    tangent_frames = new Quaternion[num_vertices];

    int i;
    for (i = 0; i < num_vertices; i++) {
        Vector3 tangent = tangents[i];
        Vector3 normal = normals[i];

        normal.safe_normalize();

        // Orthonormalize 'tangent' against 'normal'.
        tangent -= normal * dot_product(normal, tangent);

        float len = tangent.length_squared();
        const float LENGTH2_EPSILON = 0.00001;
        if (len < LENGTH2_EPSILON) {
            // We have something degenerate here... let's bail.
            tangent_frames[i] = Quaternion(0, 0, 0, 1);
            continue;
        }

        tangent.normalize();

        // Now we have a valid normal and tangent; we can go about
        // building a tangent frame.  (The interesting part is below).

        Vector3 x_axis(1, 0, 0);
        Vector3 z_axis(0, 0, 1);
        Quaternion reach = simple_rotation(z_axis, normal);

        Vector3 x_after_reach = x_axis;
        x_after_reach.rotate(reach);

        Quaternion twist = simple_rotation(x_after_reach, tangent);

        tangent_frames[i] = twist * reach;
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

Vector3 pull_a_tangent_out_of_our_ass(Vector3 normal) {
    Vector3 x(1, 0, 0);
    Vector3 y(0, 1, 0);
    Vector3 z(0, 0, 1);

    Vector3 best_axis = x;
    float best_dot = dot_product(x, normal);

    float dot_y = dot_product(y, normal);
    float dot_z = dot_product(z, normal);

    if (dot_y > best_dot) {
        best_dot = dot_y;
        best_axis = y;
    }

    if (dot_z > best_dot) {
        best_dot = dot_z;
        best_axis = z;
    }

    Vector3 tangent = best_axis;

    // Orthonormalize 'tangent' against 'normal'.
    tangent -= best_dot * normal;
    tangent.normalize();

    return tangent;
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
    cross.safe_normalize();  // For now we don't weight by face area
    normals[n0] += cross;
    normals[n1] += cross;
    normals[n2] += cross;

            // We are not normalizing the cross product yet because
            // we are weighting this triangle by its area when summing
            // the normals.  We will normalize the normals in a 
            // postpass.

    // Now let's compute the tangent vectors, while we're at it.

    Rotation_Matrix xyz_columns;
    Rotation_Matrix uv_columns;
    make_matrix_from_points(&xyz_columns, p);
    make_matrix_from_points(&uv_columns, uv);

    Vector3 tangent;

    float det = uv_columns.invert();
    if (det == 0) {
        tangent = pull_a_tangent_out_of_our_ass(cross);
    } else {
        Rotation_Matrix uv_to_xyz;
        rotation_multiply(&xyz_columns, &uv_columns, &uv_to_xyz);

        Vector3 uv_direction(1, 0, 0);
        tangent = uv_to_xyz * uv_direction;

        tangent.normalize();
    }

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

