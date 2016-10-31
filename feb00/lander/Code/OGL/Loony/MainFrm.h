///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.h : interface of the CMainFrame class
//
// Purpose:	Implementation of OpenGL Window of Cartoon Rendering System
//
// Created:
//		JL 1/12/00
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
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
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	afx_msg void OnWhichogl();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenObjectMesh();
	afx_msg void OnCartoonAntialias();
	afx_msg void OnUpdateCartoonAntialias(CCmdUI* pCmdUI);
	afx_msg void OnCartoonSettings();
	afx_msg void OnCartoonDrawsilhouette();
	afx_msg void OnUpdateCartoonDrawsilhouette(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
