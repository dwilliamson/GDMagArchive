#include "../framework.h"
#include <math.h>

void Matrix4::zero_matrix() {
    int i, j;

    for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			coef[i][j] = 0;
		}
    }
}

void Matrix4::identity() {
    zero_matrix();

    coef[0][0] = 1.0;
    coef[1][1] = 1.0;
    coef[2][2] = 1.0;
    coef[3][3] = 1.0;
}

void Matrix4::scale_coefficients(double factor) {
    int i, j;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 3; i++) {
			coef[j][i] *= factor;
		}
    }
}

void Matrix4::set_rotation(const Quaternion &q) {
    Matrix3 rm;
    rm.set(q);

    int i, j;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 3; i++) {
			coef[j][i] = rm.coef[j][i];
		}
    }
}

void Matrix4::invert_33() {
    double adj[3][3];
    double a = coef[0][0];
    double b = coef[0][1];
    double c = coef[0][2];
    double d = coef[1][0];
    double e = coef[1][1];
    double f = coef[1][2];
    double g = coef[2][0];
    double h = coef[2][1];
    double i = coef[2][2];

    adj[0][0] = e * i - f * h;
    adj[1][0] = -(d * i - f * g);
    adj[2][0] = d * h - e * g;
    adj[0][1] = -(b * i - c * h);
    adj[1][1] = a * i - c * g;
    adj[2][1] = -(a * h - b * g);
    adj[0][2] = b * f - c * e;
    adj[1][2] = -(a * f - c * d);
    adj[2][2] = a * e - b * d;
    
    double det3;
    
    double a11 = a; double a12 = b; double a13 = c;
    double a21 = d; double a22 = e; double a23 = f;
    double a31 = g; double a32 = h; double a33 = i;

    det3 = a * (a22*a33-a23*a32)
		+ d * (a32*a13-a12*a33)
		+ g * (a12*a23-a22*a13);


    double scale = 1.0f / det3;

    int x, y;
    for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			coef[y][x] = adj[y][x] * scale;
		}
    }
}

// @Incomplete: put a full 4x4 matrix inversion here
void Matrix4::invert() {
	// This is not a full 4x4 invert... no projection allowed.
	assert(coef[3][0] == 0);
	assert(coef[3][1] == 0);
	assert(coef[3][2] == 0);
	assert(coef[3][3] == 1);

	Vector3 t(coef[0][3], coef[1][3], coef[2][3]);
	invert_33();

	coef[0][3] = -(t.x * coef[0][0] + t.y * coef[0][1] + t.z * coef[0][2]);
	coef[1][3] = -(t.x * coef[1][0] + t.y * coef[1][1] + t.z * coef[1][2]);
	coef[2][3] = -(t.x * coef[2][0] + t.y * coef[2][1] + t.z * coef[2][2]);
}

void Matrix4::rotate(const Quaternion &q) {
    Matrix4 rot_matrix;
    rot_matrix.identity();
    rot_matrix.set_rotation(q);

    Matrix4 result;
    result.next = next;

    transform_multiply(&rot_matrix, this, &result);
    memcpy(this, &result, sizeof(Matrix4));
}

void Matrix4::translate(const Vector3 &t) {
    coef[0][3] += t.x;
    coef[1][3] += t.y;
    coef[2][3] += t.z;
}

void Matrix4::translate(double x, double y, double z) {
    coef[0][3] += x;
    coef[1][3] += y;
    coef[2][3] += z;
}

void Matrix4::scale(const Vector3 &v) {
    coef[0][0] *= v.x;
    coef[0][1] *= v.x;
    coef[0][2] *= v.x;
    coef[0][3] *= v.x;

    coef[1][0] *= v.y;
    coef[1][1] *= v.y;
    coef[1][2] *= v.y;
    coef[1][3] *= v.y;

    coef[2][0] *= v.z;
    coef[2][1] *= v.z;
    coef[2][2] *= v.z;
    coef[2][3] *= v.z;
}

void Matrix4::shear(const Vector3 &v) {
    Matrix4 result;
    result.next = next;

    Matrix4 shear_matrix;
    shear_matrix.identity();
    shear_matrix.coef[1][0] = v.x;
    shear_matrix.coef[2][0] = v.y;
    shear_matrix.coef[2][1] = v.z;

    transform_multiply(&shear_matrix, this, &result);
    memcpy(this, &result, sizeof(Matrix4));
}


