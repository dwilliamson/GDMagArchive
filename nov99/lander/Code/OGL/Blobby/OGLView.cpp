///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of 3D Blob Modelling
//
// Created:
//		JL 10/1/99		
//
// The function morphModel() does the main morphing work.
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Blobby.h"
#include "OGLView.h"
#include "Math.h"
#include "EditBlob.h"
#include "EditSys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define OGL_ICON_DLIST		1		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION

#define LERP(a,b,c)  (a + ((b - a) * c))
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
	m_CameraRot.x = m_CameraRot.y = m_CameraRot.z = 0.0f;
	m_CameraTrans.x = m_CameraTrans.y = 0.0f;
	m_CameraTrans.z = -80.0f;

	m_pGoopSys = Goop_InitSys();	// Initialize the Blob system
	m_pCurBlob = NULL;

	m_DrawGeometry = TRUE;
	m_DrawBlobs = TRUE;
	m_DrawField = FALSE;

	m_Subdivisions = 25;		// How fine to divide the goop field
	m_Threshold = 0.7f;			// Surface Threshold
}

COGLView::~COGLView()
{
	Goop_FreeSys();
}


BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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

	glNewList(OGL_ICON_DLIST,GL_COMPILE);
		glBegin(GL_TRIANGLES);
		glVertex3f(0.0000f, -0.5000f, 0.0000f);
		glVertex3f(0.5000f, 0.0000f, 0.0000f); 
		glVertex3f(0.0000f, 0.0000f, 0.5000f);
		glVertex3f(-0.5000f, 0.0000f, -0.0000f); 
		glVertex3f(0.0000f, 0.5000f, 0.0000f);
		glVertex3f(0.0000f, 0.0000f, -0.5000f); 
		glVertex3f(0.5000f, 0.0000f, 0.0000f);
		glVertex3f(0.0000f, -0.5000f, 0.0000f); 
		glVertex3f(0.0000f, 0.0000f, -0.5000f); 
		glVertex3f(0.0000f, 0.0000f, 0.5000f);
		glVertex3f(-0.5000f, 0.0000f, -0.0000f); 
		glVertex3f(0.0000f, -0.5000f, 0.0000f);
		glVertex3f(0.0000f, 0.0000f, -0.5000f);
		glVertex3f(0.0000f, -0.5000f, 0.0000f);
		glVertex3f(-0.5000f, 0.0000f, -0.0000f); 
		glVertex3f(0.0000f, 0.0000f, -0.5000f);
		glVertex3f(0.0000f, 0.5000f, 0.0000f);
		glVertex3f(0.5000f, 0.0000f, 0.0000f);
		glVertex3f(0.0000f, 0.5000f, 0.0000f);
		glVertex3f(0.0000f, 0.0000f, 0.5000f);
		glVertex3f(0.5000f, 0.0000f, 0.0000f);
		glVertex3f(0.0000f, 0.5000f, 0.0000f);
		glVertex3f(-0.5000f, 0.0000f, -0.0000f); 
		glVertex3f(0.0000f, 0.0000f, 0.5000f);
		glEnd();
	glEndList();

	glDisable(GL_TEXTURE_2D);

	DrawScene();
	return 0;
}

