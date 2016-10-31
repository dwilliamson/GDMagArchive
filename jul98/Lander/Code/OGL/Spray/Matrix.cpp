///////////////////////////////////////////////////////////////////////////////
//
// Matrix.cpp : implementation file
//
// Purpose:	Implementation of Matrix Math Routines
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
#include "matrix.h"

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
