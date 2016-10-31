///////////////////////////////////////////////////////////////////////////////
//
// OGLView.h : class definition file
//
// Purpose:	Implementation of OpenGL Window of Cartoon Rendering System
//
// Created:
//		JL 1/12/00
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 200 Jeff Lander, All Rights Reserved.
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
/////////////////////////////////////////////////////////////////////////////
// COGLView window

class COGLView : public CWnd
{
// Construction
public:
	COGLView();

// Attributes
public:
	HDC m_hDC;
	HGLRC m_hRC;
	CPoint m_mousepos;
	float m_Grab_Rot_X,m_Grab_Rot_Y,m_Grab_Rot_Z;
	float m_Grab_Trans_X,m_Grab_Trans_Y,m_Grab_Trans_Z;
	CStatusBar  *m_StatusBar;
	int		m_ScreenWidth, m_ScreenHeight;
	BOOL	m_AntiAlias, m_Dragging,m_Silhouette;
	t_Bone		m_Camera;					// For the Camera
	t_Visual	m_Model;					// Actual Model to be Drawn
	tVector		m_ShadeSrc[32];				// Shade Texture
	tVector		m_ShadeLight;				// Light for Calculating Shade
	tVector		m_SilhouetteColor;			// Color For silhouette line
	float		m_SilhouetteWidth;			// Width of Silhouette line

	unsigned int	m_ShadeTexture;				// Pointer to Shaded texture
// Operations
public:
	float	CalculateShadow(tVector *normal,tVector *light, tMatrix *mat);
	void	LoadShadeTexture(const char *texfile);
	BOOL	SetupPixelFormat(HDC hdc);
	GLvoid	drawModel(t_Visual *model);
	GLvoid	drawScene();
	GLvoid	initializeGL(GLsizei width, GLsizei height);
	GLvoid	resize( GLsizei width, GLsizei height );
	void	GetGLInfo();
	void	HandleKeyUp(UINT nChar);
	void	HandleKeyDown(UINT nChar);
	BOOL	LoadOBJModel(CString name);
	void	CartoonSettings();
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
	void UpdateStatusBar(int mode);
	void UpdateStatusBarFrameInfo();
	//{{AFX_MSG(COGLView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OGLVIEW_H__2AB46761_27CD_11D1_83A0_004005308EB5__INCLUDED_)
