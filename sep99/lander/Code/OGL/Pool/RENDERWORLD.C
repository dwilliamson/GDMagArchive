/////////////////////////////////////////////////////////////////////////////////////
// RenderWorld.c
// This file actually renders the world complete with balls and cuestick
// 
// The code base was pulled from the OpenGL Super Bible.
// Great book that I highly recommend
//
// Created:
//		JL 9/5/99		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
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
#include "Models.h"		// Actual Geometry for balls and cue

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define OGL_WBALL_DLIST		2		// OPENGL LIST FOR WHITE BALL
#define OGL_YBALL_DLIST		3		// OPENGL LIST FOR YELLOW BALL
#define OGL_CUE_DLIST		4		// OPENGL LIST FOR CUE STICK

#define ART_PATH		"art/"
#define MAX_TEXTURES	255

typedef struct s_TexPool
{
	char		map[255];
	GLuint		glTex;
	byte		*data;
	int			type;
}t_TexPool;

// A Polygon can be Quad or Triangle
typedef struct {
	t2DCoord	t1[4],t2[4];
	uint    TexNdx1;
	uint    TexNdx2;
	unsigned short index[4];
	long	type;
	long	color[4];		// RGB VERTEX COLOR
} tPrimPoly;

// A Scene consisting for Triangles and Quads
typedef struct 
{
	long		vertexCnt;
	tVector		*vertex;
	long		triCnt,quadCnt;
	tPrimPoly	*tri,*quad;
	char		map[255];
} t_Visual;

t_Camera	g_POV;			// Camera for View
int			g_TextureCnt;	// Number of Textures loaded
t_TexPool	g_TexPool[MAX_TEXTURES];	// Place to store texture info
t_Visual	g_Scene;		// Actual Scene

void LoadTextures();
void LoadSceneFile(char *filename);
void RenderScene();

