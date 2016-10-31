#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "quaternion.h"
#include "vector3.h"
#include "simple_rotation.h"

const double EPSILON = 0.00000001;
bool use_fast_normalize = true;

void Quaternion::set_from_axis_and_angle(double _x, double _y, double _z,
					 double theta) {
    double len = sqrt(_x * _x + _y * _y + _z * _z);
    _x /= len;
    _y /= len;
    _z /= len;

    theta *= 0.5;
    float st = sin(theta);
    float ct = cos(theta);

    x = _x * st;
    y = _y * st;
    z = _z * st;
    w = ct;

    normalize();
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
    double sum, len;

    sum = (double) w * (double) w + 
          (double) x * (double) x + 
          (double) y * (double) y + 
          (double) z * (double) z;

    len = sqrt(sum);

    w = w / len;
    x = x / len;
    y = y / len;
    z = z / len;
}

Quaternion slerp(const Quaternion &start, const Quaternion &end,
                 double perc) {
    Quaternion ret;

    double dot = start.x * end.x + start.y * end.y
               + start.z * end.z + start.w * end.w;
    
    if (dot < -1) dot = -1;
    if (dot > 1) dot = 1;

    double theta = acos(dot);
    double sint = sin(theta);

    if (sint == 0.0) return start;

    double fac1, fac2;
    double denom = 1.0 / sint;
  
    fac1 = sin((1 - perc) * theta) * denom;
    fac2 = sin(perc * theta) * denom;

    ret.x = start.x * fac1 + end.x * fac2;
    ret.y = start.y * fac1 + end.y * fac2;
    ret.z = start.z * fac1 + end.z * fac2;
    ret.w = start.w * fac1 + end.w * fac2;

    //    normalize_quaternion(&ret);

    return ret;
}

float lerp(float v0, float v1, double perc) {
    return v0 + perc * (v1 - v0);
}

Quaternion lerp(const Quaternion &start, const Quaternion &end,
                double perc) {
    Quaternion result;
    result.x = lerp(start.x, end.x, perc);
    result.y = lerp(start.y, end.y, perc);
    result.z = lerp(start.z, end.z, perc);
    result.w = lerp(start.w, end.w, perc);

    return result;
}

Quaternion simple_rotation(const Vector3 &a, const Vector3 &b) {
    Vector3 axis = cross_product(a, b);
    double dot = dot_product(a, b);
    
    if (dot >= 1 - EPSILON) return Quaternion(0, 0, 0, 1);

    double theta = acos(dot);

    axis = axis.normalize();
    Quaternion result;
    result.set_from_axis_and_angle(axis.x, axis.y, axis.z, theta);

    return result;
}

Quaternion Quaternion::multiply(const Quaternion &a) {
    Quaternion r;
    Vector3 t, rv, v, av;

    v = Vector3(x, y, z);
    av = Vector3(a.x, a.y, a.z);

    r.w = w * a.w - dot_product(v, av);
    rv = cross_product(v, av);

    av = av.scale(w);
    v = v.scale(a.w);

    rv = rv.add(av);
    rv = rv.add(v);

    r.x = rv.x;
    r.y = rv.y;
    r.z = rv.z;
    
    return r;
}

void Quaternion::get_axis_and_angle(double *x_ret, double *y_ret, double *z_ret, double *theta_ret) {
    double safe_w = w;
    if (safe_w > 1) safe_w = 1;
    if (safe_w < -1) safe_w = -1;

    double theta = acos(safe_w);

    assert(theta >= -7);
    assert(theta <= 7);
    double sin_theta = sin(theta);

    *theta_ret = theta * 2;

    if ((sin_theta < EPSILON) && (sin_theta > -EPSILON)) {
        *x_ret = 1;
        *y_ret = 0;
        *z_ret = 0;

        return;
    }

    *x_ret = x / sin_theta;
    *y_ret = y / sin_theta;
    *z_ret = z / sin_theta;
}

Vector3 Quaternion::rotate(const Vector3 &vec) {
    // This is super slow!  I just did it this way because
    // I could write it in like 1 minute without thinking.

    Quaternion v(vec.x, vec.y, vec.z, 0);
    Quaternion vq_ = v.multiply(conjugate());
    Quaternion qresult = multiply(vq_);

    Vector3 result(qresult.x, qresult.y, qresult.z);
    return result;
}

    
