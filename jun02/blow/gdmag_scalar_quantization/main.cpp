#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

struct Quantizer_TL {   // Truncate, then left-reconstruct
    Quantizer_TL(float input_min, float input_max, int resolution);

    int encode(float input);
    float decode(int encoded_value);

    float input_min, input_max;
    int resolution;
    float interval_size;
    float input_scale;
};

struct Quantizer_TC {   // Truncate, then center-reconstruct
    Quantizer_TC(float input_min, float input_max, int resolution);

    int encode(float input);
    float decode(int encoded_value);

    float input_min, input_max;
    int resolution;
    float interval_size;
    float input_scale;
};

struct Quantizer_RL {   // Round, then left-reconstruct
    Quantizer_RL(float input_min, float input_max, int resolution);

    int encode(float input);
    float decode(int encoded_value);

    float input_min, input_max;
    int resolution;
    float interval_size;
    float input_scale;
};


//
// Here's the code that implements the member functions of
// all the Quantizers.
//

float normalize_input(float input, float input_min, float input_max, float input_scale) {
    assert(input >= input_min);
    assert(input <= input_max);

    float normalized = (input - input_min) * input_scale;
    if (normalized < 0) normalized = 0;

    return normalized;
}


Quantizer_TL::Quantizer_TL(float _input_min, float _input_max, int _resolution) {
    input_min = _input_min;
    input_max = _input_max;
    resolution = _resolution;

    input_scale = 1.0f / (input_max - input_min);
    interval_size = 1.0f / (float)resolution;
}


int Quantizer_TL::encode(float input) {
    float normalized = normalize_input(input, input_min, input_max, input_scale);
    int result = (int)(normalized * resolution);
    if (result > resolution - 1) result = resolution - 1;

    return result;
}

float Quantizer_TL::decode(int encoded) {
    float normalized = encoded * interval_size;
    return normalized * (input_max - input_min) + input_min;
}



Quantizer_TC::Quantizer_TC(float _input_min, float _input_max, int _resolution) {
    input_min = _input_min;
    input_max = _input_max;
    resolution = _resolution;

    input_scale = 1.0f / (input_max - input_min);

    assert(resolution > 1);
    interval_size = 1.0f / (float)resolution;
}

int Quantizer_TC::encode(float input) {
    float normalized = normalize_input(input, input_min, input_max, input_scale);
    int result = (int)(normalized * resolution);
    if (result > resolution - 1) result = resolution - 1;

    return result;
}

float Quantizer_TC::decode(int encoded) {
    float normalized = (encoded + 0.5f) * interval_size;
    return normalized * (input_max - input_min) + input_min;
}


Quantizer_RL::Quantizer_RL(float _input_min, float _input_max, int _resolution) {
    input_min = _input_min;
    input_max = _input_max;
    resolution = _resolution;

    input_scale = 1.0f / (input_max - input_min);
    interval_size = 1.0f / (float)(resolution - 1);
}

int Quantizer_RL::encode(float input) {
    float normalized = normalize_input(input, input_min, input_max, input_scale);
    int result = (int)((normalized * (resolution - 1)) + 0.5f);
    if (result > resolution - 1) result = resolution - 1;

    return result;
}

float Quantizer_RL::decode(int encoded) {
    float normalized = encoded * interval_size;
    return normalized * (input_max - input_min) + input_min;
}


float random_in_range(float min, float max) {
	int irand = rand();
	float val = irand / (float)RAND_MAX;

    float width = max - min;
    return val * width + min;
}

//
// Float encoder stuff.
//

typedef unsigned long u32;
typedef unsigned long ulong;

inline u32 U32Ref(float f) {
    return *(u32 *)&f;
}


const int MANTISSA_BITS_32 = 23;
const u32 EXPONENT_BITS_32 = 8;
const u32 MANTISSA_MASK_32 = 0x007fffff;
const u32 EXPONENT_MASK_32 = 0x7f800000;
const u32 SIGN_MASK_32     = 0x80000000;
const int EXPONENT_BIAS_32 = 127;
const int SIGN_SHIFT_32    = 31;

struct Float_Encoder {
    Float_Encoder(int exponent_bits, int mantissa_bits);

    int   get_output_size();

    u32   encode(float f, bool rounding);
    float decode(u32 encoded);

    int exponent_bits, mantissa_bits;
    int sign_mask, mantissa_mask, exponent_mask;
    int exponent_bias;
    int sign_shift;

    int exponent_min, exponent_max;
};

Float_Encoder::Float_Encoder(int num_exponent_bits, int num_mantissa_bits) {
    exponent_bits = num_exponent_bits;
    mantissa_bits = num_mantissa_bits;
    exponent_bias = (1 << (exponent_bits - 1)) - 1;
    sign_shift = exponent_bits + mantissa_bits;

    sign_mask = 1 << sign_shift;
    exponent_mask = ((1 << exponent_bits) - 1) << mantissa_bits;
    mantissa_mask = (1 << mantissa_bits) - 1;

    exponent_max = (1 << (exponent_bits - 1)) - 1;
    exponent_min = -exponent_max - 1;

    assert(exponent_bits <= EXPONENT_BITS_32);
    assert(mantissa_bits <= MANTISSA_BITS_32);
    assert(exponent_bits > 0);
    assert(mantissa_bits > 0);
}

int Float_Encoder::get_output_size() {
    return exponent_bits + mantissa_bits + 1;
}

