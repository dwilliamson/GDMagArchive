///////////////////////////////////////////////////////////////////////////////
//
// Visual.h : Animation System structure definition file
//
// Purpose:	Structure Definition of Hierarchical Animation System
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1997 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(Visual_H__INCLUDED_)
#define Visual_H__INCLUDED_

#include "MathDefs.h"
#include "Particle.h"

#define ushort unsigned short
#define uint   unsigned int
typedef struct
{
	float r,g,b,a;
} tColor;

typedef struct
{
	float u,v;
} t2DCoord;

typedef struct {
	t2DCoord	t1[4],t2[4];
	unsigned int    TexNdx1;
	unsigned int    TexNdx2;
	unsigned short index[4];
	long	type;
	long	color[4];		// RGB VERTEX COLOR
} tPrimPoly;

typedef struct
{
	long	v[3],n[3],t[3];
	int		mat;
} t_faceIndex;

/// Structure Definitions ///////////////////////////////////////////////////////
struct t_Visual
{
	int		dataFormat;
	tVector	*vertex;		// Vertex
	long	vertexCnt;		// NUMBER OF VERTICES IN VISUAL
	tVector	*normal;		// Vertex
	tVector	*deformData;	// DEFORMED VERTEX DATA
	tVector	*texture;		// Vertex
	t_faceIndex	*index;			
	long	faceCnt;		// NUMBER OF FACES IN VISUAL
	tVector *matColor;		// POINTER TO VECTOR
	int		matCnt;
	long	vPerFace;		// VERTICES PER FACE, EITHER 3 OR 4
	tColor  Ka,Kd,Ks;		// COLOR FOR OBJECT
	float	Ns;				// SPECULAR COEFFICIENT
	char    map[255];
	uint    glTex;
	unsigned char *texData;		// TEXTURE SPACE FOR THE TEXTURE MAP
	int		texWidth, texHeight, texDepth;
	long	normalCnt;		// NUMBER OF VERTICES IN VISUAL
	long	uvCnt;		// NUMBER OF VERTICES IN VISUAL
	tVector bbox[8];		// BBOX COORDS
	tVector transBBox[8];	
	tParticle *particle;
	int		particleCnt;
};


#endif // !defined(Visual_H__INCLUDED_)
