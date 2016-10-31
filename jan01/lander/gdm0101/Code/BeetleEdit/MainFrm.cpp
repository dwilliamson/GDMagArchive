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
	ON_COMMAND(ID_PAINT_MODE_PAINT, OnPaintModePaint)
	ON_UPDATE_COMMAND_UI(ID_PAINT_MODE_PAINT, OnUpdatePaintModePaint)
	ON_COMMAND(ID_FILE_SAVEOBJFILE, OnFileSaveobjfile)
	ON_COMMAND(ID_FILE_OPENSKELETON, OnFileOpenskeleton)
	ON_COMMAND(ID_FILE_SAVESKELETON, OnFileSaveskeleton)
	ON_COMMAND(ID_PAINT_MODE_VERTEX, OnPaintModeVertex)
	ON_UPDATE_COMMAND_UI(ID_PAINT_MODE_VERTEX, OnUpdatePaintModeVertex)
	ON_COMMAND(ID_VIEW_SKELETON, OnViewSkeleton)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SKELETON, OnUpdateViewSkeleton)
	ON_COMMAND(ID_VIEW_DEFORMED, OnViewDeformed)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEFORMED, OnUpdateViewDeformed)
	ON_COMMAND(ID_VIEW_BOUNDINGSPHERES, OnViewBoundingspheres)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BOUNDINGSPHERES, OnUpdateViewBoundingspheres)
	ON_COMMAND(ID_SKELETON_RESETSKELETON, OnSkeletonResetskeleton)
	ON_COMMAND(ID_VIEW_CURRENTBONE, OnViewCurrentbone)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURRENTBONE, OnUpdateViewCurrentbone)
	ON_COMMAND(ID_VIEW_VERTICES, OnViewVertices)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VERTICES, OnUpdateViewVertices)
	ON_COMMAND(ID_SKELETON_PREVIOUSBONE, OnSkeletonPreviousbone)
	ON_COMMAND(ID_SKELETON_NEXTBONE, OnSkeletonNextbone)
	ON_COMMAND(ID_SKELETON_LARGERBSPHERE, OnSkeletonLargerbsphere)
	ON_COMMAND(ID_SKELETON_SMALLERBSPHERE, OnSkeletonSmallerbsphere)
	ON_COMMAND(ID_SKELETON_AUTOWEIGHTVERTICES, OnSkeletonAutoweightvertices)
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
	
	m_OGLView.DrawScene(FALSE);	
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


///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileOpen 
// Purpose:		Open the Mesh file as an OBJ
// Arguments:	none
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileOpen() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Object Mesh OBJ (*.obj)|*.obj||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(TRUE,"dcm",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		exten = dialog->GetFileExt();
		exten.MakeUpper();
		m_OGLView.LoadOBJModel(dialog->GetPathName());
		m_OGLView.DrawScene(FALSE);
	}
	delete dialog;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileSaveobjfile 
// Purpose:		Save the Mesh file off as an OBJ
// Arguments:	none
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileSaveobjfile() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Object Mesh OBJ (*.obj)|*.obj|Object Mesh DCF (*.dcf)|*.dcf||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(FALSE,"obj",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		exten = dialog->GetFileExt();
		exten.MakeUpper();
		if (exten == "OBJ")
			m_OGLView.SaveOBJModel(dialog->GetPathName());
		else
			m_OGLView.SaveDCFModel(dialog->GetPathName());
		m_OGLView.DrawScene(FALSE);
	}
	delete dialog;	
}

