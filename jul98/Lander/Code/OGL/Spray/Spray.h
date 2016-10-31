///////////////////////////////////////////////////////////////////////////////
//
// Spray.h : main header file for the SkinApp Demo
//
// Purpose:	header of Main Application of Quaternion Animation System
//
// Created:
//		JL 2/18/97		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPRAY_H__86F99065_D3EC_11D1_83A2_004005308EB5__INCLUDED_)
#define AFX_SPRAY_H__86F99065_D3EC_11D1_83A2_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSprayApp:
// See Spray.cpp for the implementation of this class
//

class CSprayApp : public CWinApp
{
public:
	CSprayApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSprayApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSprayApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPRAY_H__86F99065_D3EC_11D1_83A2_004005308EB5__INCLUDED_)
