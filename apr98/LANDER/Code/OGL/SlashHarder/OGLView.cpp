///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Quaternion Animation System
//
// Created:
//		JL 11/1/97		
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
#include "Slash.h"
#include "OGLView.h"
#include "SetRot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
	// INITIALIZE THE MODE KEYS
	m_DrawGeometry = TRUE;
	m_UseQuat = TRUE;
	m_Skeleton = NULL;
	m_AnimBlend = 0.0;
}

COGLView::~COGLView()
{
}

void COGLView::UpdateStatus()
{
/// Local Variables ///////////////////////////////////////////////////////////
    char message[80];
///////////////////////////////////////////////////////////////////////////////

	if (m_UseQuat)
		m_ptrStatusBar->SetPaneText(1,"Quaternion Mode");
	else
		m_ptrStatusBar->SetPaneText(1,"Euler Mode");
	
	sprintf(message,"Rot (%.1f,%.1f,%.1f)",
		m_CurBone->rot.x,
		m_CurBone->rot.y,
		m_CurBone->rot.z);
	m_ptrStatusBar->SetPaneText(2,message);

	sprintf(message,"Q (%.2f,%.2f,%.2f) %.2f",
		m_CurBone->quat.x,
		m_CurBone->quat.y,
		m_CurBone->quat.z,
		m_CurBone->quat.w);
	m_ptrStatusBar->SetPaneText(3,message);
}

BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, t_Bone *skeleton,CSlider *slider, CCreateContext* pContext) 
{
	m_Slider = slider;
	m_Skeleton = skeleton;
	// SET THE STARTING BONE TO BE THE UPPER ARM
	m_CurBone = m_Skeleton->children;

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


	drawScene();
	return 0;
}

/* OpenGL code */
GLvoid COGLView::resize( GLsizei width, GLsizei height )
{
// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
///////////////////////////////////////////////////////////////////////////////

    glViewport(0, 0, width, height);

    aspect = (GLfloat)width/(GLfloat)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, aspect, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
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
    gluPerspective(40.0, aspect, 1.0f, 2000.0f);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_FILL);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
}

// GET THE INFO ON THE VERSION OF OPENGL RUNNING
void COGLView::GetGLInfo(char *who,char *which, char *version) 
{
    strcpy(who,(char *)::glGetString( GL_VENDOR ));

    strcpy(which,(char *)::glGetString( GL_RENDERER ));

    strcpy(version, (char *)::glGetString( GL_VERSION ));
}

GLvoid COGLView::drawModel(t_Bone *curBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (curBone->desc != NULL)
	{
		// NOT HANDLING TEXTURE TYPES IN THIS DEMO
		switch (curBone->desc->type)
		{
		case 0:
			// JUST MESH NO VERTEX COLORS
			glInterleavedArrays(GL_V3F,0,(GLvoid *)curBone->desc->frame[curBone->desc->cur_frame]->data);
			break;
		case 1:
			// VERTEX COLORS
			glInterleavedArrays(GL_C3F_V3F,0,(GLvoid *)curBone->desc->frame[curBone->desc->cur_frame]->data);
			break;
		}

		glDrawArrays(GL_TRIANGLES,0,curBone->desc->pointCnt);
		glDisable(GL_TEXTURE_2D);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawSkeleton
// Purpose:		Actually draws the Skeleton it is recursive
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawSkeleton(t_Bone *rootBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
	tQuaternion primaryQuat,secondaryQuat,axisAngle;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		glPushMatrix();

		// Set base orientation and position
		glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

		EulerToQuaternion(&curBone->p_rot,&primaryQuat);
		EulerToQuaternion(&curBone->s_rot,&secondaryQuat);
		SlerpQuat(&primaryQuat,&secondaryQuat,m_AnimBlend,&curBone->quat);

		if (m_UseQuat)
		{
			// QUATERNION HAS TO BE CONVERTED TO AN AXIS/ANGLE REPRESENTATION
			QuatToAxisAngle(&curBone->quat,&axisAngle);
			// DO THE ROTATION
			glRotatef(axisAngle.w, axisAngle.x, axisAngle.y, axisAngle.z); 
		}
		else
		{
			// Set observer's orientation and position
			glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f); 
			glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
			glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f);

		}
	
		// THE SCALE IS LOCAL SO I PUSH AND POP
		glPushMatrix();
		glScalef(curBone->scale.x, curBone->scale.y, curBone->scale.z); 

		// DRAW THE LITTLE AXIS
		glCallList(OGL_AXIS_DLIST);

		// IF THERE IS A MODEL, DRAW IT
		if (curBone->desc != NULL)
			drawModel(curBone);
		
		glPopMatrix();	// THIS POP IS JUST FOR THE SCALE

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			drawSkeleton(curBone);

		glPopMatrix();	// THIS POPS THE WHOLE MATRIX

		// NEXT BONE
		curBone++;
	}
}
//// drawSkeleton /////////////////////////////////////////////////////////////

