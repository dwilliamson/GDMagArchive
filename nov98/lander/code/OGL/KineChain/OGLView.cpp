///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Inverse Kinematics System
//
// Created:
//		JL 7/1/98		
//
// Notes:	The meat of this application is the last routine in this file.
//			"ComputeCCDLink" takes a target point and solves the system.
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
#include "KineChain.h"
#include "MathDefs.h"
#include "OGLView.h"
#include "Quatern.h"
#include "Model.h"		// SOFTIMAGE MODEL DATA
#include "Restrict.h"	// DOF RESTRICTIONS DIALOG
#include "Bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		99		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION
#define EFFECTOR_POS		5		// THIS CHAIN HAS 5 LINKS
#define MAX_IK_TRIES		100		// TIMES THROUGH THE CCD LOOP (TRIES = # / LINKS) 
#define IK_POS_THRESH		1.0f	// THRESHOLD FOR SUCCESS
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
	ResetBone(&m_Link[0], NULL);
	m_Link[0].id = 1;
	m_Link[0].trans.x = 4.8f;
	m_Link[0].trans.y = 6.0f;
	strcpy(m_Link[0].name,"Base");
	m_Link[0].childCnt = 1;
	m_Link[0].children = &m_Link[1];


	ResetBone(&m_Link[1], NULL);
	m_Link[1].id = 2;
	strcpy(m_Link[1].name,"Link1");
	m_Link[1].trans.y = -1.0f;
	m_Link[1].childCnt = 1;
	m_Link[1].children = &m_Link[2];

	ResetBone(&m_Link[2], NULL);
	m_Link[2].id = 3;
	strcpy(m_Link[2].name,"Link2");
	m_Link[2].trans.y = -1.0f;
	m_Link[2].childCnt = 1;
	m_Link[2].children = &m_Link[3];

	ResetBone(&m_Link[3], NULL);
	m_Link[3].id = 4;
	strcpy(m_Link[3].name,"Link3");
	m_Link[3].trans.y = -1.0f;
	m_Link[3].childCnt = 1;
	m_Link[3].children = &m_Link[4];

	ResetBone(&m_Link[4], NULL);
	m_Link[4].id = 5;
	strcpy(m_Link[4].name,"Link4");
	m_Link[4].trans.y = -1.0f;
	m_Link[4].childCnt = 1;
	m_Link[4].children = &m_Link[5];

	// SET UP END EFFECTOR
	ResetBone(&m_Link[5], NULL);
	m_Link[5].id = 6;
	strcpy(m_Link[5].name,"Effector");
	m_Link[5].trans.y = -1.0f;

	// SET UP DEFAULT SETTINGS FOR THE DAMPING FOR SIX JOINTS
	m_Link[0].damp_width = 10.0f;
	m_Link[1].damp_width = 10.0f;
	m_Link[2].damp_width = 10.0f;
	m_Link[3].damp_width = 10.0f;
	m_Link[4].damp_width = 10.0f;
	m_Link[5].damp_width = 10.0f;			// END EFFECTOR, NOT USED REALLY

	// SET UP DEFAULT SETTINGS FOR THE DOF RESTRICTIONS
	m_Link[0].min_rz = -30;
	m_Link[1].min_rz = -30;
	m_Link[2].min_rz = -30;
	m_Link[3].min_rz = -30;
	m_Link[4].min_rz = -30;
	m_Link[5].min_rz = -30;			// END EFFECTOR, NOT USED REALLY

	m_Link[0].max_rz = 30;
	m_Link[1].max_rz = 30;
	m_Link[2].max_rz = 30;
	m_Link[3].max_rz = 30;
	m_Link[4].max_rz = 30;
	m_Link[5].max_rz = 30;			// END EFFECTOR, NOT USED REALLY

	// BY DEFAULT NO DAMPING OR DOF RESTRICTION
	m_Damping = FALSE;
	m_DOF_Restrict = FALSE;
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

	// WRITE THE ORIENTATIONS OF THE BONES IN THE WINDOW STATUS AREA
	sprintf(message,"Joint Rot Values (%.2f,%.2f,%.2f,%.2f,%.2f)",
		m_Link[0].rot.z,m_Link[1].rot.z,m_Link[2].rot.z,m_Link[3].rot.z,
		m_Link[4].rot.z);
	m_ptrStatusBar->SetPaneText(1,message);

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
	ON_WM_MBUTTONDOWN()
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

	CreateBoneDLists(&m_Link[0]);

	// LOAD THE TEXTURE MAPS FOR THE OBJECT
	LoadBoneTexture(&m_Link[0], "snake.bmp");
	m_Link[1].visuals = m_Link[0].visuals;		// BONES 1 - 3 USE INSTANCED TEXTURES
	m_Link[2].visuals = m_Link[0].visuals;
	m_Link[3].visuals = m_Link[0].visuals;
	LoadBoneTexture(&m_Link[4], "head.bmp");

	drawScene(TRUE);
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

	m_Link[0].trans.x = ((float)width / 2.0f) / m_ModelScale;
}    

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadBoneTexture
// Purpose:		Load texture images for the bone
// Notes:		Some of this code was originally from the OpenGL SuperBible 
//				by Richard Wright Jr. and Michael Sweet
//				THIS SHOULD LOOK THROUGH ALL BONES AND SEE IF TEXTURE IS 
//				ALREADY LOADED.
GLvoid COGLView::LoadBoneTexture(t_Bone *curBone, char *name)
{
	BITMAPINFO	*info;				/* Bitmap information */
	void		*bits;				/* Bitmap pixel bits */
	GLubyte		*glbits;				/* Bitmap RGB pixels */

	// GENERATE THE OPENGL TEXTURE ID
	glGenTextures(1,(unsigned int *)&curBone->visuals);
	curBone->visualCnt++;

	// LOAD THE BITMAP
	bits = LoadDIBitmap(name, &info);
	if (bits == NULL)
	{
		::MessageBox(NULL,"Unable to Open File...",name,MB_OK);
		curBone->visuals = 0;
	    return;
	}

	// CONVERT IT TO AN RGB TEXTURE
	glbits = ConvertBitsToGL(info, bits);
	if (glbits == NULL)
	{
		free(info);
		free(bits);

		return;
	};

	glBindTexture(GL_TEXTURE_2D, (unsigned int)curBone->visuals);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/*
    * Define the 2D texture image.
    */

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, info->bmiHeader.biWidth, info->bmiHeader.biHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, glbits);

	/*
	*Free the bitmap and RGB images, then return 0 (no errors).
	*/

	free(glbits);
	free(info);
	free(bits);
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

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// WAS MODULATE
	glEnable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
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
// Function:	CreateBoneDLists
// Purpose:		Creates the Drawlists for the Bones in a Skeleton
// Arguments:	Pointer to a bone hierarchy
///////////////////////////////////////////////////////////////////////////////
void COGLView::CreateBoneDLists(t_Bone *bone)
{
	// ONLY MAKE A BONE IF THERE IS A CHILD
	if (bone->childCnt > 0)
	{
		// CREATE THE DISPLAY LIST FOR A BONE
		glNewList(bone->id,GL_COMPILE);
			glBegin(GL_LINE_STRIP);
				glColor3f(1.0f, 1.0f, 0.0f);	// YELLOW
				glVertex3f( 0.0f,  0.4f, 0.0f);		// 0
				glVertex3f(-0.4f,  0.0f,-0.4f);		// 1
				glVertex3f( 0.4f,  0.0f,-0.4f);		// 2
				glVertex3f( 0.0f,  bone->children->trans.y, 0.0f);		// Base
				glVertex3f(-0.4f,  0.0f,-0.4f);		// 1
				glVertex3f(-0.4f,  0.0f, 0.4f);		// 4
				glVertex3f( 0.0f,  0.4f, 0.0f);		// 0
				glVertex3f( 0.4f,  0.0f,-0.4f);		// 2
				glVertex3f( 0.4f,  0.0f, 0.4f);		// 3
				glVertex3f( 0.0f,  0.4f, 0.0f);		// 0
				glVertex3f(-0.4f,  0.0f, 0.4f);		// 4
				glVertex3f( 0.0f,  bone->children->trans.y, 0.0f);		// Base
				glVertex3f( 0.4f,  0.0f, 0.4f);		// 3
				glVertex3f(-0.4f,  0.0f, 0.4f);		// 4
			glEnd();
		glEndList();
		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (bone->childCnt > 0)
			CreateBoneDLists(bone->children);
	}
}

