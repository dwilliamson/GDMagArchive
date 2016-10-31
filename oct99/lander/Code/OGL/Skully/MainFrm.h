///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.h : interface of the CMainFrame class
//
// Purpose:	Implementation of Main Window of Hierarchical Animation System
//
// Created:
//		JL 9/1/97
// Modified:
//		JL 7/10/99		Created skeleton Demo for Oct 99 GDMag
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4B0629BD_2696_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_MAINFRM_H__4B0629BD_2696_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "HierWin.h"
#include "OGLView.h"
#include "Skeleton.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	CString m_ClassName;
	HCURSOR m_HArrow;
	CHierWin m_HierWin;
	COGLView  m_OGLView;
	BOOL m_Wireframe;
// Operations
public:
	CMainFrame();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAddBone();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnWhichogl();
	afx_msg void OnFileOpen();
	afx_msg void OnSkeletonResetskeleton();
	afx_msg void OnViewOutline();
	afx_msg void OnUpdateViewOutline(CCmdUI* pCmdUI);
	afx_msg void OnFileOpencharactermesh();
	afx_msg void OnViewViewskeleton();
	afx_msg void OnUpdateViewViewskeleton(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnViewDrawdeformed();
	afx_msg void OnUpdateViewDrawdeformed(CCmdUI* pCmdUI);
	afx_msg void OnSkeletonSetrestpose();
	afx_msg void OnSkeletonSetboneweights();
	afx_msg void OnFileOpenweight();
	afx_msg void OnSkeletonClearselectedweights();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
