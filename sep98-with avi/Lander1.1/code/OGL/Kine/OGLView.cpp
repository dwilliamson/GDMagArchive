///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Inverse Kinematics System
//
// Created:
//		JL 7/1/98		
//		JL 9/20/98   FIXED A BUG IN THE IK CALCULATIONS
//
// Notes:	The meat of this application is the last routine in this file.
//			"ComputeIK" takes a target point and solves the two bone system.
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
#include "math.h"
#include "Kine.h"
#include "OGLView.h"
#include "Quatern.h"
#include "Model.h"		// SOFTIMAGE MODEL DATA

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION
///////////////////////////////////////////////////////////////////////////////

/// Trig Macros ///////////////////////////////////////////////////////////////
#define DEGTORAD(A)	((A * M_PI) / 180.0f)
#define RADTODEG(A)	((A * 180.0f) / M_PI)
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
	// INITIALIZE THE MODE KEYS
	m_DrawGeometry = TRUE;

	// INITIALIZE SOME OF THE SKELETON VARIABLES
	ResetBone(&m_Body, NULL);
	m_Body.id = -1;
	strcpy(m_Body.name,"Body");

	ResetBone(&m_UpArm, NULL);
	m_UpArm.id = -1;
	strcpy(m_UpArm.name,"UpArm");
	m_UpArm.trans.x = 0.7f;
	m_UpArm.trans.y = 1.7f;

	ResetBone(&m_LowArm, NULL);
	m_LowArm.id = -1;
	strcpy(m_LowArm.name,"LowArm");
	m_LowArm.trans.x = 4.0f;

	// SET UP END EFFECTOR
	ResetBone(&m_Effector, NULL);
	m_Effector.id = -1;
	strcpy(m_Effector.name,"Effector");
	m_Effector.trans.x = 3.0f;

}

COGLView::~COGLView()
{
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	UpdateStatus
// Purpose:		Update the status bar with orientation info
///////////////////////////////////////////////////////////////////////////////		
void COGLView::UpdateStatus()
{
/// Local Variables ///////////////////////////////////////////////////////////
    char message[80];
///////////////////////////////////////////////////////////////////////////////

	// WRITE THE ORIENTATIONS OF THE TWO BONES IN THE WINDOW STATUS AREA
	sprintf(message,"UpArm Rot (%.2f)",
		m_UpArm.rot.z);
	m_ptrStatusBar->SetPaneText(1,message);

	sprintf(message,"LowArm Rot (%.2f)",
		m_LowArm.rot.z);
	m_ptrStatusBar->SetPaneText(2,message);
}

BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	UpdateStatus();	// DRAW INITIAL STATUS BAR
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BEGIN_MESSAGE_MAP(COGLView, CWnd)
	//{{AFX_MSG_MAP(COGLView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
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
		glPushMatrix();
		glScalef(2.0,2.0,2.0);
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


	drawScene();
	return 0;
}

/* OpenGL code */
GLvoid COGLView::resize( GLsizei width, GLsizei height )
{
// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
///////////////////////////////////////////////////////////////////////////////
	m_Width = width;
	m_Height = height;

    glViewport(0, 0, width, height);

    aspect = (GLfloat)width/(GLfloat)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	// FOR THIS APP, I WANT A 2D Ortho View
	gluOrtho2D(0.0f,(GLfloat)width,0.0f,(GLfloat)height);	// USE WINDOW SETTINGS
//    gluPerspective(10.0, aspect, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	m_ModelScale = (float)height / 6.0f;
	glScalef(m_ModelScale,m_ModelScale,0.0f);

}    

GLvoid COGLView::initializeGL(GLsizei width, GLsizei height)
{
/// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    aspect = (GLfloat)width/(GLfloat)height;
	// Establish viewing volume
	// FOR THIS APP, I WANT A 2D Ortho View
	gluOrtho2D(0.0f,(GLfloat)width,0.0f,(GLfloat)height);	// USE WINDOW SETTINGS
//	gluPerspective(10.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_FILL);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
}

