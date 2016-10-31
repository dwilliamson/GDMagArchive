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

enum ePlanarMappings
{
	XY_PLANE,
	XZ_PLANE,
	YZ_PLANE
};

enum eDrawMode
{
	MODE_NONE,
	MODE_PAINT,
	MODE_VERTEX
};

#include "Skeleton.h"
#include "Visual.h"

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
	CPoint m_MousePos,m_DragPos;
	float m_Grab_Rot_X,m_Grab_Rot_Y,m_Grab_Rot_Z;
	float m_Grab_Trans_X,m_Grab_Trans_Y,m_Grab_Trans_Z;
	CStatusBar  *m_StatusBar;
	int		m_ScreenWidth, m_ScreenHeight;
	BOOL	m_Bilinear, m_Dragging,m_UseLighting;
	tCamera	m_Camera;					// For the Camera
	t_Visual	m_Model;					// Actual Model to be Drawn
	t_Bone	*m_Skeleton;				// Skeleton System for object
	int		m_Skeleton_BoneCnt;			// Number of bones in the skeleton
	t_Bone	*m_CurBone;					// Pointer to current bone
	float   *m_SelectBuffer;
	unsigned char *m_ByteBuffer;
	long	m_PaintColor;					// Color to Paint with
	int		m_DrawMode, m_DrawDeformed,m_DrawSkeleton,m_SelectedTri,m_SelectedVertex;
	int		m_DrawBounds,m_DrawVertices, m_DrawCurBone;
// Operations
public:
	GLvoid	FixupMapCoords();
	GLvoid	SphereMapModel();
	GLvoid	PlanarMapModel(int mapType, float scaleU, float offsetU, float scaleV, float offsetV);
	GLvoid	CylinderMapModel(float scale, float offset);
	GLvoid	GetModelBoundaries(tVector *min, tVector *max,tVector *size);
	BOOL	SetupPixelFormat(HDC hdc);
	GLvoid	DrawModel(t_Visual *model);
	GLvoid	DrawModelUV(t_Visual *model);
	GLvoid	DrawSelectionBuffer();
	GLvoid	DrawPickTri(t_Visual *visual);
	GLvoid	DrawPickBuffer();
	GLvoid	DrawVertexPos(t_Visual *visual);
	GLvoid	DrawVertexPositionBuffer();
	GLvoid	DrawSkeleton(t_Bone *rootBone);
	GLvoid	DrawScene(BOOL draglines);
	GLvoid	GetClickPos(int tri, float *click,tVector *pos);
	int		GetClickVertex(int tri, float *click);
	void	DrawTexture(float u, float v);
	void	GetTextureColor(float u, float v);
	GLvoid	initializeGL(GLsizei width, GLsizei height);
	GLvoid	resize( GLsizei width, GLsizei height );
	void	GetGLInfo();
	void	HandleKeyUp(UINT nChar);
	void	HandleKeyDown(UINT nChar);
	BOOL	LoadOBJModel(CString name);
	BOOL	SaveOBJModel(CString name);
	void	SaveTexture(char *filename);
	void	LoadTexture(char *filename,unsigned int *textureID);
	void	LoadSkeleton(char *name,char *ext);
	void	SaveSkeleton(char *name,char *ext);
	void	AutoComputeWeights(int maxBonesPerVertex);
	void	PaintWeights(int vertexNum,float factor);
	GLvoid	GetSkeletonMat(t_Bone *rootBone);
	GLvoid	GetBaseSkeletonMat(t_Bone *rootBone);
	GLvoid	DeformVertices(t_Bone *rootBone,t_Visual *visual);
	void	SetBasePose();
	BOOL	SaveDCFModel(CString name);

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
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OGLVIEW_H__2AB46761_27CD_11D1_83A0_004005308EB5__INCLUDED_)
