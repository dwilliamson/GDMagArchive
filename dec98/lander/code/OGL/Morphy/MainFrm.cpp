///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of 3D Morphing System
//
// Created:
//		JL 10/1/98		
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
#include "MainFrm.h"
#include "LoadDlg.h"

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

	m_Slider.Create(TBS_NOTICKS | TBS_BOTH | WS_CHILD | WS_VISIBLE, CRect(1, rect.bottom - 40,rect.right - 3,rect.bottom - 20),this,105);

	m_Slider.ShowWindow(TRUE);

	m_Slider.Invalidate(TRUE);

	m_OGLView.m_ptrStatusBar = &m_wndStatusBar;
	m_OGLView.Create(NULL,"Render Window",WS_CHILD | WS_VISIBLE, CRect(1, 1,rect.right - 3,rect.bottom),this,104,&m_Slider); // - 60 bottom
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
/// Local Variables ///////////////////////////////////////////////////////////
	char message[80];			// PLACE TO PUT THE MESSAGE
	char who[80],which[80],version[80],extensions[255];	// OPENGL STUFF
///////////////////////////////////////////////////////////////////////////////		
	m_OGLView.GetGLInfo(who,which,version,extensions);
	sprintf(message,"Who:\t%s\nWhich:\t%s\nVersion:\t%s\nExtensions:\t%s",
		who,which,version,extensions);
	MessageBox(message,"Which OpenGL Renderer?",MB_OK);
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

	m_OGLView.drawScene();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	m_OGLView.SetWindowPos( &wndTopMost, 1, 1, cx - 3, cy - 45 , SWP_NOZORDER ); // -60 bottom
	m_Slider.SetWindowPos( &wndTopMost, 1, cy - 44, cx - 3, 25 , SWP_NOZORDER ); // -60 bottom

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

/////////////////////////////////////////////////////////////////////////////
// Hierarchy Manipulation Functions

void CMainFrame::InitializeSkeleton()
{
}


// TOGGLE THE STATUS OF THE VIEW GEOMETRY FLAG
void CMainFrame::OnViewGeometry() 
{
	m_OGLView.m_DrawGeometry = !m_OGLView.m_DrawGeometry;
	m_OGLView.drawScene();
}

// SET THE CHECKED STATUS OF THE VIEW GEOMETRY MENU BASED ON STATUS
void CMainFrame::OnUpdateViewGeometry(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawGeometry );
}


void CMainFrame::OnFileOpen() 
{
	CLoadDlg dialog;
	if (dialog.DoModal() == IDOK)
	{
		m_OGLView.LoadFiles(dialog.m_File1, dialog.m_File2);
	}
}