void transform_multiply(Matrix4*x, Matrix4*y, Matrix4*r) {
    int i, j, k;
    double accum;

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

// Cut-and-pasted from the above.
void rotation_multiply(Matrix3 *x, Matrix3 *y,
                       Matrix3 *r) {
    int i, j, k;
    double accum;

    for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			accum = 0;
			for (k = 0; k < 3; k++) {
				accum += x->coef[i][k] * y->coef[k][j];
			}

			r->coef[i][j] = accum;
		}
    }
}

void Vector3::rotate(const Quaternion &q) {
    Matrix3 rm;
    rm.set(q);
    rotate(&rm);
}



inline Matrix4 *create_transformation_matrix(Transformer *tr) {
    Matrix4 *tm = new Matrix4;
    return tm;
}

inline void destroy_Matrix4(Transformer *tr,
										  Matrix4 *tm) {
    delete tm;
}

Transformer::Transformer() {
    current_transform = create_transformation_matrix(this);
    current_transform->identity();
    current_transform->next = NULL;
};

Transformer::~Transformer() {
    Matrix4 *m;
};

void Transformer::push_identity() {
    Matrix4 *t = create_transformation_matrix(this);
    t->identity();
    t->next = current_transform;
    current_transform = t;
}

void Transformer::push(Matrix4 *q) {
    Matrix4 *t = create_transformation_matrix(this);

    transform_multiply(current_transform, q, t);
    t->next = current_transform;
    current_transform = t;
}

void Transformer::push(Matrix3 *rmatrix, 
					   Vector3 about, Vector3 translation) {
    Matrix4 t1, t2, r1, tmp, result;

    t1.identity();
    t1.translate(-about.x, -about.y, -about.z);

    r1.identity();
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
			r1.coef[i][j] = rmatrix->coef[i][j];
		}
    }

    transform_multiply(&r1, &t1, &tmp);

    t2.identity();
    t2.translate(about.x + translation.x,
				 about.y + translation.y,
				 about.z + translation.z);

    transform_multiply(&t2, &tmp, &result);

    push(&result);
}

void Transformer::push(const Quaternion &ori, const Vector3 &pos) {
    Matrix3 rm;
    rm.set(ori);

    push(&rm, Vector3(0, 0, 0), Vector3(pos.x, pos.y, pos.z));
}

void Transformer::push_with_scale(Matrix3 *rmatrix, 
								  Vector3 about, Vector3 translation,
								  Vector3 scale) {
    Matrix4 t1, t2, r1, tmp, result;

    t1.identity();
    t1.translate(-about.x, -about.y, -about.z);

    float scale_array[3];
    scale_array[0] = scale.x;
    scale_array[1] = scale.y;
    scale_array[2] = scale.z;

    r1.identity();
    int i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
			r1.coef[i][j] = rmatrix->coef[i][j] * scale_array[i];
		}
    }

    transform_multiply(&r1, &t1, &tmp);

    t2.identity();
    t2.translate(about.x + translation.x,
				 about.y + translation.y,
				 about.z + translation.z);

    transform_multiply(&t2, &tmp, &result);

    push(&result);
}

void Transformer::pop() {
    assert(current_transform->next != NULL);

    Matrix4 *t = current_transform;
    current_transform = (Matrix4 *)t->next;
    destroy_Matrix4(this, t);
}

void Transformer::pop(Matrix4 *dest) {
    assert(current_transform->next != NULL);

    Matrix4 *t = current_transform;
    current_transform = (Matrix4 *)t->next;

    memcpy(dest, t, sizeof(Matrix4));
    dest->next = NULL;

    destroy_Matrix4(this, t);
}

void Matrix4::get_translation(Vector3 *result) {
    result->x = (coef[0])[3];
    result->y = (coef[1])[3];
    result->z = (coef[2])[3];
}

void Matrix4::remove_translation(Vector3 *result) {
    get_translation(result);
    coef[0][3] = 0;
    coef[1][3] = 0;
    coef[2][3] = 0;
}

void Matrix4::copy_from(Matrix4 *other) {
    memcpy(this, other, sizeof(Matrix4));
}

#define caseMacro(i, j, k, I, J, K) \
  case I:\
    s = sqrt((coef[I][I] - (coef[J][J] + coef[K][K])) + coef[3][3]); \
    q.i = s*0.5;\
    s = 0.5 / s;\
    q.j = (coef[I][J] + coef[J][I]) * s;\
    q.k = (coef[K][I] + coef[I][K]) * s;\
    q.w = (coef[K][J] - coef[J][K]) * s;\
    break;


