///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of 3D Paint System
//
// Created:
//		JL 8/4/00
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
#include "mmsystem.h"		// NEED THIS FOR THE TIMEGETTIME
#include "Expressionist.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Local Defines /////////////////////////////////////////////////////////////
#define OGLWIN_START_X	1			// STARTING X POSITION OF OPENGL WINDOW
#define OGLWIN_START_Y	1			// STARTING Y POSITION OF OPENGL WINDOW
#define OGLWIN_WIDTH	4			// WIDTH OF OPENGL WINDOW SUBTRACTED FROM MAX
#define OGLWIN_BOTTOM	20			// BOTTOM BORDER OF OPENGL WINDOW
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_COMMAND(ID_WHICHOGL, OnWhichogl)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_SETTINGS_USELIGHTING, OnSettingsUselighting)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_USELIGHTING, OnUpdateSettingsUselighting)
	ON_COMMAND(ID_PAINT_SETCOLOR, OnPaintSetcolor)
	ON_COMMAND(ID_UVCOORDINATES_SPHERICAL, OnUvcoordinatesSpherical)
	ON_COMMAND(ID_UVCOORDINATES_CYLINDRICAL, OnUvcoordinatesCylindrical)
	ON_COMMAND(ID_UVCOORDINATES_PLANAR_XAXIS, OnUvcoordinatesPlanarXaxis)
	ON_COMMAND(ID_UVCOORDINATES_PLANAR_YAXIS, OnUvcoordinatesPlanarYaxis)
	ON_COMMAND(ID_UVCOORDINATES_PLANAR_ZAXIS, OnUvcoordinatesPlanarZaxis)
	ON_COMMAND(ID_SETTINGS_BILINEARFILTER, OnSettingsBilinearfilter)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_BILINEARFILTER, OnUpdateSettingsBilinearfilter)
	ON_COMMAND(ID_UVCOORDINATES, OnUvcoordinates)
	ON_COMMAND(ID_FILE_SAVETEXTURE, OnFileSavetexture)
	ON_COMMAND(ID_FILE_SAVEOBJ, OnFileSaveobj)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_Wireframe = TRUE;
	m_HArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);	

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
/// Local Variables ///////////////////////////////////////////////////////////
	RECT rect;
///////////////////////////////////////////////////////////////////////////////
	GetClientRect(&rect); 

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_OGLView.Create(NULL,"Render Window",WS_CHILD | WS_VISIBLE, 
		CRect(OGLWIN_START_X, OGLWIN_START_Y,rect.right - OGLWIN_WIDTH,rect.bottom - OGLWIN_BOTTOM),this,104);
	m_OGLView.ShowWindow(TRUE);

	m_OGLView.m_StatusBar = &m_wndStatusBar;

	m_OGLView.Invalidate(TRUE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	HICON hicon;
	
	hicon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_ClassName = AfxRegisterWndClass(NULL,
		(HCURSOR)m_HArrow, (HBRUSH)::GetStockObject(DKGRAY_BRUSH), hicon); //m_HArrow
	cs.lpszClass = m_ClassName;

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame implementation

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnWhichogl
// Purpose:		Create dialog to Show which version of OGL is running
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnWhichogl()
{
	m_OGLView.GetGLInfo();
}
// OnWhichogl

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnPaint
// Purpose:		This routine grabs the message loop if I am animating and
//				handles the messages.  This way I can play back as fast
//              as possible
// Reference:	OpenGL Programming for Windows 95 by Ron Fosner
//				Sort of a variation on that code
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	m_OGLView.drawScene();	
}
/// OnPaint ////////////////////////////////////////////////////////////

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	// RESET THE m_OGLView WINDOW SIZE
	m_OGLView.SetWindowPos( &wndTopMost, OGLWIN_START_X, OGLWIN_START_Y, cx - OGLWIN_WIDTH, cy - OGLWIN_BOTTOM, SWP_NOZORDER );
	// RESET THE ACTUAL OPENGL WINDOW SIZE
	m_OGLView.resize(  cx - OGLWIN_WIDTH, cy - OGLWIN_BOTTOM);
	CFrameWnd::OnSize(nType, cx, cy);
}