GLvoid COGLView::drawScene()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	// GET THE INTERPOLATION VALUE FROM MY CUSTOM SLIDER CONTROL
	m_AnimBlend = m_Slider->GetSetting();

	if (m_Skeleton->rot.y  > 360.0f) m_Skeleton->rot.y  -= 360.0f;
    if (m_Skeleton->rot.x   > 360.0f) m_Skeleton->rot.x   -= 360.0f;
    if (m_Skeleton->rot.z > 360.0f) m_Skeleton->rot.z -= 360.0f;
	
    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Skeleton->trans.x, m_Skeleton->trans.y, m_Skeleton->trans.z);

	glRotatef(m_Skeleton->rot.z, 0.0f, 0.0f, 1.0f);
    glRotatef(m_Skeleton->rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_Skeleton->rot.x, 1.0f, 0.0f, 0.0f); 

	// DO THE REST OF THE SYSTEM
	drawSkeleton(m_Skeleton);

    glPopMatrix();

    glFinish();

    SwapBuffers(m_hDC);

	// DRAW THE STATS AT THE BOTTOM OF THE SCREEN
	UpdateStatus();
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
	drawScene();

	// Do not call CWnd::OnPaint() for painting messages
}

void COGLView::OnSize(UINT nType, int cx, int cy) 
{
	// RESIZE THE OPENGL WINDOW
	resize( cx,cy );
}

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_Slider->SetPos(0);	// I HAVE ADJUSTED SO RESET INTERPOLATOR
	m_mousepos = point;
	// SAVE THE HIT SETTINGS SO I CAN PLAY WITH THEM
	m_Grab_Rot_X = 	m_CurBone->rot.x;
	m_Grab_Rot_Y = 	m_CurBone->rot.y;
	m_Grab_Rot_Z = 	m_CurBone->rot.z;
	m_Grab_Trans_X = 	m_Skeleton->trans.x;
	m_Grab_Trans_Y = 	m_Skeleton->trans.y;
	m_Grab_Trans_Z = 	m_Skeleton->trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	m_Slider->SetPos(0);	// I HAVE ADJUSTED SO RESET INTERPOLATOR
	// SAVE THE HIT SETTINGS SO I CAN PLAY WITH THEM
	m_Grab_Rot_X = 	m_CurBone->rot.x;
	m_Grab_Rot_Y = 	m_CurBone->rot.y;
	m_Grab_Rot_Z = 	m_CurBone->rot.z;
	m_Grab_Trans_X = 	m_Skeleton->trans.x;
	m_Grab_Trans_Y = 	m_Skeleton->trans.y;
	m_Grab_Trans_Z = 	m_Skeleton->trans.z;
	CWnd::OnRButtonDown(nFlags, point);
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

void COGLView::HandleKeyUp(UINT nChar) 
{
	switch (nChar)
	{
	case '1':
		// SELECT THE UPPER ARM AS THE CURRENT BONE
		m_CurBone = &m_Skeleton->children[0];
		break;
	case '2':
		// SELECT THE LOWER ARM AS THE CURRENT BONE
		m_CurBone = &m_Skeleton->children[1];
		break;
	case '3':
		// SELECT THE HAND AS THE CURRENT BONE
		m_CurBone = &m_Skeleton->children[2];
		break;
	}
	drawScene();
}

void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (nFlags & MK_LBUTTON > 0)
	{
		// IF I AM HOLDING THE 'CTRL' BUTTON TRANSLATE
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Skeleton->trans.y = m_Grab_Trans_Y - (.1f * (point.y - m_mousepos.y));
				drawScene();
			}
		}	
		// ELSE ROTATE THE ROOT
		else
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_CurBone->rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_CurBone->rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene();
			}
			// SET THE NEW START KEYFRAME POSITION
			m_CurBone->p_rot.x = m_CurBone->rot.x;
			m_CurBone->p_rot.y = m_CurBone->rot.y;
			m_CurBone->p_rot.z = m_CurBone->rot.z;
			m_AnimBlend = 0.0;
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton->trans.z = m_Grab_Trans_Z + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
		}
		else
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_CurBone->rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
			// SET THE NEW START KEYFRAME POSITION
			m_CurBone->p_rot.x = m_CurBone->rot.x;
			m_CurBone->p_rot.y = m_CurBone->rot.y;
			m_CurBone->p_rot.z = m_CurBone->rot.z;
			m_AnimBlend = 0.0;
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}
// IF YOU DOUBLE CLICK, BRING UP A DIALOG TO EDIT THE CURRENT BONE ORIENTATION
void COGLView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CSetRot dialog;
	dialog.m_XAxis = m_CurBone->rot.x;
	dialog.m_YAxis = m_CurBone->rot.y;
	dialog.m_ZAxis = m_CurBone->rot.z;
	if (dialog.DoModal())
	{
		m_CurBone->rot.x = dialog.m_XAxis;
		m_CurBone->rot.y = dialog.m_YAxis;
		m_CurBone->rot.z = dialog.m_ZAxis;
	}
	drawScene();
}

// LOCK IN THE CURRENT BONE ORIENTATION AS ITS SECONDARY TARGET
void COGLView::SetEndKey()
{
	m_CurBone->s_rot.x = m_CurBone->rot.x;
	m_CurBone->s_rot.y = m_CurBone->rot.y;
	m_CurBone->s_rot.z = m_CurBone->rot.z;
	m_AnimBlend = 0.0;
}

