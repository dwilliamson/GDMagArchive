/*
  This file contains the necessary code to do a fast float-to-int
  conversion, where the input is an IEEE-754 32-bit floating-point
  number.  Just call FastInt(f).  The idea here is that C and C++
  have to do some extremely slow things in order to ensure proper
  rounding semantics; if you don't care about your integer result
  precisely conforming to the language spec, then you can use this.
  FastInt(f) is many many many times faster than (int)(f) in almost
  all cases.
*/

#ifndef _FASTINT
#define _FASTINT

typedef unsigned long u32;
const int   MANTISSA_BITS_32 = 23;
const u32   MANTISSA_MASK_32 = (1 << MANTISSA_BITS_32) - 1;
const u32   EXPONENT_BITS_32 = 8;
const u32   EXPONENT_MASK_32 = 0x7f800000;
const int   EXPONENT_BIAS_32 = 127;
const int   SIGN_SHIFT_32    = 31;
const u32   SIGN_MASK_32     = 0x80000000;
const float MAGIC_NUMBER_32 = (float)(1 << MANTISSA_BITS_32);
const u32   DOPED_MAGIC_NUMBER_32 = 0x4b3fffff;

inline u32 U32Ref(float f) {
    return *(u32 *)&f;
}

inline int FastInt(float f) {
    f += (float &)DOPED_MAGIC_NUMBER_32;
    int result = U32Ref(f) - DOPED_MAGIC_NUMBER_32;
    return result;
}

#endif // _FASTINT
