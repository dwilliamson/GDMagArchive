///////////////////////////////////////////////////////////////////////////////
//
// Bitmap.h : Header file
//
// Purpose:	Implementation of Windows BMP Loader
//
// Created:
//		JL 7/1/98		
//
// Notes:		This code was originally from the OpenGL SuperBible 
//				by Richard Wright Jr. and Michael Sweet
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include <GL/gl.h>
#include <GL/glu.h>

extern void	*LoadDIBitmap(char *filename, BITMAPINFO **info);
GLubyte * ConvertBitsToGL(BITMAPINFO *info,void *bits);