void Matrix4::get_orientation(Quaternion *result) {
    double trace = coef[0][0] + coef[1][1] + coef[2][2];
    double s;
    Quaternion q;

    if (trace >= 0.0) {
        s = sqrt(trace + coef[3][3]);
		q.w = s*0.5;
		s = 0.5 / s;
		q.x = (coef[2][1] - coef[1][2]) * s;
		q.y = (coef[0][2] - coef[2][0]) * s;
		q.z = (coef[1][0] - coef[0][1]) * s;
    } else {
        int h = 0;
		if (coef[1][1] > coef[h][h]) h = 1;
		if (coef[2][2] > coef[h][h]) h = 2;

		switch (h) {
			caseMacro(x, y, z, 0, 1, 2);
			caseMacro(y, z, x, 1, 2, 0);
			caseMacro(z, x, y, 2, 0, 1);
		}
    }

    assert(coef[3][3] == 1.0);
    if (coef[3][3] != 1.0) q = q.scale(1.0 / sqrt(coef[3][3]));

    q.normalize();
    *result = q;
}

// Cut-and-pasted from Matrix4::get_orientation
void Matrix3::get_orientation(Quaternion *result) {
    double trace = coef[0][0] + coef[1][1] + coef[2][2];
    double s;
    Quaternion q;

    if (trace >= 0.0) {
        s = sqrt(trace);
		q.w = s*0.5;
		s = 0.5 / s;
		q.x = (coef[2][1] - coef[1][2]) * s;
		q.y = (coef[0][2] - coef[2][0]) * s;
		q.z = (coef[1][0] - coef[0][1]) * s;
    } else {
        int h = 0;
		if (coef[1][1] > coef[h][h]) h = 1;
		if (coef[2][2] > coef[h][h]) h = 2;

		switch (h) {
			caseMacro(x, y, z, 0, 1, 2);
			caseMacro(y, z, x, 1, 2, 0);
			caseMacro(z, x, y, 2, 0, 1);
		}
    }

    q.normalize();
    *result = q;
}

Vector3 Matrix4::transform(Vector3 const &p) {
    Vector3 result;

    const double *const row_0 = coef[0];
    const double *const row_1 = coef[1];
    const double *const row_2 = coef[2];

    result.x = p.x * row_0[0] + p.y * row_0[1] + p.z * row_0[2]
		+ row_0[3];

    result.y = p.x * row_1[0] + p.y * row_1[1] + p.z * row_1[2]
		+ row_1[3];

    result.z = p.x * row_2[0] + p.y * row_2[1] + p.z * row_2[2]
		+ row_2[3];

    return result;
}

Vector3 Matrix4::transform_rotate_only(Vector3 const &p) {
    Vector3 result;

    const double *const row_0 = coef[0];
    const double *const row_1 = coef[1];
    const double *const row_2 = coef[2];

    result.x = p.x * row_0[0] + p.y * row_0[1] + p.z * row_0[2];
    result.y = p.x * row_1[0] + p.y * row_1[1] + p.z * row_1[2];
    result.z = p.x * row_2[0] + p.y * row_2[1] + p.z * row_2[2];

    return result;
}

Vector3 Transformer::transform_point(Vector3 const &p) {
    return current_transform->transform(p);
}

Vector3 Transformer::backtransform_point(Vector3 const &p) {
    const Matrix4 *const &t = current_transform;
    Vector3 mid;
    mid.x = p.x;
    mid.y = p.y;
    mid.z = p.z;

    const double *const row_0 = t->coef[0];
    const double *const row_1 = t->coef[1];
    const double *const row_2 = t->coef[2];

    mid.x -= row_0[3];
    mid.y -= row_1[3];
    mid.z -= row_2[3];

    Vector3 result;
    result.x = mid.x * row_0[0] + mid.y * row_1[0] + mid.z * row_2[0];
    result.y = mid.x * row_0[1] + mid.y * row_1[1] + mid.z * row_2[1];
    result.z = mid.x * row_0[2] + mid.y * row_1[2] + mid.z * row_2[2];

    return result;
}

