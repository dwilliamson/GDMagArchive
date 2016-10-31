struct Rotation_Matrix;  // XXX unused?
struct Quaternion;
struct Plane3;

// Vector3 is our basic class for a 3D vector.

struct Vector3 {
    float x, y, z;

    Vector3(float x, float y, float z);
    Vector3() {};

    // The 'set' functions do the same things as the
    // constructors (often constructors are not convenient
    // to use).
    void set(float x, float y, float z);
    
    // The following functions modify the vector in place:
    void rotate(const Quaternion &q);
    void rotate(Rotation_Matrix *m);
    void scale(double factor);
    void normalize();
    void safe_normalize();

    // Return the length of this vector
    float length();
    // Return the squared length of this vector (faster than
    // computing the length).
    double length_squared() const;

    // These functions don't modify the vector; rather, they
    // compute a new vector and return it.  (So they are not
    // extremely fast, but they are convenient to use.)
    Vector3 reflect(Vector3 normal);
    Vector3 reflect(Plane3 &);  // You need to include geometry_supplement.h
                                // to get the definition of this.
};

struct Vector2 {
    float x, y;

    Vector2(float x, float y);
    Vector2() {};

    void set(float x, float y);
    void normalize();
    const float length() const;
};


// 'Rotation_Matrix' is a 3x3 matrix that you can use when
// the overhead of a 4x4 matrix is undesirable.  If you have
// a Quaternion and you want to rotate many points by it,
// it is more efficient to convert that Quaternion into
// a Rotation_Matrix and then do all the transforms.
struct Rotation_Matrix {
    float coef[3][3];

    // Set the columns of the matrix to these basis vectors.
    void set_columns(Vector3 e1, Vector3 e2, Vector3 e3);

    // Set me to the matrix representing the same rotation
    // as this quaternion.
    void set(Quaternion q);

    // Make me the identity matrix.
    void identity();

    // Take the inverse; return the determinant prior to inversion.
    float invert();

    // Transpose this matrix in place.
    void transpose();

    // 'concat' makes M := OM, where 'O' is 'other', 'M' is this matrix
    void concat(Rotation_Matrix *other);
};



// The Quaternion is the primary method of talking about
// rotations in this engine.  Most of the methods here are
// self-explanatory so the commenting is minimal.
struct Quaternion {
    float x, y, z, w;

    Quaternion() {};
    Quaternion(float x, float y, float z, float w);

    void normalize();
    void set(float x, float y, float z, float w);

    // In 'set_from_axis_and_angle', the axis (x, y, z) doesn't need to
    // be pre-normalized; it can be of any non-zero length.  The angle
    // 'theta' is in radians; it is the full angle of the rotation you
    // wish to create (not the quaternion "half-angle").
    void set_from_axis_and_angle(double x, double y, double z, double theta);

    Quaternion add(const Quaternion &a);
    Quaternion subtract(const Quaternion &a);
    Quaternion conjugate();  // returns Quaternion(-x, -y, -z, w);
    Quaternion negate();
    Quaternion scale(float factor);
};


/*
  The 'Transformation_Matrix' is a 4x4 homogeneous transformation
  matrix.  It is a bit of a workhorse, as it gets used very often,
  so there are a lot of various wacky methods provided on it.  
*/
struct Transformation_Matrix {
    double coef[4][4];

    // Set all my coefficients to 0.
    void zero_matrix();

    // Set me to the 4x4 identity matrix.
    void identity();

    // Copy my coefficients from another matrix.
    void copy_from(Transformation_Matrix *other);

    // Scale every coefficient in the matrix by 'factor'.
    void scale_coefficients(double factor);

    // Copy the 3x3 coefficients in the upper-left of my
    // matrix into this Rotation_Matrix structure.
    void copy_33(Rotation_Matrix *rm);

    // Left-multiply this rotation with me.
    void rotate(const Quaternion &ori);
    // Left-multiply this scale with me.
    void scale(const Vector3 &v);
    // Left-multiply this shear with me.
    void shear(const Vector3 &v);
    // Left-multiply this translation with me.
    void translate(const Vector3 &ori);
    void translate(double x, double y, double z);

    // Set my upper-left 3x3 portion to consist of this
    // orientation (old information there is destroyed).
    void set_rotation(const Quaternion &ori);
    // Invert only the upper-left 3x3 portion of my matrix.
    void invert_33();
    // Invert the entire matrix.  This is slower than it
    // should be right now, and ought to be replaced sometime.
    void invert();

    // Extract the translation coefficients of the matrix into this Vector3.
    void get_translation(Vector3 *result);

    // Nullify the translation part of this matrix (put 0s there).
    void remove_translation(Vector3 *result);

    // Transform a Vector3 by this matrix, giving the vector a 
    // w coordinate of 1 for the purposes of transformation.
    Vector3 transform(Vector3 const &);
    // Transform a Vector3 only by the 3x3 upper corner.
    Vector3 transform_rotate_only(Vector3 const &);

