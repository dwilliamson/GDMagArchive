#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "geometry.h"

void Quaternion::normalize() {
    double sum = w * w + x * x + y * y + z * z;
    if (sum == 0.0) return;

    double scale = 1.0 / sqrt(sum);

    w *= scale;
    x *= scale;
    y *= scale;
    z *= scale;
}

void Quaternion::set_from_axis_and_angle(double _x, double _y, double _z,
					 double theta) {
    Vector vec(_x, _y, _z);
    vec.normalize();

    theta *= 0.5;
    float st = sin(theta);
    float ct = cos(theta);

    x = vec.x * st;
    y = vec.y * st;
    z = vec.z * st;
    w = ct;

    normalize();
}

Vector Vector::rotate(const Quaternion &q) {
    Rotation_Matrix rm;
    rm.set(q);
    return rotate(rm);
}

void Vector::normalize() {
    double sq = sqrt(x * x + y * y + z * z);

    if (sq == 0.0) return;

    x /= sq;
    y /= sq;
    z /= sq;
}

void Rotation_Matrix::set(Quaternion q) {
    double Nq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    double s = (Nq > 0.0) ? (2.0 / Nq) : (0.0);

    double xs = q.x * s;
    double ys = q.y * s;
    double zs = q.z * s;

    double wx = q.w * xs;
    double wy = q.w * ys;
    double wz = q.w * zs;

    double xx = q.x * xs;
    double xy = q.x * ys;
    double xz = q.x * zs;

    double yy = q.y * ys;
    double yz = q.y * zs;
    double zz = q.z * zs;

    coef[0][0] = 1.0 - (yy + zz);
    coef[0][1] = xy - wz;
    coef[0][2] = xz + wy;

    coef[1][0] = xy + wz; 
    coef[1][1] = 1.0 - (xx + zz);
    coef[1][2] = yz - wx;

    coef[2][0] = xz - wy;
    coef[2][1] = yz + wx;
    coef[2][2] = 1.0 - (xx + yy);
}

inline Transformation_Matrix *create_transformation_matrix(Transformer *tr) {
    Transformation_Matrix *tm = new Transformation_Matrix;
    return tm;
}

inline void destroy_transformation_matrix(Transformer *tr,
					  Transformation_Matrix *tm) {
    delete tm;
}


void Transformation_Matrix::blank_matrix() {
    int i, j;

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    coef[i][j] = 0;
	}
    }

    coef[0][0] = 1.0;
    coef[1][1] = 1.0;
    coef[2][2] = 1.0;
    coef[3][3] = 1.0;
}

// The code below rotates an object around the absolute x axis, then y,
// then z.  I think it is correct but this is generally not what we want;
// instead we want to rotate an object around the absolute x axis, then
// around the *relative* y axis after that rotation has occurred, then
// around the *relative* z axis after those two rotations have occurred.


void Transformation_Matrix::set_rotation_factor(double x, double y, double z) {
    double cx, sx, cy, sy, cz, sz;


    cx = cos(x);
    sx = sin(x);
    cy = cos(y);
    sy = sin(y);
    cz = cos(z);
    sz = sin(z);

    blank_matrix();
    coef[0][0] =  (cy * cz);
    coef[0][1] = -(sz * cx) + (cz * sy * sx);
    coef[0][2] =  (sz * sx) + (cx * cz * sy);
    coef[1][0] =  (sz * cy);
    coef[1][1] =  (cz * cx) + (sx * sy * sz);
    coef[1][2] = -(sx * cz) + (cx * sy * sz);
    coef[2][0] = -(sy);
    coef[2][1] =  (sx * cy);
    coef[2][2] =  (cx * cy);
}



#ifdef NOT
// Here is an updated version that hopefully does the right thing.
// It uses a helper function "rotation_about_axis_by_angle" which
// is copped from [GG1.90], p. 465 (article by Michael E. Pique).

static void rotation_about_axis_by_angle(cvector axis, angle theta,
					 Transformation_Matrix*victim) {
    double s, t, c;

    s = sin(theta);
    c = cos(theta);
    t = 1 - c;

    axis.normalize();  // If we're sure it's a unit vector coming in,
                       // we won't need this.  Right now I'm not sure.

    double x = axis.x;
    double y = axis.y;
    double z = axis.z;

    victim->coef[0][0] = t * x * x + c;
    victim->coef[0][1] = t * x * y + s * z;
    victim->coef[0][2] = t * x * z - s * y;

    victim->coef[1][0] = t * x * y - s * z;
    victim->coef[1][1] = t * y * y + c;
    victim->coef[1][2] = t * y * z + s * x;

    victim->coef[2][0] = t * x * z + s * y;
    victim->coef[2][1] = t * y * z - s * x;
    victim->coef[2][2] = t * z * z + c;
}

