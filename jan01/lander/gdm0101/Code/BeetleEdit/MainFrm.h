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
#include "Visual.h"

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
	afx_msg void OnSettingsUselighting();
	afx_msg void OnUpdateSettingsUselighting(CCmdUI* pCmdUI);
	afx_msg void OnPaintSetcolor();
	afx_msg void OnUvcoordinatesSpherical();
	afx_msg void OnUvcoordinatesCylindrical();
	afx_msg void OnUvcoordinatesPlanarXaxis();
	afx_msg void OnUvcoordinatesPlanarYaxis();
	afx_msg void OnUvcoordinatesPlanarZaxis();
	afx_msg void OnSettingsBilinearfilter();
	afx_msg void OnUpdateSettingsBilinearfilter(CCmdUI* pCmdUI);
	afx_msg void OnUvcoordinates();
	afx_msg void OnFileSavetexture();
	afx_msg void OnPaintModePaint();
	afx_msg void OnUpdatePaintModePaint(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveobjfile();
	afx_msg void OnFileOpenskeleton();
	afx_msg void OnFileSaveskeleton();
	afx_msg void OnPaintModeVertex();
	afx_msg void OnUpdatePaintModeVertex(CCmdUI* pCmdUI);
	afx_msg void OnViewSkeleton();
	afx_msg void OnUpdateViewSkeleton(CCmdUI* pCmdUI);
	afx_msg void OnViewDeformed();
	afx_msg void OnUpdateViewDeformed(CCmdUI* pCmdUI);
	afx_msg void OnViewBoundingspheres();
	afx_msg void OnUpdateViewBoundingspheres(CCmdUI* pCmdUI);
	afx_msg void OnSkeletonResetskeleton();
	afx_msg void OnViewCurrentbone();
	afx_msg void OnUpdateViewCurrentbone(CCmdUI* pCmdUI);
	afx_msg void OnViewVertices();
	afx_msg void OnUpdateViewVertices(CCmdUI* pCmdUI);
	afx_msg void OnSkeletonPreviousbone();
	afx_msg void OnSkeletonNextbone();
	afx_msg void OnSkeletonLargerbsphere();
	afx_msg void OnSkeletonSmallerbsphere();
	afx_msg void OnSkeletonAutoweightvertices();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