void Transformer::transform_point(Vector3 const &p, Vector3 *result) {
    const Matrix4 *const &t = current_transform;

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

Vector3 Transformer::transform_point_rotate_only(Vector3 const &p) {
    Matrix4 *t = current_transform;
    float x, y, z;

    x = p.x * t->coef[0][0] + p.y * t->coef[0][1] + p.z * t->coef[0][2];
    y = p.x * t->coef[1][0] + p.y * t->coef[1][1] + p.z * t->coef[1][2];
    z = p.x * t->coef[2][0] + p.y * t->coef[2][1] + p.z * t->coef[2][2];

    Vector3 result(x, y, z);

    return result;
}


void Matrix3::set(Quaternion q) {
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

void Matrix3::concat(Matrix3 *other) {
    int i, j, k;
    double accum;

    double new_coef[3][3];

    for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			accum = 0;
			for (k = 0; k < 3; k++) {
				accum += other->coef[i][k] * coef[k][j];
			}

			new_coef[i][j] = accum;
		}
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
			coef[i][j] = new_coef[i][j];
		}
    }
}

void Matrix3::identity() {
    coef[0][0] = 1.0;
    coef[0][1] = 0.0;
    coef[0][2] = 0.0;

    coef[1][0] = 0.0;
    coef[1][1] = 1.0;
    coef[1][2] = 0.0;

    coef[2][0] = 0.0;
    coef[2][1] = 0.0;
    coef[2][2] = 1.0;
}

Vector3 lerp(const Vector3 &p0, const Vector3 &p1, double fraction) {
    Vector3 result;
    result.x = p0.x + fraction * (p1.x - p0.x);
    result.y = p0.y + fraction * (p1.y - p0.y);
    result.z = p0.z + fraction * (p1.z - p0.z);

    return result;
}

inline Quaternion lerp(const Quaternion &p0, const Quaternion &p1, double fraction) {
    Quaternion result;
    result.x = p0.x + fraction * (p1.x - p0.x);
    result.y = p0.y + fraction * (p1.y - p0.y);
    result.z = p0.z + fraction * (p1.z - p0.z);
    result.w = p0.w + fraction * (p1.w - p0.w);

    return result;
}

Quaternion slerp(Quaternion &start, Quaternion &end, double t) {
    // Input quaternions should be unit length or else
    // something broken will happen.

    // The technique below will work for input vectors of any
    // number of dimensions (you could write a templatized version
    // of the code, or one that takes a generic N-vector data type,
    // and it would just work).


    // Compute the cosine of the angle between the two vectors.
    double dot = dot_product(start, end);

    const double DOT_THRESHOLD = 0.9995;
    if (dot > DOT_THRESHOLD) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        Quaternion result = lerp(start, end, t);
        result.normalize();
        return result;
    }

    Clamp(dot, -1, 1);
    double theta_0 = acos(dot);  // Angle between input vectors
    double theta = theta_0 * t;  // Angle between 'start' and result

    Quaternion e1 = start;
    Quaternion e2 = end - start * dot;
    e2.normalize();              // { e1, e2 } is now an orthonormal basis

    return (e1 * cos(theta)) + (e2 * sin(theta));
}



inline double safe_acos(double x) {
    Clamp(x, -1, 1);
    return acos(x);
}

inline double safe_asin(double x) {
    Clamp(x, -1, 1);
    return asin(x);
}


void Quaternion::set_from_axis_and_angle(double _x, double _y, double _z,
										 double theta) {
    Vector3 vec(_x, _y, _z);
    vec.normalize();

    theta *= 0.5;
    double st = sin(theta);
    double ct = cos(theta);

    x = vec.x * st;
    y = vec.y * st;
    z = vec.z * st;
    w = ct;
}


Quaternion Quaternion::scale(float factor) {
    Quaternion result;
    result.x = x * factor;
    result.y = y * factor;
    result.z = z * factor;
    result.w = w * factor;

    return result;
}

void Quaternion::normalize() {
    double sum;

    sum = (double) w * (double) w + 
		(double) x * (double) x + 
		(double) y * (double) y + 
		(double) z * (double) z;

	assert(sum > 0);
	if (sum == 0) return;
	
    double ilen = 1.0 / sqrt(sum);

    w = w * ilen;
    x = x * ilen;
    y = y * ilen;
    z = z * ilen;
}

void Vector3::normalize() {
    double dx, dy, dz;

    dx = (double) x;	
    dy = (double) y;
    dz = (double) z;

    double sq = sqrt(dx * dx + dy * dy + dz * dz);
	assert(sq > 0);
    if (sq == 0) return;

	double factor = 1.0 / sq;
    x = dx * factor;
    y = dy * factor;
    z = dz * factor;
}