GLvoid COGLView::drawModel(t_Bone *curBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (curBone->visuals > 0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (unsigned int)curBone->visuals);
	}
	glInterleavedArrays(GL_T2F_C3F_V3F,0,(GLvoid *)SNAKE);
	glDrawArrays(GL_TRIANGLES,0,SNAKEPOLYCNT * 3);
    glDisable(GL_TEXTURE_2D);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawSkeleton
// Purpose:		Actually draws the Skeleton it is recursive
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawSkeleton(t_Bone *rootBone,BOOL actuallyDraw)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		glPushMatrix();

		// Set base orientation and position
		glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

		glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f);
		glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f); 
	
		// THE SCALE IS LOCAL SO I PUSH AND POP
		glPushMatrix();
		glScalef(curBone->scale.x, curBone->scale.y, curBone->scale.z); 

		if (actuallyDraw)
		{
			if (m_DrawGeometry)
			{
				if (curBone->childCnt > 0)
				{
					drawModel(curBone);
				}
			}
			else
			{
				// DRAW THE AXIS OGL OBJECT
				glCallList(OGL_AXIS_DLIST);
				// DRAW THE ACTUAL BONE STRUCTURE
				// ONLY MAKE A BONE IF THERE IS A CHILD
				if (curBone->childCnt > 0)
				{
					glColor3f(1.0f, 1.0f, 0.0f);	// Selected bone is bright Yellow
					// DRAW THE BONE STRUCTURE
					glCallList(curBone->id);
				}
			}
		}

		// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
		glGetFloatv(GL_MODELVIEW_MATRIX,curBone->matrix.m);

		glPopMatrix();	// THIS POP IS JUST FOR THE SCALE

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			drawSkeleton(curBone,actuallyDraw);

		glPopMatrix();	// THIS POPS THE WHOLE MATRIX

		curBone++;
	}
}
//// drawSkeleton /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Procedure:	drawScene
// Purpose:		Draws the current OpenGL scene
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::drawScene(BOOL actuallyDraw)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	if (actuallyDraw)
	{
		glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING
	}

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Link[0].trans.x, m_Link[0].trans.y, m_Link[0].trans.z);

	// ROTATE THE ROOT
	glRotatef(m_Link[0].rot.z, 0.0f, 0.0f, 1.0f);
    glRotatef(m_Link[0].rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_Link[0].rot.x, 1.0f, 0.0f, 0.0f); 

	// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
	glGetFloatv(GL_MODELVIEW_MATRIX,m_Link[0].matrix.m);

	if (actuallyDraw)
	{
		if (m_DrawGeometry)
		{
			drawModel(m_Link);
		}
		else
		{
			glCallList(m_Link[0].id);
			glCallList(OGL_AXIS_DLIST);
		}
	}

	drawSkeleton(&m_Link[0],actuallyDraw);

	glPopMatrix();
    glFinish();

	if (actuallyDraw)
	{
	    SwapBuffers(m_hDC);

		// DRAW THE STATS AT THE BOTTOM OF THE SCREEN
		UpdateStatus();
	}
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
	drawScene(TRUE);

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
	CPoint joint1,joint2,effector;
