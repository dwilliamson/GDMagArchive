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

#define MAX_STRINGLENGTH	255

typedef struct
{
	long v[3],n[3],t[3];
} t_faceIndex;

#include "Skeleton.h"

BOOL LoadOBJ(char *filename,t_Visual *visual);

#endif // !defined(LoadOBJ_H__INCLUDED_)
