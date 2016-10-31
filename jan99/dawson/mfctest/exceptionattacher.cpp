#include "stdafx.h"
#include "exceptionhandler.h"

// Copyright © 1998 Bruce Dawson.

/*
This module contains the boilerplate code that you need to add to any
MFC app in order to wrap the main thread in an exception handler.
AfxWinMain() in this source file replaces the AfxWinMain() in the MFC
library.
*/

// This is the initial entry point into your MFC app. Normally this is in the
// MFC library.
int AFXAPI AfxWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	// Wrap WinMain in a structured exception handler (different from C++
	// exception handling) in order to make sure that all access violations
	// and other exceptions are displayed - regardless of when they happen.
	// This should be done for each thread, if at all possible, so that exceptions
	// will be reliably caught, even inside the debugger.
	__try
	{
		// The code inside the __try block is the MFC version of AfxWinMain(),
		// copied verbatim from the MFC source code.
		ASSERT(hPrevInstance == NULL);
		
		int nReturnCode = -1;
		CWinApp* pApp = AfxGetApp();

		// AFX internal initialization
		if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
			goto InitFailure;

		// App global initializations (rare)
		ASSERT_VALID(pApp);
		if (!pApp->InitApplication())
			goto InitFailure;
		ASSERT_VALID(pApp);

		// Perform specific initializations
		if (!pApp->InitInstance())
		{
			if (pApp->m_pMainWnd != NULL)
			{
				TRACE0("Warning: Destroying non-NULL m_pMainWnd\n");
				pApp->m_pMainWnd->DestroyWindow();
			}
			nReturnCode = pApp->ExitInstance();
			goto InitFailure;
		}
		ASSERT_VALID(pApp);

		nReturnCode = pApp->Run();
		ASSERT_VALID(pApp);

	InitFailure:
	#ifdef _DEBUG
		// Check for missing AfxLockTempMap calls
		if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
		{
			TRACE1("Warning: Temp map lock count non-zero (%ld).\n",
				AfxGetModuleThreadState()->m_nTempMapLock);
		}
		AfxLockTempMaps();
		AfxUnlockTempMaps(-1);
	#endif

		AfxWinTerm();
		return nReturnCode;
	}
	__except(RecordExceptionInfo(GetExceptionInformation(), "Main Thread"))
	{
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
	return 0;
}