    // This 'next' data element is used by the Transformer
    // below.  Applications should probably not mess with it.
    Transformation_Matrix *next;
};



/*
  The Transformer is a mechanism for maintaining a stack of transforms;
  we can push new transforms on, and pop them off.  You can think of
  this as operating like OpenGL's glRotate, glTranslate, etc, except
  that here we are making the carrier of the state explicit; so
  you can have as many parallel states as you want.
 */

// We have defined NO_TRANSFORMER here because of a
// problem Win32 has regarding being overzealous with
// simple names.  Some win32 include file defines Transformer
// to be something we don't care about in the least; that
// creates a conflict here.  Because our 'Transformer' is not
// central to most functionality in the engine, the simplest
// way to work around it was to allow a particular file to
// define NO_TRANSFORMER at the top, before including
// anything.  This is only necessary in very rare cases.
#ifndef NO_TRANSFORMER
struct Transformer {
  public:
    Transformer();
    ~Transformer();

    Transformation_Matrix *current_transform;

    // Rotate by 'rm', about the center 'about', then translate
    // by 'translation'
    void push(Rotation_Matrix *rm, Vector3 about, Vector3 translation);
    // Rotate by 'orientation' about the origin, then translate
    // by 'translation'
    void push(const Quaternion &orientation, const Vector3 &translation);
    // Like 'push', but scale by 'scale' before translating.
    void push_with_scale(Rotation_Matrix *rm, Vector3 about,
                         Vector3 translation, Vector3 scale);

    // Left-multiply whatever is on top of the stack with 'tm',
    // and push the result onto the stack.
    void push(Transformation_Matrix *tm);
    // Push the identity matrix on top of the stack.
    void push_identity();
    
    // Pop the matrix from the top of the stack and discard it.
    void pop();
    // Pop the matrix from the top of the stack and store it in 'tm'.
    void pop(Transformation_Matrix *tm);

    // Transform 'p' by the matrix on top of the stack, returning the result.
    Vector3 transform_point(Vector3 const &p);
    // Transform 'p' by the matrix on top of the stack and put the
    // result into 'result'.
    void transform_point(Vector3 const &p, Vector3 *result);
    // Transform 'p' by the upper-left 3x3 sub-matrix of the matrix
    // on top of the stack.
    Vector3 transform_point_rotate_only(Vector3 const &p);

    // Assuming that the current matrix consists only of a translation
    // and a rotation, transform us by the inverse of this matrix
    // (but don't actually compute or store the inverse anywhere).
    Vector3 backtransform_point(Vector3 const &p);
};
#endif // NO_TRANSFORMER


double distance(const Vector3 &p1, const Vector3 &p2);



inline Vector3 const operator +(Vector3 &v0, Vector3 &v1) {
    return Vector3(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z);
}

inline Vector3 const operator +(const Vector3 &v0, const Vector3 &v1) {
    return Vector3(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z);
}

inline Vector3 const operator +=(Vector3 &v0, const Vector3 &v1) {
    v0.x += v1.x;
    v0.y += v1.y;
    v0.z += v1.z;
    return v0;
}

inline Vector3 const operator *=(Vector3 &v0, const float value) {
    v0.x *= value;
    v0.y *= value;
    v0.z *= value;
    return v0;
}

inline Vector3 const operator -=(Vector3 &v0, const Vector3 &v1) {
    v0.x -= v1.x;
    v0.y -= v1.y;
    v0.z -= v1.z;
    return v0;
}

inline const Vector3 operator *(const Vector3 &v0, float factor) {
    return Vector3(v0.x*factor, v0.y*factor, v0.z*factor);
}

inline const Vector3 operator *(float factor, const Vector3 &v0) {
    return Vector3(v0.x*factor, v0.y*factor, v0.z*factor);
}

inline const Vector3 operator -(Vector3 &v0, Vector3 &v1) {
    return Vector3(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z);
}

inline const Vector3 operator -(const Vector3 &v0, const Vector3 &v1) {
    return Vector3(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z);
}


// The cross product of two 3-vectors.
inline Vector3 cross_product(const Vector3 &v1, const Vector3 &v2) {
    Vector3 n;

    n.x = (double)v1.y * (double)v2.z - (double)v1.z * (double)v2.y;
    n.y = (double)v1.z * (double)v2.x - (double)v1.x * (double)v2.z;
    n.z = (double)v1.x * (double)v2.y - (double)v1.y * (double)v2.x;

    return n;
}


// From here to the end of the file are miscellaneous inline member 
// functions that have already been commented about.
inline void Vector3::rotate(Rotation_Matrix *m) {
    double xnew, ynew, znew;
    double dx,dy,dz;

    dx = (double) x;
    dy = (double) y;
    dz = (double) z;

    xnew = x * m->coef[0][0] + y * m->coef[0][1] + z * m->coef[0][2];
    ynew = x * m->coef[1][0] + y * m->coef[1][1] + z * m->coef[1][2];
    znew = x * m->coef[2][0] + y * m->coef[2][1] + z * m->coef[2][2];

    x = xnew;
    y = ynew;
    z = znew;
}

