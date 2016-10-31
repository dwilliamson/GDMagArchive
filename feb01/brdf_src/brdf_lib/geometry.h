
#ifndef __GEOMETRY_H
#  define __GEOMETRY_H

#include <math.h>

struct Rotation_Matrix;


struct Quaternion {
    double x, y, z, w;

    Quaternion() {};
    Quaternion(double x, double y, double z, double w);

    void normalize();
    void set(double x, double y, double z, double w);
    void set_from_axis_and_angle(double x, double y, double z, double theta);

    Quaternion add(const Quaternion &a);
    Quaternion multiply(const Quaternion &a);
};

struct Vector {
    double x, y, z;

    Vector(Vector const &begin, Vector const &end);
    Vector(Vector const &p);                 
    Vector(double x, double y, double z);
    Vector() { };

    void set(Vector const &p);
    void set(double nx, double ny, double nz);
    void set(const Vector &begin, const Vector &end);

    Vector rotate(const Quaternion &q);
    Vector rotate(const Rotation_Matrix &m);
    Vector scale(double mag);

    void normalize();
    float length();
    double length_squared();

    Vector add(const Vector &);
    Vector subtract(const Vector &);
};

struct Rotation_Matrix {
    double coef[3][3];

    void set(Vector v1, Vector v2, Vector v3);
    void set(Quaternion q);
    void concat(Rotation_Matrix *);
    void identity();
};

struct Transformation_Matrix {
    double coef[4][4];

    Transformation_Matrix *next;

    void set_rotation_factor(double x, double y, double z);
    void set_translation_factor(double x, double y, double z);
    void set_scale_factor(double f);

    void blank_matrix();
};

class Transformer {
  public:
    Transformer();
    ~Transformer();

    Transformation_Matrix *current_transform;

    void push(Rotation_Matrix *rm, Vector about, Vector translation);
    void push_with_scale(Rotation_Matrix *rm, Vector about,
			 Vector translation, Vector scale);
    void push(Transformation_Matrix *);
    void push(const Quaternion &orientation, const Vector &position);

    void push_identity();
    void invert_in_place();

    void pop();
    void pop(Transformation_Matrix *);

    Vector transform_point(Vector const &p);
    void transform_point(Vector const &p, Vector *result);
    Vector transform_point_rotate_only(Vector const &p);
    Vector backtransform_point(Vector const &p);
};



double distance(const Vector &p1, const Vector &p2);

// Vector inliners:

inline Vector::Vector(Vector const &begin, Vector const &end) {
    x = end.x - begin.x;
    y = end.y - begin.y;
    z = end.z - begin.z;
}

inline Vector::Vector(Vector const &p) { 
    x = p.x; 
    y = p.y; 
    z = p.z;
}

inline Vector::Vector(double i, double j, double k) { 
    x = i; 
    y = j; 
    z = k; 
}

inline void Vector::set(Vector const &p) {
    x = p.x; 
    y = p.y; 
    z = p.z;
}

inline void Vector::set(double nx, double ny, double nz) {
    x = nx; 
    y = ny; 
    z = nz; 
}

inline void Vector::set(Vector const &begin, Vector const &end) {
    x = end.x - begin.x;
    y = end.y - begin.y;
    z = end.z - begin.z;
}

inline Vector Vector::subtract(const Vector &v) {
    return Vector(x - v.x, y - v.y, z - v.z);
}

inline Vector Vector::add(Vector const &v) {
    return Vector(x + v.x, y + v.y, z + v.z);
}

inline double Vector::length_squared() {
    double dx, dy, dz;

    dx = (double) x;	
    dy = (double) y;
    dz = (double) z;

    return dx * dx + dy * dy + dz * dz;
}

inline Vector cross_product(Vector const &v1, Vector const &v2) {
    Vector n;

    n.x = (double)v1.y * (double)v2.z - (double)v1.z * (double)v2.y;
    n.y = (double)v1.z * (double)v2.x - (double)v1.x * (double)v2.z;
    n.z = (double)v1.x * (double)v2.y - (double)v1.y * (double)v2.x;

    return n;
}

inline Vector Vector::rotate(Rotation_Matrix const &m) {
    double xnew, ynew, znew;
    double dx,dy,dz;

    dx = (double) x;
    dy = (double) y;
    dz = (double) z;

    xnew = x * m.coef[0][0] + y * m.coef[0][1] + z * m.coef[0][2];
    ynew = x * m.coef[1][0] + y * m.coef[1][1] + z * m.coef[1][2];
    znew = x * m.coef[2][0] + y * m.coef[2][1] + z * m.coef[2][2];

    return Vector(xnew, ynew, znew);
}

inline Vector Vector::scale(double dmag) {
    double nx, ny, nz;

    nx = x * dmag;
    ny = y * dmag;
    nz = z * dmag;

    return Vector(nx, ny, nz);
}

inline double dot_product(Quaternion const &a, Quaternion const &b){
    return (a.x * b.x + 
	    a.y * b.y + 
	    a.z * b.z + 
	    a.w * b.w );
}

inline double dot_product(Vector const &a, Vector const &b){
    return (a.x * b.x + 
	    a.y * b.y + 
	    a.z * b.z);
}

inline double dot_product_2d(Vector const &a, Vector const &b){
    return (a.x * b.x + a.y * b.y);
}



inline void Rotation_Matrix::set(Vector v1, Vector v2, Vector v3) {
    coef[0][0] = v1.x;
    coef[1][0] = v1.y;
    coef[2][0] = v1.z;
    coef[0][1] = v2.x;
    coef[1][1] = v2.y;
    coef[2][1] = v2.z;
    coef[0][2] = v3.x;
    coef[1][2] = v3.y;
    coef[2][2] = v3.z;
}

inline double distance_squared(const Vector &p1, const Vector &p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;

    return dx*dx + dy*dy + dz*dz;
}

inline Quaternion::Quaternion(double _i, double _j, double _k, double _w) {
    x = _i;
    y = _j;
    z = _k;
    w = _w;
}

inline Quaternion Quaternion::add(const Quaternion &a){
    Quaternion r;

    r.w = w + a.w;
    r.x = x + a.x;
    r.y = y + a.y;
    r.z = z + a.z;

    return r;
}

inline Quaternion Quaternion::multiply(const Quaternion &a){
    Quaternion r;
    Vector t,rv,v,av;

    v.set(x,y,z);
    av.set(a.x,a.y,a.z);

    r.w = w * a.w - dot_product(v, av);

    rv = cross_product(v,av);

    av = av.scale(w);
    v = v.scale(a.w);

    rv = rv.add(av);
    rv = rv.add(v);

    r.x = rv.x;
    r.y = rv.y;
    r.z = rv.z;

    return r;

}

inline void Quaternion::set(double _x, double _y, double _z, double _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

#endif // not __GEOMETRY_H



