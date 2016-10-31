struct Vector3;

struct Quaternion {
    float x, y, z, w;

    Quaternion() {};
    Quaternion(double x, double y, double z, double w);

    void normalize();
    void set(double x, double y, double z, double w);

    void set_from_axis_and_angle(double x, double y, double z, double theta);
    void get_axis_and_angle(double *x, double *y, double *z, double *theta);

    Quaternion add(const Quaternion &a);
    Quaternion subtract(const Quaternion &a);
    Quaternion multiply(const Quaternion &a);

    Quaternion conjugate();
    Quaternion scale(float factor);

    Vector3 rotate(const Vector3 &vector);
    Vector3 rotate_x_axis();
};

inline Quaternion::Quaternion(double _i, double _j, double _k, double _w) {
    x = _i;
    y = _j;
    z = _k;
    w = _w;
}

inline Quaternion Quaternion::conjugate() {
    Quaternion r;
    r.x = -x;
    r.y = -y;
    r.z = -z;
    r.w = w;

    return r;
}

inline Quaternion Quaternion::add(const Quaternion &a){
    Quaternion r;

    r.w = w + a.w;
    r.x = x + a.x;
    r.y = y + a.y;
    r.z = z + a.z;

    return r;
}

inline Quaternion Quaternion::subtract(const Quaternion &a){
    Quaternion r;

    r.w = w - a.w;
    r.x = x - a.x;
    r.y = y - a.y;
    r.z = z - a.z;

    return r;
}

inline void Quaternion::set(double _x, double _y, double _z, double _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

Quaternion slerp(const Quaternion &start, const Quaternion &end, double perc);
Quaternion lerp(const Quaternion &start, const Quaternion &end, double perc);

Quaternion simple_rotation(const Vector3 &a, const Vector3 &b);
Quaternion fast_simple_rotation(const Vector3 &a, const Vector3 &b);

inline double dot_product(const Quaternion &q1, const Quaternion &q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}