///////////////////////////////////////////////////////////////////////////////
	m_mousepos = point;

	point.y = m_Height - point.y - 1;

	ComputeCCDLink(point);
	drawScene(TRUE);
	m_Grab_Rot_X = 	m_Link[2].rot.x;
	m_Grab_Rot_Y = 	m_Link[2].rot.y;
	m_Grab_Rot_Z = 	m_Link[2].rot.z;
	m_Grab_Trans_X = 	m_Link[2].trans.x;
	m_Grab_Trans_Y = 	m_Link[2].trans.y;
	m_Grab_Trans_Z = 	m_Link[2].trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnRButtonDown
// Purpose:		Right button down grabs the current point pos so I can use it
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	point.y = m_Height - point.y - 1;

	ComputeCCDLink(point);
	drawScene(TRUE);
	m_Grab_Rot_X = 	m_Link[4].rot.x;
	m_Grab_Rot_Y = 	m_Link[4].rot.y;
	m_Grab_Rot_Z = 	m_Link[4].rot.z;
	m_Grab_Trans_X = 	m_Link[4].trans.x;
	m_Grab_Trans_Y = 	m_Link[4].trans.y;
	m_Grab_Trans_Z = 	m_Link[4].trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	point.y = m_Height - point.y - 1;

	ComputeCCDLink(point);
	drawScene(TRUE);
	m_Grab_Rot_X = 	m_Link[3].rot.x;
	m_Grab_Rot_Y = 	m_Link[3].rot.y;
	m_Grab_Rot_Z = 	m_Link[3].rot.z;
	m_Grab_Trans_X = 	m_Link[3].trans.x;
	m_Grab_Trans_Y = 	m_Link[3].trans.y;
	m_Grab_Trans_Z = 	m_Link[3].trans.z;
	CWnd::OnMButtonDown(nFlags, point);
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
				drawScene(TRUE);
			}
		}
		else
		{
			point.y = m_Height - point.y - 1;
			ComputeCCDLink(point);
			drawScene(TRUE);
		}

	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				drawScene(TRUE);
			}
		}
	}
	else if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				drawScene(TRUE);
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
// Procedure:	SetRestrictions
// Purpose:		Open Dialog to set up restrictions
///////////////////////////////////////////////////////////////////////////////		
void COGLView::SetRestrictions() 
{
	CRestrict dialog;
	dialog.m_Damp0 = m_Link[0].damp_width;
	dialog.m_Damp1 = m_Link[1].damp_width;
	dialog.m_Damp2 = m_Link[2].damp_width;
	dialog.m_Damp3 = m_Link[3].damp_width;
	dialog.m_Damp4 = m_Link[4].damp_width;
	dialog.m_MinRot0 = m_Link[0].min_rz;
	dialog.m_MinRot1 = m_Link[1].min_rz;
	dialog.m_MinRot2 = m_Link[2].min_rz;
	dialog.m_MinRot3 = m_Link[3].min_rz;
	dialog.m_MinRot4 = m_Link[4].min_rz;
	dialog.m_MaxRot0 = m_Link[0].max_rz;
	dialog.m_MaxRot1 = m_Link[1].max_rz;
	dialog.m_MaxRot2 = m_Link[2].max_rz;
	dialog.m_MaxRot3 = m_Link[3].max_rz;
	dialog.m_MaxRot4 = m_Link[4].max_rz;
	if (dialog.DoModal())
	{
		m_Link[0].damp_width = dialog.m_Damp0;
		m_Link[1].damp_width = dialog.m_Damp1;
		m_Link[2].damp_width = dialog.m_Damp2;
		m_Link[3].damp_width = dialog.m_Damp3;
		m_Link[4].damp_width = dialog.m_Damp4;
		m_Link[0].min_rz = dialog.m_MinRot0;
		m_Link[1].min_rz = dialog.m_MinRot1;
		m_Link[2].min_rz = dialog.m_MinRot2;
		m_Link[3].min_rz = dialog.m_MinRot3;
		m_Link[4].min_rz = dialog.m_MinRot4;
		m_Link[0].max_rz = dialog.m_MaxRot0;
		m_Link[1].max_rz = dialog.m_MaxRot1;
		m_Link[2].max_rz = dialog.m_MaxRot2;
		m_Link[3].max_rz = dialog.m_MaxRot3;
		m_Link[4].max_rz = dialog.m_MaxRot4;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Procedure:	ComputeCCDLink
// Purpose:		Compute an IK Solution to an end effector position
// Arguments:	End Target (x,y)
// Returns:		TRUE if a solution exists, FALSE if the position isn't in reach
///////////////////////////////////////////////////////////////////////////////		
BOOL COGLView::ComputeCCDLink(CPoint endPos)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector		rootPos,curEnd,desiredEnd,targetVector,curVector,crossResult;
	double		cosAngle,turnAngle,turnDeg;
	int			link,tries;
///////////////////////////////////////////////////////////////////////////////
	// START AT THE LAST LINK IN THE CHAIN
	link = EFFECTOR_POS - 1;
	tries = 0;						// LOOP COUNTER SO I KNOW WHEN TO QUIT
	do
	{
		// THE COORDS OF THE X,Y,Z POSITION OF THE ROOT OF THIS BONE IS IN THE MATRIX
		// TRANSLATION PART WHICH IS IN THE 12,13,14 POSITION OF THE MATRIX
		rootPos.x = m_Link[link].matrix.m[12];
		rootPos.y = m_Link[link].matrix.m[13];
		rootPos.z = m_Link[link].matrix.m[14];

		// POSITION OF THE END EFFECTOR
		curEnd.x = m_Link[EFFECTOR_POS].matrix.m[12];
		curEnd.y = m_Link[EFFECTOR_POS].matrix.m[13];
		curEnd.z = m_Link[EFFECTOR_POS].matrix.m[14];

		// DESIRED END EFFECTOR POSITION
		desiredEnd.x = (float)endPos.x;
		desiredEnd.y = (float)endPos.y;
		desiredEnd.z = 0.0f;						// ONLY DOING 2D NOW

		// SEE IF I AM ALREADY CLOSE ENOUGH
		if (VectorSquaredDistance(&curEnd, &desiredEnd) > IK_POS_THRESH)
		{
			// CREATE THE VECTOR TO THE CURRENT EFFECTOR POS
			curVector.x = curEnd.x - rootPos.x;
			curVector.y = curEnd.y - rootPos.y;
			curVector.z = curEnd.z - rootPos.z;
			// CREATE THE DESIRED EFFECTOR POSITION VECTOR
			targetVector.x = endPos.x - rootPos.x;
			targetVector.y = endPos.y - rootPos.y;
			targetVector.z = 0.0f;						// ONLY DOING 2D NOW

			// NORMALIZE THE VECTORS (EXPENSIVE, REQUIRES A SQRT)
			NormalizeVector(&curVector);
			NormalizeVector(&targetVector);

			// THE DOT PRODUCT GIVES ME THE COSINE OF THE DESIRED ANGLE
			cosAngle = DotProduct(&targetVector,&curVector);

			// IF THE DOT PRODUCT RETURNS 1.0, I DON'T NEED TO ROTATE AS IT IS 0 DEGREES
			if (cosAngle < 0.99999)
			{
				// USE THE CROSS PRODUCT TO CHECK WHICH WAY TO ROTATE
				CrossProduct(&targetVector, &curVector, &crossResult);
				if (crossResult.z > 0.0f)	// IF THE Z ELEMENT IS POSITIVE, ROTATE CLOCKWISE
				{
					turnAngle = acos((float)cosAngle);	// GET THE ANGLE
					turnDeg = RADTODEG(turnAngle);		// COVERT TO DEGREES
					// DAMPING
					if (m_Damping && turnDeg > m_Link[link].damp_width) 
						turnDeg = m_Link[link].damp_width;
					m_Link[link].rot.z -= (float)turnDeg;	// ACTUALLY TURN THE LINK
					// DOF RESTRICTIONS
					if (m_DOF_Restrict &&
						m_Link[link].rot.z < (float)m_Link[link].min_rz) 
						m_Link[link].rot.z = (float)m_Link[link].min_rz;
				}
				else if (crossResult.z < 0.0f)	// ROTATE COUNTER CLOCKWISE
				{
					turnAngle = acos((float)cosAngle);
					turnDeg = RADTODEG(turnAngle);
					// DAMPING
					if (m_Damping && turnDeg > m_Link[link].damp_width) 
						turnDeg = m_Link[link].damp_width;
					m_Link[link].rot.z += (float)turnDeg;	// ACTUALLY TURN THE LINK
					// DOF RESTRICTIONS
					if (m_DOF_Restrict &&
						m_Link[link].rot.z > (float)m_Link[link].max_rz) 
						m_Link[link].rot.z = (float)m_Link[link].max_rz;
				}
				// RECALC ALL THE MATRICES WITHOUT DRAWING ANYTHING
				drawScene(FALSE);		// CHANGE THIS TO TRUE IF YOU WANT TO SEE THE ITERATION
			}
			if (--link < 0) link = EFFECTOR_POS - 1;	// START OF THE CHAIN, RESTART
		}
	// QUIT IF I AM CLOSE ENOUGH OR BEEN RUNNING LONG ENOUGH
	} while (tries++ < MAX_IK_TRIES && 
				VectorSquaredDistance(&curEnd, &desiredEnd) > IK_POS_THRESH);
	return TRUE;
}

