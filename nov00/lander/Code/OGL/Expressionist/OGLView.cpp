///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of 3D Paint System
//
// Created:
//		JL 8/4/00
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <mmsystem.h>
#include <math.h>
#include "Expressionist.h"
#include "OGLView.h"
#include "OBJFile.h"
#include "TGAFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define OGL_PARTICLE_DLIST	2		// OPENGL DISPLAY LIST ID FOR PARTICLE
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
	// INITIALIZE THE MODE KEYS
	m_StatusBar = NULL;	// CLEAR THIS.  IT IS SET BY MAINFRAME BUT UNTIL THEN MARK IT
	m_Bilinear	= TRUE;
	m_Dragging = FALSE;
	m_UseLighting = FALSE;

	// INITIALIZE SOME OF THE CAMERA VARIABLES
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.pos.x = 0.0f;
	m_Camera.pos.y = -3.0f;
	m_Camera.pos.z = -30.0f;

	m_Model.vertex = NULL;
	m_Model.particle = NULL;

	m_SelectBuffer = NULL;
	m_ByteBuffer = NULL;

	m_DrawMode = MODE_PARTICLE;
	m_SelectedTri = -1;

	m_PaintColor = MAKERGB(255, 255, 255);	// Default paint color

	m_ParticlePos.x = m_ParticlePos.y = m_ParticlePos.z = 0.0f;

	m_Skeleton = NULL;
}

COGLView::~COGLView()
{
}

BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BEGIN_MESSAGE_MAP(COGLView, CWnd)
	//{{AFX_MSG_MAP(COGLView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COGLView message handlers