// HAVEN'T IMPLEMENTED ADDING A BONE
#if 0
void CMainFrame::OnAddBone() 
{
	m_HierWin.AddBone();
}
#endif

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_OGLView.HandleKeyDown(nChar);
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
	m_OGLView.HandleKeyUp(nChar);
}

/////////////////////////////////////////////////////////////////////////////
// Hierarchy Manipulation Functions

/////////////////////////////////////////////////////////////////////////////
// View Manipulation Functions

/////////////////////////////////////////////////////////////////////////////


void CMainFrame::OnFileOpen() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Object Mesh OBJ (*.obj)|*.obj||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(TRUE,"obj",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		exten = dialog->GetFileExt();
		exten.MakeUpper();
		m_OGLView.LoadOBJModel(dialog->GetPathName());
		m_OGLView.drawScene();
	}
	delete dialog;
}


void CMainFrame::OnFileSaveobj() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Object Mesh OBJ (*.obj)|*.obj||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(FALSE,"obj",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		m_OGLView.SaveOBJModel(dialog->GetPathName());
		m_OGLView.drawScene();
	}
	delete dialog;
}

// Turn on/off the OpenGL Normal Based Lighting
void CMainFrame::OnSettingsUselighting() 
{
	m_OGLView.m_UseLighting = !m_OGLView.m_UseLighting;
	m_OGLView.drawScene();	
}

// Update the menu bar based on lighting
void CMainFrame::OnUpdateSettingsUselighting(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_UseLighting );
}

// Turn on/off the OpenGL Bilinear Filtering
void CMainFrame::OnSettingsBilinearfilter() 
{
	m_OGLView.m_Bilinear = !m_OGLView.m_Bilinear;
	m_OGLView.drawScene();	
}

void CMainFrame::OnUpdateSettingsBilinearfilter(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_Bilinear );
}

void CMainFrame::OnPaintSetcolor() 
{
	long color;
	// Need to reverse Windows BGR color to RGB
	color = MAKERGB(GETB(m_OGLView.m_PaintColor),GETG(m_OGLView.m_PaintColor),GETR(m_OGLView.m_PaintColor));
	CColorDialog	dialog(color,CC_FULLOPEN ,this);
	
	if (dialog.DoModal() == IDOK)
	{
		color = dialog.GetColor( );
		// Need to reverse Windows BGR color to RGB
		m_OGLView.m_PaintColor = MAKERGB(GETB(color),GETG(color),GETR(color));
	}
}

void CMainFrame::OnUvcoordinatesSpherical() 
{
	m_OGLView.SphereMapModel();
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}

void CMainFrame::OnUvcoordinatesCylindrical() 
{
	m_OGLView.CylinderMapModel(1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}

void CMainFrame::OnUvcoordinatesPlanarXaxis() 
{
	m_OGLView.PlanarMapModel(YZ_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}

void CMainFrame::OnUvcoordinatesPlanarYaxis() 
{
	m_OGLView.PlanarMapModel(XZ_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}

void CMainFrame::OnUvcoordinatesPlanarZaxis() 
{
	m_OGLView.PlanarMapModel(XY_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}


// Fix up seam in UV coordinates manually if needed
void CMainFrame::OnUvcoordinates() 
{
	m_OGLView.FixupMapCoords();
	m_OGLView.drawScene();	
}

void CMainFrame::OnFileSavetexture() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "TGA Texture (*.tga)|*.tga||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(FALSE,"tga",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		exten = dialog->GetFileExt();
		exten.MakeUpper();
		m_OGLView.SaveTexture((char *)(LPCSTR)dialog->GetPathName());
	}
	delete dialog;
}