void Vector2::normalize() {
    double dx, dy;

    dx = (double) x;	
    dy = (double) y;

    double sq = sqrt(dx * dx + dy * dy);
	assert(sq > 0);
    if (sq == 0) return;

	double factor = 1.0 / sq;
    x = dx * factor;
    y = dy * factor;
}

void Vector3::safe_normalize() {
    double dx, dy, dz;

    dx = (double) x;	
    dy = (double) y;
    dz = (double) z;

    double sq = sqrt(dx * dx + dy * dy + dz * dz);
    if (sq == 0) return;  // @Numerical: Want some kind of epsilon here.

	double factor = 1.0 / sq;
    x = dx * factor;
    y = dy * factor;
    z = dz * factor;
}

float Vector3::length() {
    double dx,dy,dz;

    dx = (double) x;	
    dy = (double) y;
    dz = (double) z;

    float sq = (float)sqrt(dx * dx + dy * dy + dz * dz);
    return sq;
}

void Plane3::normal_set(Vector3 *p1, Vector3 *p2, Vector3 *p3){
    double v1_x,v1_y,v1_z;
    double v2_x,v2_y,v2_z;
    double nx,ny,nz;
    double len;

    v1_x = (double) p3->x - p2->x;
    v1_y = (double) p3->y - p2->y;
    v1_z = (double) p3->z - p2->z;

    v2_x = (double) p1->x - p2->x;
    v2_y = (double) p1->y - p2->y;
    v2_z = (double) p1->z - p2->z;

    nx = (v1_y * v2_z - v1_z * v2_y);
    ny = (v1_z * v2_x - v1_x * v2_z);
    nz = (v1_x * v2_y - v1_y * v2_x);

    len = sqrt((nx*nx) + (ny*ny) + (nz*nz));
  
    nx = nx/len;
    ny = ny/len;
    nz = nz/len;

    a = nx;
    b = ny;
    c = nz;
    d = -(a*p1->x + b*p1->y + c*p1->z);
}


void Plane3::normalize() {
    double da,db,dc,dd;

    da = (double) a;
    db = (double) b;
    dc = (double) c;
    dd = (double) d;

    double len = sqrt(da * da + db * db + dc * dc);
    double ilen = 1.0 / len;

    a = da * ilen;
    b = db * ilen;
    c = dc * ilen;
    d = dd * ilen;
}

double distance(const Vector3 &p1, const Vector3 &p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

float arctan2(float y, float x) {
    if (x == 0.0) {
        return (y < 0) ? (-M_PI/2) : (M_PI/2);
    }

    return (float)atan2(y, x);
}



void Matrix4::copy_33(Matrix3 *rm) {
    int j, i;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 3; i++) {
			coef[i][j] = rm->coef[i][j];
		}
    }
}

void orthonormalize(Vector3 *v, Vector3 *w) {
    v->normalize();
    double dot = dot_product(*v, *w);

    Vector3 proj = *v;
    proj.scale(dot);
    *w -= proj;

    w->normalize();
}



float Matrix3::invert() {
    double a = coef[0][0];
    double b = coef[0][1];
    double c = coef[0][2];
    double d = coef[1][0];
    double e = coef[1][1];
    double f = coef[1][2];
    double g = coef[2][0];
    double h = coef[2][1];
    double i = coef[2][2];

    double det3;
    det3 = a * (e*i - f*h) + d * (h*c - b*i) + g * (b*f - c*e);
    
    if (det3 == 0) return 0;
    assert(det3 != 0.0);

    double factor = 1.0 / det3;

    coef[0][0] =  (e * i - f * h) * factor;
    coef[1][0] = -(d * i - f * g) * factor;
    coef[2][0] =  (d * h - e * g) * factor;
    coef[0][1] = -(b * i - c * h) * factor;
    coef[1][1] =  (a * i - c * g) * factor;
    coef[2][1] = -(a * h - b * g) * factor;
    coef[0][2] =  (b * f - c * e) * factor;
    coef[1][2] = -(a * f - c * d) * factor;
    coef[2][2] =  (a * e - b * d) * factor;

    return det3;
}

void Matrix3::transpose() {
    double b = coef[0][1];
    double c = coef[0][2];
    double d = coef[1][0];
    double f = coef[1][2];
    double g = coef[2][0];
    double h = coef[2][1];

    coef[1][0] = b;
    coef[2][0] = c;
    coef[0][1] = d;
    coef[2][1] = f;
    coef[0][2] = g;
    coef[1][2] = h;
}
