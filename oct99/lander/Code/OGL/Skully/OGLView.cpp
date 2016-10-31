///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of Hierarchical Animation System
//
// Created:
//		JL 9/1/97		
// Versions:
//		1.01	12/20/97	Fix perspective in OpenGL Resize Code
//		1.02	1/10/97		Change to display code to handle skeletal hierarchy
// Modified:
//		JL 9/10/99		Created skeleton Demo for Oct 99 GDMag
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
#include <mmsystem.h>
#include <math.h>
#include "Skully.h"
#include "OGLView.h"
#include "LoadOBJ.h"
#include "LoadSkel.h"		// Skeleton loading

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	m_DrawSkeleton = TRUE;
	m_DrawDeformed = FALSE;
	m_PickX = -1;
	m_PickY = -1;
	ResetBone(&m_Skeleton,NULL);
	m_SelectedBone = &m_Skeleton;

	// INITIALIZE SOME OF THE CAMERA VARIABLES
	ResetBone(&m_Camera, NULL);
	m_Camera.id = -1;
	strcpy(m_Camera.name,"Camera");
	m_Camera.rot.x = 0.0f;
	m_Camera.rot.y = 0.0f;
	m_Camera.rot.z = 0.0f;
	m_Camera.b_trans.y = -6.0f;
	m_Camera.b_trans.z = -50.0f;
	m_Camera.trans.y = -6.0f;
	m_Camera.trans.z = -50.0f;

	m_Model.vertexData = NULL;
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
	drawScene(FALSE);
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
	GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat lightpos[] = { 0.5, 0.5, 1.0, 0.0 };
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
//	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_FRONT,GL_LINE);
	glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
	glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
	glPointSize(8.0);		// NICE BEEFY POINTS FOR THE VERTEX SELECTION
	glDisable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR, specular);
	glMaterialf(GL_FRONT,GL_SHININESS, 25.0f);
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

}

