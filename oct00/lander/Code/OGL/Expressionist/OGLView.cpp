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
#define OGL_SELECTED_DLIST	2		// SELECTED BONE OPENGL DISPLAY LIST
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
	m_Bilinear	= FALSE;
	m_Dragging = FALSE;
	m_UseLighting = FALSE;

	// INITIALIZE SOME OF THE CAMERA VARIABLES
	ResetBone(&m_Camera, NULL);
	m_Camera.id = -1;
	strcpy(m_Camera.name,"Camera");
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = 0.0f;
	m_Camera.b_trans.z = -20.0f;
	m_Camera.trans.y = 0.0f;
	m_Camera.trans.z = -20.0f;

	m_Model.vertex = NULL;

	m_SelectBuffer = NULL;

	m_PaintColor = MAKERGB(255, 255, 255);	// Default paint color
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
	glEndList();

	drawScene();

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
	glMaterialf(GL_FRONT,GL_SHININESS, 25.0f);
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
	glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

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
//// drawScene //////////////////////////////////////////////////////

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

		// No modulation for this pass
		glColor3f(1.0f, 1.0f, 1.0f);	

		face = visual->index;
		for (loop = 0; loop < visual->faceCnt; loop++,face++)
		{
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
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawScene
// Purpose:		Actually draw the OpenGL Scene
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawScene()
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
	glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	DrawModel(&m_Model);

	glPopMatrix();

	//    glFinish();

	SwapBuffers(m_hDC);

}
//// drawScene //////////////////////////////////////////////////////

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

	drawScene();
	// Do not call CWnd::OnPaint() for painting messages
}

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Dragging = TRUE;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.trans.x;
	m_Grab_Trans_Y = 	m_Camera.trans.y;
	m_Grab_Trans_Z = 	m_Camera.trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Dragging = TRUE;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.trans.x;
	m_Grab_Trans_Y = 	m_Camera.trans.y;
	m_Grab_Trans_Z = 	m_Camera.trans.z;
	CWnd::OnRButtonDown(nFlags, point);
}

void COGLView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE HIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Dragging = TRUE;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.trans.x;
	m_Grab_Trans_Y = 	m_Camera.trans.y;
	m_Grab_Trans_Z = 	m_Camera.trans.z;
	CWnd::OnMButtonDown(nFlags, point);
}

void COGLView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_Dragging = FALSE;
	DrawSelectionBuffer();	// Once it is rotated, get the screen rendering
	CWnd::OnMButtonUp(nFlags, point);
}

void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	float *picked;
	if (m_Dragging && m_SelectBuffer)
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
	DrawSelectionBuffer();	// Once it is rotated, get the screen rendering
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

//	UpdateStatusBar(0);
	if (nFlags & MK_LBUTTON > 0)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}	
		else if ((nFlags & MK_SHIFT) > 0)
		{
		}
		else if (m_SelectBuffer)	// When nothing held, draw
		{
			picked = (float *)&m_SelectBuffer[(((m_ScreenHeight - point.y) * m_ScreenWidth) + point.x) * 3];
			if (picked[2] == 1.0f)		// Make sure it is not the background
				DrawTexture(picked[0], picked[1]);
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		// IF I AM HOLDING THE RM BUTTON + CTRL Translate IN XZ
		if ((nFlags & MK_CONTROL) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)	// Move Camera in X
			{
				m_Camera.trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Camera.trans.z = m_Grab_Trans_Z + (.1f * (point.y - m_mousepos.y));
				drawScene();
			}
		}
		// IF I AM HOLDING THE RM BUTTON + SHIFT Translate IN XY
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)	// Move Camera in X
			{
				m_Camera.trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)	// Move Camera in Y
			{
				m_Camera.trans.y = m_Grab_Trans_Y - (.1f * (point.y - m_mousepos.y));
				drawScene();
			}
		}
		else
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)	// Rotate Camera in Y
			{
				m_Camera.rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)	// Rotate Camera in X
			{
				m_Camera.rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene();
			}
		}
	}
	else if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		UpdateStatusBar(2);
		if ((point.x - m_mousepos.x) != 0)	// Move Camera in X
		{
			m_Camera.trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
			drawScene();
		}
		if ((point.y - m_mousepos.y) != 0)
		{
			m_Camera.trans.z = m_Grab_Trans_Z + (.1f * (point.y - m_mousepos.y));
			drawScene();
		}
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
		sprintf(message,"Translate (%.2f,%.2f,%.2f)",m_Camera.trans.x,m_Camera.trans.y,m_Camera.trans.z);
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
// Arguments:	Pointer to the model
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
// Arguments:	Pointer to the model
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
// Arguments:	Pointer to the model
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
// Arguments:	Pointer to the model
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

void COGLView::HandleKeyUp(UINT nChar) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	switch (nChar)
	{
	case VK_SPACE:
		break;
	}

	Invalidate(TRUE);

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
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = 0.0f;
	m_Camera.b_trans.z = -20.0f;
	m_Camera.trans.y = 0.0f;
	m_Camera.trans.z = -20.0f;

	
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
			DrawSelectionBuffer();	// Initialize the selection buffer
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
	drawScene();
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
	drawScene();
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

