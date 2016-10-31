///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Cave Painting Rendering System
//
// Created:
//		JL 1/20/01
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2001 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <mmsystem.h>
#include <math.h>
#include "Cavern.h"
#include "OGLView.h"
#include "LoadOBJ.h"
#include "LoadTex.h"
#include "ToonSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define frand(V)	(V * ((float)rand() / (float)RAND_MAX))
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
	m_DrawFilled = TRUE;		// Do I want to draw the filled area
	m_Jitter = JITTER_PRECALC;

	m_Strokes = 3;

	// INITIALIZE SOME OF THE CAMERA VARIABLES
	ResetBone(&m_Camera, NULL);
	m_Camera.id = -1;
	strcpy(m_Camera.name,"Camera");
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = -2.0f;
	m_Camera.b_trans.z = -24.0f;
	m_Camera.trans.y = -2.0f;
	m_Camera.trans.z = -24.0f;

	m_Model.vertex = NULL;

	// Set the Default Light Direction
	m_ShadeLight.x = 0.3f;
	m_ShadeLight.y = 0.1f;
	m_ShadeLight.z = 0.8f;	
	m_ShadeLight.NormalizeVector();	// Normalize it since I know I didn't

	m_SilhouetteColor.r = 0.0f;
	m_SilhouetteColor.g = 0.0f;
	m_SilhouetteColor.b = 0.0f;
	m_SilhouetteAlpha = 0.5f;
	m_SilhouetteWidth = 3;			// Width of Silhouette line

	m_EdgeList = NULL;
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

	m_Background = LoadTextureFile("rock.tga");

	// Load my buffalo model as default
	LoadOBJModel("buffalo.obj");

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

	m_ScreenWidth = width;
	m_ScreenHeight = height;

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
	dot = post.DotProduct(light);				// Calculate the Dot Product

	if (dot < 0) dot = 0;						// Make sure the Back half dark
	return dot;									// Return the shadow value
}


