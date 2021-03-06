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
#include "GlobalUtility.h"
#include "xskinexp.h"


HINSTANCE ghInstance;
int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	ghInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		InitCustomControls(ghInstance);
		InitCommonControls();
	}

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



__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIBDESC); }


__declspec( dllexport ) int LibNumberClasses() 
{
	return 2;
}


__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		case 0: return GetGUPDesc();
      case 1: return GetXSkinExpDesc();
		default: return 0;
	}
}


__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }



TCHAR *GetString(int id)
{
	static TCHAR buf[256];
	if(ghInstance)
		return LoadString(ghInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}
