// $Header$
//
// VECTOR and MATRIX routines

#include <math.h>
#include "vec3f.h"

void vec3f_add(vec3f *d, vec3f *v0, vec3f *v1)
{
   d->x = v0->x + v1->x;
   d->y = v0->y + v1->y;
   d->z = v0->z + v1->z;
}

void vec3f_sub(vec3f *d, vec3f *v0, vec3f *v1)
{
   d->x = v0->x - v1->x;
   d->y = v0->y - v1->y;
   d->z = v0->z - v1->z;
}

void vec3f_scale(vec3f *d, vec3f *v, float scale)
{
   d->x = v->x * scale;
   d->y = v->y * scale;
   d->z = v->z * scale;
}

void vec3f_scaleeq(vec3f *vec, float scale)
{
   vec->x *= scale;
   vec->y *= scale;
   vec->z *= scale;
}

void vec3f_add_scale(vec3f *dest, vec3f *v0, vec3f *v1, float scale)
{
   dest->x = v0->x + v1->x * scale;
   dest->y = v0->y + v1->y * scale;
   dest->z = v0->z + v1->z * scale;
}

void vec3f_addeq_scale(vec3f *dest, vec3f *src, float scale)
{
   dest->x += src->x * scale;
   dest->y += src->y * scale;
   dest->z += src->z * scale;
}

// weight=0 -> a, weight=1 -> b
void vec3f_interpolate(vec3f *dest, vec3f *a, vec3f *b, float weight)
{
   vec3f delta;

   vec3f_sub(&delta, b, a);
   vec3f_add_scale(dest, a, &delta, weight);
}

void vec3f_cross(vec3f *cross, vec3f *v0, vec3f *v1)
{
   cross->x = v0->y * v1->z - v0->z * v1->y;
   cross->y = v0->z * v1->x - v0->x * v1->z;
   cross->z = v0->x * v1->y - v0->y * v1->x; // right hand rule: i x j = k
}

float vec3f_dot(vec3f *v0, vec3f*v1)
{
   return v0->x * v1->x + v0->y*v1->y + v0->z*v1->z;
}

float vec3f_mag2(vec3f *v)
{
   return v->x*v->x + v->y*v->y + v->z*v->z;
}

#pragma warning(disable: 4244)
float vec3f_mag(vec3f *v)
{
   return sqrt(vec3f_mag2(v));
}

float vec3f_one_over_mag(vec3f *v)
{
   return 1/sqrt(vec3f_mag2(v));
}

void vec3f_norm(vec3f *dest, vec3f *src)
{
   vec3f_scale(dest, src, vec3f_one_over_mag(src));
}

void vec3f_normeq(vec3f *v)
{
   vec3f_scaleeq(v, vec3f_one_over_mag(v));
}

void mat4_mul(float out[4][4], float mat1[4][4], float mat2[4][4])
{
   int i,j;

   for (j=0; j < 4; ++j)
      for (i=0; i < 4; ++i)
         out[j][i] = mat1[0][i]*mat2[j][0]
                   + mat1[1][i]*mat2[j][1]
                   + mat1[2][i]*mat2[j][2]
                   + mat1[3][i]*mat2[j][3];
}