bool COGLView::IsSilhouette(t_Edge *edge,tMatrix *mat)
{
	tVector view1,view2;
	tVector post1,post2;
	tMatrix	inv_mat;
	float dot1, dot2, dot3;
#if 0	// Rotate vertices and normals	
	// Don't do edges that share the same adjacent normals
//	if (edge->n[0].DotProduct(&edge->n[1]) == 1) return false;

	view1 = (m_Camera.trans * -1.0f);

	MultVectorByMatrix(mat, &edge->v[0], &edge->v_trans[0]);
	MultVectorByMatrix(mat, &edge->v[1], &edge->v_trans[1]);
	view1 = view1 - ((edge->v_trans[0] + edge->v_trans[1]) * 0.5f);
//	view1.MultVectorByMatrix(mat->m);
	view1.NormalizeVector();
	// Rotate the normal by the current object matrix
	MultVectorByRotMatrix(mat, &edge->n[0], &post1);
	dot1 = post1.DotProduct(&view1);				// Calculate the Dot Product
	edge->n_trans[0] = post1;

	MultVectorByRotMatrix(mat, &edge->n[1], &post2);
	dot2 = post2.DotProduct(&view1);				// Calculate the Dot Product
	edge->n_trans[1] = post2;

	if (dot1 == 0.0f || dot2 == 0.0f || (dot1 > 0.0f && dot2 < 0.0f) || (dot2 > 0.0f && dot1 < 0.0f))
	{
		return true;
	}

	dot3 = edge->n[1].DotProduct(&edge->n[0]);
	if (dot3 < 0.4f)
	{
		if (dot1 < 0 && dot2 < 0)
			return true;
	}
	return false;
#endif
#if 1	// Rotate camera
	// Don't do edges that share the same adjacent normals
//	if (edge->n[0].DotProduct(&edge->n[1]) == 1) return false;

	InvertMatrix(mat->m, inv_mat.m );
	MultVectorByRotMatrix(&inv_mat, &m_Camera.trans, &view1);
	
//	MultVectorByMatrix(mat, &edge->v[0], &edge->v_trans[0]);
//	MultVectorByMatrix(mat, &edge->v[1], &edge->v_trans[1]);
	view1 = view1 - ((edge->v[0] + edge->v[1]) * 0.5f);
//	view1.MultVectorByMatrix(mat->m);
	view1.NormalizeVector();
	// Rotate the normal by the current object matrix
	dot1 = edge->n[0].DotProduct(&view1);				// Calculate the Dot Product

	dot2 = edge->n[1].DotProduct(&view1);				// Calculate the Dot Product

	if ((dot1 > 0.0f && dot2 < 0.0f) || (dot2 > 0.0f && dot1 < 0.0f))	//dot1 == 0.0f || dot2 == 0.0f || 
	{
		return true;
	}

	// Hard edges in view??
	dot3 = edge->n[1].DotProduct(&edge->n[0]);
	if (dot3 < 0.5f)
	{
		if (dot1 < 0 || dot2 < 0)
			return true;
	}
	return false;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawModel
// Purpose:		Actually Draws the Model using the Cartoon Settings
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawModel(t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tMatrix mat;		// Needed for Lighting Calc
	int loop;
	t_faceIndex *face;
	float   u;
///////////////////////////////////////////////////////////////////////////////

	if (visual->vertex != NULL)
	{
		// Turn on anti-aliased silhouette lines if selected
		if (m_AntiAlias)
		{
			glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			glEnable(GL_LINE_SMOOTH);
		}
		else
			glDisable(GL_LINE_SMOOTH);

		// Set the Base color of the Model from the material
		glDisable(GL_LIGHTING);

		glPushMatrix();

		// Set camera's orientation and position
		glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

		glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
		glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

		// Grab the matrix for lighting calc
		glGetFloatv(GL_MODELVIEW_MATRIX,mat.m);

		// Do I draw the filled outline
		if (m_DrawFilled)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture( GL_TEXTURE_2D,m_ShadeTexture);

			face = visual->index;
			for (loop = 0; loop < visual->faceCnt; loop++,face++)
			{
				glColor3fv(&visual->Kd.r);	
				glBegin(GL_TRIANGLES);
					// calculate an index into the 1D texture using normal and light
					u = CalculateShadow(&visual->normal[face->n[0]],&m_ShadeLight, &mat);
					glTexCoord1f(u);
					glVertex3fv(&visual->vertex[face->v[0]].x);
					u = CalculateShadow(&visual->normal[face->n[1]],&m_ShadeLight, &mat);
					glTexCoord1f(u);
					glVertex3fv(&visual->vertex[face->v[1]].x);
					u = CalculateShadow(&visual->normal[face->n[2]],&m_ShadeLight, &mat);
					glTexCoord1f(u);
					glVertex3fv(&visual->vertex[face->v[2]].x);
				glEnd();
			}

			glDisable(GL_TEXTURE_2D);
		}

		if (m_Silhouette)
		{
		    glDisable(GL_TEXTURE_2D);
		    glDisable(GL_TEXTURE_1D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			glColor3fv(&m_SilhouetteColor.r);	// Set Line Color
			glLineWidth(m_SilhouetteWidth);		// Give it some beef
			glDepthFunc(GL_LEQUAL);			// Draw shared edges

			int times = m_Strokes;
			if (m_Jitter == JITTER_NONE) times = 1;
			for (int strokes = 0; strokes < times; strokes++)
			{
				t_Edge *edge = m_EdgeList;
				tVector color,step,jitter;
				color.Set(0.1f,0.1f,1.0f);
				step.Set(0.0f,0.0f,0.0f);
				int strokecnt = 0;
				jitter.Set(0.0f,0.0f,0.0f);
				for (loop = 0; loop < m_EdgeCnt; loop++,edge++)
				{
					int width = 1;

					// Recalculate the jitter each time
					if (m_Jitter == JITTER_RECALC)
						edge->jitter[strokes].Set(frand(0.1f),frand(0.1f),frand(0.1f));

					if (edge->flags == STROKE_START)
					{
						width = edge->length;
						color.Set(m_SilhouetteColor.r,m_SilhouetteColor.g,m_SilhouetteColor.b);
	//					int s = 1.0f / (float)edge->length;
	//					step = color * s;
						strokecnt++;
						if (m_Jitter > JITTER_NONE)
							jitter = edge->jitter[strokes];
					}
					if (IsSilhouette(edge,&mat))
					{
						glColor4f(color.x,color.y,color.z,m_SilhouetteAlpha);
	//					glLineWidth(width);		// Give it some beef
						
						glBegin(GL_LINES);
							tVector vertex = edge->v[0] + jitter;
							glVertex3fv(&vertex.x);
							if (m_Jitter > JITTER_NONE)
								jitter = edge->jitter[strokes];
							vertex = edge->v[1] + jitter;
							glVertex3fv(&vertex.x);
						glEnd();
					}
					width--;
					color -= step;
				}
			}

			glColor3f(1.0f, 1.0f, 1.0f);
		}
		glPopMatrix();


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

   glDepthMask(false);			// Don't write to the depth buffer
	glMatrixMode(GL_PROJECTION);			// I WANT TO PLAY WITH THE PROJECTION
	glPushMatrix();							// SAVE THE OLD ONE
		glLoadIdentity();					// LOAD A NEW ONE
		gluOrtho2D(0,m_ScreenWidth,0,m_ScreenHeight);	// USE WINDOW SETTINGS
		glEnable(GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D,m_Background);
		glColor3f(1.0f, 1.0f, 1.0f);		// DRAW A WHITE BOX
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);
			glVertex2s(0,m_ScreenHeight - 1);
			glTexCoord2f(0.0f,1.0f);
			glVertex2s(0,0);
			glTexCoord2f(1.0f,1.0f);
			glVertex2s(m_ScreenWidth - 1,0);
			glTexCoord2f(1.0f,0.0f);
			glVertex2s(m_ScreenWidth - 1,m_ScreenHeight - 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();							// RESTORE THE OLD PROJECTION
	 glMatrixMode(GL_MODELVIEW);				// BACK TO MODEL MODE
   glDepthMask(true);			// Don't write to the depth buffer

	// Draw any loaded model
	drawModel(&m_Model);
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
	case 'A':
		break;
	case 'Z':
		break;
	case 'J':
		m_Jitter = !m_Jitter;
		break;
	case 'R':
		m_Jitter = 2;
		break;
	case 'F':
		m_DrawFilled = !m_DrawFilled;
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
// Function:	GetFaceNormal
// Purpose:		Given a triangle, average the normals to get a face normal
// Arguments:	Face index and normal array
// Returns:		Average face normal
///////////////////////////////////////////////////////////////////////////////
tVector COGLView::GetFaceNormal(t_faceIndex *face, tVector *normalList)
{
	tVector ave;
	ave = normalList[face->n[0]];
	ave += normalList[face->n[1]];
	ave += normalList[face->n[2]];

	ave /= 3.0f;
	ave.NormalizeVector();
	return ave;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	AddEdge
// Purpose:		Add a triangle edge to the edge database
// Arguments:	Triangle face structure, vertex indices, vertex normal and edge lists
// Notes:		This routine makes sure each edge is only added to the list once
///////////////////////////////////////////////////////////////////////////////
void COGLView::AddEdge(t_faceIndex *face,int v1, int v2, tVector *vertices,tVector *normals, t_Edge *edgeList,long *edgeCnt)
{
	// Probably a much faster way to do this but it is offline so...
	for (int i = 0; i < (*edgeCnt); i++)
	{
		// see if it is already in the list
		if ((face->v[v1] == edgeList[i].v_index[0] && face->v[v2] == edgeList[i].v_index[1]) ||
			(face->v[v1] == edgeList[i].v_index[1] && face->v[v2] == edgeList[i].v_index[0]))
		{
			// Get the normal for the second face
			edgeList[i].n[1] = GetFaceNormal(face,normals);
			return;
		}
	}

	edgeList[*edgeCnt].v_index[0] = face->v[v1];
	edgeList[*edgeCnt].v_index[1] = face->v[v2];
	edgeList[*edgeCnt].v[0] = vertices[face->v[v1]];
	edgeList[*edgeCnt].v[1] = vertices[face->v[v2]];
	edgeList[*edgeCnt].n[0] = GetFaceNormal(face,normals);
	edgeList[*edgeCnt].flags = 0;
	edgeList[*edgeCnt].length = 0;
	edgeList[*edgeCnt].jitter[0].Set(frand(0.1f),frand(0.1f),frand(0.1f));
	edgeList[*edgeCnt].jitter[1].Set(frand(0.1f),frand(0.1f),frand(0.1f));
	edgeList[*edgeCnt].jitter[2].Set(frand(0.1f),frand(0.1f),frand(0.1f));
	*edgeCnt = *edgeCnt + 1;	// Increment the edges
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OrientEdges
// Purpose:		Given my edgelist, orient the edges into some logical order so
//					strokes can be built.  I am aligning them Top-Bottom Left-Right
///////////////////////////////////////////////////////////////////////////////
void COGLView::OrientEdges()
{
	t_Edge tempEdge;
	// Sort top to bottom, right to left
	for (int i = 0; i < m_EdgeCnt; i++)
	{
		m_EdgeList[i].flags = 0;
		tVector diff = m_EdgeList[i].v[0] - m_EdgeList[i].v[1];
		// See if I can eliminate this edge since it is planar
		if (m_EdgeList[i].n[0].DotProduct(&m_EdgeList[i].n[1]) == 1.0f)
		{
			for (int j = i; j < m_EdgeCnt - 1; j++)
			{
				memcpy(&m_EdgeList[j],&m_EdgeList[j+1],sizeof(t_Edge));
			}
			m_EdgeCnt--;
			i--;
			continue;
		}
		// Assume a top to bottom 
		if (diff.y > diff.x)
		{
			// If it is backwards, swap them
			if (m_EdgeList[i].v[0].y > m_EdgeList[i].v[1].y)
			{
				memcpy(&tempEdge,&m_EdgeList[i],sizeof(t_Edge));
				m_EdgeList[i].v[0] = m_EdgeList[i].v[1];
				m_EdgeList[i].v_index[0] = m_EdgeList[i].v_index[1];
				m_EdgeList[i].n[0] = m_EdgeList[i].n[1];
				m_EdgeList[i].v[1] = tempEdge.v[0];
				m_EdgeList[i].v_index[1] = tempEdge.v_index[0];
				m_EdgeList[i].n[1] = tempEdge.n[0];
			}
		}
		else  // side to side
		{
			// If it is backwards, swap them
			if (m_EdgeList[i].v[0].x < m_EdgeList[i].v[1].x)
			{
				memcpy(&tempEdge,&m_EdgeList[i],sizeof(t_Edge));
				m_EdgeList[i].v[0] = m_EdgeList[i].v[1];
				m_EdgeList[i].v_index[0] = m_EdgeList[i].v_index[1];
				m_EdgeList[i].n[0] = m_EdgeList[i].n[1];
				m_EdgeList[i].v[1] = tempEdge.v[0];
				m_EdgeList[i].v_index[1] = tempEdge.v_index[0];
				m_EdgeList[i].n[1] = tempEdge.n[0];
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	BuildStrokes
// Purpose:		Given my edgelist, build it into strokes
///////////////////////////////////////////////////////////////////////////////
void COGLView::BuildStrokes()
{
	t_Edge *edge,*strokeStart = NULL,*tempList = (t_Edge *)malloc(sizeof(t_Edge) * m_EdgeCnt);
	int	tempCnt = 0, curEdge = -1, backtrack;
	while (tempCnt < m_EdgeCnt)
	{
		if (curEdge < 0)
		{
			curEdge = 0;
			while (curEdge < m_EdgeCnt && m_EdgeList[curEdge].flags == STROKE_USED)
				curEdge++;
			if (curEdge == m_EdgeCnt) 
			{
				break;	// finished
			}
			// trace it back to find the start
			backtrack = 0;
			for (int loop = 0; loop < m_EdgeCnt; loop++)
			{
				if (m_EdgeList[loop].flags == 0 && m_EdgeList[loop].v_index[1] == m_EdgeList[curEdge].v_index[0])
				{
					if (m_EdgeList[loop].n[0].DotProduct(&m_EdgeList[curEdge].n[0]) > 0.80f && m_EdgeList[loop].n[1].DotProduct(&m_EdgeList[curEdge].n[1]) > 0.80f)
					{
						curEdge = loop;
						loop = 0;
						backtrack++;
						break;
					}
				}
			}
			edge = &tempList[tempCnt];
			memcpy(&tempList[tempCnt],&m_EdgeList[curEdge],sizeof(t_Edge));
			tempCnt++;
			m_EdgeList[curEdge].flags = STROKE_USED;
			edge->flags = STROKE_START;
			strokeStart = edge;
			strokeStart->length = 1;
		}
		
		for (int loop = 0; loop < m_EdgeCnt; loop++)
		{
			if (m_EdgeList[loop].flags == 0 && m_EdgeList[loop].v_index[0] == edge->v_index[1])
			{
				if (m_EdgeList[loop].n[0].DotProduct(&edge->n[0]) > 0.80f && m_EdgeList[loop].n[1].DotProduct(&edge->n[1]) > 0.80f)
				{
					edge = &tempList[tempCnt];
					memcpy(&tempList[tempCnt],&m_EdgeList[loop],sizeof(t_Edge));
					tempCnt++;
					m_EdgeList[loop].flags = STROKE_USED;
					loop = 0;	// reset to top of list
					strokeStart->length++;
				}
			}
		}

		curEdge = -1;
	}

	// Copy the list over
	memcpy(m_EdgeList,tempList,sizeof(t_Edge) * m_EdgeCnt);
}

unsigned int COGLView::LoadTextureFile(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tTGAHeader_s header;
	unsigned char *texture;
	unsigned int texID;
///////////////////////////////////////////////////////////////////////////////
	texture = LoadTGAFile((char*)(LPCSTR)name,	&header);  //
	if (texture)
	{
		// GENERATE THE OPENGL TEXTURE ID
		glGenTextures(1,&texID);

		glBindTexture(GL_TEXTURE_2D, texID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
		return texID;
	}
	return 0;
}

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
	m_Camera.b_trans.x = -1.0f;
	m_Camera.b_trans.y = -2.0f;
	m_Camera.b_trans.z = -24.0f;
	m_Camera.trans.x = -1.0f;
	m_Camera.trans.y = -2.0f;
	m_Camera.trans.z = -24.0f;

	if (strlen(m_Model.map) > 0)
	{
		pos = m_Model.map + strlen(m_Model.map);
		while (*pos != '/' && pos != m_Model.map)
			pos--;

		if (*pos == '/') pos++;

		sprintf(texname,"%s",pos);

		texture = LoadTGAFile( texname,	&header);  //

		// GENERATE THE OPENGL TEXTURE ID
		glGenTextures(1,&m_Model.glTex);

		glBindTexture(GL_TEXTURE_2D, m_Model.glTex);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
	else
		m_Model.glTex = 0;

	// Load 2nd Texture if it is there
	if (strlen(m_Model.map2) > 0)
	{
		pos = m_Model.map2 + strlen(m_Model.map2);
		while (*pos != '/' && pos != m_Model.map2)
			pos--;

		if (*pos == '/') pos++;

		sprintf(texname,"%s",pos);

		texture = LoadTGAFile( texname,	&header);  //

		// GENERATE THE OPENGL TEXTURE ID
		glGenTextures(1,&m_Model.glTex2);

		glBindTexture(GL_TEXTURE_2D, m_Model.glTex2);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
	else
		m_Model.glTex = 0;

	// Build up the edgelist for the object
	if (m_Model.faceCnt > 0)
	{
		if (m_EdgeList) free(m_EdgeList);
		m_EdgeList = (t_Edge *)malloc(sizeof(t_Edge) * m_Model.faceCnt * 3);
		m_EdgeCnt = 0;

		// For each face, add all 3 triangle edges
		for (int i = 0; i < m_Model.faceCnt; i++)
		{
			AddEdge(&m_Model.index[i],0,1, m_Model.vertex, m_Model.normal, m_EdgeList, &m_EdgeCnt);
			AddEdge(&m_Model.index[i],1,2, m_Model.vertex, m_Model.normal, m_EdgeList, &m_EdgeCnt);
			AddEdge(&m_Model.index[i],2,0, m_Model.vertex, m_Model.normal, m_EdgeList, &m_EdgeCnt);
		}

		// Orient them and build strokes
		OrientEdges();
		BuildStrokes();
	}
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
	float value1, value2;
	tVector4 *texture;
/////////////////////////////////////////////////////////////////////////////////////
	texture = (tVector4 *)malloc(sizeof(tVector4) * 32 * 32);
	// Make a Default one One shade with highlight
	for (loop = 0; loop < 32; loop++)
	{

		if (loop < 8)
		{
			MAKEVECTOR4(m_ShadeSrc[loop], 0.4f, 0.4f, 0.4f, 0.0f)
		}
		else if (loop < 28)
		{
			MAKEVECTOR4(m_ShadeSrc[loop], 0.9f, 0.9f, 0.9f, 1.0f)
		}
		else
		{
			MAKEVECTOR4(m_ShadeSrc[loop], 1.0f, 1.0f, 1.0f, 1.0f)
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
			sscanf(line,"%f %f",&value1,&value2);
			m_ShadeSrc[loop].x = m_ShadeSrc[loop].y = m_ShadeSrc[loop].z = value1;
			m_ShadeSrc[loop].w = value2;
		}
		fclose(fp);
	}

	for (int v = 0; v < 32; v++)
	{
		for (int u = 0; u < 32; u++)
		{
			float color = m_ShadeSrc[u].x;
			texture[(v * 32) + u].x = color;
			texture[(v * 32) + u].y = color;
			texture[(v * 32) + u].z = color;
			texture[(v * 32) + u].w = m_ShadeSrc[u].w;
		}
	}

	glBindTexture(GL_TEXTURE_2D, m_ShadeTexture);

	// Do not allow bilinear filtering - not for cartoon rendering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0,
			 GL_RGBA , GL_FLOAT, (float *)texture); //visual->texData);

	free(texture);
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
	dialog.m_Sil_Alpha = m_SilhouetteAlpha;
	dialog.m_LineWidth = m_SilhouetteWidth;
	dialog.m_Light_X = m_ShadeLight.x;
	dialog.m_Light_Y = m_ShadeLight.y;
	dialog.m_Light_Z = m_ShadeLight.z;
	if (dialog.DoModal())
	{
		m_SilhouetteColor.r = dialog.m_Sil_Red;
		m_SilhouetteColor.g = dialog.m_Sil_Green;
		m_SilhouetteColor.b = dialog.m_Sil_Blue;
		m_SilhouetteAlpha = dialog.m_Sil_Alpha;
		m_SilhouetteWidth = dialog.m_LineWidth;
		m_ShadeLight.x = dialog.m_Light_X;
		m_ShadeLight.y = dialog.m_Light_Y;
		m_ShadeLight.z = dialog.m_Light_Z;
		m_ShadeLight.NormalizeVector();	// Normalize it since I know I didn't
	}
	Invalidate(TRUE);
}

