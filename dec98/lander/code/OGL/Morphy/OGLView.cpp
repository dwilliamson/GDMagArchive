///////////////////////////////////////////////////////////////////////////////
//
// OGLView.cpp : implementation file
//
// Purpose:	Implementation of OpenGL Window of 3D Morphing System
//
// Created:
//		JL 10/1/98		
//
// The function morphModel() does the main morphing work.
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
#include "Morphy.h"
#include "OGLView.h"
#include "LoadOBJ.h"
#include "Bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Application Definitions ///////////////////////////////////////////////////
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define ROTATE_SPEED		1.0		// SPEED OF ROTATION

#define LERP(a,b,c)  (a + ((b - a) * c))
///////////////////////////////////////////////////////////////////////////////

/// Global Variables //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// COGLView

COGLView::COGLView()
{
	// INITIALIZE THE MODE KEYS
	m_DrawGeometry = TRUE;

	ResetBone(&m_Skeleton, NULL);
	m_Skeleton.id = -1;
	strcpy(m_Skeleton.name,"Skeleton");
	m_Skeleton.b_trans.z = -100.0f;
	m_Skeleton.trans.z = -100.0f;

}

COGLView::~COGLView()
{
	DestroySkeleton(&m_Skeleton);
}


BOOL COGLView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CSlider *slider, CCreateContext* pContext) 
{
/// Local Variables ///////////////////////////////////////////////////////////
	t_Visual	*visual = NULL;
///////////////////////////////////////////////////////////////////////////////
	// I AM GOING TO HAVE TWO OBJECTS TO MORPH BETWEEN
	visual = (t_Visual *)malloc(sizeof(t_Visual) * 3);
	m_Skeleton.visuals = visual;
	m_Skeleton.visualCnt = 3;

	strcpy(visual[0].map, "");
	strcpy(visual[1].map, "");
	strcpy(visual[2].map, "");

	m_Slider = slider;
	m_curVisual = 0;
	m_MorphPos = 0.0f;

	visual[0].vertexData = NULL;
	visual[1].vertexData = NULL;
	visual[2].vertexData = NULL;
//	LoadOBJ("Hero1.obj",&m_Skeleton.visuals[0]);
//	LoadOBJ("Hero2.obj",&m_Skeleton.visuals[1]);

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

	glDisable(GL_TEXTURE_2D);

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
    gluPerspective(10.0, aspect, 1.0, 2000.0);
    glMatrixMode(GL_MODELVIEW);
}    

GLvoid COGLView::initializeGL(GLsizei width, GLsizei height)
{
/// Local Variables ///////////////////////////////////////////////////////////
    GLfloat aspect;
	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightpos[] = { 0.30f, 0.3f, 1.0f, 0.0f };		// .5 .5 1.0
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    aspect = (GLfloat)width/(GLfloat)height;
	// Establish viewing volume
	gluPerspective(10.0, aspect,1, 2000);
    glMatrixMode(GL_MODELVIEW);

	// SET SOME OGL INITIAL STATES SO THEY ARE NOT DONE IN THE DRAW LOOP
	glPolygonMode(GL_FRONT,GL_FILL);
//	glPolygonMode(GL_FRONT,GL_LINE);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

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

/*
 * 'LoadBoneTexture()' - Load texture images for the bone
 *
 * NOTES: THIS SHOULD LOOK THROUGH ALL BONES AND SEE IF TEXTURE IS ALREADY LOADED
 */
GLvoid COGLView::LoadBoneTexture(t_Bone *curBone)
{
	GLubyte	*rgb;				/* Bitmap RGB pixels */
	char realFilename[80],texName[80];
	int pos;
	GLuint temp;
	tTGAHeader_s tgaHeader;

//	m_SelectedBone = curBone;

	// GENERATE THE OPENGL TEXTURE ID
	glGenTextures(1,&temp);

	curBone->visuals[0].glTex = temp;
	// GET THE NAME OF THE ACTUAL IMAGE FROM THE PIC RESOURCE
	strcpy(texName,curBone->visuals[0].map);

	pos = strlen(texName);
	while (texName[pos] != '\\' && texName[pos] != '/' && pos > 0)
	{
		if (texName[pos] == '.') texName[pos] = '\0';
		pos--;
	}

	if (pos > 0) pos++;

	sprintf(realFilename,"%s.tga",&texName[pos]);
	/*
	* Try loading the bitmap and converting it to RGB...
	*/

	rgb = LoadTGAFile( realFilename,&tgaHeader);
	if (rgb == NULL)
	{
		::MessageBox(NULL,"Unable to Open File...",realFilename,MB_OK);
		curBone->visuals[0].glTex = 0;
	    return;
	}

	glBindTexture(GL_TEXTURE_2D, curBone->visuals[0].glTex);

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

	if (tgaHeader.d_pixel_size == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, 4, tgaHeader.d_width, tgaHeader.d_height, 0,
                 GL_RGBA , GL_UNSIGNED_BYTE, rgb);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 3, tgaHeader.d_width, tgaHeader.d_height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, rgb);

	/*
	*Free the bitmap and RGB images, then return 0 (no errors).
	*/

	free(rgb);
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
// Procedure:	morphModel
// Purpose:		Does the Morph for the Model
// Arguments:	Pointer to main bone
// Notes:		Normals should probably be normalized (but it seems to work fine)
///////////////////////////////////////////////////////////////////////////////		
GLvoid COGLView::morphModel(t_Bone *curBone)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int loop,pointloop;
	float *dest,*src1,*src2,ratio;
///////////////////////////////////////////////////////////////////////////////
	if (curBone->visualCnt > m_curVisual && curBone->visuals[0].vertexData != NULL)
	{
		src1 = curBone->visuals[0].vertexData;		// FRAME 1
		src2 = curBone->visuals[1].vertexData;		// FRAME 2
		dest = curBone->visuals[2].vertexData;		// DESTINATION FOR MORPHED FRAME
		ratio = m_Slider->GetSetting();				// GET MORPH VALUE (0 - 1)
		// LOOP THROUGH THE VERTICES
		for (loop = 0; loop < curBone->visuals[0].triCnt * 3; loop++)	
		{
			// GO THROUGH EACH ELEMENT IN THE VERTEX STRUCTURE
			for (pointloop = 0; pointloop < curBone->visuals[0].vSize; pointloop++)
			{
				// THE NEW POSITION IS A LERP BETWEEN THE TWO POINTS
				dest[(loop * curBone->visuals[0].vSize) + pointloop] = 
					LERP(src1[(loop * curBone->visuals[0].vSize) + pointloop],
						 src2[(loop * curBone->visuals[0].vSize) + pointloop],ratio);
			}
		}
	}
}
// morphModel


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
	GLfloat  color[4];