/* OpenGL code */
GLvoid COGLView::resize( GLsizei width, GLsizei height )
{
// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    glViewport(0, 0, width, height);

    m_Aspect = (GLfloat)width/(GLfloat)height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(10.0, m_Aspect, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
}    

GLvoid COGLView::initializeGL(GLsizei width, GLsizei height)
{
/// Local Variables ///////////////////////////////////////////////////////////
 	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightpos[] = { 0.30f, 0.3f, 1.0f, 0.0f };		// .5 .5 1.0
	GLfloat ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    m_Aspect = (GLfloat)width/(GLfloat)height;
	// Establish viewing volume
	gluPerspective(10.0, m_Aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
//	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glLineWidth(1.0f);
	glPointSize(2.0f);		// JUST 1 PIXEL DOTS FOR NOW
	glDisable(GL_LINE_SMOOTH);
	glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glMaterialfv(GL_FRONT,GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR, specular);
	glMaterialf(GL_FRONT,GL_SHININESS, 100.0f);		// 12
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

}

// GET THE INFO ON THE VERSION OF OPENGL RUNNING
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

///////////////////////////////////////////////////////////////////////////////
// Procedure:	DrawField
// Purpose:		Draws the meta field cool to visualize
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::DrawField()
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector pos;
	tVector step;
	tMetaGoopEval *field,*element;
	int loopx,loopz;
///////////////////////////////////////////////////////////////////////////////
	step.x = (m_pGoopSys->vMax.x - m_pGoopSys->vMin.x) / m_Subdivisions;
	step.y = (m_pGoopSys->vMax.y - m_pGoopSys->vMin.y) / m_Subdivisions;
	step.z = (m_pGoopSys->vMax.z - m_pGoopSys->vMin.z) / m_Subdivisions;

	if (step.x == 0.0f || step.y == 0.0f || step.z == 0.0f) return;

	glBegin(GL_POINTS);

	// Allocate memory for a layer
	field = (tMetaGoopEval *)calloc( m_Subdivisions * m_Subdivisions, sizeof(tMetaGoopEval));
	element = field;

	// Go through each vertical slice
	for (pos.y = m_pGoopSys->vMin.y; pos.y <= m_pGoopSys->vMax.y; pos.y += step.y)
	{
		// Evaluate one XZ plane
		Goop_EvaluateLayer(field, m_Subdivisions,m_pGoopSys->vMin.x, m_pGoopSys->vMin.z, step.x, step.z, pos.y);
		for (loopx = 0, pos.x = m_pGoopSys->vMin.x; loopx < m_Subdivisions; loopx++, pos.x += step.x)
			for (loopz = 0, pos.z = m_pGoopSys->vMin.z; loopz < m_Subdivisions; loopz++, pos.z += step.z)
			{
				element = &field[loopx * m_Subdivisions + loopz];
				// Color each point based on the test threshold
				if (element->value < m_Threshold)
				{
					glColor3f((element->value / m_Threshold),(element->value / m_Threshold), 0.0f);	//Yellow
				}
				else
					glColor3f(0.0f, (m_Threshold / element->value), 0.0f);	//  Green
				if (fabs(element->value - m_Threshold) < 0.2f)	// Points right on the edge, color red
					glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3f(pos.x, pos.y, pos.z);
			}
	}
	glEnd();

//	free(field);	// THIS SHOULD WORK BUT CRASHES ON MY SYS.  DAMN VC PASSING POINTERS
}
// DrawField

///////////////////////////////////////////////////////////////////////////////
// Procedure:	DrawSurface
// Purpose:		Draws the model associated with a energy field
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::DrawSurface()
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	Goop_EvaluateSurface(m_Threshold,m_Subdivisions);
}
// DrawSurface

///////////////////////////////////////////////////////////////////////////////
// Procedure:	DrawBlobs
// Purpose:		Draws the elements of the blob system
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::DrawBlobs(BOOL selecting)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tMetaGoop	*pGoop;
///////////////////////////////////////////////////////////////////////////////
	pGoop = m_pGoopSys->pGoop;
	for (int loop = 0; loop < m_pGoopSys->nGoopCnt; loop++, pGoop++)
	{
		glPushMatrix();

		// Set the blobs position
		glTranslatef(pGoop->position.x, pGoop->position.y, pGoop->position.z);

		if (pGoop == m_pCurBlob)		// Is it selected
			glColor3f(1.0f, 1.0f, 0.0f);	//Yellow
		else
			glColor3f(0.0f, 0.0f, 1.0f);	// Blue

		glScalef(pGoop->radiusSquared,pGoop->radiusSquared,pGoop->radiusSquared);

		if (selecting) glLoadName(loop);	// Mark the Blobs for Picking

		glCallList(OGL_ICON_DLIST);

		glPopMatrix();
	}
}
// DrawBlobs

///////////////////////////////////////////////////////////////////////////////
// Procedure:	DrawScene
// Purpose:		Draws the current OpenGL scene
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::DrawScene(GLvoid)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_CameraTrans.x, m_CameraTrans.y, m_CameraTrans.z);

	// ROTATE THE ROOT
	glRotatef(m_CameraRot.z, 1.0f, 0.0f, 0.0f);
    glRotatef(m_CameraRot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_CameraRot.x, 0.0f, 0.0f, 1.0f); 

	if (m_DrawBlobs)
		DrawBlobs(FALSE);

	if (m_DrawField)
		DrawField();

	if (m_DrawGeometry)
		DrawSurface();

    glPopMatrix();

    glFinish();

    SwapBuffers(m_hDC);
}
// 	drawScene

