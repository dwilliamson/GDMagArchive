///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of 3D Cloth Simulation
//
// Created:
//		JL 2/12/99		
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
#include "Clothy.h"
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
	ON_COMMAND(ID_FILE_CREATECLOTHPATCH, OnFileCreateclothpatch)
	ON_COMMAND(ID_SIMULATION_SETVERTEXPROPERTIES, OnSimulationSetvertexproperties)
	ON_COMMAND(ID_VIEW_COLLISIONACTIVE, OnViewCollisionactive)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLLISIONACTIVE, OnUpdateViewCollisionactive)
	ON_COMMAND(ID_VIEW_SHOWSHEAR, OnViewShowshear)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSHEAR, OnUpdateViewShowshear)
	ON_COMMAND(ID_VIEW_SHOWSTRUCTURAL, OnViewShowstructural)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSTRUCTURAL, OnUpdateViewShowstructural)
	ON_COMMAND(ID_VIEW_SHOWBEND, OnViewShowbend)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWBEND, OnUpdateViewShowbend)
	ON_COMMAND(ID_INTEULER, OnInteuler)
	ON_UPDATE_COMMAND_UI(ID_INTEULER, OnUpdateInteuler)
	ON_COMMAND(ID_INTMIDPOINT, OnIntmidpoint)
	ON_UPDATE_COMMAND_UI(ID_INTMIDPOINT, OnUpdateIntmidpoint)
	ON_COMMAND(ID_INTRK4, OnIntrk4)
	ON_UPDATE_COMMAND_UI(ID_INTRK4, OnUpdateIntrk4)
	ON_COMMAND(ID_FILE_NEWSYSTEM, OnFileNewsystem)
	ON_COMMAND(ID_SIMULATION_SETTIMINGPROPERTIES, OnSimulationSettimingproperties)
	ON_COMMAND(ID_SIMULATION_ADDCOLLISIONSPHERE, OnSimulationAddcollisionsphere)
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
	m_OGLView.HandleKeyUp('R');
//	m_OGLView.m_SimRunning = !m_OGLView.m_SimRunning;
//	m_OGLView.Invalidate(TRUE);
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

void CMainFrame::OnSimulationSetvertexproperties() 
{
	m_OGLView.OnSetVertexProperties();
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

void CMainFrame::OnViewShowstructural() 
{
	m_OGLView.m_PhysEnv.m_DrawStructural = !m_OGLView.m_PhysEnv.m_DrawStructural;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowstructural(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_DrawStructural );
}

void CMainFrame::OnViewShowshear() 
{
	m_OGLView.m_PhysEnv.m_DrawShear = !m_OGLView.m_PhysEnv.m_DrawShear;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowshear(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_DrawShear );
}

void CMainFrame::OnViewShowbend() 
{
	m_OGLView.m_PhysEnv.m_DrawBend = !m_OGLView.m_PhysEnv.m_DrawBend;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewShowbend(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_DrawBend );
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

void CMainFrame::OnFileCreateclothpatch() 
{
	m_OGLView.CreateClothPatch();
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnViewCollisionactive() 
{
	m_OGLView.m_PhysEnv.m_CollisionActive = !m_OGLView.m_PhysEnv.m_CollisionActive;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateViewCollisionactive(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_CollisionActive );
}


void CMainFrame::OnInteuler() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = EULER_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateInteuler(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == EULER_INTEGRATOR );
}

void CMainFrame::OnIntmidpoint() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = MIDPOINT_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateIntmidpoint(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == MIDPOINT_INTEGRATOR );
}

void CMainFrame::OnIntrk4() 
{
	m_OGLView.m_PhysEnv.m_IntegratorType = RK4_INTEGRATOR;
	m_OGLView.Invalidate(TRUE);
}

void CMainFrame::OnUpdateIntrk4(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_PhysEnv.m_IntegratorType == RK4_INTEGRATOR );
}

void CMainFrame::OnSimulationSettimingproperties() 
{
	m_OGLView.OnSetTimeProperties();	
}

// Add a collision sphere to the physical simulation
void CMainFrame::OnSimulationAddcollisionsphere() 
{
	m_OGLView.m_PhysEnv.AddCollisionSphere();	
	m_OGLView.Invalidate(TRUE);
}