////////////////////////////////////////////////////////////////////////////////
// Initialize the Render System
////////////////////////////////////////////////////////////////////////////////
void InitRender(void)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
///////////////////////////////////////////////////////////////////////////////

	// Initialize position of the Point of View
	g_POV.trans.x = 0.0f;
	g_POV.trans.y = 0.0f;
	g_POV.trans.z = 8.0f;
	g_POV.rot.x = 20.0f;
	g_POV.rot.y = 90.0f;
	g_POV.rot.z = 0.0f;

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

	glNewList(OGL_WBALL_DLIST,GL_COMPILE);
		// Declare the Array of Data
		glInterleavedArrays(WBALLFORMAT,0,(GLvoid *)&WBALLMODEL);

	//	This doesn't work on my TNT 2
	//	glDrawArrays(GL_TRIANGLES,0,WBALLPOLYCNT * 3);

		// THIS CODE WAS THE EQUIVALENT OF THE FOLLOWING, BUT FASTER
		glBegin(GL_TRIANGLES);
		for (loop = 0; loop < WBALLPOLYCNT * 3; loop++)
		{
			glArrayElement(loop);
		}
		glEnd();
	glEndList();

	glNewList(OGL_YBALL_DLIST,GL_COMPILE);
		// Declare the Array of Data
		glInterleavedArrays(YBALLFORMAT,0,(GLvoid *)&YBALLMODEL);

	//	This doesn't work on my TNT 2
	//	glDrawArrays(GL_TRIANGLES,0,YBALLPOLYCNT * 3);

		// THIS CODE WAS THE EQUIVALENT OF THE FOLLOWING, BUT FASTER
		glBegin(GL_TRIANGLES);
		for (loop = 0; loop < YBALLPOLYCNT * 3; loop++)
		{
			glArrayElement(loop);
		}
		glEnd();
	glEndList();

	glNewList(OGL_CUE_DLIST,GL_COMPILE);
		// Declare the Array of Data
		glInterleavedArrays(CUEFORMAT,0,(GLvoid *)&CUEMODEL);

	//	This doesn't work on my TNT 2
	//	glDrawArrays(GL_TRIANGLES,0,YBALLPOLYCNT * 3);

		// THIS CODE WAS THE EQUIVALENT OF THE FOLLOWING, BUT FASTER
		glBegin(GL_TRIANGLES);
		for (loop = 0; loop < CUEPOLYCNT * 3; loop++)
		{
			glArrayElement(loop);
		}
		glEnd();
	glEndList();

	LoadTextures();

	LoadSceneFile("Pool.ros");			// Load the Scene Data
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadTextures
// Purpose:		Load all the TGA files in a directory and store them in a texture pool
/////////////////////////////////////////////////////////////////////////////////////
void LoadTextures()
{
	GLubyte	*rgb;				/* Bitmap RGB pixels */
	char texName[80];
	tTGAHeader_s tgaHeader;
	HANDLE specHandle;
	WIN32_FIND_DATA	fileData;
	int rv;

	// LOAD THE LIST OF FILE NAMES
	g_TextureCnt = 0;
	sprintf(texName,"%s*.tga",ART_PATH);
	if ((specHandle=FindFirstFile(texName,&fileData))!= INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(g_TexPool[g_TextureCnt].map,"%s%s",ART_PATH,fileData.cFileName);
			// GENERATE THE OPENGL TEXTURE ID
			glGenTextures(1,&g_TexPool[g_TextureCnt].glTex);

			rgb = LoadTGAFile( g_TexPool[g_TextureCnt].map,&tgaHeader);
			if (rgb == NULL)
			{
				MessageBox(NULL,"Unable to Open File...",g_TexPool[g_TextureCnt].map,MB_OK);
				g_TexPool[g_TextureCnt].glTex = 0;
				return;
			}

			glBindTexture(GL_TEXTURE_2D, g_TexPool[g_TextureCnt].glTex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			/*
			* Define the 2D texture image.
			*/

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

			if (tgaHeader.d_pixel_size == 32)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 4, tgaHeader.d_width, tgaHeader.d_height, 0,
						 GL_RGBA , GL_UNSIGNED_BYTE, rgb);
				rv = gluBuild2DMipmaps( GL_TEXTURE_2D, 4, tgaHeader.d_width, tgaHeader.d_height, 
					GL_RGBA, GL_UNSIGNED_BYTE, rgb );
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 3, tgaHeader.d_width, tgaHeader.d_height, 0,
						 GL_RGB, GL_UNSIGNED_BYTE, rgb);
				rv = gluBuild2DMipmaps( GL_TEXTURE_2D, 3, tgaHeader.d_width, tgaHeader.d_height, GL_RGB, 
					GL_UNSIGNED_BYTE, rgb );
			}
			/*
			*Free the bitmap and RGB images, then return 0 (no errors).
			*/

			free(rgb);

			g_TextureCnt++;
		}while (FindNextFile(specHandle,&fileData));
		FindClose(specHandle);
	}
}

void LoadSceneFile(char *filename)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;
	char tempstr[80];
	t_Visual	*visual;