BOOL COGLView::SetupPixelFormat(HDC hdc)
{
/// Local Variables ///////////////////////////////////////////////////////////
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int pixelformat;
///////////////////////////////////////////////////////////////////////////////
    ppfd = &pfd;
	memset(ppfd, 0, sizeof(pfd)); 

    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    ppfd->dwLayerMask = PFD_MAIN_PLANE;
    ppfd->iPixelType = PFD_TYPE_RGBA;
    ppfd->cColorBits = 16;
    ppfd->cDepthBits = 16;
    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;

    pixelformat = ChoosePixelFormat(hdc, ppfd);

    if ((pixelformat = ChoosePixelFormat(hdc, ppfd)) == 0) {
        MessageBox("ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        MessageBox("Needs palette", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hdc, pixelformat, ppfd) == FALSE) {
        MessageBox("SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}


int COGLView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	RECT rect;
///////////////////////////////////////////////////////////////////////////////
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    m_hDC = ::GetDC(m_hWnd);
    if (!SetupPixelFormat(m_hDC))
		PostQuitMessage (0);
	
    m_hRC = wglCreateContext(m_hDC);
    wglMakeCurrent(m_hDC, m_hRC);
    GetClientRect(&rect);
    initializeGL(rect.right, rect.bottom);

	// CREATE THE DISPLAY LIST FOR AN AXIS WITH ARROWS POINTING IN
	// THE POSITIVE DIRECTION Red = X, Green = Y, Blue = Z
	glNewList(OGL_AXIS_DLIST,GL_COMPILE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
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
			glColor3f(1.0f, 1.0f, 1.0f);	// Y AXIS STARTS - COLOR GREEN
		glEnd();
	glEndList();

	// Create a Display list for a Art card for rendering particles
	glNewList(OGL_PARTICLE_DLIST,GL_COMPILE);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f);	
			glVertex3f(-0.5f,  0.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f);	
			glVertex3f(0.5f,  0.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f);	
			glVertex3f(0.5f,  -1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0f);	
			glVertex3f(-0.5f,  -1.0f, 0.0f);
		glEnd();
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	glEndList();

	DrawScene(FALSE);

	return 0;
}

/* OpenGL code */

///////////////////////////////////////////////////////////////////////////////
// Function:	resize
// Purpose:		This code handles the windows resize for OpenGL
// Arguments:	Width and heights of the view window
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::resize( GLsizei width, GLsizei height )
{
// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
///////////////////////////////////////////////////////////////////////////////

    glViewport(0, 0, width, height);

    aspect = (GLfloat)width/(GLfloat)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(20.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);
	m_ScreenWidth = width;
	m_ScreenHeight = height;

}    
//// resize /////////////////////////////////////////////////////////////////

GLvoid COGLView::initializeGL(GLsizei width, GLsizei height)
{
/// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
	GLfloat ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightpos[] = { 0.5f, 0.5f, 1.0f, 0.0f };
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    aspect = (GLfloat)width/(GLfloat)height;
	// Establish viewing volume
	gluPerspective(60.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_FILL);
	glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
	glPointSize(8.0);		// NICE BEEFY POINTS FOR THE VERTEX SELECTION
	glEnable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR, specular);
	glMaterialf(GL_FRONT,GL_SHININESS, 2.0f);
	glLightfv(GL_LIGHT0,GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);

}


///////////////////////////////////////////////////////////////////////////////
// Function:	DrawModelUV
// Purpose:		Draw the Mesh model using UV coordinates
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawModelUV(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////

	if (visual->vertex != NULL)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);

		glDisable(GL_TEXTURE_2D);
		// Do not allow bilinear filtering 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
			glBegin(GL_TRIANGLES);
				glColor3f(visual->texture[face->t[0]].u,visual->texture[face->t[0]].v,1.0f);	
				glVertex3fv(&visual->vertex[face->v[0]].x);
				glColor3f(visual->texture[face->t[1]].u,visual->texture[face->t[1]].v,1.0f);	
				glVertex3fv(&visual->vertex[face->v[1]].x);
				glColor3f(visual->texture[face->t[2]].u,visual->texture[face->t[2]].v,1.0f);	
				glVertex3fv(&visual->vertex[face->v[2]].x);
			glEnd();
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawSelectionBuffer
// Purpose:		Draw the scene for use as a selection buffer
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawSelectionBuffer()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (m_Camera.rot.y  > 360.0f) m_Camera.rot.y  -= 360.0f;
	if (m_Camera.rot.x   > 360.0f) m_Camera.rot.x   -= 360.0f;
	if (m_Camera.rot.z > 360.0f) m_Camera.rot.z -= 360.0f;
	
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

	glPushMatrix();

	// Set camera's orientation and position
	glTranslatef(m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	DrawModelUV(&m_Model);

	glPopMatrix();

	//	SwapBuffers(m_hDC);		// I dont want to show this

	if (m_SelectBuffer)
	{
		free(m_SelectBuffer);
	}
	m_SelectBuffer = (float *)malloc(sizeof(float) * m_ScreenWidth * m_ScreenHeight * 3);

	glFlush();

	glReadBuffer(GL_BACK);
	glReadPixels(0,0, m_ScreenWidth, m_ScreenHeight, GL_RGB, GL_FLOAT,m_SelectBuffer);

}
//// DrawSelectionBuffer //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawPickTri
// Purpose:		Draw the Mesh model using UV coordinates
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawPickTri(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////

	if (visual->vertex != NULL)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);

		glDisable(GL_TEXTURE_2D);
		// Do not allow bilinear filtering 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
			glColor3ub(loop % 256,loop / 256,0);	
			glBegin(GL_TRIANGLES);
				glVertex3fv(&visual->vertex[face->v[0]].x);
				glVertex3fv(&visual->vertex[face->v[1]].x);
				glVertex3fv(&visual->vertex[face->v[2]].x);
			glEnd();
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawPickBuffer
// Purpose:		Draw the scene for use as a selection buffer
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawPickBuffer()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (m_Camera.rot.y  > 360.0f) m_Camera.rot.y  -= 360.0f;
	if (m_Camera.rot.x   > 360.0f) m_Camera.rot.x   -= 360.0f;
	if (m_Camera.rot.z > 360.0f) m_Camera.rot.z -= 360.0f;
	
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

	glPushMatrix();

	// Set camera's orientation and position
	glTranslatef(m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	DrawPickTri(&m_Model);

	glPopMatrix();

	if (m_ByteBuffer)
	{
		free(m_ByteBuffer);
	}
	m_ByteBuffer = (unsigned char *)malloc(sizeof(unsigned char) * m_ScreenWidth * m_ScreenHeight * 3);

	glFlush();

	glReadBuffer(GL_BACK);
	glReadPixels(0,0, m_ScreenWidth, m_ScreenHeight, GL_RGB, GL_UNSIGNED_BYTE,m_ByteBuffer);

}
//// DrawPickBuffer //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawVertexPos
// Purpose:		Draw the Mesh model using UV coordinates
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawVertexPos(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////

	if (visual->vertex != NULL)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);

		glDisable(GL_TEXTURE_2D);
		// Do not allow bilinear filtering 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
			glBegin(GL_TRIANGLES);
				glColor3f(1.0f, 0.0f, 0.0f); // Vertex 1 is red	
				glVertex3fv(&visual->vertex[face->v[0]].x);
				glColor3f(0.0f, 1.0f, 0.0f); // Vertex 2 is green
				glVertex3fv(&visual->vertex[face->v[1]].x);
				glColor3f(0.0f, 0.0f, 1.0f); // Vertex 3 is blue	
				glVertex3fv(&visual->vertex[face->v[2]].x);
			glEnd();
		}

/*		face = visual->index[triNumber];
		v1 = &visual->vertex[face->v[0]].x;
		v2 = &visual->vertex[face->v[0]].x;
		v3 = &visual->vertex[face->v[0]].x;

		click.x = (v1.x * vColor.r) + (v2.x * vColor.g) + (v3.x * vColor.b);
		click.y = (v1.y * vColor.r) + (v2.y * vColor.g) + (v3.y * vColor.b);
		click.z = (v1.z * vColor.r) + (v2.z * vColor.g) + (v3.z * vColor.b);
*/

	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawVertexPositionBuffer
// Purpose:		Draw the scene for use as a selection buffer
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawVertexPositionBuffer()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (m_Camera.rot.y  > 360.0f) m_Camera.rot.y  -= 360.0f;
	if (m_Camera.rot.x   > 360.0f) m_Camera.rot.x   -= 360.0f;
	if (m_Camera.rot.z > 360.0f) m_Camera.rot.z -= 360.0f;
	
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

	glPushMatrix();

	// Set camera's orientation and position
	glTranslatef(m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	DrawVertexPos(&m_Model);

	glPopMatrix();

	if (m_SelectBuffer)
	{
		free(m_SelectBuffer);
	}
	m_SelectBuffer = (float *)malloc(sizeof(float) * m_ScreenWidth * m_ScreenHeight * 3);

	glFlush();

	glReadBuffer(GL_BACK);
	glReadPixels(0,0, m_ScreenWidth, m_ScreenHeight, GL_RGB, GL_FLOAT,m_SelectBuffer);

}
//// DrawVertexPositionBuffer //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawParticles
// Purpose:		Draw the Model particles
// Arguments:	Pointer to Visual
///////////////////////////////////////////////////////////////////////////////
void COGLView::DrawParticles(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tParticle *particle;
///////////////////////////////////////////////////////////////////////////////
	if (!m_Model.particleCnt) return;

	particle = &m_Model.particle[0];

	glDepthMask(FALSE);
	for (int loop = 0; loop < m_Model.particleCnt; loop++, particle++)
	{
		glPushMatrix();
			glTranslatef(particle->pos.x,particle->pos.y,particle->pos.z);
	
			glRotatef(particle->rot.y, 0.0f, 1.0f, 0.0f);
			glRotatef(particle->rot.x, 1.0f, 0.0f, 0.0f);
			glRotatef(particle->rot.z, 0.0f, 0.0f, 1.0f); 

			glScalef(particle->size,particle->size,particle->size);
			glColor3fv((float *)&particle->color);	
			glBindTexture(GL_TEXTURE_2D, m_ParticleTexture);
			glCallList(OGL_PARTICLE_DLIST);

		glPopMatrix();
	}
	glDepthMask(TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawSkeleton
// Purpose:		Actually draws the Skeleton it is recursive
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawSkeleton(t_Bone *rootBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
	t_Bone *curBone;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone;
	glPushMatrix();

	// Set base orientation and position
	glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

	// DO THE ROTATION
	glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f); 

	// DO I WISH TO DISPLAY ANYTHING
//	if (m_DrawBoneSystem)
	{
		// DRAW THE AXIS OGL OBJECT
		glCallList(OGL_AXIS_DLIST);
	}

	// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
	if (curBone->childCnt > 0)
	{
		for (int i = 0; i < curBone->childCnt; i++)
			DrawSkeleton(curBone->children[i]);
	}
	glPopMatrix();	// THIS POPS THE WHOLE MATRIX
}
//// DrawSkeleton /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawModel
// Purpose:		Draw the Mesh model in paint mode
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawModel(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////

	if (visual->vertex != NULL)
	{
		if (m_UseLighting)
		{
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, visual->glTex);

		if (m_Bilinear)
		{
			// Allow bilinear filtering 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else
		{
			// Do not allow bilinear filtering 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}	


		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
			if (loop == m_SelectedTri)
				glColor3f(1.0f, 0.5f, 0.5f);	
			else
				// No modulation for this pass
				glColor3f(1.0f, 1.0f, 1.0f);	

			glBegin(GL_TRIANGLES);
				glTexCoord2f(visual->texture[face->t[0]].u,visual->texture[face->t[0]].v);	
				glNormal3fv(&visual->normal[face->n[0]].x);
				glVertex3fv(&visual->vertex[face->v[0]].x);
				glTexCoord2f(visual->texture[face->t[1]].u,visual->texture[face->t[1]].v);	
				glNormal3fv(&visual->normal[face->n[1]].x);
				glVertex3fv(&visual->vertex[face->v[1]].x);
				glTexCoord2f(visual->texture[face->t[2]].u,visual->texture[face->t[2]].v);	
				glNormal3fv(&visual->normal[face->n[2]].x);
				glVertex3fv(&visual->vertex[face->v[2]].x);
			glEnd();
		}

	}

	// Handle any attached particles
	DrawParticles(visual);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawScene
// Purpose:		Actually draw the OpenGL Scene
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DrawScene(BOOL draglines)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (m_Camera.rot.y  > 360.0f) m_Camera.rot.y  -= 360.0f;
	if (m_Camera.rot.x   > 360.0f) m_Camera.rot.x   -= 360.0f;
	if (m_Camera.rot.z > 360.0f) m_Camera.rot.z -= 360.0f;
	
    glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
	glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

	glPushMatrix();

	// Set camera's orientation and position
	glTranslatef(m_Camera.pos.x, m_Camera.pos.y, m_Camera.pos.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	DrawModel(&m_Model);
	
	if (m_Skeleton) 
		DrawSkeleton(m_Skeleton);

	glPushMatrix();
		glTranslatef(m_ParticlePos.x,m_ParticlePos.y,m_ParticlePos.z);
		glCallList(OGL_AXIS_DLIST);
	glPopMatrix();

	glPopMatrix();

	if (draglines && m_SelectBuffer && m_DrawMode == MODE_PARTICLE)	// When nothing held, draw
	{
		glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR
		glMatrixMode(GL_PROJECTION);			// I WANT TO PLAY WITH THE PROJECTION
		glPushMatrix();							// SAVE THE OLD ONE
			glLoadIdentity();					// LOAD A NEW ONE
			gluOrtho2D(0,m_ScreenWidth,0,m_ScreenHeight);	// USE WINDOW SETTINGS
			glColor3f(1.0f, 1.0f, 1.0f);	// X AXIS STARTS - COLOR RED
			glBegin(GL_LINES);
//				glVertex3f(m_ParticlePos.x,m_ParticlePos.y,m_ParticlePos.z);
//				glVertex3f(0,0,0);
				glVertex2s(m_MousePos.x,m_ScreenHeight - m_MousePos.y);
				glVertex2s(m_DragPos.x,m_ScreenHeight - m_DragPos.y);
			glEnd();
		glPopMatrix();							// RESTORE THE OLD PROJECTION
	    glMatrixMode(GL_MODELVIEW);				// BACK TO MODEL MODE
		glEnable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR
	}
	//    glFinish();

	SwapBuffers(m_hDC);

}
//// drawScene //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetClickPos
// Purpose:		GetCoordinates for a click
// Arguments:	triangle, vertex offsets of click
// Returns:		3D point in space
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::GetClickPos(int tri, float *click,tVector *pos)
{
/// Local Variables ///////////////////////////////////////////////////////////
	t_faceIndex *face;
	tVector vertex1,vertex2,vertex3;
///////////////////////////////////////////////////////////////////////////////

	if (m_Model.vertex != NULL)
	{
		face = &m_Model.index[tri];

		ScaleVector((tVector *)&m_Model.vertex[face->v[0]].x,click[0],&vertex1);
		ScaleVector((tVector *)&m_Model.vertex[face->v[1]].x,click[1],&vertex2);
		ScaleVector((tVector *)&m_Model.vertex[face->v[2]].x,click[2],&vertex3);

		VectorSum(&vertex1,&vertex2,pos);
		VectorSum(pos,&vertex3,pos);
	}
}

void COGLView::OnDestroy() 
{
	CWnd::OnDestroy();
	if (m_hRC)
		wglDeleteContext(m_hRC);
    if (m_hDC)
		::ReleaseDC(m_hWnd,m_hDC);
    m_hRC = 0;
    m_hDC = 0;
	
	
}

void COGLView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawScene(FALSE);
	// Do not call CWnd::OnPaint() for painting messages
}

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	float *picked;
	unsigned char *byte;
	int tri;
	// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_MousePos = point;
	m_Dragging = TRUE;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.pos.x;
	m_Grab_Trans_Y = 	m_Camera.pos.y;
	m_Grab_Trans_Z = 	m_Camera.pos.z;
	if (m_SelectBuffer && m_DrawMode == MODE_PARTICLE)
	{
		byte = (unsigned char *)&m_ByteBuffer[(((m_ScreenHeight - point.y) * m_ScreenWidth) + point.x) * 3];
		if (byte[2] == 0)
		{
			tri = byte[0] + (byte[1] * 256);
//			m_SelectedTri = tri;
			picked = (float *)&m_SelectBuffer[(((m_ScreenHeight - point.y) * m_ScreenWidth) + point.x) * 3];
			GetClickPos(tri, picked, &m_ParticlePos);
			DrawScene(FALSE);
		}
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_MousePos = point;
	m_Dragging = TRUE;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.pos.x;
	m_Grab_Trans_Y = 	m_Camera.pos.y;
	m_Grab_Trans_Z = 	m_Camera.pos.z;
	CWnd::OnRButtonDown(nFlags, point);
}

void COGLView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if (m_DrawMode == MODE_PARTICLE && (nFlags & MK_CONTROL) == 0)
	{
		AddParticle();
		DrawScene(FALSE);
	}
	else
	{
		// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
		m_MousePos = point;
		m_Dragging = TRUE;
		m_Grab_Rot_X = 	m_Camera.rot.x;
		m_Grab_Rot_Y = 	m_Camera.rot.y;
		m_Grab_Rot_Z = 	m_Camera.rot.z;
		m_Grab_Trans_X = 	m_Camera.pos.x;
		m_Grab_Trans_Y = 	m_Camera.pos.y;
		m_Grab_Trans_Z = 	m_Camera.pos.z;
	}
	CWnd::OnMButtonDown(nFlags, point);
}

void COGLView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_Dragging = FALSE;
	if (m_DrawMode == MODE_PAINT)
		DrawSelectionBuffer();	// Once it is rotated, get the screen rendering
	else if (m_DrawMode == MODE_PARTICLE)
	{
		DrawPickBuffer();	// Once it is rotated, get the screen rendering
		DrawVertexPositionBuffer();	// And one for the vertices
	}
	CWnd::OnMButtonUp(nFlags, point);
}

void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	float *picked;
	m_DragPos = point;
	if (m_Dragging && m_SelectBuffer && m_DrawMode == MODE_PAINT)
	{
		picked = (float *)&m_SelectBuffer[(((m_ScreenHeight - point.y) * m_ScreenWidth) + point.x) * 3];
		if (picked[2] == 1.0f)		// Make sure it is not the background
		{
			if ((nFlags & MK_CONTROL) > 0)
			{
				char buffer[80];
				sprintf(buffer,"UV %.4f %.4f %.4f",picked[0],picked[1],picked[2]);
				MessageBox(buffer,"Picked");
			}	
			else if ((nFlags & MK_SHIFT) > 0)
			{
				GetTextureColor(picked[0], picked[1]);
			}	
			else
			{
				DrawTexture(picked[0], picked[1]);
			}
		}
	}
	m_Dragging = FALSE;
	CWnd::OnLButtonUp(nFlags, point);
}