void Transformation_Matrix::set_rotation_factor(angle x, angle y, angle z) {
    double cx, sx, cy, sy, cz, sz;

    // XXX I don't know why I need to do the negation below... but I do.

    x = -x;
    y = -y;
    z = -z;

    Transformation_Matrixrx, ry, rz, r1;

    cvector x_axis(1, 0, 0);

    rx.blank_matrix();
    rotation_about_axis_by_angle(x_axis, x, &rx);

    cvector rel_y_axis(rx.coef[0][1], rx.coef[1][1], rx.coef[2][1]);
    ry.blank_matrix();
    rotation_about_axis_by_angle(rel_y_axis, y, &ry);

    transform_multiply(&ry, &rx, &r1);

    cvector rel_z_axis(r1.coef[0][2], r1.coef[1][2], r1.coef[2][2]);
    rz.blank_matrix();
    rotation_about_axis_by_angle(rel_z_axis, z, &rz);

    transform_multiply(&rz, &r1, this);
}

#endif // NOT


void Transformation_Matrix::set_translation_factor(double x, double y, double z) {
    double a, b, c, d;

    a = coef[0][3];
    b = coef[1][3];
    c = coef[2][3];
//    d = coef[3][3];

    coef[0][3] = x * coef[0][0] + y * coef[0][1] + z * coef[0][2] + a;
    coef[1][3] = x * coef[1][0] + y * coef[1][1] + z * coef[1][2] + b;
    coef[2][3] = x * coef[2][0] + y * coef[2][1] + z * coef[2][2] + c;
//    coef[3][3] = d;
}


void Transformation_Matrix::set_scale_factor(double f) {
    coef[0][0] *= f;
    coef[1][1] *= f;
    coef[2][2] *= f;
}


void transform_multiply(Transformation_Matrix*x, Transformation_Matrix*y, Transformation_Matrix*r) {
    int i, j, k;
    register double accum;

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    accum = 0;
	    for (k = 0; k < 4; k++) {
		accum += x->coef[i][k] * y->coef[k][j];
	    }

	    r->coef[i][j] = accum;
	}
    }
}



Transformer::Transformer() {
    current_transform = create_transformation_matrix(this);
    current_transform->blank_matrix();
    current_transform->next = NULL;
};

Transformer::~Transformer() {
    Transformation_Matrix *m;
};

void Transformer::push_identity() {
    Transformation_Matrix *t = create_transformation_matrix(this);
    t->blank_matrix();
    t->next = current_transform;
    current_transform = t;
}

void Transformer::push(Transformation_Matrix *q) {
    Transformation_Matrix *t = create_transformation_matrix(this);

    transform_multiply(current_transform, q, t);
    t->next = current_transform;
    current_transform = t;
}

void Transformer::push(Rotation_Matrix *rmatrix, 
		       Vector about, Vector translation) {
    Transformation_Matrix t1, t2, r1, tmp, result;

    t1.blank_matrix();
    t1.set_translation_factor(-about.x, -about.y, -about.z);

    r1.blank_matrix();
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
	    r1.coef[i][j] = rmatrix->coef[i][j];
	}
    }

    transform_multiply(&r1, &t1, &tmp);

    t2.blank_matrix();
    t2.set_translation_factor(about.x + translation.x,
			      about.y + translation.y,
			      about.z + translation.z);

    transform_multiply(&t2, &tmp, &result);

    push(&result);
}

void Transformer::push(const Quaternion &orientation,
		       const Vector &position) {
    Rotation_Matrix rm;
    rm.set(orientation);
    push(&rm, Vector(0, 0, 0), position);
}

void Transformer::push_with_scale(Rotation_Matrix *rmatrix, 
				  Vector about, Vector translation,
				  Vector scale) {
    Transformation_Matrix t1, t2, r1, tmp, result;

    t1.blank_matrix();
    t1.set_translation_factor(-about.x, -about.y, -about.z);

    float scale_array[3];
    scale_array[0] = scale.x;
    scale_array[1] = scale.y;
    scale_array[2] = scale.z;

    r1.blank_matrix();
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
	    r1.coef[i][j] = rmatrix->coef[i][j] * scale_array[i];
	}
    }

    transform_multiply(&r1, &t1, &tmp);

    t2.blank_matrix();
    t2.set_translation_factor(about.x + translation.x,
			      about.y + translation.y,
			      about.z + translation.z);

    transform_multiply(&t2, &tmp, &result);

    push(&result);
}

void Transformer::pop() {
    assert(current_transform->next != NULL);

    Transformation_Matrix *t = current_transform;
    current_transform = (Transformation_Matrix *)t->next;
    destroy_transformation_matrix(this, t);
}

void Transformer::pop(Transformation_Matrix *dest) {
    assert(current_transform->next != NULL);

    Transformation_Matrix *t = current_transform;
    current_transform = (Transformation_Matrix *)t->next;

    memcpy(dest, t, sizeof(Transformation_Matrix));
    dest->next = NULL;

    destroy_transformation_matrix(this, t);
}

Vector Transformer::transform_point(Vector const &p) {
    const Transformation_Matrix *const &t = current_transform;
    Vector result;

    const double *const row_0 = t->coef[0];
    const double *const row_1 = t->coef[1];
    const double *const row_2 = t->coef[2];

    result.x = p.x * row_0[0] + p.y * row_0[1] + p.z * row_0[2]
	     + row_0[3];

    result.y = p.x * row_1[0] + p.y * row_1[1] + p.z * row_1[2]
	     + row_1[3];

    result.z = p.x * row_2[0] + p.y * row_2[1] + p.z * row_2[2]
	     + row_2[3];

    return result;
}

