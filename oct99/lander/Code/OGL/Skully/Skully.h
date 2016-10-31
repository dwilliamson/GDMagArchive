///////////////////////////////////////////////////////////////////////////////
//
// Skully.h : main header file for the Skully application
//
// Purpose:	header of Main Application of Hierarchical Animation System
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

#if !defined(AFX_Skully_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_Skully_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID

/////////////////////////////////////////////////////////////////////////////
// CSkullyApp:
// See Skully.cpp for the implementation of this class
//

class CSkullyApp : public CWinApp
{
public:
	CSkullyApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkullyApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSkullyApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Skully_H__4B0629B9_2696_11D1_83A0_004005308EB5__INCLUDED_)
