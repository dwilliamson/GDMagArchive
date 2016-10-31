///////////////////////////////////////////////////////////////////////////////
//
// OBJFile.h : header file
//
// Purpose:	Header of OpenGL Window of OBJ Loader/Saver
//
// Created:
//		JL 9/23/98		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#if !defined(OBJFILE_H__INCLUDED_)
#define OBJFILE_H__INCLUDED_

#define MAX_STRINGLENGTH	255

#define FACE_TYPE_TRI		1
#define FACE_TYPE_QUAD		2
#define FACE_TYPE_NORMAL	4
#define FACE_TYPE_TEXTURE	8

#include "Skeleton.h"

BOOL LoadOBJ(const char *filename,t_Visual *visual);
BOOL SaveOBJ(const char *filename,t_Visual *visual);

#endif // !defined(OBJFILE_H__INCLUDED_)
