///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Cartoon Rendering System
//
// Created:
//		JL 1/12/00
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
#include "Loony.h"
#include "OGLView.h"
#include "LoadOBJ.h"
#include "ToonSet.h"

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
	m_AntiAlias	= FALSE;
	m_Dragging = FALSE;
	m_Silhouette = TRUE;

	// INITIALIZE SOME OF THE CAMERA VARIABLES
	ResetBone(&m_Camera, NULL);
	m_Camera.id = -1;
	strcpy(m_Camera.name,"Camera");
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = 0.0f;
	m_Camera.b_trans.z = -50.0f;
	m_Camera.trans.y = 0.0f;
	m_Camera.trans.z = -50.0f;

	m_Model.vertexData = NULL;

	// Set the Default Light Direction
	m_ShadeLight.x = 0.3f;
	m_ShadeLight.y = 0.1f;
	m_ShadeLight.z = 0.8f;	
	NormalizeVector(&m_ShadeLight);	// Normalize it since I know I didn't

	m_SilhouetteColor.r = 0.0f;
	m_SilhouetteColor.g = 0.0f;
	m_SilhouetteColor.b = 0.0f;
	m_SilhouetteWidth = 3;			// Width of Silhouette line

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

	// GENERATE THE OPENGL TEXTURE ID
	glGenTextures(1,&m_ShadeTexture);

	LoadShadeTexture("default.shd");

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
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CalculateShadow
// Purpose:		Calculate the shadow coordinate value for a normal
// Arguments:	The vertex normal, Light vector, and Object rotation matrix
// Returns:		An index coordinate into the shade table
///////////////////////////////////////////////////////////////////////////////
float COGLView::CalculateShadow(tVector *normal,tVector *light, tMatrix *mat)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	tVector post;
	float dot;
/////////////////////////////////////////////////////////////////////////////////////
	// Rotate the normal by the current object matrix
	MultVectorByRotMatrix(mat, normal, &post);
	dot = DotProduct(&post,light);				// Calculate the Dot Product

	if (dot < 0) dot = 0;						// Make sure the Back half dark
	return fabs(dot);			// Return the shadow value
}


///////////////////////////////////////////////////////////////////////////////
// Function:	drawModel
// Purpose:		Draw the Mesh model either deformed or not
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawModel(t_Visual *model)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tMatrix mat;		// Needed for Lighting Calc
	int loop;
	float   u;
	tVector	*vertex, *normal;
///////////////////////////////////////////////////////////////////////////////

	if (model->vertexData != NULL)
	{
		// Turn on anti-aliased silhouette lines if selected
		if (m_AntiAlias)
		{
			glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			glEnable(GL_LINE_SMOOTH);
		}
		else
			glDisable(GL_LINE_SMOOTH);

		glPolygonMode(GL_FRONT,GL_FILL);

		// Bind my 1D shade texture
		glEnable(GL_TEXTURE_1D);
		glBindTexture( GL_TEXTURE_1D,m_ShadeTexture);

		// Set the Base color of the Model from the material
		glColor3fv(&model->Kd.r);
		glDisable(GL_LIGHTING);

		// Grab the matrix for lighting calc
		glGetFloatv(GL_MODELVIEW_MATRIX,mat.m);

		// Get the Normal and Vertex from any format model that has vertex and normal
		switch(model->dataFormat)
		{
		case GL_T2F_N3F_V3F:
			vertex = (tVector *)&model->vertexData[5];
			normal = (tVector *)&model->vertexData[2];
			break;
		case GL_N3F_V3F:
			vertex = (tVector *)&model->vertexData[3];
			normal = (tVector *)&model->vertexData[0];
			break;
		case GL_V3F:
			vertex = (tVector *)&model->vertexData[0];
			normal = NULL;
			break;
		}

		glBegin(GL_TRIANGLES);
		for (loop = 0; loop < model->faceCnt * 3; loop++)
		{
			// calculate an index into the 1D texture using normal and light
			u = CalculateShadow(normal,&m_ShadeLight, &mat);
			glTexCoord1f(u);
			glVertex3fv((float *)vertex);
			// Increment pointers
			vertex = (tVector *)((float *)vertex + model->vSize);
			normal = (tVector *)((float *)normal + model->vSize);
		}
		glEnd();

		glDisable(GL_TEXTURE_1D);

		// Do the silhouette lines if desired
		if (m_Silhouette)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glColor3fv(&m_SilhouetteColor.r);	// Set Line Color
			glLineWidth(m_SilhouetteWidth);		// Give it some beef
			glDepthFunc(GL_LEQUAL);			// Draw shared edges
			glPolygonMode(GL_BACK,GL_LINE);	// Draw Lines
			glCullFace(GL_FRONT);			// Draw backfacing edges only

			glInterleavedArrays(model->dataFormat,0,(GLvoid *)model->vertexData);
			glDrawArrays(GL_TRIANGLES,0,model->faceCnt * 3);

			// Set Everything Back to original settings
			glDepthFunc(GL_LESS);
			glColor3f(1.0f, 1.0f, 1.0f);
			glCullFace(GL_BACK);
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
	
	glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

	glPushMatrix();

	// Set camera's orientation and position
	glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// Draw any loaded model
	drawModel(&m_Model);

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


void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_Dragging = FALSE;
	CWnd::OnLButtonUp(nFlags, point);
}

