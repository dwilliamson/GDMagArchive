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
#if !defined(LoadOBJ_H__INCLUDED_)
#define LoadOBJ_H__INCLUDED_

#define MAX_STRINGLENGTH	2048

#define FACE_TYPE_TRI		1
#define FACE_TYPE_QUAD		2
#define FACE_TYPE_NORMAL	4
#define FACE_TYPE_TEXTURE	8

int LoadOBJ(const char *filename,t_Visual *visual);

#endif // !defined(LoadOBJ_H__INCLUDED_)