///////////////////////////////////////////////////////////////////////////////

	fp = fopen(filename,"rb");
	if (fp != NULL)
	{
		fread(tempstr,1,4,fp); // FDAT
		if (strncmp(tempstr,"ROSC",4)!= 0)
		{
			MessageBox(NULL,"Not a Valid Data File","Load File", MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		visual = &g_Scene;

		fread(&visual->vertexCnt,sizeof(long),1,fp);

		visual->vertex = (tVector *)malloc(sizeof(tVector) * visual->vertexCnt);
		fread(visual->vertex,sizeof(tVector),visual->vertexCnt,fp);

		fread(&visual->triCnt,sizeof(long),1,fp);

		visual->tri = (tPrimPoly *)malloc(sizeof(tPrimPoly) * (visual->triCnt));
		fread(visual->tri,sizeof(tPrimPoly),visual->triCnt,fp);
	
		fread(&visual->quadCnt,sizeof(long),1,fp);

		visual->quad = (tPrimPoly *)malloc(sizeof(tPrimPoly) * (visual->quadCnt));
		fread(visual->quad,sizeof(tPrimPoly),visual->quadCnt,fp);
	
		fclose(fp);
	}
}

// TODO: If I add more billiard balls, this will need some work.  Assumes two.
void RenderCueAndBalls()
{
	// Draw the White Ball
	glPushMatrix();
		glTranslatef(g_CurrentSys[0].pos.x,g_CurrentSys[0].pos.y,g_CurrentSys[0].pos.z);
		glCallList(OGL_WBALL_DLIST);
	glPopMatrix();

	// Draw the White Ball
	glPushMatrix();
		glTranslatef(g_CurrentSys[1].pos.x,g_CurrentSys[1].pos.y,g_CurrentSys[1].pos.z);
		glCallList(OGL_YBALL_DLIST);
	glPopMatrix();

	// Draw the Cue Stick
	glPushMatrix();
		glTranslatef(g_CueStick.pos.x,g_CueStick.pos.y,g_CueStick.pos.z);
		glRotatef(g_CueStick.yaw, 0.0f, 1.0f, 0.0f); 
		glRotatef(-5.0f, 1.0f, 0.0f, 0.0f);			// Tilt it back a little for looks
		glTranslatef(0,0,g_CueStick.draw);
		glCallList(OGL_CUE_DLIST);
	glPopMatrix();
}

void RenderScene()
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop,loop2;
	t_Visual *visual;	// IN THIS CASE THE DATA IS COLOR VERTICES
	tPrimPoly *poly;
//////////////////////////////////////////////////////////////////////////////
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE

	visual = &g_Scene;
	for (loop2 = 0; loop2 < g_TextureCnt; loop2++)
	{
		poly = visual->quad;
		glBindTexture(GL_TEXTURE_2D, g_TexPool[loop2].glTex);
		
		for (loop = 0; loop < visual->quadCnt; loop++)
		{
			if ((poly->type & POLY_TEXTURED) > 0 && poly->TexNdx1 == (uint)loop2)
			{		
				glBegin(GL_QUADS);
				glTexCoord2fv((float *)&poly->t1[0]);
				glColor3ubv((unsigned char *)&poly->color[0]);
				glVertex3fv((float *)&visual->vertex[poly->index[0]]);
				glTexCoord2fv((float *)&poly->t1[1]);
				glColor3ubv((unsigned char *)&poly->color[1]);
				glVertex3fv((float *)&visual->vertex[poly->index[1]]);
				glTexCoord2fv((float *)&poly->t1[2]);
				glColor3ubv((unsigned char *)&poly->color[2]);
				glVertex3fv((float *)&visual->vertex[poly->index[2]]);
				glTexCoord2fv((float *)&poly->t1[3]);
				glColor3ubv((unsigned char *)&poly->color[3]);
				glVertex3fv((float *)&visual->vertex[poly->index[3]]);
				glEnd();
			}
			poly++;
		}

		poly = visual->tri;
		for (loop = 0; loop < visual->triCnt; loop++)
		{
			if ((poly->type & POLY_TEXTURED) > 0 && poly->TexNdx1 == (uint)loop2)
			{		
				glBegin(GL_TRIANGLES);
				glTexCoord2fv((float *)&poly->t1[0]);
				glColor3ubv((unsigned char *)&poly->color[0]);
				glVertex3fv((float *)&visual->vertex[poly->index[0]]);
				glTexCoord2fv((float *)&poly->t1[1]);
				glColor3ubv((unsigned char *)&poly->color[1]);
				glVertex3fv((float *)&visual->vertex[poly->index[1]]);
				glTexCoord2fv((float *)&poly->t1[2]);
				glColor3ubv((unsigned char *)&poly->color[2]);
				glVertex3fv((float *)&visual->vertex[poly->index[2]]);
				glEnd();
			}
			poly++;
		}
	}
	glDisable(GL_TEXTURE_2D);
}

////////////////////////////////////////////////////////////////////////////////
// Render the entire scene
////////////////////////////////////////////////////////////////////////////////
void RenderWorld(void)
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	g_CueStick.yaw = -g_POV.rot.y;		// Set the Cue To be centered on my view

	// Set root skeleton's orientation and position
	glTranslatef(-g_POV.trans.x, -g_POV.trans.y, -g_POV.trans.z);

	glRotatef(g_POV.rot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(g_POV.rot.x, 1.0f, 0.0f, 0.0f); 
	glRotatef(g_POV.rot.y, 0.0f, 1.0f, 0.0f);

	glTranslatef(-g_CueStick.pos.x, -g_CueStick.pos.y, -g_CueStick.pos.z);
//	glTranslatef(0,-TABLE_POSITION,0);

	RenderScene();	// Draw the actual Scene

	RenderCueAndBalls();

	glPopMatrix();

	SwapBuffers(g_hDC);
}

