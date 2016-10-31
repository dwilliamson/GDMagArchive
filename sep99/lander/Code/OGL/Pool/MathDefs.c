///////////////////////////////////////////////////////////////////////////////
//
// MathDefs.c : implementation file
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

#include <math.h>
#include "mathdefs.h"

///////////////////////////////////////////////////////////////////////////////
// Function:	IdentityMatrix
// Purpose:		Creates and Identity 4x4 Matrix in OpenGL Format
// Arguments:	Matrix
// Notes:		This routing is tweaked to handle OpenGLs column-major format
//				This is one obvious place for optimization perhaps asm code
///////////////////////////////////////////////////////////////////////////////
void IdentityMatrix(tMatrix *mat) 
{
///// Local Variables /////////////////////////////////////////////////////////
	int loop;
///////////////////////////////////////////////////////////////////////////////
	for (loop = 0; loop < 16; loop++)
		mat->m[loop] = 0.0f;
	mat->m[0] = 
	mat->m[5] = 
	mat->m[10] = 
	mat->m[15] = 
		1.0f;
}
//// IdentityMatrix ///////////////////////////////////////////////////////////

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
// Purpose:		Multiplies a vector by a 4x4 Rotation Matrix in OpenGL Format
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
//// MultVectorByRotMatrix ///////////////////////////////////////////////////

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
