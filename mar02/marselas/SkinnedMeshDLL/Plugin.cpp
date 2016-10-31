/*===========================================================================*\
 | 
 |  FILE:	Plugin.cpp
 |			Skeleton project and code for a Global Utility Plugin (GUP)
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 21-1-99
 | 
 |  Portions Copyright (c) 2001 Microsoft Corporation. All rights reserved.
\*===========================================================================*/

#include "pch.h"
#include "skinnedmesh.h"
#include "plugin.h"

HINSTANCE ghInstance;

extern CMyD3DApplication g_d3dApp;

extern HWND createRenderWindow(HINSTANCE hInst, HWND hParent, int x, int y, int w, int h);

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	ghInstance = hinstDLL;

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
}



__declspec( dllexport ) void startGame(void)
{
   createRenderWindow(ghInstance, 0, 0, 0, 640, 480);
}

__declspec( dllexport ) void stopGame(void)
{
   PostMessage(g_d3dApp.GetMainWindow(), WM_CLOSE, 0, 0);
}