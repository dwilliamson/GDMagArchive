///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.h : interface of the CMainFrame class
//
// Purpose:	Implementation of Main Window of Quaternion Animation System
//
// Created:
//		JL 11/1/97		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1997 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__082DB1E8_6069_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_MAINFRM_H__082DB1E8_6069_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "OGLView.h"
#include "Slider.h"
#include "Skeleton.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	CString		m_ClassName;
	HCURSOR		m_HArrow;
	COGLView	m_OGLView;
	CSlider		m_Slider;

// Operations
public:
	CMainFrame();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
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
	t_Bone		m_Skeleton;

	void InitializeSkeleton();
	void	LoadObjectFile(char *filename,t_Bone *bonePtr);

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHelpWhichopengl();
	afx_msg void OnEndkey();
	afx_msg void OnControlUpperarm();
	afx_msg void OnUpdateControlUpperarm(CCmdUI* pCmdUI);
	afx_msg void OnControlLowerarm();
	afx_msg void OnUpdateControlLowerarm(CCmdUI* pCmdUI);
	afx_msg void OnControlHand();
	afx_msg void OnUpdateControlHand(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__082DB1E8_6069_11D1_83A0_004005308EB5__INCLUDED_)