void COGLView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_Dragging = FALSE;
	if (m_DrawMode == MODE_PAINT)
		DrawSelectionBuffer();	// Once it is rotated, get the screen rendering
	else if (m_DrawMode == MODE_PARTICLE)
	{
		DrawPickBuffer();	// Once it is rotated, get the screen rendering
		DrawVertexPositionBuffer();	// And one for the vertices
	}
	CWnd::OnRButtonUp(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnMouseMove
// Purpose:		Handler for the mouse.  Handles movement when pressed
// Arguments:	Flags for key masks and point
///////////////////////////////////////////////////////////////////////////////
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	float *picked;
	if (!m_Dragging) return;
	m_DragPos = point;

//	UpdateStatusBar(0);
	if ((nFlags & MK_CONTROL) > 0)
	{
		// IF I AM HOLDING THE RM BUTTON + CTRL Translate IN XZ
		if ((nFlags & MK_LBUTTON) > 0 && (nFlags & MK_MBUTTON) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_MousePos.x) != 0)	// Move Camera in X
			{
				m_Camera.pos.x = m_Grab_Trans_X + (.1f * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)
			{
				m_Camera.pos.z = m_Grab_Trans_Z + (.1f * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}
		// IF I AM HOLDING THE RM BUTTON + SHIFT Translate IN XY
		else if ((nFlags & MK_MBUTTON) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_MousePos.x) != 0)	// Move Camera in X
			{
				m_Camera.pos.x = m_Grab_Trans_X + (.05f * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)	// Move Camera in Y
			{
				m_Camera.pos.y = m_Grab_Trans_Y - (.05f * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}
		else if ((nFlags & MK_LBUTTON) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_MousePos.x) != 0)	// Rotate Camera in Y
			{
				m_Camera.rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)	// Rotate Camera in X
			{
				m_Camera.rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}
		else if ((nFlags & MK_RBUTTON) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_MousePos.x) != 0)	// Move Camera in X
			{
				m_Camera.pos.x = m_Grab_Trans_X + (.1f * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)
			{
				m_Camera.pos.z = m_Grab_Trans_Z + (.1f * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}
	}
	else if (nFlags & MK_LBUTTON > 0)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}	
		else if ((nFlags & MK_SHIFT) > 0)
		{
		}
		else if (m_SelectBuffer && m_DrawMode == MODE_PAINT)	// When nothing held, draw
		{
			picked = (float *)&m_SelectBuffer[(((m_ScreenHeight - point.y) * m_ScreenWidth) + point.x) * 3];
			if (picked[2] == 1.0f)		// Make sure it is not the background
				DrawTexture(picked[0], picked[1]);
		}
		else if (m_SelectBuffer && m_DrawMode == MODE_PARTICLE)	// When nothing held, draw
		{
			DrawScene(TRUE);
		}

	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_MousePos.x) != 0)	// Move Camera in X
			{
				m_Camera.pos.x = m_Grab_Trans_X + (.05f * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)	// Move Camera in Y
			{
				m_Camera.pos.y = m_Grab_Trans_Y - (.05f * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}	
		else 
		{
			UpdateStatusBar(1);
			if ((point.x - m_MousePos.x) != 0)	// Rotate Camera in Y
			{
				m_Camera.rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_MousePos.x));
				DrawScene(FALSE);
			}
			if ((point.y - m_MousePos.y) != 0)	// Rotate Camera in X
			{
				m_Camera.rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_MousePos.y));
				DrawScene(FALSE);
			}
		}	
	}
	else if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
	}
	
	CWnd::OnMouseMove(nFlags, point);
}
//// OnMouseMove //////////////////////////////////////////////////////

