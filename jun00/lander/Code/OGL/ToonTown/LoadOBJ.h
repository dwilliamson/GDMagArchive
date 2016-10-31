///////////////////////////////////////////////////////////////////////////////
//
// LoadOBJ.h : header file
//
// Purpose:	Header of OpenGL Window of OBJ Loader
//
// Created:
//		JL 9/23/98		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#if !defined(LoadOBJ_H__INCLUDED_)
#define LoadOBJ_H__INCLUDED_

#include "MathDefs.h"

#define MAX_STRINGLENGTH	2048

#define FACE_TYPE_TRI		1
#define FACE_TYPE_QUAD		2
#define FACE_TYPE_NORMAL	4
#define FACE_TYPE_TEXTURE	8

int LoadOBJ(char *filename,t_ToonVisual *visual);

#endif // !defined(LoadOBJ_H__INCLUDED_)
