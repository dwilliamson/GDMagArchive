/////////////////////////////////////////////////////////////////////////////////////
// RenderWorld.c
// This file actually renders the world complete
// 
// Some code was pulled from the OpenGL Super Bible.
// Great book that I highly recommend
//
// Created:
//		JL 2/15/00		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>	// Normal Windows stuff
#include <math.h>
#include <stdio.h>
#include <gl/gl.h>		// Core OpenGL functions
#include <gl/glu.h>		// OpenGL Utility functions
#include <gl/glaux.h>
#include "externs.h"	// Data shared between files
#include "loadtga.h"	// Routines to load the texture files

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID

t_Camera	g_POV;			// Camera for View
t_Mesh		g_Mesh;
tVector		g_ShadeLight;
tVector		g_ShadeSrc[32];				// Shade Texture
unsigned int	g_ShadeTexture;				// Pointer to Shaded texture
int			g_DrawInfluence = FALSE;
tMatrix		g_ViewMatrix;
void LoadSceneFile(char *filename);
void LoadShadeTexture(char *texfile);
void LoadMesh(char *filename);
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
	g_POV.trans.y = -4.0f;
	g_POV.trans.z = 15.0f;
	g_POV.rot.x = 20.0f;
	g_POV.rot.y = 90.0f;
	g_POV.rot.z = 0.0f;

	// Set the Default Light Direction
	g_ShadeLight.x = 0.0f;
	g_ShadeLight.y = 0.8f;
	g_ShadeLight.z = 0.3f;	
	NormalizeVector(&g_ShadeLight);	// Normalize it since I know I didn't

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

	// Load the Cartoon Shade Table
	LoadShadeTexture("twoshad.shd");
	// Load the Mesh
	LoadMesh("car.dcf");

	// Pass the Mesh through the FFD formula to create the deformation weights
	SetFFDWeights(g_Mesh.visuals);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadShadeTexture
// Purpose:		Load a shaded environment texture
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
void LoadShadeTexture(char *texfile)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int loop;
	FILE *fp;
	char line[255];
	float value;
/////////////////////////////////////////////////////////////////////////////////////

	// Make a Default one One shade with highlight
	for (loop = 0; loop < 32; loop++)
	{

		if (loop < 8)
		{
			MAKEVECTOR(g_ShadeSrc[loop], 0.4f, 0.4f, 0.4f)
		}
		else if (loop < 28)
		{
			MAKEVECTOR(g_ShadeSrc[loop], 0.9f, 0.9f, 0.9f)
		}
		else
		{
			MAKEVECTOR(g_ShadeSrc[loop], 1.0f, 1.0f, 1.0f)
		}
	}

	// Totally simple file format to load a 1D shade table
	// just a list of floats in a text file
	fp = fopen(texfile,"r");
	if (fp)
	{
		for (loop = 0; loop < 32; loop++)
		{
			if (feof(fp))
				break;
			// Get a line from the file
			fgets(line,255,fp);
			// Convert it to a shade value
			value = atof(line);
			g_ShadeSrc[loop].x = g_ShadeSrc[loop].y = g_ShadeSrc[loop].z = value;
		}
		fclose(fp);
	}
	glBindTexture(GL_TEXTURE_1D, g_ShadeTexture);

	// Do not allow bilinear filtering - not for cartoon rendering
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 32, 0,
			 GL_RGB , GL_FLOAT, (float *)g_ShadeSrc); //visual->texData);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadMesh
// Purpose:		Load a Darwin Format Character
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
void LoadMesh(char *name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;		// I PREFER THIS STYLE OF FILE ACCESS
	char	temp[5] = "DCF1";
	long	vertexCnt = 0;
	t_ToonVisual *toonVisual;
///////////////////////////////////////////////////////////////////////////////
	if (fp = fopen(name,"rb")) {
		fread(temp,sizeof(char),4,fp);
		fread(&g_Mesh,sizeof(t_Mesh),1,fp);

		fread(&g_Mesh,sizeof(t_Mesh),1,fp);
		g_Mesh.visuals = (t_ToonVisual *)malloc(sizeof(t_ToonVisual));
		fread(g_Mesh.visuals,sizeof(t_ToonVisual),1,fp);
		toonVisual = (t_ToonVisual *)g_Mesh.visuals;
		toonVisual->vertex = (tVector *)malloc(sizeof(tVector) * toonVisual->vertexCnt);
		toonVisual->deformData = (tVector *)malloc(sizeof(tVector) * toonVisual->vertexCnt);
		toonVisual->normal = (tVector *)malloc(sizeof(tVector) * toonVisual->normalCnt);
		toonVisual->texture = (tVector *)malloc(sizeof(tVector) * toonVisual->uvCnt);
		toonVisual->index = (t_faceIndex *)malloc(sizeof(t_faceIndex) * toonVisual->faceCnt);
		toonVisual->matColor = (tVector *)malloc(sizeof(tVector) * toonVisual->matCnt);
		fread(toonVisual->vertex,sizeof(tVector),toonVisual->vertexCnt,fp);
		fread(toonVisual->normal,sizeof(tVector),toonVisual->normalCnt,fp);
		fread(toonVisual->texture,sizeof(tVector),toonVisual->uvCnt,fp);
		fread(toonVisual->index,sizeof(t_faceIndex),toonVisual->faceCnt,fp);
		fread(toonVisual->matColor,sizeof(tVector),toonVisual->matCnt,fp);

		fclose(fp);

	}
}