void COGLView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_Dragging = FALSE;
	CWnd::OnRButtonUp(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnMouseMove
// Purpose:		Handler for the mouse.  Handles movement when pressed
// Arguments:	Flags for key masks and point
///////////////////////////////////////////////////////////////////////////////
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_Dragging) return;

//	UpdateStatusBar(0);
	if (nFlags & MK_LBUTTON > 0)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}	
		// ELSE "SHIFT" MOVE THE BONE IN XY
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)	// Rotate Camera in Z
			{
				m_Camera.rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
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
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
		}
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
		// IF I AM HOLDING THE RM BUTTON Translate IN Z
		else
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

void COGLView::HandleKeyUp(UINT nChar) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	switch (nChar)
	{
	case VK_SPACE:
		break;
	case 'I':
		break;
	case 'W':
		break;
	case 'F':
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
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertexData != NULL)	// Free model data if exists
	{
		free(m_Model.vertexData);
		m_Model.vertexData = NULL;
	}
	LoadOBJ((LPCSTR)name,&m_Model);
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = 0.0f;
	m_Camera.b_trans.z = -50.0f;
	m_Camera.trans.y = 0.0f;
	m_Camera.trans.z = -50.0f;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadShadeTexture
// Purpose:		Load a shaded environment texture
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
void COGLView::LoadShadeTexture(const char *texfile)
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
			MAKEVECTOR(m_ShadeSrc[loop], 0.4f, 0.4f, 0.4f)
		}
		else if (loop < 28)
		{
			MAKEVECTOR(m_ShadeSrc[loop], 0.9f, 0.9f, 0.9f)
		}
		else
		{
			MAKEVECTOR(m_ShadeSrc[loop], 1.0f, 1.0f, 1.0f)
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
			m_ShadeSrc[loop].x = m_ShadeSrc[loop].y = m_ShadeSrc[loop].z = value;
		}
		fclose(fp);
	}
	glBindTexture(GL_TEXTURE_1D, m_ShadeTexture);

	// Do not allow bilinear filtering - not for cartoon rendering
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 32, 0,
			 GL_RGB , GL_FLOAT, (float *)m_ShadeSrc); //visual->texData);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CartoonSettings
// Purpose:		Adjust Line Settings for Cartoon Render
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
void COGLView::CartoonSettings()
{
//// Local Variables ////////////////////////////////////////////////////////////////
	CToonSet	dialog;
/////////////////////////////////////////////////////////////////////////////////////
	dialog.m_Sil_Red = m_SilhouetteColor.r;
	dialog.m_Sil_Green = m_SilhouetteColor.g;
	dialog.m_Sil_Blue = m_SilhouetteColor.b;
	dialog.m_LineWidth = m_SilhouetteWidth;
	dialog.m_Light_X = m_ShadeLight.x;
	dialog.m_Light_Y = m_ShadeLight.y;
	dialog.m_Light_Z = m_ShadeLight.z;
	if (dialog.DoModal())
	{
		m_SilhouetteColor.r = dialog.m_Sil_Red;
		m_SilhouetteColor.g = dialog.m_Sil_Green;
		m_SilhouetteColor.b = dialog.m_Sil_Blue;
		m_SilhouetteWidth = dialog.m_LineWidth;
		m_ShadeLight.x = dialog.m_Light_X;
		m_ShadeLight.y = dialog.m_Light_Y;
		m_ShadeLight.z = dialog.m_Light_Z;
		NormalizeVector(&m_ShadeLight);	// Normalize it since I know I didn't
	}
	Invalidate(TRUE);
}
