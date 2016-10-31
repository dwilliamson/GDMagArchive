///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of Friction Demonstration
//
// Created:
//		JL 11/20/98		
//		JL 06/01/99		Modified from other code
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Friction.h"
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
	ON_COMMAND(ID_SIMULATION_RUNNING, OnSimulationRunning)
	ON_UPDATE_COMMAND_UI(ID_SIMULATION_RUNNING, OnUpdateSimulationRunning)
	ON_COMMAND(ID_SIMULATION_RESET, OnSimulationReset)
	ON_COMMAND(ID_SIMULATION_SETSIMPROPERTIES, OnSimulationSetsimproperties)
	ON_COMMAND(ID_SIMULATION_USEGRAVITY, OnSimulationUsegravity)
	ON_UPDATE_COMMAND_UI(ID_SIMULATION_USEGRAVITY, OnUpdateSimulationUsegravity)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_VIEW_SHOWGEOMETRY, OnViewShowgeometry)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWGEOMETRY, OnUpdateViewShowgeometry)
	ON_COMMAND(ID_VIEW_SHOWSPRINGS, OnViewShowsprings)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSPRINGS, OnUpdateViewShowsprings)
	ON_COMMAND(ID_VIEW_SHOWVERTICES, OnViewShowvertices)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWVERTICES, OnUpdateViewShowvertices)
	ON_WM_CLOSE()
	ON_COMMAND(ID_SIMULATION_SETVERTEXMASS, OnSimulationSetvertexmass)
	ON_COMMAND(ID_FILE_NEWSYSTEM, OnFileNewsystem)
	ON_COMMAND(ID_SIMULATION_SETTIMINGPROPERTIES, OnSimulationSettimingproperties)
	ON_COMMAND(ID_INTEGRATOR_EULER, OnIntegratorEuler)
	ON_UPDATE_COMMAND_UI(ID_INTEGRATOR_EULER, OnUpdateIntegratorEuler)
	ON_COMMAND(ID_INTEGRATOR_MIDPOINT, OnIntegratorMidpoint)
	ON_UPDATE_COMMAND_UI(ID_INTEGRATOR_MIDPOINT, OnUpdateIntegratorMidpoint)
	ON_COMMAND(ID_INTEGRATOR_RUNGEKUTTA4, OnIntegratorRungekutta4)
	ON_UPDATE_COMMAND_UI(ID_INTEGRATOR_RUNGEKUTTA4, OnUpdateIntegratorRungekutta4)
	ON_COMMAND(ID_SIMULATION_USEFRICTION, OnSimulationUsefriction)
	ON_UPDATE_COMMAND_UI(ID_SIMULATION_USEFRICTION, OnUpdateSimulationUsefriction)
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
/// Local Variables ///////////////////////////////////////////////////////////
	char message[1024];			// PLACE TO PUT THE MESSAGE
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
//	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
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


void CMainFrame::OnFileNewsystem() 
{
	m_OGLView.NewSystem();
}

void CMainFrame::OnFileOpen() 
{
	char szFilter[] = "DPS files (*.dps)|*.dps|OBJ files (*.obj)|*.obj||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( TRUE, ".obj", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		m_OGLView.LoadFile(dialog.GetFileName( ),dialog.GetFileTitle( ),dialog.GetFileExt()  );
		m_OGLView.Invalidate(TRUE);
	}
}

void CMainFrame::OnFileSave() 
{
	char szFilter[] = "DPS files (*.dps)|*.dps||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( FALSE, ".dps", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		m_OGLView.SaveFile(dialog.GetFileName( ),dialog.GetFileTitle( ));
		m_OGLView.Invalidate(TRUE);
	}
}

void CMainFrame::OnSimulationRunning() 
{
	m_OGLView.HandleKeyUp('R');  // FORCE SYSTEM TO START RUNNING THROUGH KEYPRESS
}

void CMainFrame::OnUpdateSimulationRunning(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_SimRunning );
}

void CMainFrame::OnSimulationReset() 
{
	m_OGLView.m_PhysEnv.ResetWorld();
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnSimulationSetsimproperties() 
{
	m_OGLView.OnSimulationSetsimproperties();
}

void CMainFrame::OnSimulationSettimingproperties() 
{
	m_OGLView.OnSetTimeProperties();	
}

void CMainFrame::OnSimulationSetvertexmass() 
{
	m_OGLView.OnSimulationSetVertexMass();
}

void CMainFrame::OnSimulationUsegravity() 
{
	m_OGLView.m_PhysEnv.m_UseGravity = !m_OGLView.m_PhysEnv.m_UseGravity;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateSimulationUsegravity(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_UseGravity );
}

void CMainFrame::OnSimulationUsefriction() 
{
	m_OGLView.m_PhysEnv.m_UseFriction = !m_OGLView.m_PhysEnv.m_UseFriction;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateSimulationUsefriction(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_UseFriction );
}

void CMainFrame::OnViewShowgeometry() 
{
	m_OGLView.m_DrawGeometry = !m_OGLView.m_DrawGeometry;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowgeometry(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawGeometry );
}

void CMainFrame::OnViewShowsprings() 
{
	m_OGLView.m_PhysEnv.m_DrawSprings = !m_OGLView.m_PhysEnv.m_DrawSprings;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowsprings(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_DrawSprings );
}

void CMainFrame::OnViewShowvertices() 
{
	m_OGLView.m_PhysEnv.m_DrawVertices = !m_OGLView.m_PhysEnv.m_DrawVertices;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowvertices(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_DrawVertices );
}

void CMainFrame::OnClose() 
{
	m_OGLView.m_SimRunning = FALSE;
	
	CFrameWnd::OnClose();
}

BOOL CMainFrame::DestroyWindow() 
{
	
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnIntegratorEuler() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = EULER_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateIntegratorEuler(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == EULER_INTEGRATOR );
}

void CMainFrame::OnIntegratorMidpoint() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = MIDPOINT_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateIntegratorMidpoint(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == MIDPOINT_INTEGRATOR );
}

void CMainFrame::OnIntegratorRungekutta4() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = RK4_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateIntegratorRungekutta4(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == RK4_INTEGRATOR );
}

