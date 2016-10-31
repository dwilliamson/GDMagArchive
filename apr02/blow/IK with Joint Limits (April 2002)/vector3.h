struct Vector3 {
    float x, y, z;

    Vector3(float x, float y, float z);
    Vector3();

    Vector3 scale(float factor);
    Vector3 add(const Vector3 &arg);
    Vector3 subtract(const Vector3 &arg);

    Vector3 normalize();
    float length_squared();
};


inline Vector3::Vector3() {
}

inline Vector3::Vector3(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
}

inline float Vector3::length_squared() {
    return x*x + y*y + z*z;
}

double dot_product(const Vector3 &a, const Vector3 &b);
Vector3 cross_product(const Vector3 &a, const Vector3 &b);