// GET THE INFO ON THE VERSION OF OPENGL RUNNING
void COGLView::GetGLInfo(char *who,char *which, char *version, char *extensions) 
{
    strcpy(who,(char *)::glGetString( GL_VENDOR ));

    strcpy(which,(char *)::glGetString( GL_RENDERER ));

    strcpy(version, (char *)::glGetString( GL_VERSION ));
    strcpy(extensions, (char *)::glGetString( GL_EXTENSIONS ));
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	drawModel
// Purpose:		Draws the model associated with a bone
// Notes:		Currently uses a global model not associated with the bone
//              The data uses Quads with shared vertices and vertex coloring 
//				so I chose to use indexed vertex arrays
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::drawModel(t_Bone *curBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	// Declare the Array of Data
	glInterleavedArrays(UPARMFORMAT,0,(GLvoid *)&UPARM);
	// Draw all the Quads at once
	glDrawArrays(GL_TRIANGLES,0,UPARMPOLYCNT * 3);

}
// drawModel

///////////////////////////////////////////////////////////////////////////////
// Procedure:	drawScene
// Purpose:		Draws the current OpenGL scene
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::drawScene(GLvoid)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Body.trans.x, m_Body.trans.y, m_Body.trans.z);

	// ROTATE THE ROOT
	glRotatef(m_Body.rot.z, 0.0f, 0.0f, 1.0f);
    glRotatef(m_Body.rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_Body.rot.x, 1.0f, 0.0f, 0.0f); 

	// Declare the Array of Data
	glInterleavedArrays(BODYFORMAT,0,(GLvoid *)&BODY);
	// Draw all the Quads at once
	glDrawArrays(GL_TRIANGLES,0,BODYPOLYCNT * 3);


    // Set root skeleton's orientation and position
    glTranslatef(m_UpArm.trans.x, m_UpArm.trans.y, m_UpArm.trans.z);

	// ROTATE THE ROOT
	glRotatef(m_UpArm.rot.z, 0.0f, 0.0f, 1.0f);
    glRotatef(m_UpArm.rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_UpArm.rot.x, 1.0f, 0.0f, 0.0f); 

	// Declare the Array of Data
	glInterleavedArrays(UPARMFORMAT,0,(GLvoid *)&UPARM);
	// Draw all the Quads at once
	glDrawArrays(GL_TRIANGLES,0,UPARMPOLYCNT * 3);

	// DRAW THE AXIS
	glCallList(OGL_AXIS_DLIST);

    // Set root skeleton's orientation and position
    glTranslatef(m_LowArm.trans.x, m_LowArm.trans.y, m_LowArm.trans.z);

	// ROTATE THE ROOT
	glRotatef(m_LowArm.rot.z, 0.0f, 0.0f, 1.0f);
    glRotatef(m_LowArm.rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_LowArm.rot.x, 1.0f, 0.0f, 0.0f); 

	// Declare the Array of Data
	glInterleavedArrays(LOWARMFORMAT,0,(GLvoid *)&LOWARM);
	// Draw all the Quads at once
	glDrawArrays(GL_TRIANGLES,0,LOWARMPOLYCNT * 3);
    
	// DRAW THE AXIS
	glCallList(OGL_AXIS_DLIST);

    // Set root skeleton's orientation and position
    glTranslatef(m_Effector.trans.x, m_Effector.trans.y, m_Effector.trans.z);

	// DRAW THE AXIS
	glCallList(OGL_AXIS_DLIST);

	glPopMatrix();
    glFinish();

    SwapBuffers(m_hDC);

	// DRAW THE STATS AT THE BOTTOM OF THE SCREEN
	UpdateStatus();
}
// 	drawScene

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

