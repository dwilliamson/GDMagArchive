// Agua.h : main header file for the AGUA application
//

#if !defined(AFX_AGUA_H__23260AB3_C27C_4749_9E84_57A4358F4623__INCLUDED_)
#define AFX_AGUA_H__23260AB3_C27C_4749_9E84_57A4358F4623__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAguaApp:
// See Agua.cpp for the implementation of this class
//

class CAguaApp : public CWinApp
{
public:
	CAguaApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAguaApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAguaApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AGUA_H__23260AB3_C27C_4749_9E84_57A4358F4623__INCLUDED_)
