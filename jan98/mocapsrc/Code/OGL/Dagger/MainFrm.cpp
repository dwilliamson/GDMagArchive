///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of Main Window of Hierarchical Animation System
//
// Created:
//		JL 9/1/97		
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
#include "mmsystem.h"		// NEED THIS FOR THE TIMEGETTIME
#include "Dagger.h"
#include "HierWin.h"
#include "LoadAnim.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/// Local Defines /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
// FOR NOW I DON'T HANDLE THE ADD BONE
//	ON_COMMAND(ID_ADD_BONE, OnAddBone)
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_RESETSKELETON, OnViewResetskeleton)
	ON_COMMAND(ID_BACK_FRAME, OnBackFrame)
	ON_COMMAND(ID_PLAY_BACK, OnPlayBack)
	ON_COMMAND(ID_STOP, OnStop)
	ON_COMMAND(ID_PLAY_FORWARD, OnPlayForward)
	ON_COMMAND(ID_FORWARD_FRAME, OnForwardFrame)
	ON_COMMAND(ID_FILE_LOADANIM, OnFileLoadanim)
	ON_WM_PAINT()
	ON_COMMAND(ID_ANIMATION_PLAYBACKSPEED, OnAnimationPlaybackspeed)
	ON_COMMAND(ID_WHICHOGL, OnWhichogl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_FRAME2,		// FOR DISPLAY OF FRAMECOUNT AND CURRENT FRAME
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_Animating = FALSE;
	m_ElapsedTime = ~(DWORD)0; // initialize to a big number
	m_Animation_Direction = 1;
	m_AnimSpeed = 30;			// DEFAULT PLAYBACK SPEED AS FRAME DELAY
	m_HArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);	

	InitializeSkeleton();
}

