///////////////////////////////////////////////////////////////////////////////
//
// HierWin.h : class definition file
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

#if !defined(AFX_HierWin_H__2AB46760_27CD_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_HierWin_H__2AB46760_27CD_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// HierWin.h : header file
//
#include "Skeleton.h"

/////////////////////////////////////////////////////////////////////////////
// CHierWin window

class CHierWin : public CTreeCtrl
{
// Construction
public:
	CHierWin();

// Attributes
protected:
	HTREEITEM m_TreeRoot;
	t_Bone	*m_Skeleton;
// Operations
public:
	void ResetSkeleton();
	void SetSkeleton(t_Bone *skeleton, HTREEITEM item);
	void AddBone();

protected:
	void EditBone();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHierWin)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHierWin();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHierWin)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CBoneInfo dialog

class CBoneInfo : public CDialog
{
// Construction
public:
	CBoneInfo(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBoneInfo)
	enum { IDD = IDD_BONE_INFO };
	CString	m_BoneName;
	float	m_Rot_X;
	float	m_Rot_Y;
	float	m_Rot_Z;
	float	m_Trans_X;
	float	m_Trans_Y;
	float	m_Trans_Z;
	float	m_MaxX;
	float	m_MaxY;
	float	m_MaxZ;
	float	m_MinX;
	float	m_MinY;
	float	m_MinZ;
	BOOL	m_DOF_Active;
	float	m_Weight;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoneInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBoneInfo)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_HierWin_H__2AB46760_27CD_11D1_83A0_004005308EB5__INCLUDED_)
