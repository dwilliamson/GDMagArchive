#ifndef __GEOMETRY_SUPPLEMENT_H
#  define __GEOMETRY_SUPPLEMENT_H

#pragma once

/*
  'Plane3' represents a plane equation of the form
  ax + by + cz + d = 0.
*/
struct Plane3 {
    double a, b, c, d;

    Plane3(float a, float b, float c, float d);
    Plane3();

    // Set the plane from a point and normal.
    void set(Vector3 *p, Vector3 *n);

    // Compute the plane containing these three points
    // (you should make sure they are not colinear!)  The plane
    // equation resulting from this will generally have
    // (a, b, c) as a non-unit vector.  So don't use it if
    // you are then going to go find the distances of a 
    // bunch of points to this plane.  But sometimes you only
    // need to know which side of a plane something is on,
    // and this is fine for that.
    void set(Vector3 *p1, Vector3 *p2, Vector3 *p3);

    // 'normal_set' is like 'set' but it ensures that the
    // (a, b, c) vector is of unit length.  So it's slower
    // but useful in more situations.
    void normal_set(Vector3 *p1, Vector3 *p2, Vector3 *p3);

    // 'eval' evaluates the plane equation for some point p.
    double eval(const Vector3 &p);
    // 'project' projects the point p onto the plane.
    Vector3 project(const Vector3 &p);
    // 'get_normal' just returns to you the plane's normal (a, b, c)
    // as a Vector3.
    Vector3 get_normal();

    // 'normalize' ensures that (a, b, c) is of unit length, and
    // scales all the plane's coefficients accordingly.
    void normalize();
};

// Transform 'p' from Cartesian coordinates to spherical coordinates.
void cartesian_to_spherical(Vector3 p,
                            float *r_azimuth, 
                            float *r_elevation, 
                            float *r_distance);
double plane_dot(Plane3 *plane, Vector3 *pos);


// Plane3 inliners:

inline Plane3::Plane3() {
}

inline Plane3::Plane3(float i, float j, float k, float l) {
    a = i;
    b = j;
    c = k;
    d = l;
}

inline double plane_dot(Plane3 *plane, Vector3 *pos) {
    return plane->a * pos->x + plane->b * pos->y 
           + plane->c * pos->z + plane->d;
}

inline Vector3 Plane3::get_normal() {
    Vector3 r;
    
    r.x = a;
    r.y = b;
    r.z = c;

    r.normalize();

    return r;
}

inline void Plane3::set(Vector3  *p, Vector3 *n){
	a = n->x;
	b = n->y;
	c = n->z;
	d = -((double)a * (double)p->x  +  
		  (double)b * (double)p->y  +  
		  (double)c * (double)p->z);
}

inline void Plane3::set(Vector3 *p1, Vector3 *p2, Vector3 *p3){
	Vector3 w0 = *p2 - *p1;
	Vector3 w1 = *p3 - *p1;
	Vector3 crossprod = cross_product(w0, w1);

	a = crossprod.x;
	b = crossprod.y;
	c = crossprod.z;
	d = -((double)a * p1->x  +  (double)b * p1->y  +  (double)c * p1->z);
}


inline double Plane3::eval(const Vector3 &p) {
    return a * p.x + b * p.y + c * p.z + d;
}

inline Vector3 Plane3::project(const Vector3 &p) {
    double dist = eval(p);

    Vector3 result;
    result.x = p.x - a * dist;
    result.y = p.y - b * dist;
    result.z = p.z - c * dist;

    return result;
}

// The cross product of two vectors that are stored as
// arrays of type 'double'.
inline Vector3 cross_product(double *v1, double *v2) {
    Vector3 n;

    n.x = v1[1] * v2[2] - v1[2] * v2[1];
    n.y = v1[2] * v2[0] - v1[0] * v2[2];
    n.z = v1[0] * v2[1] - v1[1] * v2[0];

    return n;
}
inline Vector3 Vector3::reflect(Plane3 &p) {
    Vector3 normal(p.a, p.b, p.c);
    return reflect(normal);
}
    

inline Vector3 Vector3::reflect(Vector3 normal) {
    float dot = dot_product(*this, normal);
    if (dot < 0.0) {
        normal.scale(-2.0 * dot);
        return normal + *this;
    } else {
        return Vector3(0, 0, 0);
    }
}

#endif // __GEOMETRY_SUPPLEMENT_H


