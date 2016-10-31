///////////////////////////////////////////////////////////////////////////////
//
// MetaGoop.cpp : implementation file
//
// Purpose:	Implementation of MetaGoop
//			This is the main file for the MetaGoop system.
//		
// Created:
//		JL  10/18/99
// Revisions: Jan 2000
//	Lots of UI and tweaking
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <float.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "MetaGoop.h"

tMetaGoopSys	m_GoopSys;

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_InitSys
// Purpose:		Initialize System
/////////////////////////////////////////////////////////////////////////////////////
tMetaGoopSys	*Goop_InitSys()
{
	m_GoopSys.nGoopCnt = 0;
	m_GoopSys.pGoop = NULL;
	return(&m_GoopSys);
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_FreeSys
// Purpose:		Clear System
/////////////////////////////////////////////////////////////////////////////////////
void	Goop_FreeSys()
{
	if (m_GoopSys.nGoopCnt > 0)
		free(m_GoopSys.pGoop);
	m_GoopSys.nGoopCnt = 0;
	m_GoopSys.pGoop = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_AddBlob
// Purpose:		Add a new blob into the system and initialize it
/////////////////////////////////////////////////////////////////////////////////////
void Goop_AddBlob(tMetaGoopSys	*pGoopSys)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tMetaGoop	*goop;
/////////////////////////////////////////////////////////////////////////////////////
	goop = (tMetaGoop *)malloc(sizeof(tMetaGoop) * (pGoopSys->nGoopCnt + 1));
	if (pGoopSys->nGoopCnt > 0)
	{
		memcpy(goop,pGoopSys->pGoop,sizeof(tMetaGoop) * pGoopSys->nGoopCnt);
		free(pGoopSys->pGoop);
	}
	// Initialize the Blob
	goop[pGoopSys->nGoopCnt].position.x = 0.0f;
	goop[pGoopSys->nGoopCnt].position.y = 0.0f;
	goop[pGoopSys->nGoopCnt].position.z = 0.0f;
	goop[pGoopSys->nGoopCnt].radiusSquared = 4.0f;
	goop[pGoopSys->nGoopCnt].strength = 4.0f;
	pGoopSys->pGoop = goop;
	pGoopSys->nGoopCnt++;
}
// Goop_AddBlob

	tVector		vMin,vMax;	// Bounding box of the goop field

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_FindBounds
// Purpose:		Find boundary of the energy meta field
/////////////////////////////////////////////////////////////////////////////////////
void Goop_FindBounds()
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector	min = {FLT_MAX,FLT_MAX,FLT_MAX};
	tVector max = {-FLT_MAX,-FLT_MAX,-FLT_MAX};
	tMetaGoop	*goop;
	float		radius;
/////////////////////////////////////////////////////////////////////////////////////
	goop = m_GoopSys.pGoop;
	for (int loop = 0; loop < m_GoopSys.nGoopCnt; loop++,goop++)
	{
		// Beef up radius to give me some room
		radius = (float)sqrt(goop->radiusSquared) * 1.5f;
		if (goop->position.x - radius < min.x) min.x = goop->position.x - radius;
		if (goop->position.x + radius > max.x) max.x = goop->position.x + radius;
		if (goop->position.y - radius < min.y) min.y = goop->position.y - radius;
		if (goop->position.y + radius > max.y) max.y = goop->position.y + radius;
		if (goop->position.z - radius < min.z) min.z = goop->position.z - radius;
		if (goop->position.z + radius > max.z) max.z = goop->position.z + radius;
	}

	memcpy(&m_GoopSys.vMin,&min,sizeof(tVector));
	memcpy(&m_GoopSys.vMax,&max,sizeof(tVector));
}
// Goop_FindBounds

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_EvaluateField
// Purpose:		Evaluate the energy in the meta field at a location
// Arguments:	Position to test
// Returns:		Field Strength
/////////////////////////////////////////////////////////////////////////////////////
float Goop_EvaluateField(tVector *pos)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	float	distance, fieldStrength = 0.0f, falloff;
	tMetaGoop	*goop;
/////////////////////////////////////////////////////////////////////////////////////
	goop = m_GoopSys.pGoop;
	for (int loop = 0; loop < m_GoopSys.nGoopCnt; loop++,goop++)
	{
		distance = (float)VectorSquaredDistance(&goop->position, pos);
		if (distance < goop->radiusSquared)
		{
			falloff = 1.0f - (distance / goop->radiusSquared);
			fieldStrength += goop->strength * falloff * falloff;
		}
	}

	return(fieldStrength);
}
// Goop_EvaluateField

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_EvaluateLayer
// Purpose:		Evaluate the energy in the meta field at a XZ plane
// Arguments:	Pointer to layer, subdiv, and position variables
/////////////////////////////////////////////////////////////////////////////////////
void Goop_EvaluateLayer(tMetaGoopEval *field,int subDiv,float sx, float sz, float stepX, float stepZ, float y)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector pos;
	tMetaGoopEval *element;
	int loopx, loopz;
