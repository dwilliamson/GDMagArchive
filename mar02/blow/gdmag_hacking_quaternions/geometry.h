
#ifndef __GEOMETRY_H
#  define __GEOMETRY_H

struct Vector {
    float x, y, z;

    Vector(Vector const &p);                 
    Vector(float x, float y, float z);
    Vector() { };

    void set(float x, float y, float z);
    Vector scale(float mag);

    void normalize();

    Vector add(const Vector &);
    Vector subtract(const Vector &);
};

inline Vector::Vector(Vector const &p) { 
    x = p.x; 
    y = p.y; 
    z = p.z;
}

inline Vector::Vector(float i, float j, float k) { 
    x = i; 
    y = j; 
    z = k; 
}

inline void Vector::set(float nx, float ny, float nz) {
    x = nx; 
    y = ny; 
    z = nz; 
}

inline Vector Vector::subtract(const Vector &v) {
    return Vector(x - v.x, y - v.y, z - v.z);
}

inline Vector Vector::add(Vector const &v) {
    return Vector(x + v.x, y + v.y, z + v.z);
}

inline Vector Vector::scale(float dmag) {
    float nx, ny, nz;

    nx = x * dmag;
    ny = y * dmag;
    nz = z * dmag;

    return Vector(nx, ny, nz);
}

#endif