void COGLView::OnSize(UINT nType, int cx, int cy) 
{
	// RESIZE THE OPENGL WINDOW
	resize( cx,cy );
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnLButtonDown
// Purpose:		Left button down grabs the current point pos so I can use it
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char mess[80];
	float radZ;
	CPoint joint1,joint2,effector;
///////////////////////////////////////////////////////////////////////////////
	m_mousepos = point;

	point.y = m_Height - point.y - 1;

	// IF YOU CLICK ANYWHERE, SOLVE THE SYSTEM
	if ((nFlags & MK_CONTROL) == 0)
	{
		// COMPUTE THE ROTATIONS NEEDED TO REACH THE POINT
		if (ComputeIK(point))
			drawScene();
		else // COULDN'T REACH IT, GIVE AN ERROR
			MessageBox("Point is not reachable","ERROR",MB_OK);
	}
	m_Grab_Rot_X = 	m_UpArm.rot.x;
	m_Grab_Rot_Y = 	m_UpArm.rot.y;
	m_Grab_Rot_Z = 	m_UpArm.rot.z;
	m_Grab_Trans_X = 	m_UpArm.trans.x;
	m_Grab_Trans_Y = 	m_UpArm.trans.y;
	m_Grab_Trans_Z = 	m_UpArm.trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnRButtonDown
// Purpose:		Right button down grabs the current point pos so I can use it
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	m_Grab_Rot_X = 	m_UpArm.rot.x;
	m_Grab_Rot_Y = 	m_UpArm.rot.y;
	m_Grab_Rot_Z = 	m_UpArm.rot.z;
	m_Grab_Trans_X = 	m_LowArm.trans.x;
	m_Grab_Trans_Y = 	m_LowArm.trans.y;
	m_Grab_Trans_Z = 	m_LowArm.trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

void COGLView::HandleKeyUp(UINT nChar) 
{
	switch (nChar)
	{
	case 'G':
		m_DrawGeometry = !m_DrawGeometry;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnMouseMove
// Purpose:		Handle mouse moves while pressed
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (nFlags & MK_LBUTTON > 0)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_UpArm.rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
		}
		else
		{
			point.y = m_Height - point.y - 1;
			if (ComputeIK(point))
				drawScene();
		}

	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_LowArm.rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnLButtonDblClk
// Purpose:		Left Double click, get dialog for Orientation
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	ComputeIK
// Purpose:		Compute an IK Solution to an end effector position
// Arguments:	End Target (x,y)
// Returns:		TRUE if a solution exists, FALSE if the position isn't in reach
// Notes:		There was a bug in this in the Sept Game Developer source
//				for this in the final angle calculation
///////////////////////////////////////////////////////////////////////////////		
BOOL COGLView::ComputeIK(CPoint endPos)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float l1,l2;		// BONE LENGTH FOR BONE 1 AND 2
	float ex,ey;		// ADJUSTED TARGET POSITION
	float sin2,cos2;	// SINE AND COSINE OF ANGLE 2
	float angle1,angle2;// ANGLE 1 AND 2 IN RADIANS
	float tan1;			// TAN OF ANGLE 1
///////////////////////////////////////////////////////////////////////////////

	// SUBTRACT THE INITIAL OFFSET FROM THE TARGET POS
	ex = endPos.x - (m_UpArm.trans.x * m_ModelScale);
	ey = endPos.y - (m_UpArm.trans.y * m_ModelScale);

	// MULTIPLY THE BONE LENGTHS BY THE WINDOW SCALE
	l1 = m_LowArm.trans.x * m_ModelScale;
	l2 = m_Effector.trans.x * m_ModelScale;
	
	// CALCULATE THE COSINE OF ANGLE 2
	cos2 = ((ex * ex) + (ey * ey) - (l1 * l1) - (l2 * l2)) / (2 * l1 * l2);

	// IF IT IS NOT IN THIS RANGE, IT IS UNREACHABLE
	if (cos2 >= -1.0 && cos2 <= 1.0)
	{
		angle2 = (float)acos(cos2);	// GET THE ANGLE WITH AN ARCCOSINE
		m_LowArm.rot.z = RADTODEG(angle2);	// CONVERT IT TO DEGREES

		sin2 = (float)sin(angle2);		// CALC THE SINE OF ANGLE 2

		// COMPUTE ANGLE 1
		// HERE IS WHERE THE BUG WAS SEE THE README.TXT FOR MORE INFO
		// CALCULATE THE TAN OF ANGLE 1
		tan1 = (-(l2 * sin2 * ex) + ((l1 + (l2 * cos2)) * ey)) / 
				  ((l2 * sin2 * ey) + ((l1 + (l2 * cos2)) * ex));
		// GET THE ACTUAL ANGLE
		angle1 = atan(tan1);
		m_UpArm.rot.z = RADTODEG(angle1);	// CONVERT IT TO DEGREES
		return TRUE;
	}
	else
		return FALSE;
}