void COGLView::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	resize( x,y );
	
}

// 0 = READY
// 1 = ROTATE
// 2 = TRANSLATE
void COGLView::UpdateStatusBar(int mode) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char message[80];
///////////////////////////////////////////////////////////////////////////////
	if (mode == 1)
	{
		sprintf(message,"Rotate (%.2f,%.2f,%.2f)",m_Camera.rot.x,m_Camera.rot.y,m_Camera.rot.z);
	}
	else if (mode == 2)
	{
		sprintf(message,"Translate (%.2f,%.2f,%.2f)",m_Camera.pos.x,m_Camera.pos.y,m_Camera.pos.z);
	}
	else
	{
		strcpy(message,"Ready");
	}
	m_StatusBar->SetPaneText(0,message);
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

///////////////////////////////////////////////////////////////////////////////
// Function:	GetModelBoundaries
// Purpose:		Calculate the min and max size of the model
// Arguments:	min and max, and size vectors
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::GetModelBoundaries(tVector *min, tVector *max,tVector *size)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	tVector *vertex;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)
	{
		vertex = m_Model.vertex;
		// Initialize the values with the first vector;
		memcpy(min,vertex,sizeof(tVector));
		memcpy(max,vertex,sizeof(tVector));
		for (loop = 0; loop < m_Model.vertexCnt; loop++,vertex++)
		{
			if (vertex->x > max->x) max->x = vertex->x;
			if (vertex->y > max->y) max->y = vertex->y;
			if (vertex->z > max->z) max->z = vertex->z;

			if (vertex->x < min->x) min->x = vertex->x;
			if (vertex->y < min->y) min->y = vertex->y;
			if (vertex->z < min->z) min->z = vertex->z;
		}
		// Get the size of the object
		VectorDifference(max, min, size);
	}

}

