// Agua.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Agua.h"
#include "AguaDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAguaApp

BEGIN_MESSAGE_MAP(CAguaApp, CWinApp)
	//{{AFX_MSG_MAP(CAguaApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAguaApp construction

CAguaApp::CAguaApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAguaApp object

CAguaApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAguaApp initialization

BOOL CAguaApp::InitInstance()
{

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CAguaDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();

	return FALSE;
}
