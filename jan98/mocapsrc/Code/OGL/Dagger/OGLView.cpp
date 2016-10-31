///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Hierarchical Animation System
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

#include "stdafx.h"
#include "Dagger.h"
#include "OGLView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ENABLE THE USE OF DISPLAY LISTS...
// FOR TESTING I CAN COMMENT THEM OUT
#define USE_DRAWLISTS		1

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
	m_Skeleton = NULL;
	m_StatusBar = NULL;	// CLEAR THIS.  IT IS SET BY MAINFRAME BUT UNTIL THEN MARK IT
}

COGLView::~COGLView()
{
}

BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, t_Bone *skeleton, CCreateContext* pContext) 
{
	m_Skeleton = skeleton;
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

///////////////////////////////////////////////////////////////////////////////
// Function:	DrawAxis
// Purpose:		Draws the Axis model using GL Lines
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
void COGLView::DrawAxis(void)
{
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
}
//// DrawAxis /////////////////////////////////////////////////////////////////

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

#if USE_DRAWLISTS

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

	// CREATE THE DISPLAY LIST THE SELECTED BONE JUST A CUBE
	glNewList(OGL_SELECTED_DLIST,GL_COMPILE);
		glBegin(GL_QUADS);
			glColor3f(1.0f, 1.0f, 0.0f);		// YELLOW
			// BOTTOM
			glVertex3f(-0.05f, -0.05f, -0.05f);
			glVertex3f( 0.05f, -0.05f, -0.05f);
			glVertex3f( 0.05f, -0.05f,  0.05f);
			glVertex3f(-0.05f, -0.05f,  0.05f);
			// BACK
			glVertex3f(-0.05f,  0.05f, -0.05f);
			glVertex3f( 0.05f,  0.05f, -0.05f);
			glVertex3f( 0.05f, -0.05f, -0.05f);
			glVertex3f(-0.05f, -0.05f, -0.05f);
			// FRONT
			glVertex3f(-0.05f, -0.05f,  0.05f);
			glVertex3f( 0.05f, -0.05f,  0.05f);
			glVertex3f( 0.05f,  0.05f,  0.05f);
			glVertex3f(-0.05f,  0.05f,  0.05f);
			// RIGHT
			glVertex3f(-0.05f, -0.05f, -0.05f);
			glVertex3f(-0.05f, -0.05f,  0.05f);
			glVertex3f(-0.05f,  0.05f,  0.05f);
			glVertex3f(-0.05f,  0.05f, -0.05f);
			// LEFT
			glVertex3f( 0.05f,  0.05f, -0.05f);
			glVertex3f( 0.05f,  0.05f,  0.05f);
			glVertex3f( 0.05f, -0.05f,  0.05f);
			glVertex3f( 0.05f, -0.05f, -0.05f);
			// TOP
			glVertex3f(-0.05f, 0.05f,  0.05f);
			glVertex3f( 0.05f, 0.05f,  0.05f);
			glVertex3f( 0.05f, 0.05f, -0.05f);
			glVertex3f(-0.05f, 0.05f, -0.05f);
		glEnd();
	glEndList();
#endif
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
    gluPerspective(45.0, aspect, 3.0, 7.0);
    glMatrixMode(GL_MODELVIEW);
}    
//// resize /////////////////////////////////////////////////////////////////

GLvoid COGLView::initializeGL(GLsizei width, GLsizei height)
{
/// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    aspect = (GLfloat)width/(GLfloat)height;
	// Establish viewing volume
	gluPerspective(10.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_FILL);
	glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
	glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawScene
// Purpose:		Actually draw the OpenGL Scene
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawScene(GLvoid)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
///////////////////////////////////////////////////////////////////////////////

	if (m_Skeleton->rot.y  > 360.0f) m_Skeleton->rot.y  -= 360.0f;
    if (m_Skeleton->rot.x   > 360.0f) m_Skeleton->rot.x   -= 360.0f;
    if (m_Skeleton->rot.z > 360.0f) m_Skeleton->rot.z -= 360.0f;
	
    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Skeleton->trans.x, m_Skeleton->trans.y, m_Skeleton->trans.z);

    glRotatef(m_Skeleton->rot.x, 1.0f, 0.0f, 0.0f);
    glRotatef(m_Skeleton->rot.y, 0.0f, 1.0f, 0.0f);
    glRotatef(m_Skeleton->rot.z, 0.0f, 0.0f, 1.0f); 

	curBone = m_Skeleton->children;
	for (loop = 0; loop < m_Skeleton->childCnt; loop++)
	{
		glPushMatrix();

		glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

		// Set observer's orientation and position
		glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f);
		glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f); 