///////////////////////////////////////////////////////////////////////////////
// Function:	SphereMapModel
// Purpose:		Calculate UV coordinates for model using spherical projection
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::SphereMapModel()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loop2;
	tVector vect;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)
	{
		face = m_Model.index;
		for (loop = 0; loop < m_Model.faceCnt; loop++,face++)
		{
			for (loop2 = 0; loop2 < 3; loop2++)
			{
				face->t[loop2] = loop * 3 + loop2;	// Redo all texture coordinate pointers
				// Calculate the U coordinate 
				m_Model.texture[face->t[loop2]].u = atan2(m_Model.vertex[face->v[loop2]].x,m_Model.vertex[face->v[loop2]].z) / PI_TIMES_TWO + 0.5;
				// Calculate the V coordinate
				MAKEVECTOR(vect,m_Model.vertex[face->v[loop2]].x,m_Model.vertex[face->v[loop2]].y, m_Model.vertex[face->v[loop2]].z)
				NormalizeVector(&vect);
				// Take the Arcsin of Y and scale to 0-1 range
				m_Model.texture[face->t[loop2]].v = asin(vect.y) / M_PI + 0.5;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CylinderMapModel
// Purpose:		Calculate UV coordinates for model using cylindrical projection
// Arguments:	Scale and Offset
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::CylinderMapModel(float scale, float offset)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loop2;
	tVector vect;
	t_faceIndex *face;
	tVector min, max, size;
///////////////////////////////////////////////////////////////////////////////

	if (m_Model.vertex != NULL)
	{
		GetModelBoundaries(&min, &max, &size);	// Get the size of the object

		face = m_Model.index;
		for (loop = 0; loop < m_Model.faceCnt; loop++,face++)
		{
			for (loop2 = 0; loop2 < 3; loop2++)
			{
				face->t[loop2] = loop * 3 + loop2;	// Redo all texture coordinate pointers
				MAKEVECTOR(vect,m_Model.vertex[face->v[loop2]].x,m_Model.vertex[face->v[loop2]].y, m_Model.vertex[face->v[loop2]].z)
				NormalizeVector(&vect);
				m_Model.texture[face->t[loop2]].u = (atan2(vect.x,vect.z) / PI_TIMES_TWO) + 0.5f;		// Added 0.5 to the value to move it from -.5 -> .5  to 0.0 -> 1.0
				m_Model.texture[face->t[loop2]].v = ((m_Model.vertex[face->v[loop2]].y - min.y) / size.y) * scale + offset;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	PlanarMapModel
// Purpose:		Calculate UV coordinates for model using planar projection
// Arguments:	Projection Type, scales and offsets
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::PlanarMapModel(int mapType, float scaleU, float offsetU, float scaleV, float offsetV)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loop2;
	t_faceIndex *face;
	tVector min, max, size;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)
	{
		GetModelBoundaries(&min, &max, &size);	// Get the size of the object

		face = m_Model.index;
		for (loop = 0; loop < m_Model.faceCnt; loop++,face++)
		{
			for (loop2 = 0; loop2 < 3; loop2++)
			{
				face->t[loop2] = loop * 3 + loop2;	// Redo all texture coordinate pointers
				switch (mapType)
				{
				case XY_PLANE:
					m_Model.texture[face->t[loop2]].u = ((m_Model.vertex[face->v[loop2]].x - min.x) / size.x) * scaleU + offsetU;
					m_Model.texture[face->t[loop2]].v = ((m_Model.vertex[face->v[loop2]].y - min.y) / size.y) * scaleV + offsetV;
					break;
				case XZ_PLANE:
					m_Model.texture[face->t[loop2]].u = ((m_Model.vertex[face->v[loop2]].x - min.x) / size.x) * scaleU + offsetU;
					m_Model.texture[face->t[loop2]].v = ((m_Model.vertex[face->v[loop2]].z - min.z) / size.z) * scaleV + offsetV;
					break;
				case YZ_PLANE:
					m_Model.texture[face->t[loop2]].u = ((m_Model.vertex[face->v[loop2]].z - min.z) / size.z) * scaleU + offsetU;
					m_Model.texture[face->t[loop2]].v = ((m_Model.vertex[face->v[loop2]].y - min.y) / size.y) * scaleV + offsetV;
					break;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	FixupMapCoords
// Purpose:		Run through the mesh and fix any coordinates that are problems
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::FixupMapCoords()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_faceIndex *face;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)
	{
		// Fix up any texture coordinates that span the seam
		face = m_Model.index;
		for (loop = 0; loop < m_Model.faceCnt; loop++,face++)
		{

			if ((m_Model.texture[face->t[0]].u - m_Model.texture[face->t[1]].u) > 0.9f)
					m_Model.texture[face->t[1]].u += 1.0f;

			if ((m_Model.texture[face->t[1]].u - m_Model.texture[face->t[0]].u) > 0.9f)
					m_Model.texture[face->t[0]].u += 1.0f;

			if ((m_Model.texture[face->t[0]].u - m_Model.texture[face->t[2]].u) > 0.9f)
					m_Model.texture[face->t[2]].u += 1.0f;

			if ((m_Model.texture[face->t[2]].u - m_Model.texture[face->t[0]].u) > 0.9f)
					m_Model.texture[face->t[0]].u += 1.0f;

			if ((m_Model.texture[face->t[1]].u - m_Model.texture[face->t[2]].u) > 0.9f)
					m_Model.texture[face->t[2]].u += 1.0f;

			if ((m_Model.texture[face->t[2]].u - m_Model.texture[face->t[1]].u) > 0.9f)
					m_Model.texture[face->t[1]].u += 1.0f;

			if ((m_Model.texture[face->t[0]].v - m_Model.texture[face->t[1]].v) > 0.8f)
					m_Model.texture[face->t[1]].v += 1.0f;

			if ((m_Model.texture[face->t[1]].v - m_Model.texture[face->t[0]].v) > 0.8f)
					m_Model.texture[face->t[0]].v += 1.0f;

			if ((m_Model.texture[face->t[0]].v - m_Model.texture[face->t[2]].v) > 0.8f)
					m_Model.texture[face->t[2]].v += 1.0f;

			if ((m_Model.texture[face->t[2]].v - m_Model.texture[face->t[0]].v) > 0.8f)
					m_Model.texture[face->t[0]].v += 1.0f;

			if ((m_Model.texture[face->t[1]].v - m_Model.texture[face->t[2]].v) > 0.8f)
					m_Model.texture[face->t[2]].v += 1.0f;

			if ((m_Model.texture[face->t[2]].v - m_Model.texture[face->t[1]].v) > 0.8f)
					m_Model.texture[face->t[1]].v += 1.0f;

		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	FixupMapCoords
// Purpose:		Run through the mesh and fix any coordinates that are problems
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
void COGLView::AddParticle()
{
/// Local Variables ///////////////////////////////////////////////////////////
	tParticle *particle;
///////////////////////////////////////////////////////////////////////////////
	// Max Reached
	if (m_Model.particleCnt == MAX_PARTICLES)
		return;
	particle = &m_Model.particle[m_Model.particleCnt];
	memcpy(&particle->pos, &m_ParticlePos, sizeof(tVector));

	particle->rot.x = -m_Camera.rot.x;
	particle->rot.y = -m_Camera.rot.y;
	particle->rot.z = 0.0f;

	MAKEVECTOR(particle->color,(float)GETR(m_PaintColor)/255.0f,
								(float)GETG(m_PaintColor)/255.0f,
								(float)GETB(m_PaintColor)/255.0f)

	// Calculate Squared Distance of mouse drag for size
	int dist = (m_DragPos.x - m_MousePos.x) * (m_DragPos.x - m_MousePos.x) +
			(m_DragPos.y - m_MousePos.y) * (m_DragPos.y - m_MousePos.y);

	// If not dragged default to size of 1 and no rotation
	if (dist == 0)
		particle->size = 1.0f;
	else
	{
		particle->size = sqrt((float)dist) / 50.0f;	// Arbitrary scaling term

		// Get the Z rotation based on the movement of the cursor
		tVector v1,v2,cross;
		MAKEVECTOR(v1,0.0f,1.0f, 0.0f)
		MAKEVECTOR(v2,m_DragPos.x - m_MousePos.x,m_DragPos.y - m_MousePos.y, 0.0f)

		NormalizeVector(&v2);
		float dot = DotProduct(&v1, &v2);
		particle->rot.z = RADTODEG(acos(dot));

		CrossProduct(&v1,&v2,&cross);
		if (cross.z > 0) 
			particle->rot.z = -particle->rot.z;
	}

	m_Model.particleCnt++;
}

void COGLView::DeleteParticle()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.particleCnt > 0 ) m_Model.particleCnt--;
}

void COGLView::HandleKeyUp(UINT nChar) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	switch (nChar)
	{
	case VK_SPACE:
		if (m_DrawMode == MODE_PARTICLE)
			AddParticle();
		DrawScene(FALSE);
		break;
	case VK_DELETE:
		if (m_DrawMode == MODE_PARTICLE)
			DeleteParticle();
		DrawScene(FALSE);
		break;
	case 'F':
		glPolygonMode(GL_FRONT,GL_FILL);
		break;
	case 'L':
		glPolygonMode(GL_FRONT,GL_LINE);
		break;
	case 'T':
		DrawPickBuffer();
		SwapBuffers(m_hDC);		// I dont want to show this
		break;
	case 'P':
		DrawVertexPositionBuffer();
		SwapBuffers(m_hDC);		// I dont want to show this
		break;

	}


}

///////////////////////////////////////////////////////////////////////////////
// Function:	GetGLInfo
// Purpose:		Get the OpenGL Vendor and Renderer
///////////////////////////////////////////////////////////////////////////////
void COGLView::GetGLInfo() 
{
//// Local Variables ////////////////////////////////////////////////////////////////
	char *who, *which, *ver, *ext, *message;
	int len;
/////////////////////////////////////////////////////////////////////////////////////
	who = (char *)::glGetString( GL_VENDOR );
	which = (char *)::glGetString( GL_RENDERER );
	ver = (char *)::glGetString( GL_VERSION );
	ext = (char *)::glGetString( GL_EXTENSIONS );

	len = 200 + strlen(who) + strlen(which) + strlen(ver) + strlen(ext);

	message = (char *)malloc(len);
	sprintf(message,"Who:\t%s\nWhich:\t%s\nVersion:\t%s\nExtensions:\t%s",
		who, which, ver, ext);

	::MessageBox(NULL,message,"GL Info",MB_OK);

	free(message);
}
//// GetGLInfo /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadOBJModel
// Purpose:		Load an OBJ Model into the system
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::LoadOBJModel(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	char texname[255],*pos;
	tTGAHeader_s header;
	unsigned char *texture;

///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)	// Free model data if exists
	{
		free(m_Model.vertex);
		m_Model.vertex = NULL;
	}
	LoadOBJ((LPCSTR)name,&m_Model);
	
	if (strlen(m_Model.map) > 0)
	{
		pos = m_Model.map + strlen(m_Model.map);
		while (*pos != '/' && pos != m_Model.map)
			pos--;

		if (*pos == '/') pos++;

		sprintf(texname,"%s",pos);

		texture = LoadTGAFile( texname,	&header);  //

		if (texture)
		{
			
			m_Model.texWidth = header.d_width;
			m_Model.texHeight = header.d_height;
			m_Model.texDepth = header.d_pixel_size / 8;	// Number of bytes in color

			// GENERATE THE OPENGL TEXTURE ID
			glGenTextures(1,&m_Model.glTex);

			glBindTexture(GL_TEXTURE_2D, m_Model.glTex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			m_Model.texData = texture;

			/*
			* Define the 2D texture image.
			*/

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

			if (header.d_pixel_size == 32)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 4, header.d_width, header.d_height, 0,
						 GL_RGBA , GL_UNSIGNED_BYTE, texture);

			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, 3, header.d_width, header.d_height, 0,
						 GL_RGB , GL_UNSIGNED_BYTE, texture);

			}

			if (m_Model.particle)
				free(m_Model.particle);
			m_Model.particle = (tParticle *)malloc(sizeof(tParticle) * MAX_PARTICLES);
			m_Model.particleCnt = 0;

			if (m_DrawMode == MODE_PAINT)
				DrawSelectionBuffer();	// Once it is rotated, get the screen rendering
			else if (m_DrawMode == MODE_PARTICLE)
			{
				DrawPickBuffer();	// Draw one for the triangles
				DrawVertexPositionBuffer();	// And one for the vertices
			}
		}

	}
	else
		m_Model.glTex = 0;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveOBJModel
// Purpose:		Save an OBJ Model
// Arguments:	Name of the file to save
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::SaveOBJModel(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertex != NULL)	// Free model data if exists
	{
		SaveOBJ((LPCSTR)name,&m_Model);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadParticleSystem
// Purpose:		Load the particles
// Arguments:	name of the file
///////////////////////////////////////////////////////////////////////////////
void COGLView::LoadParticleSystem(char *name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE		*fp;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(name,"rb");
	if (fp)
	{
		fread(&m_Model.particleCnt,sizeof(int),1,fp);
		fread(m_Model.particle,sizeof(tParticle),m_Model.particleCnt,fp);
		fclose(fp);
	}
	DrawScene(FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveParticleSystem 
// Purpose:		Load the texture map to put on the particles
// Arguments:	name of the file
///////////////////////////////////////////////////////////////////////////////
void COGLView::SaveParticleSystem(char *name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE		*fp;
///////////////////////////////////////////////////////////////////////////////
	if (!m_Model.particleCnt) return;
	
	fp = fopen(name,"wb");
	if (fp)
	{
		fwrite(&m_Model.particleCnt,sizeof(int),1,fp);
		fwrite(m_Model.particle,sizeof(tParticle),m_Model.particleCnt,fp);
		fclose(fp);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Function:	DrawTexture
// Purpose:		Modify the texture based on drawn UV coords
// Arguments:	UV coordinates to modify
///////////////////////////////////////////////////////////////////////////////
void COGLView::DrawTexture(float u, float v)
{
	int iu = (int)((u * (m_Model.texWidth - 1)) + 0.5f),
		iv = (int)((v * (m_Model.texHeight - 1)) + 0.5f);
	unsigned char *texture;

	if (m_Model.texData)
	{
		texture = &m_Model.texData[((iv * m_Model.texWidth) + iu) * m_Model.texDepth];
		texture[0] = GETR(m_PaintColor);
		texture[1] = GETG(m_PaintColor);
		texture[2] = GETB(m_PaintColor);

		glBindTexture(GL_TEXTURE_2D, m_Model.glTex);

		if (m_Model.texDepth == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Model.texWidth, m_Model.texHeight, 0,
					 GL_RGBA , GL_UNSIGNED_BYTE, m_Model.texData);

		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_Model.texWidth, m_Model.texHeight, 0,
					 GL_RGB , GL_UNSIGNED_BYTE, m_Model.texData);

		}
	}
	DrawScene(FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	GetTexture Color
// Purpose:		Get the Color selected
// Arguments:	UV coordinates to modify
///////////////////////////////////////////////////////////////////////////////
void COGLView::GetTextureColor(float u, float v)
{
	int iu = (int)((u * (m_Model.texWidth - 1)) + 0.5f),
		iv = (int)((v * (m_Model.texHeight - 1)) + 0.5f);
	unsigned char *texture;

	if (m_Model.texData)
	{
		texture = &m_Model.texData[((iv * m_Model.texWidth) + iu) * m_Model.texDepth];
		m_PaintColor = MAKERGB(texture[0],texture[1],texture[2]);
	}
	DrawScene(FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveTexture 
// Purpose:		Save the texture map to a TGA file
// Arguments:	name of the file
///////////////////////////////////////////////////////////////////////////////
void COGLView::SaveTexture(char *filename)
{
	if (m_Model.texData)
	{
		SaveTGAFile( filename,	m_Model.texWidth, m_Model.texHeight, m_Model.texDepth, m_Model.texData);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadTexture 
// Purpose:		Load the texture map
// Arguments:	name of the file, GL int to store the texture
///////////////////////////////////////////////////////////////////////////////
void COGLView::LoadTexture(char *filename,unsigned int *textureID)
{
	tTGAHeader_s header;
	unsigned char *texture;

	texture = LoadTGAFile( filename,	&header);
	if (texture)
	{
		// GENERATE THE OPENGL TEXTURE ID
		glGenTextures(1,textureID);

		glBindTexture(GL_TEXTURE_2D, *textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		/*
		* Define the 2D texture image.
		*/

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

		if (header.d_pixel_size == 32)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, header.d_width, header.d_height, 0,
					 GL_RGBA , GL_UNSIGNED_BYTE, texture);

		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 3, header.d_width, header.d_height, 0,
					 GL_RGB , GL_UNSIGNED_BYTE, texture);

		}
		free(texture);
	}
	DrawScene(FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadParticleTexture 
// Purpose:		Load the texture map to put on the particles
// Arguments:	name of the file
///////////////////////////////////////////////////////////////////////////////
void COGLView::LoadParticleTexture(char *name)
{
	LoadTexture(name,&m_ParticleTexture);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadSkeleton 
// Purpose:		Load a D3D skeletal file
// Arguments:	name of the file
// Notes:		This opens a simply ASCII skeleton format
///////////////////////////////////////////////////////////////////////////////
void COGLView::LoadSkeleton(char *name)
{
	if (!LoadD3DSkeleton(&m_Skeleton,name))
		MessageBox("Couldn't open Skeleton File","ERROR");
}
