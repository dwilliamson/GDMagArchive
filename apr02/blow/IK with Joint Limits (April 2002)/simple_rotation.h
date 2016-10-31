extern bool use_fast_normalize;

const float DOT_EPSILON = 0.00001;

inline float isqrt_approx_in_neighborhood2(double s) {
    const float NEIGHBORHOOD = 0.959066;
    const float SCALE = 1.000311;
    const float ADDITIVE_CONSTANT = SCALE / sqrt(NEIGHBORHOOD);
    const float FACTOR = SCALE * (-0.5 / (NEIGHBORHOOD * sqrt(NEIGHBORHOOD)));

    return ADDITIVE_CONSTANT + (s - NEIGHBORHOOD) * FACTOR;
}

// Note: we can work the 0.5f scale factors into the
// normalize functions, and we will eliminate a few
// multiplies that way... it'll get even faster.

inline float fast_normalize(float value) {
    float s = value;
    float k = isqrt_approx_in_neighborhood2(s);

    if (s <= 0.91521198) {
        k *= isqrt_approx_in_neighborhood2(k * k * s);

        if (s <= 0.65211970) {
            k *= isqrt_approx_in_neighborhood2(k * k * s);
        }
    }

    return k;
}

inline void fast_normalize(float s, Quaternion *q) {
    float scale = fast_normalize(s);
    q->x *= scale;
    q->y *= scale;
    q->z *= scale;
    q->w *= scale;
}

// NOTE:  You actually don't want to make sure that both quaternions
// are in the same half-sphere before you slerp them here because
// that hoses the math!  Interesting huh!
inline Quaternion fast_simple_rotation(const Vector3 &a, const Vector3 &b) {
    Vector3 axis = cross_product(a, b);
    double dot = dot_product(a, b);
    if (dot < -1.0f + DOT_EPSILON) return Quaternion(0, 1, 0, 0);
    
    Quaternion result(axis.x * 0.5f, axis.y * 0.5f, axis.z * 0.5f, 
                      (dot + 1.0f) * 0.5f);

    // The following expression for 's' (squared length of quaternion)
    // came as a suggestion from Bill Budge.  Thanks Bill!  He also
    // suggested another optimization that I left out in order to
    // keep things readable (basically he pointed out that you don't
    // need to scale the quaternion above by 0.5f, you can factor
    // that into the normalization routines).

    if (use_fast_normalize) {
        float s = 0.5f + 0.5f * dot;
        if (s >= 0.5f) {
            fast_normalize(s, &result);
        } else {
            // For now we punt and use a slow normalizer if the
            // result is not within the happiness range of our
            // fast normalizer from last month.  The right thing
            // to do here would be to concoct a different fast
            // normalizer that works in the zone of 0.0 - 0.5,
            // or at least works well enough to kick everything
            // above 0.5 so that we can then use our regular
            // fast normalizer on it.  This shouldn't be difficult
            // but I didn't have time to do it before this code
            // had to be turned in.

            result.normalize();
        }
    } else {
        result.normalize();
    }

    return result;
}

// fast_simple_rotation_from_x_axis is like the above, 
// but hardcoded to know 'a' is the x axis (1, 0, 0).
// It uses fast_normalize2 as a helper function.

inline void fast_normalize2(Quaternion *q) {
    double s = q->y*q->y + q->z*q->z + q->w*q->w;
    float scale = fast_normalize(s);
    q->y *= scale;
    q->z *= scale;
    q->w *= scale;
}

inline Quaternion fast_simple_rotation_from_x_axis(const Vector3 &b) {
    if (b.x < -1.0f + DOT_EPSILON) return Quaternion(0, 1, 0, 0);

    Quaternion result(0, -b.z * 0.5f, b.y * 0.5f, (b.x + 1.0f) * 0.5f);

    fast_normalize2(&result);
    fast_normalize2(&result);
    fast_normalize2(&result);
    fast_normalize2(&result);
    fast_normalize2(&result);

    return result;
}

// The following function rotates (1, 0, 0) by a quaternion.

inline Vector3 Quaternion::rotate_x_axis() {
    Vector3 result;
    result.x = w*w + x*x - y*y - z*z;
    result.y = 2*(w*z + x*y);
    result.z = 2*(-w*y + x*z);
    return result;
}

