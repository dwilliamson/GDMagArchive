// MaxUIMod.h : main header file for the MAXUIMOD DLL
//

#if !defined(AFX_MAXUIMOD_H__D25BEAA5_5D90_11D4_B21D_86F81FF96D7C__INCLUDED_)
#define AFX_MAXUIMOD_H__D25BEAA5_5D90_11D4_B21D_86F81FF96D7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMaxUIModApp
// See MaxUIMod.cpp for the implementation of this class
//

class CMaxUIModApp : public CWinApp
{
public:
	CMaxUIModApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaxUIModApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMaxUIModApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXUIMOD_H__D25BEAA5_5D90_11D4_B21D_86F81FF96D7C__INCLUDED_)
