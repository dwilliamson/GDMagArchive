///////////////////////////////////////////////////////////////////////////////
//
// HierWin.cpp : implementation file
//
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
#include "Dagger.h"
#include "HierWin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHierWin

CHierWin::CHierWin()
{
	m_Skeleton = NULL;
}

CHierWin::~CHierWin()
{
}


BOOL CHierWin::PreCreateWindow(CREATESTRUCT& cs) 
{
	CString className;
	HCURSOR arrow;

	arrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);	

//	className = AfxRegisterWndClass(NULL,
//		(HCURSOR)arrow, (HBRUSH)::GetStockObject(DKGRAY_BRUSH), 0); //m_HArrow
//	cs.lpszClass = className;
	
	return CTreeCtrl::PreCreateWindow(cs);
}

int CHierWin::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{	
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	ResetSkeleton();
	return 0;
}

void CHierWin::ResetSkeleton() 
{	
	DeleteAllItems();	// CLEAR THE CONTROL
	m_TreeRoot = InsertItem( "Skeleton", TVI_ROOT, TVI_LAST );
	// SET THE ROOT DATA TO NULL
	SetItemData( m_TreeRoot, NULL );
}

void CHierWin::SetSkeleton(t_Bone *skeleton, HTREEITEM item) 
{	
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Bone *child;
	HTREEITEM curItem;
	TV_ITEM pItem;
///////////////////////////////////////////////////////////////////////////////
	
	if (item == NULL) 
	{
		item = m_TreeRoot;
		m_Skeleton = skeleton;
		SetItemData( item, (DWORD)m_Skeleton );
	}

	if (skeleton->childCnt > 0)
	{
		child = skeleton->children;
		for (loop = 0; loop < skeleton->childCnt; loop++,child++)
		{
			curItem = InsertItem( child->name, item, TVI_LAST );
			SetItemData( curItem, (DWORD)child );
			EnsureVisible( curItem );
			if (child->childCnt > 0)
				SetSkeleton(child,curItem);
		}
	}
}

void CHierWin::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// RESET FOCUS TO MAINFRAME
	// NEEDED TO ADD THIS SINCE THE TREECTRL WILL HOLD FOCUS
	GetParent()->SetFocus( );
	//CTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CHierWin::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// RESET FOCUS TO MAINFRAME
	// NEEDED TO ADD THIS SINCE THE TREECTRL WILL HOLD FOCUS
	GetParent()->SetFocus( );
	//CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

BEGIN_MESSAGE_MAP(CHierWin, CTreeCtrl)
	//{{AFX_MSG_MAP(CHierWin)
	ON_WM_CREATE()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHierWin message handlers

void CHierWin::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// RESET FOCUS TO MAINFRAME
	// NEEDED TO ADD THIS SINCE THE TREECTRL WILL HOLD FOCUS
	GetParent()->SetFocus( );
	
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CHierWin::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	EditBone();	
	CTreeCtrl::OnLButtonDblClk(nFlags, point);
}

void CHierWin::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	if (m_Skeleton > NULL)
		m_Skeleton->id = GetItemData(GetSelectedItem());
	GetParent()->Invalidate(TRUE );
	*pResult = 0;
}

void CHierWin::AddBone() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	HTREEITEM item,curItem;
	CBoneInfo  dialog;
	t_Bone *bonePtr;
///////////////////////////////////////////////////////////////////////////////
	item = GetSelectedItem();
	if (item == NULL)
	{
		dialog.m_Trans_X = m_Skeleton->trans.x;
		dialog.m_Trans_Y = m_Skeleton->trans.y;
		dialog.m_Trans_Z = m_Skeleton->trans.z;
		dialog.m_Rot_X = m_Skeleton->rot.x;
		dialog.m_Rot_Y = m_Skeleton->rot.y;
		dialog.m_Rot_Z = m_Skeleton->rot.z;
	}
	else
	{
		bonePtr = (t_Bone *)GetItemData( item);
		dialog.m_Rot_X = 0.0f;
		dialog.m_Rot_Y = 0.0f;
		dialog.m_Rot_Z = 0.0f;
		dialog.m_Trans_X = 0.0f;
		dialog.m_Trans_Y = 0.0f;
		dialog.m_Trans_Z = 0.0f;
	}
	if (dialog.DoModal())
	{
		if (item == NULL)
			InsertItem( dialog.m_BoneName, TVI_ROOT, TVI_LAST );
		else
			curItem = InsertItem( dialog.m_BoneName, item, TVI_LAST );
			EnsureVisible( curItem );
	}	
}

void CHierWin::EditBone() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	HTREEITEM item,curItem;
	CBoneInfo  dialog;
	t_Bone *bonePtr;
///////////////////////////////////////////////////////////////////////////////
	item = GetSelectedItem();
	if (item != NULL)
	{
		bonePtr = (t_Bone *)GetItemData( item);
		dialog.m_BoneName = bonePtr->name;
		dialog.m_Trans_X = bonePtr->trans.x;
		dialog.m_Trans_Y = bonePtr->trans.y;
		dialog.m_Trans_Z = bonePtr->trans.z;
		dialog.m_Rot_X = bonePtr->rot.x;
		dialog.m_Rot_Y = bonePtr->rot.y;
		dialog.m_Rot_Z = bonePtr->rot.z;
		if (dialog.DoModal())
		{
			strcpy(bonePtr->name,(LPCTSTR)(dialog.m_BoneName));
			bonePtr->trans.x = dialog.m_Trans_X;
			bonePtr->trans.y = dialog.m_Trans_Y;
			bonePtr->trans.z = dialog.m_Trans_Z;
			bonePtr->rot.x = dialog.m_Rot_X;
			bonePtr->rot.y = dialog.m_Rot_Y;
			bonePtr->rot.z = dialog.m_Rot_Z;
			ResetSkeleton();
			SetSkeleton(m_Skeleton, NULL);
		}	
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBoneInfo dialog


CBoneInfo::CBoneInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CBoneInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBoneInfo)
	m_BoneName = _T("");
	m_Rot_X = 0.0f;
	m_Rot_Y = 0.0f;
	m_Rot_Z = 0.0f;
	m_Trans_X = 0.0f;
	m_Trans_Y = 0.0f;
	m_Trans_Z = 0.0f;
	//}}AFX_DATA_INIT
}


void CBoneInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBoneInfo)
	DDX_Text(pDX, IDC_BONE_NAME, m_BoneName);
	DDX_Text(pDX, IDC_ROT_X, m_Rot_X);
	DDX_Text(pDX, IDC_ROT_Y, m_Rot_Y);
	DDX_Text(pDX, IDC_ROT_Z, m_Rot_Z);
	DDX_Text(pDX, IDC_TRANS_X, m_Trans_X);
	DDX_Text(pDX, IDC_TRANS_Y, m_Trans_Y);
	DDX_Text(pDX, IDC_TRANS_Z, m_Trans_Z);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBoneInfo, CDialog)
	//{{AFX_MSG_MAP(CBoneInfo)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBoneInfo message handlers
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