///////////////////////////////////////////////////////////////////////////////
	if (curBone->visualCnt > 2 && curBone->visuals[2].vertexData != NULL)
	{
		glEnable(GL_LIGHTING);
		if (curBone->visuals[0].glTex > 0)
			glEnable(GL_TEXTURE_2D);
		morphModel(curBone);		// PERFORM THE MORPH
		color[0] = curBone->visuals[0].Kd.r;
		color[1] = curBone->visuals[0].Kd.g;
		color[2] = curBone->visuals[0].Kd.b;
		color[3] = 1.0f;
		glMaterialfv(GL_FRONT,GL_DIFFUSE,color);
		color[0] = curBone->visuals[0].Ka.r;
		color[1] = curBone->visuals[0].Ka.g;
		color[2] = curBone->visuals[0].Ka.b;
		color[3] = 1.0f;
		glMaterialfv(GL_FRONT,GL_AMBIENT,color);
		color[0] = curBone->visuals[0].Ks.r;
		color[1] = curBone->visuals[0].Ks.g;
		color[2] = curBone->visuals[0].Ks.b;
		color[3] = 1.0f;
		glMaterialfv(GL_FRONT,GL_SPECULAR,color);
		// Declare the Array of Data
		glInterleavedArrays(curBone->visuals[2].dataFormat,0,(GLvoid *)curBone->visuals[2].vertexData);
		glDrawArrays(GL_TRIANGLES,0,curBone->visuals[2].triCnt * 3);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
	}
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

	if (m_Skeleton.rot.y  > 360.0f) m_Skeleton.rot.y  -= 360.0f;
    if (m_Skeleton.rot.x   > 360.0f) m_Skeleton.rot.x   -= 360.0f;
    if (m_Skeleton.rot.z > 360.0f) m_Skeleton.rot.z -= 360.0f;
	
    glDisable(GL_DEPTH_TEST);	// TURN OFF DEPTH TEST FOR CLEAR

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);	// ENABLE DEPTH TESTING

    glPushMatrix();

    // Set root skeleton's orientation and position
    glTranslatef(m_Skeleton.trans.x, m_Skeleton.trans.y, m_Skeleton.trans.z);

	// ROTATE THE ROOT
	glRotatef(m_Skeleton.rot.z, 1.0f, 0.0f, 0.0f);
    glRotatef(m_Skeleton.rot.y, 0.0f, 1.0f, 0.0f);
 	glRotatef(m_Skeleton.rot.x, 0.0f, 0.0f, 1.0f); 

	// IF I WANT TO, DRAW THE GEOMETRY
	if (m_DrawGeometry)
		drawModel(&m_Skeleton);

	// DRAW THE AXIS
	glCallList(OGL_AXIS_DLIST);

    glPopMatrix();
    glFinish();

    SwapBuffers(m_hDC);
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
	m_mousepos = point;
	m_Grab_Rot_X = 	m_Skeleton.rot.x;
	m_Grab_Rot_Y = 	m_Skeleton.rot.y;
	m_Grab_Rot_Z = 	m_Skeleton.rot.z;
	m_Grab_Trans_X = 	m_Skeleton.trans.x;
	m_Grab_Trans_Y = 	m_Skeleton.trans.y;
	m_Grab_Trans_Z = 	m_Skeleton.trans.z;
	CWnd::OnLButtonDown(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnRButtonDown
// Purpose:		Right button down grabs the current point pos so I can use it
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_mousepos = point;
	m_Grab_Rot_X = 	m_Skeleton.rot.x;
	m_Grab_Rot_Y = 	m_Skeleton.rot.y;
	m_Grab_Rot_Z = 	m_Skeleton.rot.z;
	m_Grab_Trans_X = 	m_Skeleton.trans.x;
	m_Grab_Trans_Y = 	m_Skeleton.trans.y;
	m_Grab_Trans_Z = 	m_Skeleton.trans.z;
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
	case '1': m_curVisual = 0;
		break;
	case '2': m_curVisual = 1;
		break;
	case 'O': 
		glPolygonMode(GL_FRONT,GL_LINE);
		break;
	case 'F': 
		glPolygonMode(GL_FRONT,GL_FILL);
		break;
	}
	drawScene();
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnMouseMove
// Purpose:		Handle mouse moves while pressed
///////////////////////////////////////////////////////////////////////////////		
void COGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (nFlags & MK_LBUTTON > 0)
	{
		// IF I AM HOLDING THE 'CONTROL' BUTTON TRANSLATE
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton.trans.x = m_Grab_Trans_X + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Skeleton.trans.y = m_Grab_Trans_Y - (.1f * (point.y - m_mousepos.y));
				drawScene();
			}
		}	
		// ELSE ROTATE THE BONE
		else
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton.rot.y = m_Grab_Rot_Y + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
				drawScene();
			}
			if ((point.y - m_mousepos.y) != 0)
			{
				m_Skeleton.rot.z = m_Grab_Rot_Z + ((float)ROTATE_SPEED * (point.y - m_mousepos.y));
				drawScene();
			}
		}
	}
	else if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
	{
		if ((nFlags & MK_CONTROL) > 0)
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton.trans.z = m_Grab_Trans_Z + (.1f * (point.x - m_mousepos.x));
				drawScene();
			}
		}
		else
		{
			if ((point.x - m_mousepos.x) != 0)
			{
				m_Skeleton.rot.x = m_Grab_Rot_X + ((float)ROTATE_SPEED * (point.x - m_mousepos.x));
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
// Procedure:	LoadFiles
// Purpose:		Loads the OBJ files into memory
///////////////////////////////////////////////////////////////////////////////		
void COGLView::LoadFiles(CString file1, CString file2) 
{
	if (m_Skeleton.visuals[0].vertexData != NULL)
	{
		free(m_Skeleton.visuals[0].vertexData);
		m_Skeleton.visuals[0].vertexData = NULL;
	}
	if (m_Skeleton.visuals[1].vertexData != NULL)
	{
		free(m_Skeleton.visuals[1].vertexData);
		m_Skeleton.visuals[1].vertexData = NULL;
	}
	if (m_Skeleton.visuals[2].vertexData != NULL)
	{
		free(m_Skeleton.visuals[2].vertexData);
		m_Skeleton.visuals[2].vertexData = NULL;
	}

	if (file1.GetLength() > 0 && LoadOBJ((char *)(LPCTSTR)file1 ,&m_Skeleton.visuals[0]))
	{
		if (file2.GetLength() > 0 && LoadOBJ((char *)(LPCTSTR)file2,&m_Skeleton.visuals[1]))
		{
			if (m_Skeleton.visuals[0].triCnt == m_Skeleton.visuals[1].triCnt)
			{
				m_Skeleton.visuals[2].dataFormat = m_Skeleton.visuals[0].dataFormat;
				m_Skeleton.visuals[2].vertexData = (float *)malloc(sizeof(float) * m_Skeleton.visuals[0].vSize * m_Skeleton.visuals[0].triCnt * 3);
				memcpy(m_Skeleton.visuals[2].vertexData,m_Skeleton.visuals[0].vertexData,sizeof(float) * m_Skeleton.visuals[0].vSize * m_Skeleton.visuals[0].triCnt * 3);

				m_Skeleton.visuals[2].triCnt = m_Skeleton.visuals[0].triCnt;

				if (m_Skeleton.visualCnt > 0 && strlen(m_Skeleton.visuals[0].map) > 0 )
					LoadBoneTexture(&m_Skeleton);
				else
				{
					m_Skeleton.visuals[0].glTex = 0;
					glBindTexture(GL_TEXTURE_2D, 0);
				}

			}
			else
			{
				MessageBox("OBJ Files must have the same number of triangles","Error",MB_OK);
				free(m_Skeleton.visuals[0].vertexData);
				free(m_Skeleton.visuals[1].vertexData);
				m_Skeleton.visuals[0].vertexData = NULL;
				m_Skeleton.visuals[1].vertexData = NULL;
			}
		}
		else
		{
			MessageBox("Two Valid OBJ Files must be loaded","Error",MB_OK);
			free(m_Skeleton.visuals[0].vertexData);
			m_Skeleton.visuals[0].vertexData = NULL;
		}
	}
	else
		MessageBox("Two Valid OBJ Files must be loaded","Error",MB_OK);

}
