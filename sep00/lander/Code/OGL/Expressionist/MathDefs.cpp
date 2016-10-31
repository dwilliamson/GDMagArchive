///////////////////////////////////////////////////////////////////////////////
//
// MathDefs.cpp : implementation file
//
// Purpose:	Implementation of Math Routines
//
// Created:
//		JL  2/18/98
// Revisions:
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include "mathdefs.h"

#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

///////////////////////////////////////////////////////////////////////////////
// Function:	MultVectorByMatrix
// Purpose:		Multiplies a vector by a 4x4 Matrix in OpenGL Format
// Arguments:	Matrix, Vector in, and result Vector
// Notes:		This routing is tweaked to handle OpenGLs column-major format
//				This is one obvious place for optimization perhaps asm code
///////////////////////////////////////////////////////////////////////////////
void MultVectorByMatrix(tMatrix *mat, tVector *v,tVector *result)
{
	result->x = (mat->m[0] * v->x) +
			   (mat->m[4] * v->y) +	
			   (mat->m[8] * v->z) +
			   mat->m[12];
	result->y = (mat->m[1] * v->x) +
			   (mat->m[5] * v->y) +	
			   (mat->m[9] * v->z) +
			   mat->m[13];
	result->z = (mat->m[2] * v->x) +
			   (mat->m[6] * v->y) +	
			   (mat->m[10] * v->z) +
			   mat->m[14];
}
//// MultVectorByMatrix //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	MultVectorByRotMatrix
// Purpose:		Multiplies a vector by a 4x4 Matrix in OpenGL Format
// Arguments:	Matrix, Vector in, and result Vector
// Notes:		This routing is tweaked to handle OpenGLs column-major format
//				This is one obvious place for optimization perhaps asm code
///////////////////////////////////////////////////////////////////////////////
void MultVectorByRotMatrix(tMatrix *mat, tVector *v,tVector *result)
{
	result->x = (mat->m[0] * v->x) +
			   (mat->m[4] * v->y) +	
			   (mat->m[8] * v->z);
	result->y = (mat->m[1] * v->x) +
			   (mat->m[5] * v->y) +	
			   (mat->m[9] * v->z);
	result->z = (mat->m[2] * v->x) +
			   (mat->m[6] * v->y) +	
			   (mat->m[10] * v->z);
}
//// MultVectorByMatrix //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Two Utility functions that I pulled from the Mesa GL source
// This is a great source of information about the inner working of functions
// in OpenGL
// 
// www.mesagl.com for info
//
// Adapted to work with my data types
///////////////////////////////////////////////////////////////////////////////

// Multiply two OpenGL Matrices together
void MultMatrix(tMatrix *product, tMatrix *a, tMatrix *b)
{
   /* This matmul was contributed by Thomas Malik */
   int i;

#define A(row,col)  a->m[(col<<2)+row]
#define B(row,col)  b->m[(col<<2)+row]
#define P(row,col)  product->m[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }

#undef A
#undef B
#undef P
}

// Invert an OpenGL 4x4 matrix
BOOL InvertMatrix(float  *m, float *out )
{
/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

 float wtmp[4][8];
 float m0, m1, m2, m3, s;
 float *r0, *r1, *r2, *r3;

 r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

 r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
 r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
 r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

 r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
 r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
 r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

 r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
 r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
 r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

 r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
 r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
 r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

 /* choose pivot - or die */
 if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
 if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
 if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
 if (0.0 == r0[0])  return FALSE;

 /* eliminate first variable     */
 m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
 s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
 s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
 s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
 s = r0[4];
 if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
 s = r0[5];
 if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
 s = r0[6];
 if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
 s = r0[7];
 if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

 /* choose pivot - or die */
 if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
 if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
 if (0.0 == r1[1])  return FALSE;

 /* eliminate second variable */
 m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
 r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
 r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
 s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
 s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
 s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
 s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

 /* choose pivot - or die */
 if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
 if (0.0 == r2[2])  return FALSE;

 /* eliminate third variable */
 m3 = r3[2]/r2[2];
 r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
 r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
 r3[7] -= m3 * r2[7];

 /* last check */
 if (0.0 == r3[3]) return FALSE;

 s = 1.0/r3[3];              /* now back substitute row 3 */
 r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

 m2 = r2[3];                 /* now back substitute row 2 */
 s  = 1.0/r2[2];
 r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
 r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
 m1 = r1[3];
 r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
 r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
 m0 = r0[3];
 r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
 r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

 m1 = r1[2];                 /* now back substitute row 1 */
 s  = 1.0/r1[1];
 r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
 r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
 m0 = r0[2];
 r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
 r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

 m0 = r0[1];                 /* now back substitute row 0 */
 s  = 1.0/r0[0];
 r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
 r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

 MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
 MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
 MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
 MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
 MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
 MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
 MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
 MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7]; 

 return TRUE;

#undef MAT
#undef SWAP_ROWS
}

/* returns squared length of input vector */    
double VectorSquaredLength(tVector *v) 
{
	return((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

/* returns length of input vector */
double VectorLength(tVector *v) 
{
	return(sqrt(VectorSquaredLength(v)));
}

/* destructively normalizes the input vector */
void NormalizeVector(tVector *v) 
{
	float len = (float)VectorLength(v);
    if (len != 0.0) 
	{ 
		v->x /= len;  
		v->y /= len; 
		v->z /= len; 
	}
}

double DotProduct(tVector *v1, tVector *v2)
{
	return ((v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z));
}

/* return the cross product result = v1 cross v2 */
void CrossProduct(tVector *v1, tVector *v2, tVector *result)
{
	result->x = (v1->y * v2->z) - (v1->z * v2->y);
	result->y = (v1->z * v2->x) - (v1->x * v2->z);
	result->z = (v1->x * v2->y) - (v1->y * v2->x);
}

double VectorSquaredDistance(tVector *v1, tVector *v2) 
{
	return(	((v1->x - v2->x) * (v1->x - v2->x)) + 
			((v1->y - v2->y) * (v1->y - v2->y)) + 	
			((v1->z - v2->z) * (v1->z - v2->z)) ); 	
}

void ScaleVector(tVector *v, float scale, tVector *result) 
{
	result->x = v->x * scale;
	result->y = v->y * scale;
	result->z = v->z * scale;
}

void VectorSum(tVector *v1, tVector *v2, tVector *result) 
{
	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
	result->z = v1->z + v2->z;
}

void VectorDifference(tVector *v1, tVector *v2, tVector *result) 
{
	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
	result->z = v1->z - v2->z;
}
