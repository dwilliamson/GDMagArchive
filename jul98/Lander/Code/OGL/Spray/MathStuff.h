///////////////////////////////////////////////////////////////////////////////
//
// MathStuff.h : Math Structure Header File
//
// Purpose:	Declare Basic Math Structures
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

#if !defined(MATHSTUFF_H__INCLUDED_)
#define MATHSTUFF_H__INCLUDED_

#define M_PI        3.14159265358979323846
#define HALF_PI	    1.57079632679489661923

#define DEGTORAD(d)	((d * (float)M_PI) / 180.0f);
#define RADTODEG(r)	((r * 180.0f) /(float)M_PI);

typedef struct
{
	float x,y,z;
} tVector;

// NOT DECLARED AS float[4][4] BECAUSE OPENGL ACCESSES THIS STRANGLY
typedef struct
{
	float m[16];
} tMatrix;

// SOME STRUCTURES TO HELP ME ACCESS VERTEX DATA IN AN ARRAY
typedef struct
{
	float r,g,b;
	float x,y,z;
} tColoredVertex;

typedef struct
{
	float u,v;
	float x,y,z;
} tTexturedVertex;

typedef struct
{
	float u,v;
	float r,g,b;
	float x,y,z;
} tTexturedColoredVertex;

/// Quaternion Definitions ////////////////////////////////////////////////////
typedef struct
{
	float x,y,z,w;
} tQuaternion;
///////////////////////////////////////////////////////////////////////////////

#endif // !defined(MATH_H__INCLUDED_)