/////////////////////////////////////////////////////////////////////////////////////
	pos.y = y;
	for (loopx = 0, pos.x = sx; loopx < subDiv; loopx++, pos.x += stepX)
		for (loopz = 0, pos.z = sz; loopz < subDiv; loopz++, pos.z += stepZ)
		{
			element = &field[loopx * subDiv + loopz];
			element->value = Goop_EvaluateField(&pos);
			memcpy(&element->pos,&pos,sizeof(tVector));
		}
}
// Goop_EvaluateLayer

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_CheckEdge
// Purpose:		Check the Final edge to see if it crosses the meta surface
// Arguments:	threshold for edge and edge vertices, collection of final points
/////////////////////////////////////////////////////////////////////////////////////
void Goop_CheckEdge(float threshold, tMetaGoopEval *a, tMetaGoopEval *b, tVector *vertex)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector diff;
	float	ratio;
/////////////////////////////////////////////////////////////////////////////////////
	VectorSubtract(&a->pos, &b->pos, &diff);
	ratio = (threshold - a->value) / (b->value - a->value);
	VectorMultiply(&diff, ratio);
	VectorSubtract(&a->pos,&diff,vertex);
}
// 

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_CheckPyramid
// Purpose:		Check a Pyramid to see if it is on the edge of the meta surface
// Arguments:	threshold for edge and list of cube vertices
/////////////////////////////////////////////////////////////////////////////////////
void Goop_CheckPyramid(float threshold,tMetaGoopEval **pyramid)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int inside = 0, outside = 0;
	int firstIn = -1, firstOut = -1;
	tVector vertex[4];
	int	vCnt;
/////////////////////////////////////////////////////////////////////////////////////
	// Count corners to see what is inside and outside
	for (int loop = 0; loop < 4; loop++)
	{
		// Is it inside or outside
		if (pyramid[loop]->inside)
		{
			inside++;
			if (firstIn == -1) firstIn = loop;	// Track the first
		}
		else 
		{
			outside++;
			if (firstOut == -1) firstOut = loop;	// Track the first
		}
	}
	// All in or all out, bail now
	if (inside == 4 || outside == 4)
		return;

	// TODO: Here I really need to track inside vs outside so polygons are created
	// facing the right direction.  This will be needed for culling and lighting
	vCnt = 0;
	if (pyramid[0]->inside != pyramid[1]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[0], pyramid[1], &vertex[vCnt]);
		vCnt++;
	}
	if (pyramid[1]->inside != pyramid[2]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[1], pyramid[2], &vertex[vCnt]);
		vCnt++;
	}
	if (pyramid[2]->inside != pyramid[0]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[2], pyramid[0], &vertex[vCnt]);
		vCnt++;
	}

	if (pyramid[2]->inside != pyramid[3]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[2], pyramid[3], &vertex[vCnt]);
		vCnt++;
	}
	if (pyramid[0]->inside != pyramid[3]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[0], pyramid[3], &vertex[vCnt]);
		vCnt++;
	}
	if (pyramid[1]->inside != pyramid[3]->inside)
	{
		Goop_CheckEdge(threshold, pyramid[1], pyramid[3], &vertex[vCnt]);
		vCnt++;
	}
	if (outside == 3 || inside == 3)
	{
		// TODO: Need to determine facing here.
		glColor3f(1.0f, 0.0f, 0.0f);	
		glBegin(GL_TRIANGLES);

			glVertex3fv((float *)&vertex[0]);
			glVertex3fv((float *)&vertex[1]);
			glVertex3fv((float *)&vertex[2]);
		glEnd();
	}
	else	// Handle the case with four vertices
	{
		// TODO: This is wrong, I think there are cases where the triangles will cross
		glColor3f(1.0f, 0.0f, 0.0f);	
		glBegin(GL_TRIANGLES);
			glVertex3fv((float *)&vertex[0]);
			glVertex3fv((float *)&vertex[1]);
			glVertex3fv((float *)&vertex[2]);
			glVertex3fv((float *)&vertex[0]);
			glVertex3fv((float *)&vertex[2]);
			glVertex3fv((float *)&vertex[3]);
		glEnd();
	}
}
// 


/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_CheckCubeSurf
// Purpose:		Check a Cube to see if it is on the edge of the meta surface
// Arguments:	threshold for edge and list of cube vertices
/////////////////////////////////////////////////////////////////////////////////////
void Goop_CheckCubeSurf(float threshold,tMetaGoopEval **cube)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int inside = 0, outside = 0;
	tMetaGoopEval *pyramid[4];