#define BUFSIZE 512

GLvoid COGLView::SelectScene(int x, int y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	GLuint	selectbuf[BUFSIZE];
	GLint	hits;
	GLint	viewport[4];
///////////////////////////////////////////////////////////////////////////////

	glGetIntegerv(GL_VIEWPORT,viewport);
	glSelectBuffer(BUFSIZE,selectbuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);
	gluPerspective(10.0f, m_Aspect,1.0f, 2000.0f);
	glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_CameraTrans.x, m_CameraTrans.y, m_CameraTrans.z);

	// ROTATE THE ROOT
	glRotatef(m_CameraRot.z, 1.0f, 0.0f, 0.0f);
    glRotatef(m_CameraRot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_CameraRot.x, 0.0f, 0.0f, 1.0f); 

	DrawBlobs(TRUE);

    glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

    glFlush();

	hits = glRenderMode(GL_RENDER);
	ProcessSelection(hits, selectbuf);

}

void COGLView::ProcessSelection(GLint hits, GLuint buffer[]) 
{
	int i;
	unsigned int j;
	GLuint names, *ptr,item;
	float z1,z2,nearest = 99999.0f;

	ptr = (GLuint *)buffer;
	for (i = 0; i < hits; i++)
	{
		names = *ptr;
		ptr++;
		z1 = (float)*ptr/0x7fffffff; ptr++;
		z2 = (float)*ptr/0x7fffffff; ptr++;
		for (j = 0; j < names; j++)
		{
			item = *ptr;
			ptr++;

			if (z1 < nearest)
			{			
				m_pCurBlob = &m_pGoopSys->pGoop[item];	
				nearest = z1;
			}
		}

	}
	if (nearest == 99999.0f)	// NOTHING HIT SET TO NOTHING
		m_pCurBlob = NULL;
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
	DrawScene();

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
	m_mousepos = point;

	// Pick a blob to move or manipulate
	SelectScene(point.x,point.y);

	m_Base_Rot_X = 	m_CameraRot.x;
	m_Base_Rot_Y = 	m_CameraRot.y;
	m_Base_Rot_Z = 	m_CameraRot.z;
	if (m_pCurBlob != NULL)
	{
		m_Grab_Trans_X = 	m_pCurBlob->position.x;
		m_Grab_Trans_Y = 	m_pCurBlob->position.y;
		m_Grab_Trans_Z = 	m_pCurBlob->position.z;
	}
	DrawScene();
	CWnd::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnRButtonDown
// Purpose:		Right button down grabs the current point pos so I can use it
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	m_Base_Rot_X = 	m_CameraRot.x;
	m_Base_Rot_Y = 	m_CameraRot.y;
	m_Base_Rot_Z = 	m_CameraRot.z;
	if (m_pCurBlob != NULL)
	{
		m_Grab_Trans_X = 	m_pCurBlob->position.x;
		m_Grab_Trans_Y = 	m_pCurBlob->position.y;
		m_Grab_Trans_Z = 	m_pCurBlob->position.z;
	}
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
	case 'O': 
		glPolygonMode(GL_FRONT,GL_LINE);
		break;
	case 'F': 
		m_DrawField = !m_DrawField;
//		glPolygonMode(GL_FRONT,GL_FILL);
		break;
	case ' ':
		break;
	}
	DrawScene();
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnMouseMove
// Purpose:		Handle mouse moves while pressed
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	float sinAngle,cosAngle;
	sinAngle = (float)sin(DEGTORAD(m_CameraRot.y));
	cosAngle = (float)cos(DEGTORAD(m_CameraRot.y));
	if (nFlags & MK_LBUTTON > 0)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}	
		else if ((nFlags & MK_SHIFT) > 0)
		{
		}
		else
		{
			if (m_pCurBlob)
			{
				m_pCurBlob->position.x = m_Grab_Trans_X + ((float)(point.x - m_mousepos.x) * 0.028f * cosAngle);
				m_pCurBlob->position.z = m_Grab_Trans_Z + ((float)(point.x - m_mousepos.x) * 0.028f * sinAngle);
				m_pCurBlob->position.y = m_Grab_Trans_Y - ((float)(point.y - m_mousepos.y) * 0.028f);
				Goop_FindBounds();
				DrawScene();
			}
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}
		else if ((nFlags & MK_SHIFT) > 0)
		{
		}
		else
		{
			m_CameraRot.x = m_Base_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
			m_CameraRot.y = m_Base_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
			DrawScene();
		}
	}
	CWnd::OnMouseMove(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnLButtonDblClk
// Purpose:		Left Double click, get dialog for Current Blob
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	EditBlob();
}