// Find the dot product of 3-vectors v1 and v2.
inline float dot_product(Vector3 &v1, Vector3 &v2) {
    return ((double)v1.x * (double)v2.x + 
	    (double)v1.y * (double)v2.y + 
	    (double)v1.z * (double)v2.z);
}

// Find the dot product of two 4-vectors v1 and v2.
inline double dot_product(const Quaternion &v1, const Quaternion &v2){
    return (v1.x*v2.x +  v1.y*v2.y +  v1.z*v2.z + v1.w*v2.w );
}


inline Quaternion const operator *(const Quaternion &m, const Quaternion &a){
    Quaternion r;
    Vector3 t,rv,v,av;

    v.set(m.x,m.y,m.z);
    av.set(a.x,a.y,a.z);

    r.w = m.w * a.w - dot_product(v,av);

    rv = cross_product(v,av);

    av.scale(m.w);
    v.scale(a.w);

    rv = rv + av + v;

    r.x = rv.x;
    r.y = rv.y;
    r.z = rv.z;

    return r;

}

// 'transform_multiply' computes the matrix R = XY
void transform_multiply(Transformation_Matrix *x,
                        Transformation_Matrix *y,
                        Transformation_Matrix *r);
// Similarly for 'rotation_multiply'
void rotation_multiply(Rotation_Matrix *x, 
                       Rotation_Matrix *y, 
                       Rotation_Matrix *r);


// Slerp between 'start' and 'end' by parameter 't' (t=0 -> start, t=1 -> end).
Quaternion slerp(Quaternion &start, Quaternion &end, double t);

// Lerp between two 3D vectors by parameter 't' (t=0 -> start, t=1 -> end).
Vector3 lerp(const Vector3 &start, const Vector3 &end, double t);

// Find the distance between p1 and p2.
double distance(const Vector3 &p1, const Vector3 &p2);


inline const Quaternion operator *(const Quaternion &v0, float factor) {
    return Quaternion(v0.x*factor, v0.y*factor, v0.z*factor, v0.w*factor);
}

inline Quaternion const operator +(const Quaternion &v0, const Quaternion &v1) {
    return Quaternion(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z, v0.w+v1.w);
}

inline Quaternion const operator -(const Quaternion &v0, const Quaternion &v1) {
    return Quaternion(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z, v0.w-v1.w);
}

inline double Vector3::length_squared() const {
    double dx, dy, dz;

    dx = (double) x;	
    dy = (double) y;
    dz = (double) z;

    return dx * dx + dy * dy + dz * dz;
}

inline Vector3::Vector3(float i, float j, float k) { 
    x = i; 
    y = j; 
    z = k; 
}

inline void Vector3::set(float _x, float _y, float _z) {
    x = _x; 
    y = _y; 
    z = _z; 
}

inline Quaternion::Quaternion(float _i, float _j, float _k, float _w) {
    x = _i;
    y = _j;
    z = _k;
    w = _w;
}

inline void Vector3::scale(double dmag) {
    x *= dmag;
    y *= dmag;
    z *= dmag;
}

inline Vector3 const operator *(const Rotation_Matrix &m, const Vector3 &v) {
    float xnew, ynew, znew;

    xnew = v.x * m.coef[0][0] + v.y * m.coef[0][1] + v.z * m.coef[0][2];
    ynew = v.x * m.coef[1][0] + v.y * m.coef[1][1] + v.z * m.coef[1][2];
    znew = v.x * m.coef[2][0] + v.y * m.coef[2][1] + v.z * m.coef[2][2];

    return Vector3(xnew, ynew, znew);
}

inline float distance_squared(const Vector3 &v0, const Vector3 &v1) {
    float dx = v1.x - v0.x;
    float dy = v1.y - v0.y;
    float dz = v1.z - v0.z;

    return dx*dx + dy*dy + dz*dz;
}


inline Vector2::Vector2(float _x, float _y) {
	x = _x;
	y = _y;
}



inline Vector2 const operator +(const Vector2 &v0, const Vector2 &v1) {
    return Vector2(v0.x + v1.x, v0.y + v1.y);
}

inline const Vector2 operator -(const Vector2 &v0, const Vector2 &v1) {
    return Vector2(v0.x-v1.x, v0.y-v1.y);
}

inline const Vector2 operator *(const Vector2 &v0, float factor) {
    return Vector2(v0.x*factor, v0.y*factor);
}

inline void Rotation_Matrix::set_columns(Vector3 v1, Vector3 v2, Vector3 v3) {
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

inline Quaternion Quaternion::conjugate() {
    Quaternion r;
    r.x = -x;
    r.y = -y;
    r.z = -z;
    r.w = w;

    return r;
}