/////////////////////////////////////////////////////////////////////////////////////
	// Count corners to see what is inside and outside
	for (int loop = 0; loop < 8; loop++)
	{
		// Is it inside or outside
		if (cube[loop]->value > threshold)
		{
			cube[loop]->inside = TRUE;
			inside++;
		}
		else
		{
			cube[loop]->inside = FALSE;
			outside++;
		}
	}
	// All in or all out, bail now
	if (inside == 8 || outside == 8)
		return;

	// Now that I know the cube is on the surface, break out pyramids
	// TODO: This should be a table of vertex indices and optimize it a bit
	pyramid[0] = cube[0]; pyramid[1] = cube[1]; pyramid[2] = cube[3]; pyramid[3] = cube[4];
	Goop_CheckPyramid(threshold, pyramid);
	pyramid[0] = cube[1]; pyramid[1] = cube[2]; pyramid[2] = cube[3]; pyramid[3] = cube[5];
	Goop_CheckPyramid(threshold, pyramid);
	pyramid[0] = cube[4]; pyramid[1] = cube[5]; pyramid[2] = cube[7]; pyramid[3] = cube[3];
	Goop_CheckPyramid(threshold, pyramid);
	pyramid[0] = cube[5]; pyramid[1] = cube[6]; pyramid[2] = cube[7]; pyramid[3] = cube[2];
	Goop_CheckPyramid(threshold, pyramid);
	pyramid[0] = cube[3]; pyramid[1] = cube[2]; pyramid[2] = cube[7]; pyramid[3] = cube[5];
	Goop_CheckPyramid(threshold, pyramid);
	pyramid[0] = cube[1]; pyramid[1] = cube[4]; pyramid[2] = cube[5]; pyramid[3] = cube[3];
	Goop_CheckPyramid(threshold, pyramid);
}
// 

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Goop_EvaluateSurface
// Purpose:		Evaluate the energy in the meta field at a XZ plane
// Arguments:	edge threshold and subdivision count
/////////////////////////////////////////////////////////////////////////////////////
void Goop_EvaluateSurface(float threshold,int subDiv)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector step;
	float y;
	tMetaGoopEval *layer[2];	// Store 2 layers
	tMetaGoopEval *element[8];
/////////////////////////////////////////////////////////////////////////////////////

	step.x = (m_GoopSys.vMax.x - m_GoopSys.vMin.x) / subDiv;
	step.y = (m_GoopSys.vMax.y - m_GoopSys.vMin.y) / subDiv;
	step.z = (m_GoopSys.vMax.z - m_GoopSys.vMin.z) / subDiv;

	if (step.x == 0.0f || step.y == 0.0f || step.z == 0.0f) return;

	// Allocate memory for a layers
	layer[0] = (tMetaGoopEval *)malloc( subDiv * subDiv * sizeof(tMetaGoopEval));
	layer[1] = (tMetaGoopEval *)malloc( subDiv * subDiv * sizeof(tMetaGoopEval));

	// Evaluate first XZ plane
	Goop_EvaluateLayer(layer[0], subDiv, m_GoopSys.vMin.x, m_GoopSys.vMin.z, step.x, step.z, m_GoopSys.vMin.y);

	// Go through each vertical slice
	for (y = m_GoopSys.vMin.y + step.y; y <= m_GoopSys.vMax.y; y += step.y)
	{
		// Evaluate next XZ plane
		Goop_EvaluateLayer(layer[1], subDiv, m_GoopSys.vMin.x, m_GoopSys.vMin.z, step.x, step.z, y);
		for (int loopx = 0; loopx < subDiv - 1; loopx++)
		{
			for (int loopz = 0; loopz < subDiv - 1; loopz++)
			{
				// Gather eight corners of cube
				element[0] = &layer[0][loopx * subDiv + loopz];
				element[1] = &layer[0][(loopx + 1) * subDiv + loopz];
				element[2] = &layer[0][(loopx + 1) * subDiv + loopz + 1];
				element[3] = &layer[0][loopx * subDiv + loopz + 1];
				element[4] = &layer[1][loopx * subDiv + loopz];
				element[5] = &layer[1][(loopx + 1) * subDiv + loopz];
				element[6] = &layer[1][(loopx + 1) * subDiv + loopz + 1];
				element[7] = &layer[1][loopx * subDiv + loopz + 1];
				Goop_CheckCubeSurf(threshold,element);
			}
		}

		// Copy Layer for next pass
		memcpy(layer[0],layer[1],subDiv * subDiv * sizeof(tMetaGoopEval));
	}
}
// Goop_EvaluateSurface