void COGLView::EditBlob()
{
/// Local Variables ///////////////////////////////////////////////////////////
	CEditBlob	dialog;
///////////////////////////////////////////////////////////////////////////////
	if (m_pCurBlob)
	{
		dialog.m_RadiusSquared = m_pCurBlob->radiusSquared;
		dialog.m_Strength = m_pCurBlob->strength;
		if (dialog.DoModal())
		{
			m_pCurBlob->radiusSquared = dialog.m_RadiusSquared;
			m_pCurBlob->strength = dialog.m_Strength;
			DrawScene();
		}
	}
}

void COGLView::EditSys()
{
/// Local Variables ///////////////////////////////////////////////////////////
	CEditSys	dialog;
///////////////////////////////////////////////////////////////////////////////
	dialog.m_SubDiv = m_Subdivisions;
	dialog.m_Threshold = m_Threshold;
	if (dialog.DoModal())
	{
		m_Subdivisions = dialog.m_SubDiv;
		m_Threshold = dialog.m_Threshold;
		DrawScene();
	}
}


///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadFiles
// Purpose:		Loads the Blob files into memory
///////////////////////////////////////////////////////////////////////////////		
void COGLView::LoadFile(CString file1,CString baseName) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;
	int loop,cnt;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen((LPCSTR)file1,"rb");
	if (fp)
	{
		Goop_FreeSys();
		fread(&m_Subdivisions, sizeof(int),1,fp);
		fread(&m_Threshold, sizeof(int),1,fp);
		fread(&cnt,sizeof(int),1,fp);
		for (loop = 0; loop < cnt; loop++)
		{
			Goop_AddBlob(m_pGoopSys);	
			m_pCurBlob = &m_pGoopSys->pGoop[m_pGoopSys->nGoopCnt - 1];	
			fread(&m_pCurBlob->position, sizeof(tVector),1,fp);
			fread(&m_pCurBlob->radiusSquared, sizeof(float),1,fp);
			fread(&m_pCurBlob->strength, sizeof(float),1,fp);
		}
		Goop_FindBounds();
		DrawScene();
		m_pCurBlob = NULL;
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	SaveFiles
// Purpose:		Saves the Blob files into memory
///////////////////////////////////////////////////////////////////////////////		
void COGLView::SaveFile(CString file1,CString baseName) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;
	int loop;
	tMetaGoop	*pGoop;
///////////////////////////////////////////////////////////////////////////////
	fp = fopen((LPCSTR)file1,"wb");
	if (fp)
	{
		fwrite(&m_Subdivisions, sizeof(int),1,fp);
		fwrite(&m_Threshold, sizeof(int),1,fp);
		fwrite(&m_pGoopSys->nGoopCnt,sizeof(int),1,fp);
		pGoop = m_pGoopSys->pGoop;
		for (loop = 0; loop < m_pGoopSys->nGoopCnt; loop++, pGoop++)
		{
			fwrite(&pGoop->position, sizeof(tVector),1,fp);
			fwrite(&pGoop->radiusSquared, sizeof(float),1,fp);
			fwrite(&pGoop->strength, sizeof(float),1,fp);
		}
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnGoopAddblob
// Purpose:		Add a blob to the system
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnGoopAddblob() 
{
	Goop_AddBlob(m_pGoopSys);	
	m_pCurBlob = &m_pGoopSys->pGoop[m_pGoopSys->nGoopCnt - 1];	
	Goop_FindBounds();
	DrawScene();
}
