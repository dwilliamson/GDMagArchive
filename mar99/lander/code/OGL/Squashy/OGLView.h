///////////////////////////////////////////////////////////////////////////////
//
// OGLView.h : class definition file
//
// Purpose:	Implementation of OpenGL Window of 3D Morphing System
//
// Created:
//		JL 10/1/98		
//
// The function morphModel() does the main morphing work.
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OGLVIEW_H__2AB46761_27CD_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_OGLVIEW_H__2AB46761_27CD_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OGLView.h : header file
//

#include <GL/gl.h>
#include <GL/glu.h>

#include "Skeleton.h"
#include "PhysEnv.h"
/////////////////////////////////////////////////////////////////////////////
// COGLView window

class COGLView : public CWnd
{
// Construction
public:
	COGLView();

// Attributes
public:
	CStatusBar  *m_ptrStatusBar;
	HDC		m_hDC;
	HGLRC	m_hRC;
	CPoint	m_mousepos;
	float	m_Base_Rot_X,m_Base_Rot_Y,m_Base_Rot_Z;
	float	m_Grab_Rot_X,m_Grab_Rot_Y,m_Grab_Rot_Z;
	float	m_Grab_Trans_X,m_Grab_Trans_Y,m_Grab_Trans_Z;
	BOOL	m_DrawGeometry,m_SimRunning;
	int		m_curVisual;
	float	m_MorphPos;
	DWORD	m_StartTime;
	DWORD	m_FrameCnt;
	int		m_TimeIterations;
	BOOL	m_UseFixedTimeStep;
	float	m_MaxTimeStep;
	float	m_LastTime;


	CPhysEnv		m_PhysEnv;
// Operations
public:
	BOOL	SetupPixelFormat(HDC hdc);
	GLvoid	drawScene(GLvoid);
	GLvoid  drawModel(t_Bone *curBone);
	GLvoid	initializeGL(GLsizei width, GLsizei height);
	void	OnSimulationSetVertexMass();
	void	OnSimulationSetsimproperties();
	void	OnSetTimeProperties();
	GLvoid	resize( GLsizei width, GLsizei height );
	void	GetGLInfo(char *who,char *which, char *version, char *extensions);
	void	HandleKeyUp(UINT nChar);
	void	HandleKeyDown(UINT nChar);
	GLvoid  morphModel(t_Bone *curBone);
	GLvoid	LoadBoneTexture(t_Bone *curBone);
	void	NewSystem();
	void	LoadFile(CString file1,CString baseName,CString ext);
	void	SaveFile(CString file1,CString baseName);
	void	RunSim();
	float	GetTime( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COGLView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COGLView();

	// Generated message map functions
protected:
	t_Bone	m_Skeleton,*m_CurBone;
	int		m_PickX, m_PickY;
	int		m_ScreenWidth;
	int		m_ScreenHeight;
	//{{AFX_MSG(COGLView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OGLVIEW_H__2AB46761_27CD_11D1_83A0_004005308EB5__INCLUDED_)