// Turn on/off the OpenGL Normal Based Lighting
void CMainFrame::OnSettingsUselighting() 
{
	m_OGLView.m_UseLighting = !m_OGLView.m_UseLighting;
	m_OGLView.DrawScene(FALSE);	
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
	m_OGLView.DrawScene(FALSE);	
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
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUvcoordinatesCylindrical() 
{
	m_OGLView.CylinderMapModel(1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUvcoordinatesPlanarXaxis() 
{
	m_OGLView.PlanarMapModel(YZ_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUvcoordinatesPlanarYaxis() 
{
	m_OGLView.PlanarMapModel(XZ_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUvcoordinatesPlanarZaxis() 
{
	m_OGLView.PlanarMapModel(XY_PLANE,1.0f, 0.0f, 1.0f, 0.0f);
	m_OGLView.FixupMapCoords();
	m_OGLView.DrawScene(FALSE);	
}


// Fix up seam in UV coordinates manually if needed
void CMainFrame::OnUvcoordinates() 
{
	m_OGLView.FixupMapCoords();
	m_OGLView.DrawScene(FALSE);	
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

void CMainFrame::OnPaintModePaint() 
{
	m_OGLView.m_DrawMode = MODE_PAINT;
	m_OGLView.DrawSelectionBuffer();	// Get the screen rendering
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdatePaintModePaint(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawMode == MODE_PAINT );
}

void CMainFrame::OnPaintModeVertex() 
{
	m_OGLView.m_DrawMode = MODE_VERTEX;
	m_OGLView.DrawPickBuffer();	// Get the screen rendering
	m_OGLView.DrawVertexPositionBuffer();	// And one for the vertices
	m_OGLView.m_PaintColor = MAKERGB(50, 50, 50);	// Default paint color
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdatePaintModeVertex(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawMode == MODE_VERTEX );
}

///////////////////////////////////////////////////////////////////////////////
// Function:	OnFileOpenskeletalsystem 
// Purpose:		Open a Skeletal system
// Arguments:	none
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnFileOpenskeleton() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "D3D Skeleton File (*.d3d)|*.d3d|D3D Pose File (*.pos)|*.pos|D3D Weight File (*.wgt)|*.wgt||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(TRUE,"d3d",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		m_OGLView.LoadSkeleton((char *)(LPCSTR)dialog->GetPathName(),(char *)(LPCSTR)dialog->GetFileExt());
	}
	delete dialog;
}

void CMainFrame::OnFileSaveskeleton() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "D3D Skeleton File (*.d3d)|*.d3d||";  
	CFileDialog	*dialog;
	CString exten;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(FALSE,"d3d",NULL, NULL,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		m_OGLView.SaveSkeleton((char *)(LPCSTR)dialog->GetPathName(),(char *)(LPCSTR)dialog->GetFileExt());
	}
	delete dialog;
}


void CMainFrame::OnViewSkeleton() 
{
	m_OGLView.m_DrawSkeleton = !m_OGLView.m_DrawSkeleton;
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdateViewSkeleton(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawSkeleton);
}

void CMainFrame::OnViewDeformed() 
{
	m_OGLView.m_DrawDeformed = !m_OGLView.m_DrawDeformed;
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdateViewDeformed(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawDeformed);
}

void CMainFrame::OnViewBoundingspheres() 
{
	m_OGLView.m_DrawBounds = !m_OGLView.m_DrawBounds;
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdateViewBoundingspheres(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawBounds);
}

void CMainFrame::OnViewCurrentbone() 
{
	m_OGLView.m_DrawCurBone = !m_OGLView.m_DrawCurBone;
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdateViewCurrentbone(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawCurBone);
}

void CMainFrame::OnViewVertices() 
{
	m_OGLView.m_DrawVertices = !m_OGLView.m_DrawVertices;
	m_OGLView.DrawScene(FALSE);	
}

void CMainFrame::OnUpdateViewVertices(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( m_OGLView.m_DrawVertices);
}

void CMainFrame::OnSkeletonResetskeleton() 
{
	if (m_OGLView.m_Skeleton) ResetSkeleton(m_OGLView.m_Skeleton);
	m_OGLView.DrawScene(FALSE);	
}

// Select the Previous Bone in the Skeleton Hierarchy as active
void CMainFrame::OnSkeletonPreviousbone() 
{
	m_OGLView.HandleKeyUp(VK_SUBTRACT);
}

// Select the Next Bone in the Skeleton Hierarchy as active
void CMainFrame::OnSkeletonNextbone() 
{
	m_OGLView.HandleKeyUp(VK_ADD);
}

// Increase Current Bounding Sphere
void CMainFrame::OnSkeletonLargerbsphere() 
{
	m_OGLView.HandleKeyUp(190);		// > key
}

// Decrease Current Bounding Sphere
void CMainFrame::OnSkeletonSmallerbsphere() 
{
	m_OGLView.HandleKeyUp(188);		// < key
}

// Calculate the Automatic Weighting for vertices
// Calc max 3 bones per vertex as a default
void CMainFrame::OnSkeletonAutoweightvertices() 
{
	m_OGLView.HandleKeyUp('C');		// C - key to calculate
}
