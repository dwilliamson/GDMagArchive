//===========================================================================
/*
 | 
 |  FILE:	GlobalUtility.cpp
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
*/
//===========================================================================

#include "pch.h"
#include "skinnedmesh.h"
#include "GlobalUtility.h"

//===========================================================================
// 
//===========================================================================
#define MENU_TAG 0x1010101

#define IDC_UPDATEVIEWPORT (WM_APP + 237)
#define IDC_ABOUT          (WM_APP + 238)

//===========================================================================
//
//===========================================================================
static INode *gpRootNode;
static WNDPROC gpMainWndProc;
static char gcExportFilename[_MAX_PATH] = { 0 };

SkeletonGUP *gpcontext;

//===========================================================================
//
//===========================================================================
extern HRESULT ExportXFile(const TCHAR *szFilename,
                           Interface *pInterface, BOOL bSuppressPrompts, BOOL bSaveSelection,
                           HWND hwndParent, INode *pRootNode);

extern CMyD3DApplication g_d3dApp;

//===========================================================================
//	Class Descriptor
//===========================================================================

class SkeletonGUPClassDesc:public ClassDesc {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return new SkeletonGUP; }
	const TCHAR *	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return GUP_CLASS_ID; }
	Class_ID 		ClassID()					{ return SKGUP_CLASSID; }
	const TCHAR* 	Category()					{ return _T("");  }
};

//===========================================================================
//
//===========================================================================
static SkeletonGUPClassDesc SkeletonGUPCD;
ClassDesc* GetGUPDesc() {return &SkeletonGUPCD;}

//===========================================================================
// viewFile
//===========================================================================
void viewFile(void)
{
   // see if they've created a viewport, and whether it's active

   if (!IsWindow(g_d3dApp.GetMainWindow()))
   {
      MessageBox(GetActiveWindow(), "There is no Direct3D Viewport active.\n"
                  "Please create one before trying to view something", 
                  "No Direct3D Viewport Found", MB_OK);
      return;
   }

   if (!g_d3dApp.GetActive())
   {
      MessageBox(GetActiveWindow(), "There is no Direct3D Viewport active.\n"
                  "Please activate it before trying to view something", 
                  "No Direct3D Viewport Active", MB_OK);
      return;
   }

   // delete any existing temp file

   if (gcExportFilename[0])
   {
      DeleteFile(gcExportFilename);
      gcExportFilename[0] = 0;
   }

   // get a new temp filename 
   // - we do this instead of re-using the old name, cause the old file might 
   //   be open and in-use by somebody.  you never know

   char cExportDir[_MAX_PATH];
   GetTempPath(sizeof(cExportDir), cExportDir);

   GetTempFileName(cExportDir, "dvp", 0, gcExportFilename);

   DeleteFile(gcExportFilename);

   // export file

   HRESULT hr = ExportXFile(gcExportFilename, gpcontext->Max(), FALSE, FALSE, GetActiveWindow(), gpcontext->Max()->GetRootNode());

   if (FAILED(hr))
      return;

   // load into viewport

   g_d3dApp.loadAndDisplayMesh(gcExportFilename);

} // viewFile

//===========================================================================
//
//===========================================================================
LRESULT CALLBACK SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // could make this a global instead.  either way it's a waste to be a local
   static MENUINFO mi = { sizeof(MENUINFO), MIM_MENUDATA, 0, 0, 0, 0 };

   switch (uMsg)
   {
      case WM_INITMENU:
      case WM_MENUSELECT:
         GetMenuInfo((HMENU) lParam, &mi);

         if (mi.fMask & MIM_MENUDATA)
            if (mi.dwMenuData == MENU_TAG)
               return 0;
         break;         

      case WM_COMMAND:
         switch (LOWORD(wParam))
         {
            case IDC_UPDATEVIEWPORT:
               viewFile();
               return 0;

            case IDC_ABOUT:
               MessageBox(hwnd, "Direct3D Viewport Sample\n"
                                "Game Developer Magazine 2002\n"
                                "Author: herb marselas (herbm@microsoft.com)\n"
                                "Portions Copyright (c) Discreet 1999-2001\n"
                                "Portions Copyright (c) Microsoft Corporation 2000-2001", "About...", MB_OK);
               return 0;

            default:
               break;
         
         } // switch (LOWORD(wParam))

         break;

   } // switch(...)

   return CallWindowProc(gpMainWndProc, hwnd, uMsg, wParam, lParam);

} // WindowProc

//===========================================================================
// 
//===========================================================================
void SkeletonGUP::addnewMenus(HWND hwnd, HMENU hmenu)
{
   BOOL rc;

   HMENU hMenu = CreatePopupMenu();

   rc = AppendMenu(hMenu, MF_BYPOSITION | MF_ENABLED | MF_STRING, IDC_UPDATEVIEWPORT, "Update Viewport...");
   rc = AppendMenu(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
   rc = AppendMenu(hMenu, MF_ENABLED | MF_STRING, IDC_ABOUT, "About...");

   rc = InsertMenu(hmenu, 13, MF_BYPOSITION | MF_ENABLED | MF_POPUP, (UINT) hMenu, "Direct3D Tools");

   // fill in a tag value that we can use

   MENUINFO mi;

   memset(&mi, 0, sizeof(mi));

   mi.cbSize = sizeof(mi);
   mi.fMask = MIM_MENUDATA | MIM_APPLYTOSUBMENUS;
   mi.dwMenuData = MENU_TAG;

   rc = SetMenuInfo(hMenu, &mi);

   // all done

   rc = DrawMenuBar(hwnd);

} // addnewMenus

//===========================================================================
// Global Utility interface (start/stop/control)
//===========================================================================
DWORD SkeletonGUP::Start( ) 
{
   // create and register extended D3D viewport window

	SkeletonViewWindow *skvw = new SkeletonViewWindow(this);
	Max()->RegisterViewWindow(skvw);

   // store off MAX root node

   gpRootNode = Max()->GetRootNode();

   gpcontext = this;

   // setup window subclassing

   HWND hwnd = MaxWnd();

   gpMainWndProc = (WNDPROC) GetWindowLong(hwnd, GWL_WNDPROC);

   SetWindowLong(hwnd, GWL_WNDPROC, (long) SubclassWndProc);

   addnewMenus(hwnd, GetMenu(hwnd));

   // all done

   return GUPRESULT_KEEP;
}

//===========================================================================
//
//===========================================================================
void SkeletonGUP::Stop( ) 
{
   // if we have a valid filename, there must be a temp file we need to delete

   if (gcExportFilename[0])
   {
      DeleteFile(gcExportFilename);
      gcExportFilename[0] = 0;
   }
}

//===========================================================================
//
//===========================================================================
DWORD SkeletonGUP::Control( DWORD parameter ) 
{
	return 0;
}

//===========================================================================
// eof: globalutility.cpp
//===========================================================================
