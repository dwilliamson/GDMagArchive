#ifndef MATH_H
#define MATH_H

//
// Math
//
// The math object is home to a number of "utility" functions, similar to the kind of stuff in math.h
//

class Math
{
public:

  __inline static float fastInverseSqrt(float);

// Should put the templated ones back in sometime, but I was having some
// issues with MSVC...
//  template <class T> static __inline T maxOf(T a, T b) { return (a > b) ? a : b; } 
//  template <class T> static __inline T minOf(T a, T b) { return (a < b) ? a : b; } 
  static __inline float maxOf(float a, float b) { return (a > b) ? a : b; } 
  static __inline float minOf(float a, float b) { return (a < b) ? a : b; } 

  // Handy values.
  static float Pi;

protected:

};

// Gary Tarolli's clever inverse square root technique
const float ONE_HALF = 0.5f;
const float THREE_HALVES = 1.5f;
/*
float fsqrt_inv(float f)
{
 long i;
 float x2, y;
 x2 = 0.5f*f;
 i = *(long *)&f;
 i = 0x5f3759df - (i>>1);
 y = *(float *)&i;
 // repeat this iteration for more accuracy
 y = 1.5f*y - (x2*y * y*y);
 return y;
}
*/
// same as above but in assembly
__inline __declspec(naked) float __cdecl Math::fastInverseSqrt(float f)
{
 __asm // 18 cycles
 {
  fld   dword ptr [esp + 4]
  fmul  dword ptr [ONE_HALF]
  mov   eax, [esp + 4]
  mov   ecx, 0x5f3759df
  shr   eax, 1
  sub   ecx, eax
  mov   [esp + 4], ecx
  fmul  dword ptr [esp + 4]
  fld   dword ptr [esp + 4]
  fmul  dword ptr [esp + 4]
  fld   dword ptr [THREE_HALVES]
  fmul  dword ptr [esp + 4]
  fxch  st(2)
  fmulp  st(1), st
  fsubp  st(1), st
  ret
 }
};

#endif //MATH_H
