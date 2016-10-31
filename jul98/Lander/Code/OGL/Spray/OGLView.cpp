///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Particle System
//
// Created:
//		JL  2/18/98
// Revisions:
//		Integrated into Particle System Demo		4/14/98
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Spray.h"
#include "OGLView.h"
#include "particle.h"
#include "EditEmit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
// IF YOU WANT TO TRY A QUICKER TWO BONE WEIGHTING SYSTEM UNDEF THIS
#define DEFORM_GENERAL_SOLUTION		// FULL MULTI-BONE DEFORMATION
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0f		// SPEED OF ROTATION
#define ANGLE_SPEED			0.01f		// SPEED OF ANGLE ROTATION
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// Message Maps //////////////////////////////////////////////////////////////
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
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	initParticleSystem();
	initEmitter(&m_Emitter);
	m_CurrentEmitter = &m_Emitter;
	m_DrawAxis = TRUE;				// DRAW ORIGIN AXIS
	m_DrawSystem = TRUE;			// ANIMATE THE PARTICLES
	m_AntiAlias = TRUE;				// ANTIALIAS THE PARTICLES

	m_ViewRot.x = 0.0;
	m_ViewRot.y = 0.0;
	m_ViewRot.z = 0.0;
}

COGLView::~COGLView()
{
}

BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	UpdateStatus();	// DRAW INITIAL STATUS BAR
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void COGLView::UpdateStatus()
{
/// Local Variables ///////////////////////////////////////////////////////////
    char message[80];
///////////////////////////////////////////////////////////////////////////////
	sprintf(message,"# %d",m_Emitter.particleCount);

	m_ptrStatusBar->SetPaneText(1,message);

}

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
//		glScalef(1.0,1.0,1.0);
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

	drawScene(FALSE);
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
    gluPerspective(35.0, aspect, 1.0, 2000.0);
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
	gluPerspective(40.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_LINE);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
	glPointSize(1.0);		// JUST 1 PIXEL DOTS FOR NOW
}

// GET THE INFO ON THE VERSION OF OPENGL RUNNING
void COGLView::GetGLInfo(char *who,char *which, char *version) 
{
    strcpy(who,(char *)::glGetString( GL_VENDOR ));

    strcpy(which,(char *)::glGetString( GL_RENDERER ));

    strcpy(version, (char *)::glGetString( GL_VERSION ));
}


GLvoid COGLView::drawScene(BOOL drawSelectRect)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	
    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Emitter.pos.x, m_Emitter.pos.y, m_Emitter.pos.z);

	glRotatef(m_ViewRot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(m_ViewRot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_ViewRot.x, 1.0f, 0.0f, 0.0f); 
	
	// DRAW THE AXIS OGL OBJECT
	if (m_DrawAxis)
		glCallList(OGL_AXIS_DLIST);

	renderEmitter(&m_Emitter,m_AntiAlias);

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
	drawScene(FALSE);

	// Do not call CWnd::OnPaint() for painting messages
}