CMainFrame::~CMainFrame()
{
	m_HierWin.DestroyWindow();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
/// Local Variables ///////////////////////////////////////////////////////////
	RECT rect;
///////////////////////////////////////////////////////////////////////////////
	GetClientRect(&rect); 

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
/*	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
*/
	m_HierWin.Create( TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS, CRect(0, 32,160,rect.bottom - 20), this, 1001);
	m_HierWin.ShowWindow(TRUE);


	m_OGLView.Create(NULL,"Render Window",WS_CHILD | WS_VISIBLE, CRect(162, 32,rect.right - 2,rect.bottom - 20),this,104,&m_Skeleton);
	m_OGLView.ShowWindow(TRUE);

	m_OGLView.m_StatusBar = &m_wndStatusBar;

	m_OGLView.Invalidate(TRUE);

	
	// Now we initialize the animation code

	m_StartTime = ::timeGetTime(); // get time in ms

	// need a previous time if we start off animated
	if ( m_Animating )
		{
		m_previousElapsedTime = m_StartTime;
		}
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
/// Local Variables ///////////////////////////////////////////////////////////
	char message[80];			// PLACE TO PUT THE MESSAGE
	char who[80],which[80],version[80];	// OPENGL STUFF
///////////////////////////////////////////////////////////////////////////////		
	m_OGLView.GetGLInfo(who,which,version);
	sprintf(message,"Who:\t%s\nWhich:\t%s\nVersion:\t%s",who,which,version);
	MessageBox(message,"Which OpenGL Renderer?",MB_OK);
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
	MSG msg;
	CPaintDC dc(this); // device context for painting

	// IF I AM IN ANIMATING MODE
	while ( m_Animating == TRUE)
	{
		while (::PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_Animating = FALSE;
				::PostQuitMessage(0);
			}

			// Dispatch any messages as needed
			if (!AfxGetApp()->PreTranslateMessage(&msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			
			// Give the Idle system some time
			AfxGetApp()->OnIdle(0);
			AfxGetApp()->OnIdle(1);

		}
		m_ElapsedTime = ::timeGetTime(); // get current time

		// CHECK IF ENOUGH TIME HAS GONE BY TO DRAW THE NEXT FRAME
		if (ElapsedTimeRender() > (unsigned int)(1000/m_AnimSpeed) )
		{
			// ADVANCE THE ANIMATION
			BoneAdvanceFrame(&m_Skeleton,m_Animation_Direction,TRUE);
			// REDRAW THE OGL WINDOW
			m_OGLView.drawScene();	
			// RESET THE TIME COUNTER
			m_previousElapsedTime = m_ElapsedTime;
		}
	}
	
	m_OGLView.drawScene();	
}
/// OnPaint ////////////////////////////////////////////////////////////

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	// RESET THE HierWin WINDOW SIZE
	m_HierWin.SetWindowPos( &wndTopMost, 0, 32, 160, cy - 52, SWP_NOZORDER ); // 160
	m_OGLView.SetWindowPos( &wndTopMost, 162, 32, cx - 2 - 162, cy - 52, SWP_NOZORDER );
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
	m_OGLView.HandleKeyUp(nChar);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Hierarchy Manipulation Functions

void CMainFrame::InitializeSkeleton()
{
	// INITIALIZE SOME OF THE SKELETON VARIABLES
	ResetBone(&m_Skeleton, NULL);
	m_Skeleton.id = -1;
	strcpy(m_Skeleton.name,"Skeleton");
	m_Skeleton.rot.x = 0.0f;
	m_Skeleton.rot.y = 0.0f;
	m_Skeleton.rot.z = 0.0f;
	m_Skeleton.b_trans.y = -3.0f;
	m_Skeleton.b_trans.z = -40.0f;
	m_Skeleton.trans.y = -3.0f;
	m_Skeleton.trans.z = -40.0f;
}

/////////////////////////////////////////////////////////////////////////////
// View Manipulation Functions

void CMainFrame::OnViewResetskeleton() 
{
	// PASS THIS MESSAGE OFF TO THE OGL CLASS
	m_OGLView.OnViewResetskeleton();
}

/////////////////////////////////////////////////////////////////////////////
// Animation Control Functions

void CMainFrame::OnPlayBack() 
{
	m_Animating = TRUE;
	m_Animation_Direction = -1;
	Invalidate(TRUE);
}

void CMainFrame::OnBackFrame() 
{
	m_Animating = FALSE;
	BoneAdvanceFrame(&m_Skeleton,-1,TRUE);
	m_OGLView.drawScene();
}

void CMainFrame::OnStop() 
{
	m_Animating = FALSE;
}

void CMainFrame::OnForwardFrame() 
{
	m_Animating = FALSE;
	BoneAdvanceFrame(&m_Skeleton,1,TRUE);
	m_OGLView.drawScene();
}

void CMainFrame::OnPlayForward() 
{
	m_Animation_Direction = 1;
	m_Animating = TRUE;
	Invalidate(TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnAnimationPlaybackspeed
// Purpose:		Create dialog to change speed of playback
//				Response to Menu Message
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnAnimationPlaybackspeed() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	CPlaySpeed dialog;		// Class of Dialog box
///////////////////////////////////////////////////////////////////////////////		
	dialog.m_Playback_Speed = m_AnimSpeed;
	if (dialog.DoModal())
	{
		m_AnimSpeed = dialog.m_Playback_Speed;
		Invalidate(TRUE);
	}
}
/// OnAnimationPlaybackspeed ///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Animation File Functions

///////////////////////////////////////////////////////////////////////////////
// Procedure:	OnFileLoadanim
// Purpose:		Create dialog to load an animation file of various formats
//				Response to Menu Message
///////////////////////////////////////////////////////////////////////////////		
void CMainFrame::OnFileLoadanim() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Biovision BVA (*.bva)|*.bva||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|Acclaim File (*.asf)|*.asf|
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////

	dialog = new CFileDialog(TRUE,"bva",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		exten = dialog->GetFileExt();
		exten.MakeUpper();
		if (exten == "BVA")
		{
			DestroySkeleton(&m_Skeleton);
			InitializeSkeleton();
			LoadBVA(dialog->GetPathName(),&m_Skeleton);
			m_HierWin.ResetSkeleton();
			m_HierWin.SetSkeleton(&m_Skeleton, NULL);
			m_OGLView.drawScene();
			m_OGLView.OnViewResetskeleton();
		}

	}
	delete dialog;
}
/// OnFileLoadanim ////////////////////////////////////////////////////////////


/// Local Dialog Code /////////////////////////////////////////////////////////
//
// MFC LIKES TO PUT THESE IN A SEPARATE FILE, BUT IT LIKE THEM IN CODE THAT
// USES THEM
//
///////////////////////////////////////////////////////////////////////////////		

/////////////////////////////////////////////////////////////////////////////
// CPlaySpeed dialog


CPlaySpeed::CPlaySpeed(CWnd* pParent /*=NULL*/)
	: CDialog(CPlaySpeed::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlaySpeed)
	m_Playback_Speed = 0;
	//}}AFX_DATA_INIT
}


void CPlaySpeed::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaySpeed)
	DDX_Text(pDX, IDC_PLAYBACK_SPEED, m_Playback_Speed);
	DDV_MinMaxUInt(pDX, m_Playback_Speed, 1, 99999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlaySpeed, CDialog)
	//{{AFX_MSG_MAP(CPlaySpeed)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlaySpeed message handlers