Vector Transformer::backtransform_point(Vector const &p) {
    const Transformation_Matrix *const &t = current_transform;
    Vector mid;
    mid.x = p.x;
    mid.y = p.y;
    mid.z = p.z;

    const double *const row_0 = t->coef[0];
    const double *const row_1 = t->coef[1];
    const double *const row_2 = t->coef[2];

    mid.x -= row_0[3];
    mid.y -= row_1[3];
    mid.z -= row_2[3];

    Vector result;
    result.x = mid.x * row_0[0] + mid.y * row_1[0] + mid.z * row_2[0];
    result.y = mid.x * row_0[1] + mid.y * row_1[1] + mid.z * row_2[1];
    result.z = mid.x * row_0[2] + mid.y * row_1[2] + mid.z * row_2[2];

    return result;
}

void Transformer::transform_point(Vector const &p, Vector *result) {
    const Transformation_Matrix *const &t = current_transform;

    const double *const row_0 = t->coef[0];
    const double *const row_1 = t->coef[1];
    const double *const row_2 = t->coef[2];

    result->x = p.x * row_0[0] + p.y * row_0[1] + p.z * row_0[2]
	      + row_0[3];

    result->y = p.x * row_1[0] + p.y * row_1[1] + p.z * row_1[2]
	      + row_1[3];

    result->z = p.x * row_2[0] + p.y * row_2[1] + p.z * row_2[2]
	      + row_2[3];
}

Vector Transformer::transform_point_rotate_only(Vector const &p) {
    Transformation_Matrix *t = current_transform;
    float x, y, z;

    x = p.x * t->coef[0][0] + p.y * t->coef[0][1] + p.z * t->coef[0][2];
    y = p.x * t->coef[1][0] + p.y * t->coef[1][1] + p.z * t->coef[1][2];
    z = p.x * t->coef[2][0] + p.y * t->coef[2][1] + p.z * t->coef[2][2];

    Vector result(x, y, z);

    return result;
}

double det33(double a, double b, double c, 
	     double d, double e, double f,
	     double g, double h, double i) {

    double result = d * (c * h - b * i)
                  + e * (a * i - c * g) 
                  + f * (b * g - a * h);
    return result;
}

void Transformer::invert_in_place() {
    Transformation_Matrix *t = current_transform;

    double a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    a = t->coef[0][0];
    b = t->coef[0][1];
    c = t->coef[0][2];
    d = t->coef[0][3];
    e = t->coef[1][0];
    f = t->coef[1][1];
    g = t->coef[1][2];
    h = t->coef[1][3];
    i = t->coef[2][0];
    j = t->coef[2][1];
    k = t->coef[2][2];
    l = t->coef[2][3];
    m = t->coef[3][0];
    n = t->coef[3][1];
    o = t->coef[3][2];
    p = t->coef[3][3];

    double r[4][4];

    // Compute matrix of cofactors.

    r[0][0] = +det33(f, g, h, j, k, l, n, o, p);
    r[0][1] = -det33(e, g, h, i, k, l, m, o, p);
    r[0][2] = +det33(e, f, h, i, j, l, m, n, p);
    r[0][3] = -det33(e, f, g, i, j, k, m, n, o);
    r[1][0] = -det33(b, c, d, j, k, l, n, o, p);
    r[1][1] = +det33(a, c, d, i, k, l, m, o, p);
    r[1][2] = -det33(a, b, d, i, j, l, m, n, p);
    r[1][3] = +det33(a, b, c, i, j, k, m, n, o);
    r[2][0] = +det33(b, c, d, f, g, h, n, o, p);
    r[2][1] = -det33(a, c, d, e, g, h, m, o, p);
    r[2][2] = +det33(a, b, d, e, f, h, m, n, p);
    r[2][3] = -det33(a, b, c, e, f, g, m, n, o);
    r[3][0] = -det33(b, c, d, f, g, h, j, k, l);
    r[3][1] = +det33(a, c, d, e, g, h, i, k, l);
    r[3][2] = -det33(a, b, d, e, f, h, i, j, l);
    r[3][3] = +det33(a, b, c, e, f, g, i, j, k);

    // Compute (det A) from one of these rows.

    double det = m * r[3][0] + n * r[3][1] + o * r[3][2] + p * r[3][3];

    // Transpose r.

    int x, y;
    for (x = 0; x < 4; x++) {
        for (y = 0; y < x; y++) {
	    double tmp = r[x][y];
	    r[x][y] = r[y][x];
	    r[y][x] = tmp;
	}
    }

    // Scale by (det A), storing result into A.

    double factor = 1.0 / det;
    for (x = 0; x < 4; x++) {
        for (y = 0; y < 4; y++) {
	    t->coef[x][y] = r[x][y] * factor;
	}
    }
}


float Vector::length() {
    return sqrt(x*x + y*y + z*z);
}

double distance(const Vector &p1, const Vector &p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;

    return sqrt(dx*dx + dy*dy + dz*dz);
}
