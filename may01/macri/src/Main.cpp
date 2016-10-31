// Main.cpp App Wizard Version 2.0 Beta 1
// ----------------------------------------------------------------------
// 
// Copyright © 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// ----------------------------------------------------------------------
//
// PURPOSE:
//    
// Main entry point for the application
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "stdafx.h"
#pragma comment(lib,"winmm")
#pragma comment(lib,"d3d8")
#pragma comment(lib,"dxguid")
#pragma comment(lib,"user32")
#pragma comment(lib,"gdi32")
#pragma comment(lib,"kernel32")



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{  
	IawWindow3d wnd;
	AGP_Performance *pApp;

	
	bool bDone;

	// Initialize the window required by Windows
	if( !wnd.Init( hInstance, nCmdShow ) )
	{
		MessageBox( NULL, "Couldn't create a window!", "DX8Shell", MB_ICONEXCLAMATION | MB_OK );
		return 1;
	}



	pApp = new AGP_Performance();
	pApp->InitWorld( wnd.GetWrapper() );
	wnd.SetApp( pApp );
	wnd.Enable( true ); 

						

	do
	{
		bDone = wnd.ProcessMessages();
	} while (!bDone);

	return 0;
}


