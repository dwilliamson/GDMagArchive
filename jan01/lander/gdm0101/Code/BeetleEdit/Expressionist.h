///////////////////////////////////////////////////////////////////////////////
//
// Expressionist.h : main header file for the Expressionist application
//
// Purpose:	Implementation of OpenGL Window of 3D Paint System
//
// Created:
//		JL 8/4/00
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_Expressionist_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_Expressionist_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID

/////////////////////////////////////////////////////////////////////////////
// CExpressionistApp:
// See Expressionist.cpp for the implementation of this class
//

class CExpressionistApp : public CWinApp
{
public:
	CExpressionistApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpressionistApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CExpressionistApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Expressionist_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_)
