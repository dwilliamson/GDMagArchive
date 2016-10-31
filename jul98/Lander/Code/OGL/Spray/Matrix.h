///////////////////////////////////////////////////////////////////////////////
//
// Matrix.h : Matrix Structure Header File
//
// Purpose:	Declare Basic Matrix Functions
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
// Revisions:
//		Integrated into Skinning Demo		2/18/98
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(MATRIX_H__INCLUDED_)
#define MATRIX_H__INCLUDED_

#include "Math.h"	// MATH SYSTEM HEADER

///////////////////////////////////////////////////////////////////////////////
// Function:	MultVectorByMatrix
// Purpose:		Multiplies a vector by a 4x4 Matrix in OpenGL Format
// Arguments:	Matrix, Vector in, and result Vector
// Notes:		This routing is tweaked to handle OpenGLs column-major format
//				This is one obvious place for optimization perhaps asm code
///////////////////////////////////////////////////////////////////////////////
void MultVectorByMatrix(tMatrix *mat, tVector *v,tVector *result);

#endif // !defined(MATRIX_H__INCLUDED_)

