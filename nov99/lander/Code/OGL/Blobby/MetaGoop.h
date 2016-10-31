///////////////////////////////////////////////////////////////////////////////
//
// MetaGoop.h : implementation file
//
// Purpose:	Implementation of Blob routines
//
// Created:
//		JL  10/18/99
// Revisions:
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(METAGOOP_H__INCLUDED_)
#define METAGOOP_H__INCLUDED_

#include "MathDefs.h"

// The metagoop structure.  It is pretty simple.  This could be modified
// in a number of ways for example you could create non-sphere blobs via 
// scaling and rotation.
typedef struct
{
	tVector		position;		// Where is the blob
	float		radiusSquared;	// Size of blob
	float		strength;		// Amount of influence
} tMetaGoop;

typedef struct
{
	tMetaGoop	*pGoop;		// Where is the Goop
	int			nGoopCnt;
	tVector		vMin,vMax;	// Bounding box of the goop field
} tMetaGoopSys;

typedef struct
{
	float		value;		// Evaluated field value
	tVector		pos;		// Position in the Field
	BOOL		inside;		// Is it in or out of the threshold
} tMetaGoopEval;

tMetaGoopSys	*Goop_InitSys();
void			Goop_FreeSys();
void			Goop_AddBlob(tMetaGoopSys	*pGoopSys);
float			Goop_EvaluateField(tVector *pos);
void			Goop_FindBounds();
void			Goop_EvaluateLayer(tMetaGoopEval *field,int subDiv,float sx, float sz, float stepX, float stepZ, float y);
void			Goop_EvaluateSurface(float threshold,int subDiv);
#endif // !defined(METAGOOP_H__INCLUDED_)
