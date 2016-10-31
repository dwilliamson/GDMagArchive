#ifndef INC_VEC3f_H
#define INC_VEC3f_H

#pragma warning(disable:4244)

typedef struct
{
   float x,y,z;
} vec3f;

typedef struct
{
   float x,y,z,w;
} vec4f;

extern void vec3f_add(vec3f *d, vec3f *v0, vec3f *v1);
extern void vec3f_sub(vec3f *d, vec3f *v0, vec3f *v1);
extern void vec3f_scale(vec3f *d, vec3f *v, float scale);
extern void vec3f_scaleeq(vec3f *vec, float scale);

extern void vec3f_add_scale(vec3f *dest, vec3f *v0, vec3f *v1, float scale);
extern void vec3f_addeq_scale(vec3f *dest, vec3f *src, float scale);

// weight=0 -> a, weight=1 -> b
extern void vec3f_interpolate(vec3f *dest, vec3f *a, vec3f *b, float weight);

extern void vec3f_cross(vec3f *cross, vec3f *v0, vec3f *v1);
extern float vec3f_dot(vec3f *v0, vec3f*v1);
extern float vec3f_mag2(vec3f *v);
extern float vec3f_mag(vec3f *v);
extern float vec3f_one_over_mag(vec3f *v);
extern void vec3f_norm(vec3f *dest, vec3f *src);
extern void vec3f_normeq(vec3f *v);

extern void mat4_mul(float out[4][4], float mat1[4][4], float mat2[4][4]);

#endif