void COGLView::OnSize(UINT nType, int cx, int cy) 
{
	// RESIZE THE OPENGL WINDOW
	resize( cx,cy );
	m_ScreenWidth = cx;
	m_ScreenHeight = cy;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	IdleFunc
// Purpose:		Process state changes if animating
///////////////////////////////////////////////////////////////////////////////
void COGLView::IdleFunc() 
{
	if (m_DrawSystem)
	{
		updateEmitter(&m_Emitter);
		drawScene(FALSE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Input Functions
///////////////////////////////////////////////////////////////////////////////

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	if (m_CurrentEmitter != NULL)
	{
		m_Grab_Pitch = 	m_CurrentEmitter->pitch;
		m_Grab_Yaw = 	m_CurrentEmitter->yaw;
		m_Grab_Rot_X = 	m_ViewRot.x;
		m_Grab_Rot_Y = 	m_ViewRot.y;
		m_Grab_Rot_Z = 	m_ViewRot.z;
		m_Grab_Trans_X = 	m_CurrentEmitter->pos.x;
		m_Grab_Trans_Y = 	m_CurrentEmitter->pos.y;
		m_Grab_Trans_Z = 	m_CurrentEmitter->pos.z;
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_SHIFT) > 0)
	{
		drawScene(FALSE);
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	if (m_CurrentEmitter != NULL)
	{
		m_Grab_Pitch = 	m_CurrentEmitter->pitch;
		m_Grab_Yaw = 	m_CurrentEmitter->yaw;
		m_Grab_Rot_X = 	m_ViewRot.x;
		m_Grab_Rot_Y = 	m_ViewRot.y;
		m_Grab_Rot_Z = 	m_ViewRot.z;
		m_Grab_Trans_X = 	m_CurrentEmitter->pos.x;
		m_Grab_Trans_Y = 	m_CurrentEmitter->pos.y;
		m_Grab_Trans_Z = 	m_CurrentEmitter->pos.z;
	}
	CWnd::OnRButtonDown(nFlags, point);
}

void COGLView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_SHIFT) > 0)
	{
		drawScene(FALSE);
	}
	
	
	CWnd::OnRButtonUp(nFlags, point);
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

void COGLView::HandleKeyUp(UINT nChar) 
{
	switch (nChar)
	{
	case ' ':			// MANUALLY STEP THE SYSTEM
		m_DrawSystem = FALSE;
		updateEmitter(&m_Emitter);
		break;
	case 13:
		m_DrawSystem = !m_DrawSystem;
		break;
	case 'D':
		m_DrawAxis = !m_DrawAxis;
		break;
	case 'E':
		editEmitter(&m_Emitter);
		break;
	case 'O':
		glPolygonMode(GL_FRONT,GL_LINE);
		break;
	case 'F':
		glPolygonMode(GL_FRONT,GL_FILL);
		break;
	case 'A':
		m_AntiAlias = !m_AntiAlias;
		break;
	}
	drawScene(FALSE);
}

void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_CurrentEmitter != NULL)
	{
		if (nFlags & MK_LBUTTON > 0)
		{
			// IF I AM HOLDING THE 'CTRL' BUTTON TRANSLATE
			if ((nFlags & MK_CONTROL) > 0)
			{
				if ((point.x - m_mousepos.x) != 0)
				{
					m_CurrentEmitter->pos.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
					drawScene(FALSE);
				}
				if ((point.y - m_mousepos.y) != 0)
				{
					m_CurrentEmitter->pos.y = m_Grab_Trans_Y - (.1f * (point.y - m_mousepos.y));
					drawScene(FALSE);
				}
			}	
			// ELSE ROTATE THE ROOT
			else if ((nFlags & MK_SHIFT) > 0)
			{
				if ((point.x - m_mousepos.x) != 0)
				{
					m_ViewRot.y = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
					drawScene(FALSE);
				}
			}
		}
		else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
		{
			if ((nFlags & MK_CONTROL) > 0)
			{
				if ((point.x - m_mousepos.x) != 0)
				{
					m_CurrentEmitter->pos.z = m_Grab_Trans_Z + (.1f * (point.x - m_mousepos.x));
					drawScene(FALSE);
				}
			}
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

// Double click runs editEmitter
void COGLView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	editEmitter(&m_Emitter);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	editEmitter
// Purpose:		editEmitter settings
// Arguments:	The Emitter to update
// Notes:		Allows you to tweak the emitter settings
///////////////////////////////////////////////////////////////////////////////
void COGLView::editEmitter(tEmitter *emitter)
{
/// Local Variables ///////////////////////////////////////////////////////////
	CEditEmit dial;
///////////////////////////////////////////////////////////////////////////////
	dial.m_TotalParticles = emitter->totalParticles;
	dial.m_emits = emitter->emitsPerFrame;
	dial.m_emitVar = emitter->emitVar;
	dial.m_forceX = emitter->force.x;
	dial.m_forceY = emitter->force.y;
	dial.m_forceZ = emitter->force.z;
	dial.m_life = emitter->life;
	dial.m_lifeVar = emitter->lifeVar;
	dial.m_startColorB = emitter->startColor.b;
	dial.m_startColorBVar = emitter->startColorVar.b;
	dial.m_startColorG = emitter->startColor.g;
	dial.m_startColorGVar = emitter->startColorVar.g;
	dial.m_startColorR = emitter->startColor.r;
	dial.m_startColorRVar = emitter->startColorVar.r;
	dial.m_endColorB = emitter->endColor.b;
	dial.m_endColorBVar = emitter->endColorVar.b;
	dial.m_endColorG = emitter->endColor.g;
	dial.m_endColorGVar = emitter->endColorVar.g;
	dial.m_endColorR = emitter->endColor.r;
	dial.m_endColorRVar = emitter->endColorVar.r;
	dial.m_speed = emitter->speed;
	dial.m_speedVar = emitter->speedVar;
	dial.m_yaw = RADTODEG(emitter->yaw);		// DISPLAY IN DEGREES SINCE IT MAKES SENSE
	dial.m_yawVar = RADTODEG(emitter->yawVar);	
	dial.m_pitch = RADTODEG(emitter->pitch);	
	dial.m_pitchVar = RADTODEG(emitter->pitchVar);
	dial.m_name = emitter->name;
	if (dial.DoModal())
	{
		emitter->totalParticles	= dial.m_TotalParticles;
		emitter->emitsPerFrame = dial.m_emits;
		emitter->emitVar = dial.m_emitVar;
		emitter->force.x = dial.m_forceX;
		emitter->force.y = dial.m_forceY;
		emitter->force.z = dial.m_forceZ;
		emitter->life = dial.m_life;
		emitter->lifeVar = dial.m_lifeVar;
		emitter->startColor.b = dial.m_startColorB;
		emitter->startColorVar.b = dial.m_startColorBVar;
		emitter->startColor.g = dial.m_startColorG;
		emitter->startColorVar.g = dial.m_startColorGVar;
		emitter->startColor.r = dial.m_startColorR;
		emitter->startColorVar.r = dial.m_startColorRVar;
		emitter->endColor.b = dial.m_endColorB;
		emitter->endColorVar.b = dial.m_endColorBVar;
		emitter->endColor.g = dial.m_endColorG;
		emitter->endColorVar.g = dial.m_endColorGVar;
		emitter->endColor.r = dial.m_endColorR;
		emitter->endColorVar.r = dial.m_endColorRVar;
		emitter->speed = dial.m_speed;
		emitter->speedVar = dial.m_speedVar;
		emitter->yaw = DEGTORAD(dial.m_yaw);		// CONVERT TO RADIANS
		emitter->yawVar = DEGTORAD(dial.m_yawVar);
		emitter->pitch = DEGTORAD(dial.m_pitch);
		emitter->pitchVar = DEGTORAD(dial.m_pitchVar);
		strcpy(emitter->name,dial.m_name);
	}
}
/// editEmitter ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	resetEmitter
// Purpose:		Reset emitter to initial settings
///////////////////////////////////////////////////////////////////////////////
void COGLView::resetEmitter()
{
	setDefaultEmitter(&m_Emitter);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	GetEmitter
// Purpose:		Get the Emitter Settings from a file
// Arguments:	Filename to get it from
// Returns:		Success
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::GetEmitter(CString filename)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tParticle	*particle;					// NULL TERMINATED LINKED LIST
	FILE *fp;
///////////////////////////////////////////////////////////////////////////////
	particle = m_Emitter.particle;	// SAVE THIS SINCE IT IS THE ACTIVE LIST
	fp = fopen(filename,"rb");
	if (fp != NULL)
	{
		fread(&m_Emitter,sizeof(tEmitter),1,fp);
		// LOAD THE VIEWING ANGLE ALSO
		fread(&m_ViewRot.z,sizeof(float),1,fp);
		fread(&m_ViewRot.y,sizeof(float),1,fp);
		fread(&m_ViewRot.x,sizeof(float),1,fp);
		fclose(fp);
		m_Emitter.particle = particle;	// RESTORE THE ACTIVE LIST
		drawScene(FALSE);
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveEmitter
// Purpose:		Save the Emitter settings to a file
// Arguments:	Filename to put it in
// Returns:		Success
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::SaveEmitter(CString filename)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"wb");
	if (fp != NULL)
	{
		fwrite(&m_Emitter,sizeof(tEmitter),1,fp);
		// SAVE OFF THE VIEWING ANGLE ALSO
		fwrite(&m_ViewRot.z,sizeof(float),1,fp);
		fwrite(&m_ViewRot.y,sizeof(float),1,fp);
		fwrite(&m_ViewRot.x,sizeof(float),1,fp);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

