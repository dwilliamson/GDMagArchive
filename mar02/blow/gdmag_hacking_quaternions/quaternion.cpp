#include <stdlib.h>
#include <math.h>
#include "quaternion.h"

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
    z =_z * st;
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
