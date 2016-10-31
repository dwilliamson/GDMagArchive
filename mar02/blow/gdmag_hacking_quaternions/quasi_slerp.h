// The two entry points to this wad of inlined functions are:

// normalized_lerp: Linearly interpolate between two quaternions,
//     and normalize that.  The result will waver in speed compared
//     to a real slerp.

// quasi_slerp: Perform a normalized_lerp, but pre-warp the time
//     parameter to compensate for the distortion that linear
//     interpolation produces.


// Compute 1/sqrt(s) using a tangent line approximation.
inline float isqrt_approx_in_neighborhood(float s) {
    const float NEIGHBORHOOD = 0.959066;
    const float SCALE = 1.000311;
    const float ADDITIVE_CONSTANT = SCALE / sqrt(NEIGHBORHOOD);
    const float FACTOR = SCALE * (-0.5 / (NEIGHBORHOOD * sqrt(NEIGHBORHOOD)));

    return ADDITIVE_CONSTANT + (s - NEIGHBORHOOD) * FACTOR;
}

// Normalize a quaternion using the above approximation.
inline void fast_normalize(Quaternion *q) {
    float s = q->length_squared();
    float k = isqrt_approx_in_neighborhood(s);

    if (s <= 0.91521198) {
        k *= isqrt_approx_in_neighborhood(k * k * s);

        if (s <= 0.65211970) {
            k *= isqrt_approx_in_neighborhood(k * k * s);
        }
    }

    q->x *= k;
    q->y *= k;
    q->z *= k;
    q->w *= k;
}

// counter_warp: A helper function used by quasi_slerp.
inline float counter_warp(float t, float cos_alpha) {
    const float ATTENUATION = 0.82279687;
    const float WORST_CASE_SLOPE = 0.58549219;

    float factor = 1 - ATTENUATION * cos_alpha;
    factor *= factor;
    float k = WORST_CASE_SLOPE * factor;

    return t*(k*t*(2*t - 3) + 1 + k);
}


//                            //
// *** Entry Points Below *** //
//                            //


// normalized_lerp: An approximation to slerp that is most useful
//    when the initial quaternions are close together.
inline Quaternion normalized_lerp(const Quaternion &q1, const Quaternion &q2, 
                                  float t) {
    Quaternion result = lerp(q1, q2, t);
    fast_normalize(&result);
    return result;
}

// quasi_slerp: An approximation to slerp with reasonably small error
//    no matter what the two input quaternions are (so long as they are
//    pointing into the same half-space, as is traditional for slerp).
inline Quaternion quasi_slerp(const Quaternion &q1, const Quaternion &q2,
                              float t) {
    float cos_alpha = dot_product(q1, q2);

    float t_prime;
    if (t <= 0.5f) {
        t_prime = counter_warp(t, cos_alpha);
    } else {
        t_prime = 1 - counter_warp(1.0f - t, cos_alpha);
    }

    Quaternion result = lerp(q1, q2, t_prime);
    fast_normalize(&result);
    return result;
}
