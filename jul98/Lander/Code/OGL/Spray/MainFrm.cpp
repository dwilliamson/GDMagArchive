///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of Main Window of Particle System
//
// Created:
//		JL 2/18/98		
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
#include <Direct.h>
#include "Spray.h"

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
	ON_COMMAND(ID_HELP_WHICHOPENGL, OnHelpWhichopengl)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_SETTINGS_ANTIALIASPOINTS, OnSettingsAntialiaspoints)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_ANTIALIASPOINTS, OnUpdateSettingsAntialiaspoints)
	ON_COMMAND(ID_SETTINGS_EDITEMITTER, OnSettingsEditemitter)
	ON_COMMAND(ID_SETTINGS_SHOWAXIS, OnSettingsShowaxis)
	ON_UPDATE_COMMAND_UI(ID_SETTINGS_SHOWAXIS, OnUpdateSettingsShowaxis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_COUNT2,	// MY ADDITION FOR PUTTING SETTINGS
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
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnHelpWhichopengl() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char message[80];			// PLACE TO PUT THE MESSAGE
	char who[80],which[80],version[80];	// OPENGL STUFF
///////////////////////////////////////////////////////////////////////////////		
	m_OGLView.GetGLInfo(who,which,version);
	sprintf(message,"Who:\t%s\nWhich:\t%s\nVersion:\t%s",who,which,version);
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

	m_OGLView.drawScene(FALSE);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	m_OGLView.SetWindowPos( &wndTopMost, 1, 1, cx - 3, cy - 20 , SWP_NOZORDER ); // -60 bottom
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
		case 'Q':
			break;
	}
	m_OGLView.HandleKeyUp(nChar);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Hierarchy Manipulation Functions

///////////////////////////////////////////////////////////////////////////////
// Function:	IdleFunc
// Purpose:		Process state changes if animating
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::IdleFunc() 
{
	m_OGLView.IdleFunc();
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileNew
// Purpose:		Clear the weight settings for the model
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileNew() 
{
	m_OGLView.resetEmitter();
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileOpen
// Purpose:		Load the Emitter settings for the system
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileOpen() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Particle Emitter Files (*.pef)|*.PEF|All Files (*.*)|*.*||";
	char directory[80];
	CFileDialog	*dialog;
///////////////////////////////////////////////////////////////////////////////		
	// HAD TO ADD DIRECTORY STUFF SINCE DIALOG CHANGES DIRECTORY
	_getcwd(directory,80);
	dialog = new CFileDialog(TRUE,"PEF",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		if (!m_OGLView.GetEmitter(dialog->GetPathName()))
		{
			MessageBox("Unable to load Emitter File","Error",MB_OK);
		}
	}
	// RESET THE MAIN DIRECTORY
	_chdir(directory);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileSave
// Purpose:		Save the Emitter settings for the system
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileSave() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Weight Files (*.PEF)|*.PEF|All Files (*.*)|*.*||";
	char directory[80];
	CFileDialog	*dialog;
///////////////////////////////////////////////////////////////////////////////		
	// HAD TO ADD DIRECTORY STUFF SINCE DIALOG CHANGES DIRECTORY
	_getcwd(directory,80);
	dialog = new CFileDialog(FALSE,"PEF",NULL,OFN_OVERWRITEPROMPT,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		if (!m_OGLView.SaveEmitter(dialog->GetPathName()))
		{
			MessageBox("Unable to Save Emitter File","Error",MB_OK);
		}
	}
	// RESET THE MAIN DIRECTORY
	_chdir(directory);
}

void CMainFrame::OnSettingsAntialiaspoints() 
{
	m_OGLView.m_AntiAlias = !m_OGLView.m_AntiAlias;
}

void CMainFrame::OnUpdateSettingsAntialiaspoints(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_AntiAlias );
}

void CMainFrame::OnSettingsEditemitter() 
{
	m_OGLView.editEmitter(&m_OGLView.m_Emitter);
}

void CMainFrame::OnSettingsShowaxis() 
{
	m_OGLView.m_DrawAxis = !m_OGLView.m_DrawAxis;
}

void CMainFrame::OnUpdateSettingsShowaxis(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawAxis );
}