#if USE_DRAWLISTS
		// DRAW THE AXIS OGL OBJECT
		glCallList(OGL_AXIS_DLIST);
		// IF SOMETHING IS SELECTED, DRAW TAG BOX
		if (m_Skeleton->id == (long)curBone)
			glCallList(OGL_SELECTED_DLIST);
#else
		// DRAW THE AXIS OGL OBJECT USING DRAW ROUTINE
		DrawAxis();
#endif

		glPopMatrix();

		curBone++;
	}

    glPopMatrix();
    glFinish();

    SwapBuffers(m_hDC);

	UpdateStatusBarFrameInfo();
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
	// STORE OFF THE KIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Grab_Rot_X = 	m_Skeleton->rot.x;
	m_Grab_Rot_Y = 	m_Skeleton->rot.y;
	m_Grab_Rot_Z = 	m_Skeleton->rot.z;
	m_Grab_Trans_X = 	m_Skeleton->trans.x;
	m_Grab_Trans_Y = 	m_Skeleton->trans.y;
	m_Grab_Trans_Z = 	m_Skeleton->trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE KIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Grab_Rot_X = 	m_Skeleton->rot.x;
	m_Grab_Rot_Y = 	m_Skeleton->rot.y;
	m_Grab_Rot_Z = 	m_Skeleton->rot.z;
	m_Grab_Trans_X = 	m_Skeleton->trans.x;
	m_Grab_Trans_Y = 	m_Skeleton->trans.y;
	m_Grab_Trans_Z = 	m_Skeleton->trans.z;
	CWnd::OnRButtonDown(nFlags, point);
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
		strcpy(message,"Rotate");
	}
	else if (mode == 2)
	{
		strcpy(message,"Translate");
	}
	else
	{
		strcpy(message,"Ready");
	}
	m_StatusBar->SetPaneText(0,message);
}

void COGLView::UpdateStatusBarFrameInfo() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char message[80];
///////////////////////////////////////////////////////////////////////////////
	if (m_StatusBar != NULL && m_Skeleton->children != NULL)
	{
		sprintf(message,"Frame %3d/%3d",(int)m_Skeleton->children->primCurFrame,(int)m_Skeleton->children->primFrameCount);
		//m_StatusBar->SetPaneStyle(1,SBPS_POPOUT);
		m_StatusBar->SetPaneText(1,message);
	}
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

void COGLView::HandleKeyUp(UINT nChar) 
{
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnMouseMove
// Purpose:		Handler for the mouse.  Handles movement when pressed
// Arguments:	Flags for key masks and point
///////////////////////////////////////////////////////////////////////////////
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	UpdateStatusBar(0);
	if (nFlags & MK_LBUTTON > 0)
	{
		// IF I AM HOLDING THE 'CTRL' BUTTON TRANSLATE
		if ((nFlags & MK_CONTROL) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->trans.x = m_Grab_Trans_X + (.05f * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Skeleton->trans.y = m_Grab_Trans_Y - (.05f * (point.y - m_mousepos.y));
				drawScene();
			}
		}	
		// ELSE "SHIFT" ROTATE THE ROOT
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Skeleton->rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene();
			}
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->trans.z = m_Grab_Trans_Z + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
		}
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
}
//// OnMouseMove //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	OnViewResetskeleton
// Purpose:		Reset the view settings for the skeleton
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
void COGLView::OnViewResetskeleton() 
{
	m_Skeleton->trans.x = 0.0f;
	m_Skeleton->trans.y = -3.0f;
	m_Skeleton->trans.z = -40.0f;
	m_Skeleton->rot.x = 0.0f;
	m_Skeleton->rot.y = 0.0f;
	m_Skeleton->rot.z = 0.0f;
	drawScene();	
}
//// OnViewResetskeleton //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetGLInfo
// Purpose:		Get the OpenGL Vendor and Rederer info as strings
// Arguments:	Strings to put the info in
///////////////////////////////////////////////////////////////////////////////
void COGLView::GetGLInfo(char *who,char *which, char *version) 
{
    strcpy(who,(char *)::glGetString( GL_VENDOR ));

    strcpy(which,(char *)::glGetString( GL_RENDERER ));

    strcpy(version, (char *)::glGetString( GL_VERSION ));
}
//// GetGLInfo /////////////////////////////////////////////////////////////////
