///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of 3D Blob modelling
//
// Created:
//		JL 11/20/99		
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
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_GEOMETRY, OnViewGeometry)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GEOMETRY, OnUpdateViewGeometry)
	ON_COMMAND(ID_HELP_WHICHOPENGL, OnHelpWhichopengl)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_GOOP_ADDBLOB, OnGoopAddblob)
	ON_COMMAND(ID_VIEW_DRAWBLOBS, OnViewDrawblobs)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAWBLOBS, OnUpdateViewDrawblobs)
	ON_COMMAND(ID_VIEW_DRAWFIELD, OnViewDrawfield)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAWFIELD, OnUpdateViewDrawfield)
	ON_COMMAND(ID_GOOP_EDITBLOB, OnGoopEditblob)
	ON_COMMAND(ID_GOOP_SYSTEMSETTINGS, OnGoopSystemsettings)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
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

	m_OGLView.m_ptrStatusBar = &m_wndStatusBar;
	m_OGLView.Create(NULL,"Render Window",WS_CHILD | WS_VISIBLE, CRect(1, 1,rect.right - 3,rect.bottom),this,104); // - 60 bottom
	m_OGLView.ShowWindow(TRUE);

	m_OGLView.Invalidate(TRUE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
/// Local Variables ///////////////////////////////////////////////////////////
	HICON hicon;
///////////////////////////////////////////////////////////////////////////////
	
	hicon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_ClassName = AfxRegisterWndClass(NULL,
		(HCURSOR)m_HArrow, (HBRUSH)::GetStockObject(DKGRAY_BRUSH), hicon); //m_HArrow
	cs.lpszClass = m_ClassName;

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnHelpWhichopengl
// Purpose:		Create dialog to Show which version of OGL is running
// Notes:		Pretty Handy info for debugging
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnHelpWhichopengl() 
{
	m_OGLView.GetGLInfo();
}
// OnWhichogl

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

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	m_OGLView.DrawScene();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	m_OGLView.SetWindowPos( &wndTopMost, 1, 1, cx - 3, cy - 21 , SWP_NOZORDER ); // -60 bottom

	CFrameWnd::OnSize(nType, cx, cy);
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_OGLView.HandleKeyDown(nChar);
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	switch (nChar)
	{
		case ' ':
			break;
	}
	m_OGLView.HandleKeyUp(nChar);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

// TOGGLE THE STATUS OF THE VIEW GEOMETRY FLAG
void CMainFrame::OnViewGeometry() 
{
	m_OGLView.m_DrawGeometry = !m_OGLView.m_DrawGeometry;
	m_OGLView.DrawScene();
}

// SET THE CHECKED STATUS OF THE VIEW GEOMETRY MENU BASED ON STATUS
void CMainFrame::OnUpdateViewGeometry(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawGeometry );
}

void CMainFrame::OnViewDrawblobs() 
{
	m_OGLView.m_DrawBlobs = !m_OGLView.m_DrawBlobs;
	m_OGLView.DrawScene();
}

void CMainFrame::OnUpdateViewDrawblobs(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawBlobs );	
}

void CMainFrame::OnViewDrawfield() 
{
	m_OGLView.m_DrawField = !m_OGLView.m_DrawField;
	m_OGLView.DrawScene();
}

void CMainFrame::OnUpdateViewDrawfield(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawField );	
}

void CMainFrame::OnFileOpen() 
{
	char szFilter[] = "Blob files (*.blb)|*.blb||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( TRUE, ".obj", NULL, NULL, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		m_OGLView.LoadFile(dialog.GetFileName( ),dialog.GetFileTitle( ) );
	}
}

void CMainFrame::OnFileSave() 
{
	char szFilter[] = "Blob files (*.blb)|*.blb||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( FALSE, ".obj", NULL, NULL, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		m_OGLView.SaveFile(dialog.GetFileName( ),dialog.GetFileTitle( ) );
	}
}

void CMainFrame::OnGoopAddblob() 
{
	m_OGLView.OnGoopAddblob();
}

void CMainFrame::OnGoopEditblob() 
{
	m_OGLView.EditBlob();
}

void CMainFrame::OnGoopSystemsettings() 
{
	m_OGLView.EditSys();	
}