///////////////////////////////////////////////////////////////////////////////
// At run-time, at each frame of animation, you need to get the
// current matrices for every bone.  This is multiplied by the
// inverted matrix from the rest bone and stored.  Must be
// recursive.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetSkeletonMat
// Purpose:		Gets the Matrix values for the character
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::GetSkeletonMat(t_Bone *rootBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
	tMatrix tempMatrix;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		glPushMatrix();

		glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

		// Set observer's orientation and position
		glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f);
		glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f); 
	
		// Get the current matrix
		glGetFloatv(GL_MODELVIEW_MATRIX,tempMatrix.m);

		// Multiply it by the inverted root bone and store
		MultMatrix(curBone->curMatrix,&tempMatrix,&curBone->matrix);

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			GetSkeletonMat(curBone);

		glPopMatrix();

		curBone++;
	}
}
//// GetSkeletonMat ///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// When the base skeleton is set, the matrix for each bones must
// be grabbed, inverted, then stored for run time use.  As the
// skeleton is a hierarchy, this must be done recursively
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetBaseSkeletonMat
// Purpose:		Gets the Matrix values for the character
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::GetBaseSkeletonMat(t_Bone *rootBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
	tMatrix tempMatrix;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		glPushMatrix();

		glTranslatef(curBone->b_trans.x, curBone->b_trans.y, curBone->b_trans.z);

		// Set observer's orientation and position
		glRotatef(curBone->b_rot.z, 0.0f, 0.0f, 1.0f);
		glRotatef(curBone->b_rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(curBone->b_rot.x, 1.0f, 0.0f, 0.0f); 
	
		// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
		glGetFloatv(GL_MODELVIEW_MATRIX,tempMatrix.m);
		// Invert the matrix and store it for later
		InvertMatrix(tempMatrix.m,curBone->matrix.m);

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			GetBaseSkeletonMat(curBone);

		glPopMatrix();

		curBone++;
	}
}
//// GetBaseSkeletonMat ///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This function actually will deform a mesh given a
// skeletal system.  At each bone, a combined matrix that
// tranforms the vertex from the rest bone space to the final
// bone space is stored.  Each vertex is multiplied by this
// matrix then scaled by the weight.  This is accumulated to
// give a fine vertex position
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DeformVertices
// Purpose:		Bends the Bodies Based on the Vertices
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::DeformVertices(t_Bone *rootBone,t_Visual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop,loop2;
	t_Bone *curBone;
	int vertex;
	float weight;
	tVector post;
	float *deformData,*vertexData;
///////////////////////////////////////////////////////////////////////////////
	if (rootBone == &m_Skeleton && visual->vertexData != NULL)
	{
		// ZERO THE DEFORMATION
		if (m_Skeleton.childCnt > 0)
		{
			for (loop = 0; loop < m_Skeleton.childCnt; loop++)
			{
				memcpy(visual->deformData,visual->vertexData,sizeof(float) * visual->vSize * visual->vertexCnt);
				deformData = (float *)(visual->deformData + (visual->vSize - 3));
				for (loop2 = 0; loop2 < visual->vertexCnt; loop2++, deformData += visual->vSize)
				{
					deformData[0] = 0.0f;
					deformData[1] = 0.0f;
					deformData[2] = 0.0f;
				}
			}
		}
	}

	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		for (loop2 = 0; loop2 < visual->vertexCnt; loop2++)
		{
			// GET THE WEIGHT
			weight = curBone->CV_weight[loop2].weight;
			// The weight is between 0-1
			if (weight > 0.0f)
			{
				vertex = curBone->CV_weight[loop2].vertex;

				deformData = (float *)(visual->deformData + (visual->vSize * (vertex + 1)) - 3);
				vertexData = (float *)(visual->vertexData + (visual->vSize * (vertex + 1)) - 3);

				// Multiply the vertex by the combined matrix
				MultVectorByMatrix(curBone->curMatrix, (tVector *)vertexData, &post);
// Since the Matrix above is a combination of the rest and current matrices, it does
//		the same as the following two calls would
//				MultVectorByMatrix(&curBone->matrix, (tVector *)&vertexData[vertex].x, &pre);
//				MultVectorByMatrix(curBone->curMatrix, &pre, &post);

				// Accumulate the result
				deformData[0] += (post.x * weight);
				deformData[1] += (post.y * weight);
				deformData[2] += (post.z * weight);
			}
		}

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			DeformVertices(curBone, visual);

		curBone++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawModel
// Purpose:		Draw the Mesh model either deformed or not
// Arguments:	Pointer to the model
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawModel(t_Visual *model)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tColoredVertex *cdata;
	float *data;
	int loop;
///////////////////////////////////////////////////////////////////////////////
	if (model->vertexData != NULL)
	{

		glColor3f(1.0f, 1.0f, 1.0f);	// NO MODULATION

		if (m_DrawDeformed)
			data = (float *)model->deformData;
		else
			data = (float *)model->vertexData;

		// Declare the Array of Data
		glInterleavedArrays(model->dataFormat,0,(GLvoid *)data);
		if (model->vPerFace == 3)		// Model is Triangles
			glDrawArrays(GL_TRIANGLES,0,model->faceCnt * 3);
		else
			glDrawArrays(GL_QUADS,0,model->faceCnt * 4);

/* Same as above but unrolled
		cdata = (tColoredVertex *)data;
		glBegin(GL_TRIANGLES);
		for (loop = 0; loop < model->faceCnt; loop++)
		{
//			glTexCoord2f(data[loop * 3].u, data[loop * 3].v);
			glNormal3fv((float *)&cdata[loop * 3].r);
			glVertex3fv((float *)&cdata[loop * 3].x);
//			glTexCoord2f(data[loop * 3 + 1].u, data[loop * 3 + 1].v);
			glNormal3fv((float *)&cdata[loop * 3 + 1].r);
			glVertex3fv((float *)&cdata[loop * 3 + 1].x);
//			glTexCoord2f(data[loop * 3 + 2].u, data[loop * 3 + 2].v);
			glNormal3fv((float *)&cdata[loop * 3 + 2].r);
			glVertex3fv((float *)&cdata[loop * 3 + 2].x);
		}
		glEnd();
*/

		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		// NOW DRAW THE VERTEX MARKERS IF THEY ARE SELECTED
		glColor3f(1.0f, 0.0f, 0.0f);	// Selected Vertices are Red
		glBegin(GL_POINTS);
		for (loop = 0; loop < model->vertexCnt; loop++)
		{
			// IF A POINT IS SELECTED DRAW IT
			if (model->CV_select[loop])
			{
				glVertex3fv((float *)&data[((loop + 1) * model->vSize) - 3]);
			}
		}
		glEnd();
	    glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	drawSkeleton
// Purpose:		Actually draws the Skeleton it is recursive
// Arguments:	Pointer to the bone
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawSkeleton(t_Bone *rootBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop;
	t_Bone *curBone;
///////////////////////////////////////////////////////////////////////////////
	curBone = rootBone->children;
	for (loop = 0; loop < rootBone->childCnt; loop++)
	{
		glPushMatrix();

		glTranslatef(curBone->trans.x, curBone->trans.y, curBone->trans.z);

		// Set observer's orientation and position
		glRotatef(curBone->rot.z, 0.0f, 0.0f, 1.0f);
		glRotatef(curBone->rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(curBone->rot.x, 1.0f, 0.0f, 0.0f); 
	
		// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
//		glGetFloatv(GL_MODELVIEW_MATRIX,curBone->matrix.m);

		// THE SCALE IS LOCAL SO I PUSH AND POP
		glPushMatrix();
		glScalef(curBone->bsphere, curBone->bsphere, curBone->bsphere); 

		if (m_DrawSkeleton)
		{
			// DRAW THE AXIS OGL OBJECT
			glCallList(OGL_AXIS_DLIST);
			// IF SOMETHING IS SELECTED, DRAW TAG BOX
			if (m_Skeleton.id == (long)curBone)
			{
				glDisable(GL_CULL_FACE);
				m_SelectedBone = curBone;
				glCallList(OGL_SELECTED_DLIST);
				glEnable(GL_CULL_FACE);
			}
		}
		glPopMatrix();

		// CHECK IF THIS BONE HAS CHILDREN, IF SO RECURSIVE CALL
		if (curBone->childCnt > 0)
			drawSkeleton(curBone);

		glPopMatrix();

		curBone++;
	}
}
//// drawSkeleton /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	drawScene
// Purpose:		Actually draw the OpenGL Scene
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::drawScene(BOOL drawSelectRect)
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

	// Set root skeleton's orientation and position
	glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

	glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
	glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
	glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

	// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
	glGetFloatv(GL_MODELVIEW_MATRIX,m_Skeleton.matrix.m);

	glDisable(GL_LIGHTING);
	drawSkeleton(&m_Skeleton);
	glEnable(GL_LIGHTING);

	if (m_Skeleton.childCnt > 0)
	{
		DeformVertices(&m_Skeleton,&m_Model);
	}
	drawModel(&m_Model);

	m_PickX = -1;
	m_PickY = -1;

	glPopMatrix();

	GetSkeletonMat(&m_Skeleton);	// GET THE SKELETON INFO

	// IF I AM DRAGGING A SELECTION BOX, DRAW IT
	if (drawSelectRect)
	{
		glMatrixMode(GL_PROJECTION);			// I WANT TO PLAY WITH THE PROJECTION
		glPushMatrix();							// SAVE THE OLD ONE
			glLoadIdentity();					// LOAD A NEW ONE
			gluOrtho2D(0,m_ScreenWidth,0,m_ScreenHeight);	// USE WINDOW SETTINGS
			glColor3f(1.0f, 1.0f, 1.0f);		// DRAW A WHITE BOX
			glBegin(GL_LINE_STRIP);
			glVertex2s((short)m_SelectRect.left,(short)m_SelectRect.top);
			glVertex2s((short)m_SelectRect.right,(short)m_SelectRect.top);
			glVertex2s((short)m_SelectRect.right,(short)m_SelectRect.bottom);
			glVertex2s((short)m_SelectRect.left,(short)m_SelectRect.bottom);
			glVertex2s((short)m_SelectRect.left,(short)m_SelectRect.top);
			glEnd();
		glPopMatrix();							// RESTORE THE OLD PROJECTION
		glMatrixMode(GL_MODELVIEW);				// BACK TO MODEL MODE
	}
	
	//    glFinish();

	SwapBuffers(m_hDC);

	UpdateStatusBarFrameInfo();
}
//// drawScene //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	SelectVertices
// Purpose:		Use Feedback to get all the vertices in the view
// Arguments:	Should I select or de-select?
///////////////////////////////////////////////////////////////////////////////
void COGLView::SelectVertices(BOOL select)
{
/// Local Variables ///////////////////////////////////////////////////////////
	GLfloat *feedBuffer;
	GLint hitCount;
	int loop2;
	float *data;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertexData != NULL)
	{
		// INITIALIZE A PLACE TO PUT ALL THE FEEDBACK INFO (3 DATA, 1 TAG, 2 TOKENS)
		feedBuffer = (GLfloat *)malloc(sizeof(GLfloat) * m_Model.vertexCnt * 6);
		// TELL OPENGL ABOUT THE BUFFER
		glFeedbackBuffer(m_Model.vertexCnt * 6,GL_3D,feedBuffer);
		(void)glRenderMode(GL_FEEDBACK);	// SET IT IN FEEDBACK MODE
		glPushMatrix();

		// Set root skeleton's orientation and position
		glTranslatef(m_Camera.trans.x, m_Camera.trans.y, m_Camera.trans.z);

		glRotatef(m_Camera.rot.x, 1.0f, 0.0f, 0.0f);
		glRotatef(m_Camera.rot.y, 0.0f, 1.0f, 0.0f);
		glRotatef(m_Camera.rot.z, 0.0f, 0.0f, 1.0f); 

		for (loop2 = 0; loop2 < m_Model.vertexCnt; loop2++)
		{
			if (m_DrawDeformed)
				data = (float *)m_Model.deformData;
			else
				data = (float *)m_Model.vertexData;
			// PASS THROUGH A MARKET LETTING ME KNOW WHAT VERTEX IT WAS
			glPassThrough((float)loop2);
			// SEND THE VERTEX
			glBegin(GL_POINTS);
			glVertex3fv((float *)&data[(m_Model.vSize * (loop2 + 1)) - 3]);
			glEnd();
		}

	    glPopMatrix();
		hitCount = glRenderMode(GL_RENDER); // HOW MANY HITS DID I GET
		CompareBuffer(hitCount,feedBuffer, select);		// CHECK THEM AGAINST MY SELECTION 
		free(feedBuffer);		// GET RID OF THE MEMORY
	}
}
////// SelectVertices /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	CompareBuffer
// Purpose:		Check the feedback buffer to see if anything is tagged
// Arguments:	Number of hits, pointer to buffer, Should I select or de-select
///////////////////////////////////////////////////////////////////////////////
void COGLView::CompareBuffer(GLint size, GLfloat *buffer,BOOL select)
{
/// Local Variables ///////////////////////////////////////////////////////////
	GLint count;
	GLfloat token,point[3];
	int loop,currentVertex;
	int *data;
///////////////////////////////////////////////////////////////////////////////
	count = size;
	while (count)
	{
		token = buffer[size - count];	// CHECK THE TOKEN
		count--;
		if (token == GL_PASS_THROUGH_TOKEN)	// VERTEX MARKER
		{
			currentVertex = (int)buffer[size - count]; // WHAT VERTEX
			count--;
		}
		else if (token == GL_POINT_TOKEN)
		{
			// THERE ARE THREE ELEMENTS TO A POINT TOKEN
			for (loop = 0; loop < 3; loop++)
			{
				point[loop] = buffer[size - count];
				count--;
			}
			data = (BOOL *)m_Model.CV_select;
			// CHECK IF THE POINT WAS IN MY SELECTION RECTANGLE
			// FLOATS 0 AND 1 ARE SCREEN X AND Y
			// NOTE: OPENGL SETS THE BOTTOM Y=0
			if (point[0] >= m_SelectRect.left && 
				point[0] <= m_SelectRect.right && 
				point[1] <= m_SelectRect.top && 
				point[1] >= m_SelectRect.bottom)
				// SET THIS VERTEX TO THE CURRENT SELECTION VALUE
				data[currentVertex] = select;
		}
	}
}
////// CompareBuffer //////////////////////////////////////////////////////////

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

void COGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE KIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Grab_Rot_X = 	m_SelectedBone->rot.x;
	m_Grab_Rot_Y = 	m_SelectedBone->rot.y;
	m_Grab_Rot_Z = 	m_SelectedBone->rot.z;
	m_Grab_Trans_X = 	m_SelectedBone->trans.x;
	m_Grab_Trans_Y = 	m_SelectedBone->trans.y;
	m_Grab_Trans_Z = 	m_SelectedBone->trans.z;
	m_SelectRect.left = point.x;
	m_SelectRect.top = m_ScreenHeight - point.y;
	if ((nFlags & MK_CONTROL) == 0 && (nFlags & MK_SHIFT) == 0)
	{
		m_SelectRect.left = point.x;
		m_SelectRect.top = m_ScreenHeight - point.y;
		SetCapture( );
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// STORE OFF THE KIT POINT AND SETTINGS FOR THE MOVEMENT LATER
	m_mousepos = point;
	m_Grab_Rot_X = 	m_Camera.rot.x;
	m_Grab_Rot_Y = 	m_Camera.rot.y;
	m_Grab_Rot_Z = 	m_Camera.rot.z;
	m_Grab_Trans_X = 	m_Camera.trans.x;
	m_Grab_Trans_Y = 	m_Camera.trans.y;
	m_Grab_Trans_Z = 	m_Camera.trans.z;
	m_SelectRect.left = point.x;
	m_SelectRect.top = m_ScreenHeight - point.y;
	CWnd::OnRButtonDown(nFlags, point);
}


void COGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ((nFlags & MK_CONTROL) == 0 && (nFlags & MK_SHIFT) == 0)

	{
		m_SelectRect.right = point.x;
		m_SelectRect.bottom = m_ScreenHeight - point.y;
		SelectVertices(TRUE);
		drawScene(FALSE);
	}
	ReleaseCapture( );
	CWnd::OnLButtonUp(nFlags, point);
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
		// IF I AM HOLDING THE 'CTRL' BUTTON ROTATE IN Z
		if ((nFlags & MK_CONTROL) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_SelectedBone->rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene(FALSE);
			}
		}	
		// ELSE "SHIFT" ROTATE THE BONE IN XY
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_SelectedBone->rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene(FALSE);
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_SelectedBone->rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene(FALSE);
			}
		}
		// ELSE MY SELECTION BOX
		else
		{
			m_SelectRect.right = point.x;
			m_SelectRect.bottom = m_ScreenHeight - point.y;
			drawScene(TRUE);
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)		// Handle the Camera
	{
		if ((nFlags & MK_CONTROL) > 0)	// Move Camera in Z
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Camera.trans.z = m_Grab_Trans_Z + (.1f * (point.x - m_mousepos.x));
				drawScene(FALSE);
			}
		}
		else if ((nFlags & MK_SHIFT) > 0)
		{
			UpdateStatusBar(2);
			if ((point.x - m_mousepos.x) != 0)	// Move Camera in X
			{
				m_Camera.trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
				drawScene(FALSE);
			}
			if ((point.y - m_mousepos.y) != 0)	// Move Camera in Y
			{
				m_Camera.trans.y = m_Grab_Trans_Y - (.1f * (point.y - m_mousepos.y));
				drawScene(FALSE);
			}
		}
		else
		{
			UpdateStatusBar(1);
			if ((point.x - m_mousepos.x) != 0)	// Rotate Camera in Y
			{
				m_Camera.rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene(FALSE);
			}
			if ((point.y - m_mousepos.y) != 0)	// Rotate Camera in X
			{
				m_Camera.rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene(FALSE);
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
	if (m_StatusBar != NULL && m_Skeleton.children != NULL)
	{
		if (m_SelectedBone != NULL)
		sprintf(message,"CurBone (%2.2f,%2.2f,%2.2f)",
			m_SelectedBone->rot.x,m_SelectedBone->rot.y,m_SelectedBone->rot.z);
		else
		sprintf(message,"No CurBone");
		//m_StatusBar->SetPaneStyle(1,SBPS_POPOUT);
		m_StatusBar->SetPaneText(1,message);
	}
}

void COGLView::HandleKeyDown(UINT nChar) 
{
}

void COGLView::HandleKeyUp(UINT nChar) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop2;
///////////////////////////////////////////////////////////////////////////////
	switch (nChar)
	{
	case VK_SPACE:
		if (m_Model.vertexData != NULL)
		{
			for (loop2 = 0; loop2 < m_Model.vertexCnt; loop2++)
				m_Model.CV_select[loop2] = FALSE;
		}
		m_Skeleton.id = (long)&m_Skeleton;
		break;
	case 'W':
		WeightBones();
		break;
	case 'B':
		break;
	case 'R':
		m_SelectedBone->rot.x = m_SelectedBone->b_rot.x;
		m_SelectedBone->rot.y = m_SelectedBone->b_rot.y;
		m_SelectedBone->rot.z = m_SelectedBone->b_rot.z;
		break;
	case 'D':
		m_DrawDeformed = !m_DrawDeformed;
		break;
	case 'F':
		break;
	}

	Invalidate(TRUE);

}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnViewResetskeleton
// Purpose:		Reset the view settings for the skeleton
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
void COGLView::OnViewResetskeleton() 
{
	ResetSkeleton(&m_Skeleton);
	drawScene(FALSE);	
	Invalidate(TRUE);
}
//// OnViewResetskeleton //////////////////////////////////////////////////////

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
// Function:	SetSkeletonList
// Purpose:		Sets up the Weights in the Skeleton for all vertices. Recursive
// Arguments:	Bone pointer
///////////////////////////////////////////////////////////////////////////////
void COGLView::SetSkeletonList(t_Bone *skeleton) 
{	
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,loop2;
	long vptr;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	
	if (skeleton->childCnt > 0 && m_Model.vertexData != NULL)
	{
		child = skeleton->children;
		for (loop = 0; loop < skeleton->childCnt; loop++,child++)
		{
			if (child->CV_weight) free(child->CV_weight);
			child->CV_weight = (t_VWeight *)malloc(sizeof(t_VWeight) * m_Model.vertexCnt);
			vptr = 0;
			for (loop2 = 0; loop2 < m_Model.vertexCnt; loop2++,vptr++)
			{
				child->CV_weight[vptr].vertex = loop2;
				child->CV_weight[vptr].weight = 0.0f;
			}
			if (child->childCnt > 0)
				SetSkeletonList(child);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SetBasePose
// Purpose:		Lock the current pose as the base for deformation
///////////////////////////////////////////////////////////////////////////////
void COGLView::SetBasePose()
{
	FreezeSkeleton(&m_Skeleton);
	GetBaseSkeletonMat(&m_Skeleton);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	WeightBones
// Purpose:		Set the weighting for a selected bone
// Arguments:	None
///////////////////////////////////////////////////////////////////////////////
GLvoid COGLView::WeightBones()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop3;
	long vptr;
///////////////////////////////////////////////////////////////////////////////
	if (m_Model.vertexData != NULL)
	{
		vptr = 0;
		for (loop3 = 0; loop3 < m_Model.vertexCnt; loop3++,vptr++)
		{
			// IF A POINT IS SELECTED SET THE WEIGHT
			if (m_Model.CV_select[loop3])
			{
				m_SelectedBone->CV_weight[loop3].weight = m_SelectedBone->animBlend;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	ClearBoneWeights
// Purpose:		Go through bones and clear weights for any selected vertex
// Arguments:	Bone pointer
///////////////////////////////////////////////////////////////////////////////
void COGLView::ClearBoneWeights(t_Bone *skeleton) 
{	
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	
	if (skeleton->childCnt > 0 && m_Model.vertexData != NULL)
	{
		child = skeleton->children;
		for (loop = 0; loop < skeleton->childCnt; loop++,child++)
		{
			for (int loop3 = 0; loop3 < m_Model.vertexCnt; loop3++)
			{
				// IF A POINT IS SELECTED SET THE WEIGHT
				if (m_Model.CV_select[loop3])
				{
					child->CV_weight[loop3].weight = 0.0f;
				}
			}

			// Recurse through Hierarchy
			if (child->childCnt > 0)
				ClearBoneWeights(child);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	IterateBoneWeights
// Purpose:		Go through bones and either read or write weight values
// Arguments:	Bone pointer, read or write BOOL, and file pointer
///////////////////////////////////////////////////////////////////////////////
void COGLView::IterateBoneWeights(t_Bone *skeleton, BOOL read, FILE *fp) 
{	
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
///////////////////////////////////////////////////////////////////////////////
	
	if (skeleton->childCnt > 0 && m_Model.vertexData != NULL)
	{
		child = skeleton->children;
		for (loop = 0; loop < skeleton->childCnt; loop++,child++)
		{
			if (read)
				fread(child->CV_weight,sizeof(t_VWeight),m_Model.vertexCnt, fp);
			else
				fwrite(child->CV_weight,sizeof(t_VWeight),m_Model.vertexCnt, fp);

			// Recurse through Hierarchy
			if (child->childCnt > 0)
				IterateBoneWeights(child, read, fp);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadWeights
// Purpose:		Load a Weight File
// Arguments:	Name of the file to open 
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::LoadWeights(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;		// I PREFER THIS STYLE OF FILE ACCESS
	int count;
///////////////////////////////////////////////////////////////////////////////
	if (fp = fopen((LPCTSTR)name,"rb")) {
		fread(&count,sizeof(int),1,fp);
		if (m_Model.vertexCnt == count)
		{
			IterateBoneWeights(&m_Skeleton, TRUE, fp);
		}
		else
		{
			::MessageBox(NULL,"Weight File Vertex Count Doesn't Match Model","Weight File Load Error",MB_OK);
		}

		// Assume if a Weight File is loaded it should be deformed
		m_DrawDeformed = TRUE;

		fclose(fp);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveWeights
// Purpose:		Save a Set of Weight Files
// Arguments:	Name of the file to save
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::SaveWeights(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;		// I PREFER THIS STYLE OF FILE ACCESS
///////////////////////////////////////////////////////////////////////////////
	if (fp = fopen((LPCTSTR)name,"wb")) {
		fwrite(&m_Model.vertexCnt,sizeof(int),1,fp);
		IterateBoneWeights(&m_Skeleton, FALSE, fp);
		fclose(fp);
	}

	return TRUE;
}

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
		free(m_Model.deformData);
		free(m_Model.CV_select);
		m_Model.vertexData = NULL;
		m_Model.deformData = NULL;
		m_Model.CV_select = NULL;
	}
	LoadOBJ((LPCSTR)name,&m_Model);
	SetSkeletonList(&m_Skeleton);		// Set up Weights
	SetBasePose();						// Lock in the Rest State
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadSkeletonFile
// Purpose:		Load a Skeleton into the system
// Arguments:	Name of the file to open
///////////////////////////////////////////////////////////////////////////////
BOOL COGLView::LoadSkeletonFile(CString name)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	DestroySkeleton(&m_Skeleton);
	LoadSkeleton(name,&m_Skeleton);
	SetSkeletonList(&m_Skeleton);		// Set up Weights
	return TRUE;
}

