#include <math.h>
#include "vector3.h"

double dot_product(const Vector3 &a, const Vector3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 cross_product(const Vector3 &a, const Vector3 &b) {
    Vector3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = -(a.x * b.z - a.z * b.x);
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

Vector3 Vector3::scale(float factor) {
    Vector3 result;
    result.x = x * factor;
    result.y = y * factor;
    result.z = z * factor;

    return result;
}

Vector3 Vector3::normalize() {
    double length = sqrt(x * x + y * y + z * z);
    if (length == 0) return Vector3(0, 0, 0);

    return Vector3(x / length, y / length, z / length);
}

Vector3 Vector3::add(const Vector3 &other) {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::subtract(const Vector3 &other) {
    return Vector3(x - other.x, y - other.y, z - other.z);
}
