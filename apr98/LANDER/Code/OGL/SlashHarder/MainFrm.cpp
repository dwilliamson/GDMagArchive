///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of Main Window of Quaternion Animation System
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
#include "Slash.h"

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
	ON_COMMAND(ID_ENDKEY, OnEndkey)
	ON_COMMAND(ID_CONTROL_UPPERARM, OnControlUpperarm)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_UPPERARM, OnUpdateControlUpperarm)
	ON_COMMAND(ID_CONTROL_LOWERARM, OnControlLowerarm)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_LOWERARM, OnUpdateControlLowerarm)
	ON_COMMAND(ID_CONTROL_HAND, OnControlHand)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_HAND, OnUpdateControlHand)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_STATUS,	// MY ADDITION FOR PUTTING SETTINGS
	ID_INDICATOR_ROT,		// MY ADDITION FOR PUTTING SETTINGS
	ID_INDICATOR_QUAT,		// MY ADDITION FOR PUTTING SETTINGS
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_HArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);	

	InitializeSkeleton();	
}

CMainFrame::~CMainFrame()
{
	DestroySkeleton(&m_Skeleton);
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
	m_OGLView.Create(NULL,"Render Window",WS_CHILD | WS_VISIBLE, CRect(1, 1,rect.right - 3,rect.bottom),this,104,&m_Skeleton,&m_Slider);
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

	m_OGLView.drawScene();
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	m_OGLView.SetWindowPos( &wndTopMost, 1, 1, cx - 3, cy - 46 , SWP_NOZORDER ); // -60 bottom
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

	m_OGLView.HandleKeyUp(nChar);
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// Hierarchy Manipulation Functions
void CMainFrame::LoadObjectFile(char *filename,t_Bone *bonePtr)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;
	tObjDesc	*desc;			/* Pointer to Desc */
	tObjFrame	*frame;			/* Pointer to Desc */
	short loop2;
	char tempstr[80];
///////////////////////////////////////////////////////////////////////////////

	fp = fopen(filename,"rb");
	if (fp != NULL)
	{
		fread(tempstr,1,4,fp); // FDAT
		if (strncmp(tempstr,"DARW",4)!= 0)
		{
			MessageBox("Not a Valid DGF File","Load File", MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		fread(tempstr,1,4,fp); // FDAT

		fread(bonePtr,sizeof(t_Bone),1,fp);
		fread(tempstr,1,4,fp); // FDAT
		bonePtr->desc = (tObjDesc *)malloc(sizeof(tObjDesc));
		desc = bonePtr->desc;
		fread(desc,sizeof(tObjDesc),1,fp);
		for (loop2 = 0; loop2 < desc->frameCnt; loop2++)
		{
			fread(tempstr,1,4,fp); // FDAT
			desc->frame[loop2] = (tObjFrame	*)malloc(sizeof(tObjFrame));
			frame = desc->frame[loop2];
			fread(frame,sizeof(tObjFrame),1,fp);
			frame->data = (float *)malloc(sizeof(float) * desc->dataSize * desc->pointCnt);
			fread(frame->data,sizeof(float),desc->dataSize * desc->pointCnt,fp);
		}
		bonePtr->cur_desc = 0;
		fclose(fp);
	}
}

void CMainFrame::InitializeSkeleton()
{

/// Local Variables ///////////////////////////////////////////////////////////
	t_Bone *tempBones;
///////////////////////////////////////////////////////////////////////////////

	ResetBone(&m_Skeleton, NULL);
	m_Skeleton.id = -1;
	strcpy(m_Skeleton.name,"Skeleton");
	m_Skeleton.p_trans.x = 5.0f;
	m_Skeleton.p_trans.y = -4.0f;
	m_Skeleton.p_trans.z = -10.0f;
	m_Skeleton.trans.x = 5.0f;
	m_Skeleton.trans.y = -4.0f;
	m_Skeleton.trans.z = -10.0f;

	// ALLOC ROOM FOR BONES
	tempBones = (t_Bone *)malloc(3 * sizeof(t_Bone));

	ResetBone(&tempBones[0], &m_Skeleton);		// SETUP INITIAL BONE SETTINGS
	ResetBone(&tempBones[1], &m_Skeleton);		// SETUP INITIAL BONE SETTINGS
	ResetBone(&tempBones[2], &m_Skeleton);		// SETUP INITIAL BONE SETTINGS

	m_Skeleton.childCnt = 1;
	m_Skeleton.children = tempBones;

	// THIS IS THE BONE FOR THE UPPER ARM
	LoadObjectFile("uparm.dgf",&tempBones[0]);
	strcpy(tempBones[0].name,"UpperArm");
	tempBones[0].id = 100;			// GIVE IT A UNIQUE ID
	tempBones[0].childCnt = 1;
	tempBones[0].children = &tempBones[1];
	tempBones[0].trans.x = 0.0;
	tempBones[0].trans.y = 0.0;
	tempBones[0].trans.z = 0.0;
	// SET UP SOME NICE DEFAULT MOVES
	tempBones[0].rot.x = 35.0;
	tempBones[0].rot.y = -40.0;
	tempBones[0].rot.z = 0.0;
	tempBones[0].p_rot.x = 35.0;
	tempBones[0].p_rot.y = -40.0;
	tempBones[0].p_rot.z = 0.0;
	tempBones[0].s_rot.x = -1.0;
	tempBones[0].s_rot.y = 40.0;
	tempBones[0].s_rot.z = 0.0;

	// THIS IS THE BONE FOR THE LOWER ARM
	LoadObjectFile("lowarm.dgf",&tempBones[1]);
	strcpy(tempBones[1].name,"LowerArm");
	tempBones[1].id = 101;			// GIVE IT A UNIQUE ID
	tempBones[1].trans.x = 0.0;
	tempBones[1].trans.y = 0.0;
	// SET UP SOME NICE DEFAULT MOVES
	tempBones[1].trans.z = -5.0;
	tempBones[1].rot.x = -332.0;
	tempBones[1].rot.y = 27.0;
	tempBones[1].rot.z = -253.0;
	tempBones[1].p_rot.x = -332.0;
	tempBones[1].p_rot.y = 27.0;
	tempBones[1].p_rot.z = -253.0;
	tempBones[1].s_rot.x = -6.0;
	tempBones[1].s_rot.y = 51.0;
	tempBones[1].s_rot.z = 0.0;
	tempBones[1].childCnt = 1;
	tempBones[1].children = &tempBones[2];

	// THIS IS THE BONE FOR THE HAND
	LoadObjectFile("hand.dgf",&tempBones[2]);
	strcpy(tempBones[2].name,"Hand");
	tempBones[2].id = 102;			// GIVE IT A UNIQUE ID
	tempBones[2].trans.x = 0.0;
	tempBones[2].trans.y = 0.0;
	tempBones[2].trans.z = -4.0;
	// SET UP SOME NICE DEFAULT MOVES
	tempBones[2].rot.x = -21.0;
	tempBones[2].rot.y = -47.0;
	tempBones[2].rot.z = -35.0;
	tempBones[2].p_rot.x = -21.0;
	tempBones[2].p_rot.y = 12.0;
	tempBones[2].p_rot.z = -47.0;
	tempBones[2].s_rot.x = -22.0;
	tempBones[2].s_rot.y = -7.0;
	tempBones[2].s_rot.z = -84.0;
}

void CMainFrame::OnEndkey() 
{	
	m_OGLView.SetEndKey();
}

// SELECTION CONTROLS FOR THE MENU OPTIONS 
// THESE ROUTINES CONTROL WHICH BONE IS UNDER UI CONTROL
void CMainFrame::OnControlUpperarm() 
{
	m_OGLView.m_CurBone = &m_Skeleton.children[0];
}

void CMainFrame::OnUpdateControlUpperarm(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_OGLView.m_CurBone == &m_Skeleton.children[0]);
}

void CMainFrame::OnControlLowerarm() 
{
	m_OGLView.m_CurBone = &m_Skeleton.children[1];
}

void CMainFrame::OnUpdateControlLowerarm(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_OGLView.m_CurBone == &m_Skeleton.children[1]);
}

void CMainFrame::OnControlHand() 
{
	m_OGLView.m_CurBone = &m_Skeleton.children[2];
}

void CMainFrame::OnUpdateControlHand(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_OGLView.m_CurBone == &m_Skeleton.children[2]);
}