////////////////////////////////////////////////////////////////////////////////
// Render the entire scene
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Function:	DeformMesh
// Purpose:		Deform the mesh based on the offset of the control points and weights
// Arguments:	The mesh to deform
///////////////////////////////////////////////////////////////////////////////
void DeformMesh(t_Mesh *mesh)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	t_ToonVisual *visual;
    int loop,loop2;
	tVector *vertex,*defVertex,delta;
	float weight;
/////////////////////////////////////////////////////////////////////////////////////
	visual = (t_ToonVisual *)mesh->visuals;
	vertex = visual->vertex;
	defVertex = visual->deformData;
	memcpy((void *)defVertex,(void *)vertex,sizeof(tVector) * visual->vertexCnt);
	for (loop = 0; loop < visual->vertexCnt; loop++,defVertex++)
	{
		for (loop2 = 0; loop2 < 64; loop2++)
		{
			weight = visual->weightData[(loop * 64) + loop2];
			if (weight > 0.0f)
			{
				VectorDifference(&g_CurrentSys[loop2].pos,&g_CurrentSys[loop2].rest_pos,&delta);
				ScaleVector(&delta,weight,&delta);
				VectorSum(defVertex,&delta,defVertex);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CalculateShadow
// Purpose:		Calculate the shadow coordinate value for a normal
// Arguments:	The vertex normal, Light vector, and Object rotation matrix
// Returns:		An index coordinate into the shade table
///////////////////////////////////////////////////////////////////////////////
float CalculateShadow(tVector *normal,tVector *light, tMatrix *mat)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector post;
	float dot;
/////////////////////////////////////////////////////////////////////////////////////
	// Rotate the normal by the current object matrix
	MultVectorByRotMatrix(mat, normal, &post);
	dot = DotProduct(&post,light);				// Calculate the Dot Product

	if (dot < 0) dot = 0;						// Make sure the Back half dark
	return dot;									// Return the shadow value
}


///////////////////////////////////////////////////////////////////////////////
// Function:	DrawToonMesh
// Purpose:		Actually draws the mesh in Toon style
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
void DrawToonMesh(t_Mesh *mesh)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop,loop2;
	t_faceIndex *face;
	t_ToonVisual *visual;
	tVector	*vertex, *normal;
	float u;
	float	weight,max = -1.0f;
	int winner;
///////////////////////////////////////////////////////////////////////////////

	DeformMesh(mesh);		// Perform the FFD deformation

	visual = (t_ToonVisual *)mesh->visuals;

	vertex = visual->deformData;
	normal = visual->normal;

	// Bind my 1D shade texture
	glEnable(GL_TEXTURE_1D);
	glBindTexture( GL_TEXTURE_1D,g_ShadeTexture);

	glDisable(GL_LIGHTING);

	// Grab the matrix for lighting calc
	glGetFloatv(GL_MODELVIEW_MATRIX,g_ViewMatrix.m);

	if (g_DrawMesh)
	{
		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
			glColor3fv(&visual->matColor[face->mat].r);	
			glBegin(GL_TRIANGLES);
				for (loop2 = 0; loop2 < 3; loop2++)
				{
					// calculate an index into the 1D texture using normal and light
					u = CalculateShadow(&normal[face->n[loop2]],&g_ShadeLight, &g_ViewMatrix);
					glTexCoord1f(u);
					glVertex3fv((float *)&vertex[face->v[loop2]]);
				}
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_1D);

	// NOW DRAW THE VERTEX MARKERS IF THEY ARE SELECTED
	if (g_Pick[0] > -1 && g_DrawInfluence)
	{
		for (loop = 0; loop < visual->vertexCnt; loop++)
		{
			weight = visual->weightData[(loop * 64) + g_Pick[0]];
			if (weight > max) 
			{
				max = weight;
				winner = loop;
			}
			weight = weight * 10;
			if (weight > 1.0f) weight = 1.0f;
			glColor3f(weight, 0.2f, 0.2f);	// View based on weighting
			glBegin(GL_POINTS);
				glVertex3fv((float *)&visual->deformData[loop].x);
			glEnd();
		}
		// Show closest
		glDisable(GL_DEPTH_TEST);
		glColor3f(1.0f, 1.0f, 1.0f);	
		glEnable(GL_DEPTH_TEST);
	}

}
//// DrawToonCharacter /////////////////////////////////////////////////////////////

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

	DrawToonMesh(&g_Mesh);

	DrawSimWorld();

	glPopMatrix();

	SwapBuffers(g_hDC);
}

