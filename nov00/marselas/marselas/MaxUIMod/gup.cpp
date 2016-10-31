//==============================================================================
// gup.cpp
//==============================================================================

#include "stdafx.h"
#include "gup.h"

//==============================================================================
//
//==============================================================================
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//==============================================================================
//
//==============================================================================
#define MAX_FILEOPEN (0x9C43)

#define IDC_MAXMENUMOD (WM_APP + 400)

//==============================================================================
//
//==============================================================================
static WNDPROC gpMainWndProc = 0;

static HWND ghWndMenu = 0;

static bool gbMenuModified = false;

static MaxUIModGUP *gpcontext = 0;

const int cLargeBufferSize = 4096;
static char szOutputBuffer[cLargeBufferSize];

//==============================================================================
// MaxUIModClassDesc
//==============================================================================
class MaxUIModClassDesc : public ClassDesc 
{
	public:
	   int            IsPublic(void) { return TRUE; }

	   void           *Create(BOOL loading) { return new MaxUIModGUP; }
	   
      const TCHAR    *ClassName(void) { return "MAX UI Mod"; }
	   
      SClass_ID		SuperClassID(void) { return GUP_CLASS_ID; }
	   
      Class_ID 		ClassID(void) { return SKGUP_CLASSID; }
	   
      const TCHAR    *Category(void) { return _T("");  }

}; // MaxUIModClassDesc

//==============================================================================
//
//==============================================================================
static MaxUIModClassDesc gMaxUIModClassDesc;

//==============================================================================
// LibDescription
//==============================================================================
extern "C" __declspec( dllexport ) const TCHAR *LibDescription(void) { return "MAX UI Mod"; }

//==============================================================================
// LibNumberClasses
//==============================================================================
extern "C" __declspec( dllexport ) int LibNumberClasses(void) { return 1; }

//==============================================================================
// LibClassDesc
//==============================================================================
extern "C" __declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
   if (!i)
      return &gMaxUIModClassDesc;

   return 0;

} // LibClassDesc

//==============================================================================
// LibVersion
//==============================================================================
extern "C" __declspec( dllexport ) ULONG LibVersion(void) { return VERSION_3DSMAX; }

//==============================================================================
// message
//==============================================================================
static void message(const char *lpszFormat, ...)
{
	long nBuf;

	va_list args;
	va_start(args, lpszFormat);

	nBuf = vsprintf(szOutputBuffer, lpszFormat, args);

   ASSERT(nBuf >= 0);               // no errors
   ASSERT(nBuf < cLargeBufferSize); // it fit

   if (szOutputBuffer[nBuf-1] != '\n') 
   {
      szOutputBuffer[nBuf] = '\n';
      szOutputBuffer[nBuf+1] = 0;
   }
   ASSERT(nBuf+1 < cLargeBufferSize);

	OutputDebugString(szOutputBuffer);

   AfxMessageBox(szOutputBuffer);

	va_end(args);

} // message

//==============================================================================
// modifyMenu
//==============================================================================
void modifyMenu(HWND hwnd, HMENU hmenu)
{
   char cMenuName[128] = { 0 };

   // make sure this is the file menu we're dealing with

   long ret = GetMenuString(hmenu, 0, cMenuName, sizeof(cMenuName), MF_BYPOSITION);

   if (!ret)
      return;

   if (!strstr(cMenuName, "File"))
      return;

   // now, get the file menu

   HMENU hfilemenu = GetSubMenu(hmenu, 0);

   if (!hfilemenu)
      return;

   // this must be it, so make the mods

   ghWndMenu = hwnd;

   gbMenuModified = true;

   // add entries to file menu

   InsertMenu(hfilemenu, 8, MF_BYPOSITION | MF_ENABLED | MF_STRING, IDC_MAXMENUMOD, "Force Complete Redraw");

   InsertMenu(hfilemenu, 9, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

   // make sure the update takes

   DrawMenuBar(hwnd);

} // modifyMenu
   
//==============================================================================
// SubclassWndProc
//==============================================================================
LRESULT CALLBACK SubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITMENU:
         if (!gbMenuModified && (GetMenuItemCount((HMENU) wParam) > 1))
            modifyMenu(hwnd, (HMENU) wParam);
         break;

      case WM_COMMAND:
         if (ghWndMenu != hwnd)
            break;

         switch (LOWORD(wParam))
         {
            case MAX_FILEOPEN:
               {
                  // get current filename

                  Interface *pi = gpcontext->Max();

                  CString csoldfile = pi->GetCurFilePath();

                  // let them call open file dlg

                  LRESULT lret = CallWindowProc(gpMainWndProc, hwnd, uMsg, wParam, lParam);

                  // get new filename

                  CString csnewfile = pi->GetCurFilePath();

                  if (!csnewfile.IsEmpty())
                     if (csnewfile != csoldfile)
                        message("A new MAX file was opened\nold file: <%s>\nnew file <%s>", 
                                (const char *) csoldfile, (const char *) csnewfile);

                  return lret;
               }
               break;

            case IDC_MAXMENUMOD:
               {
                  gpcontext->Max()->ForceCompleteRedraw(TRUE);
               }
               return 0;

            default:
               break;
         
         } // switch (LOWORD(wParam))
         break;

   } // switch(...)

   return CallWindowProc(gpMainWndProc, hwnd, uMsg, wParam, lParam);

} // WindowProc

//==============================================================================
// MaxUIModGUP::Start
//==============================================================================
DWORD MaxUIModGUP::Start( ) 
{
   gpcontext = this;

   HWND hwnd = MaxWnd();

   gpMainWndProc = (WNDPROC) GetWindowLong(hwnd, GWL_WNDPROC);

   SetWindowLong(hwnd, GWL_WNDPROC, (long) SubclassWndProc);

   return GUPRESULT_KEEP;

} // MaxUIModGUP::Start

//==============================================================================
// eof: gup.cpp
//==============================================================================

