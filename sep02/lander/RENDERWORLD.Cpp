/////////////////////////////////////////////////////////////////////////////////////
// RenderWorld.c
// This file actually renders the world complete
// 
// Created:
//		JL 2/15/00		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2002 Darwin 3D, LLC., All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>	// Normal Windows stuff
#include <math.h>
#include <stdio.h>
#include <gl/gl.h>		// Core OpenGL functions
#include <gl/glu.h>		// OpenGL Utility functions
#include <gl/glaux.h>
#include "externs.h"	// Data shared between files

/// Application Definitions ///////////////////////////////////////////////////
t_Camera	g_POV;			// Camera for View
int			g_DrawInfluence = FALSE;
float		g_ViewMatrix[16];
void RenderScene();
////////////////////////////////////////////////////////////////////////////////
// Initialize the Render System
////////////////////////////////////////////////////////////////////////////////
void InitRender(void)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	// Initialize position of the Point of View
	g_POV.trans.x = 0.0f;
	g_POV.trans.y = 2.0f;
	g_POV.trans.z = 18.0f;
	g_POV.rot = CVector(0.0f, 0.0f,0.0f);

	glNewList(OGL_AXIS_DLIST,GL_COMPILE);
		glPushMatrix();
		glScalef(4.0,4.0,4.0);
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);	// X AXIS STARTS - COLOR RED
			glVertex3f(-0.2f,  0.0f, 0.0f);
			glVertex3f( 0.2f,  0.0f, 0.0f);
			glVertex3f( 0.2f,  0.0f, 0.0f);	// TOP PIECE OF ARROWHEAD
			glVertex3f( 0.15f,  0.04f, 0.0f);
			glVertex3f( 0.2f,  0.0f, 0.0f);	// BOTTOM PIECE OF ARROWHEAD
			glVertex3f( 0.15f, -0.04f, 0.0f);
			glColor3f(0.0f, 1.0f, 0.0f);	// Y AXIS STARTS - COLOR GREEN
			glVertex3f( 0.0f,  0.2f, 0.0f);
			glVertex3f( 0.0f, -0.2f, 0.0f);			
			glVertex3f( 0.0f,  0.2f, 0.0f);	// TOP PIECE OF ARROWHEAD
			glVertex3f( 0.04f,  0.15f, 0.0f);
			glVertex3f( 0.0f,  0.2f, 0.0f);	// BOTTOM PIECE OF ARROWHEAD
			glVertex3f( -0.04f,  0.15f, 0.0f);
			glColor3f(0.0f, 0.0f, 1.0f);	// Z AXIS STARTS - COLOR BLUE
			glVertex3f( 0.0f,  0.0f,  0.2f);
			glVertex3f( 0.0f,  0.0f, -0.2f);
			glVertex3f( 0.0f,  0.0f, 0.2f);	// TOP PIECE OF ARROWHEAD
			glVertex3f( 0.0f,  0.04f, 0.15f);
			glVertex3f( 0.0f, 0.0f, 0.2f);	// BOTTOM PIECE OF ARROWHEAD
			glVertex3f( 0.0f, -0.04f, 0.15f);
		glEnd();
		glPopMatrix();
	glEndList();

}

////////////////////////////////////////////////////////////////////////////////
// Render the entire scene
////////////////////////////////////////////////////////////////////////////////
void RenderWorld(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	// Set root skeleton's orientation and position
	glTranslatef(-g_POV.trans.x, -g_POV.trans.y, -g_POV.trans.z);

	glRotatef(g_POV.rot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(g_POV.rot.x, 1.0f, 0.0f, 0.0f); 
	glRotatef(g_POV.rot.y, 0.0f, 1.0f, 0.0f);

	// Grab the matrix for lighting calc
	glGetFloatv(GL_MODELVIEW_MATRIX,g_ViewMatrix);

	DrawSimWorld();

	glPopMatrix();

	SwapBuffers(g_hDC);
}


void ChangeGround(int hitX, int hitY)
{
	double modelMatrix[16],projMatrix[16];
	int viewport[4];

	double picked[3];

	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glPushMatrix();

	// Set root skeleton's orientation and position
	glTranslatef(-g_POV.trans.x, -g_POV.trans.y, -g_POV.trans.z);

	glRotatef(g_POV.rot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(g_POV.rot.x, 1.0f, 0.0f, 0.0f); 
	glRotatef(g_POV.rot.y, 0.0f, 1.0f, 0.0f);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	glPopMatrix();

	// Get the place the mouse is clicking by using GL_UNPROJECT
	glGetIntegerv(GL_VIEWPORT,viewport);

	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, viewport, &picked[0],&picked[1],&picked[2]);
	gluUnProject((double)hitX, (double)hitY,picked[2], modelMatrix, projMatrix, viewport, &picked[0],&picked[1],&picked[2]);

	int closest = -1;
	float xdist = 99999.0f;
	for (int i = 0; i < GROUND_POINTS; i++)
	{
		if (fabs(g_Ground[i].x - picked[0]) < xdist)
		{
			xdist = fabsf(g_Ground[i].x - (float)picked[0]);
			closest = i;
		}
	}
	if (closest >= 0)
		g_Ground[closest].y = (float)picked[1];
}