u32 Float_Encoder::encode(float f, bool rounding) {
    if (f == 0.0f) return 0;     // IEEE 0 is a special case.
    u32 src = U32Ref(f);

    int mantissa_shift = (MANTISSA_BITS_32 - mantissa_bits);

    // Mask out the mantissa, exponent, and sign fields.

    u32 mantissa = (src & MANTISSA_MASK_32);
    int exponent = (src & EXPONENT_MASK_32) >> MANTISSA_BITS_32;
    u32 sign     = (src >> SIGN_SHIFT_32);

    // Subtract the IEEE-754 number's exponent bias, then add our own.

    exponent -= EXPONENT_BIAS_32;

    // Round the mantissa, and bump up the exponent if necessary.
    if (rounding) {
        int rounding_constant = 1 << (mantissa_shift - 1);
        int test_bit = 1 << MANTISSA_BITS_32;
        mantissa += rounding_constant;
        if (mantissa & test_bit) {
            mantissa = 0;
            exponent++;  // XXX exponent overflow
        }
    }

    // Shift the mantissa to the right, killing the extra precision.

    mantissa >>= mantissa_shift;

    // Deal with the exponent.

    printf("  exponent %d, min %d, max %d\n", exponent, exponent_min, exponent_max);
    if (exponent < exponent_min) {
        if (exponent < exponent_min - 1) return 0;
        exponent = exponent_min;
    }

    if (exponent > exponent_max) exponent = exponent_max;

    exponent = (exponent - exponent_min);

    // Put the pieces back together.

    u32 result = (sign << sign_shift) | (exponent << mantissa_bits)
               | (mantissa);

    return result;
}

float Float_Encoder::decode(u32 src) {
    if (src == 0) return 0.0f;

    // Mask out the mantissa, exponent, and sign fields.

    u32 mantissa = (src & mantissa_mask);
    int exponent = (src & exponent_mask) >> mantissa_bits;
    u32 sign     = (src >> sign_shift);

    // Subtract our exponent bias, then add IEEE-754's.

    exponent += exponent_min;
    exponent += EXPONENT_BIAS_32;

    // Adjust the mantissa.

    mantissa <<= (MANTISSA_BITS_32 - mantissa_bits);

    // Assemble the pieces.

    u32 result = (sign << SIGN_SHIFT_32) | (exponent << MANTISSA_BITS_32)
               | (mantissa);

    return *(float *)&result;
}


//
// Now come the test cases, and main().
//


void test_case(Float_Encoder *encoder, float f) {
    float f2;
    u32 j;

    printf("Input number is %f (%08x)\n", f, U32Ref(f));

    j = encoder->encode(f, false);
    f2 = encoder->decode(j);

    printf("     unrounded:  %x -> %f\n", j, f2);

    j = encoder->encode(f, true);
    f2 = encoder->decode(j);

    printf("     rounded:    %x -> %f\n", j, f2);
}


void main(void) {

    printf("Constant Error Quantization Test:\n\n");

    const float INPUT_MIN = -15.0f;
    const float INPUT_MAX = 35.0f;
    const int RESOLUTION = 50;

    float error_tl = 0;
    float error_tc = 0;
    float error_rl = 0;

    float error_mag_tl = 0;
    float error_mag_tc = 0;
    float error_mag_rl = 0;

    const int NUM_EVALUATIONS = 40000;

    Quantizer_TL quantizer_tl(INPUT_MIN, INPUT_MAX, RESOLUTION);
    Quantizer_TC quantizer_tc(INPUT_MIN, INPUT_MAX, RESOLUTION);
    Quantizer_RL quantizer_rl(INPUT_MIN, INPUT_MAX, RESOLUTION);

    int i;
    for (i = 0; i < NUM_EVALUATIONS; i++) {
        float input = random_in_range(INPUT_MIN, INPUT_MAX);

        int encoded_tl = quantizer_tl.encode(input);
        int encoded_tc = quantizer_tc.encode(input);
        int encoded_rl = quantizer_rl.encode(input);

        float decoded_tl = quantizer_tl.decode(encoded_tl);
        float decoded_tc = quantizer_tc.decode(encoded_tc);
        float decoded_rl = quantizer_rl.decode(encoded_rl);

        error_tl += decoded_tl - input;
        error_tc += decoded_tc - input;
        error_rl += decoded_rl - input;

        error_mag_tl += fabs(decoded_tl - input);
        error_mag_tc += fabs(decoded_tc - input);
        error_mag_rl += fabs(decoded_rl - input);
    }

    printf("%d numbers tested (in the range from %f to %f), resolution %d.\n",
           NUM_EVALUATIONS, INPUT_MIN, INPUT_MAX, RESOLUTION);

    error_mag_tl /= (float)NUM_EVALUATIONS;
    error_mag_tc /= (float)NUM_EVALUATIONS;
    error_mag_rl /= (float)NUM_EVALUATIONS;

    error_tl /= (float)NUM_EVALUATIONS;
    error_tc /= (float)NUM_EVALUATIONS;
    error_rl /= (float)NUM_EVALUATIONS;

    printf("Error for TL: magnitude %f, drift %f\n", error_mag_tl, error_tl);
    printf("Error for TC: magnitude %f, drift %f\n", error_mag_tc, error_tc);
    printf("Error for RL: magnitude %f, drift %f\n", error_mag_rl, error_rl);


    printf("\n\n\nFloating-Point Repacking Test:\n\n");

    Float_Encoder encoder(4, 13);

    test_case(&encoder, 0.0);
    test_case(&encoder, 37.2518);
    test_case(&encoder, -37.2518);
    test_case(&encoder, 220.13538);
    test_case(&encoder, 1.1753123);
    test_case(&encoder, 1.2342348);
    test_case(&encoder, -.124998);
    test_case(&encoder, -.125 + .00001);
    test_case(&encoder, -0.0028);
}
