//////////////////////////////////////
// Program Name: Window1
// Programmer: Charlie Calvert
// Description: Example Windows program.
//////////////////////////////////////

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <initguid.h>
#pragma warning (disable: 4068)
#pragma warning (disable: 4100)
 
static char szAppName[] = "CFire";
static HWND MainWindow;

/* 
 You do not need to use _export in WIN32. The 
 EXPORT16 macro provides compatibility between
 WIN16 and WIN32. 
*/

#if !defined(__WIN32__) && !defined(_WIN32)
#define EXPORT16 __export
#else
#define EXPORT16 
#endif

DEFINE_GUID(CLSID_ISPEEDDRAW, 0x185FF680, 0xDD53, 0x11CF, 0x92,0xC4,0x00,0x80,0xC8,0x0C,0xF1,0xD2);

LRESULT CALLBACK EXPORT16 WndProc(HWND hWindow, UINT Message,
                                  WPARAM wParam, LPARAM lParam);
BOOL Register(HINSTANCE hInst);
HWND Create(HINSTANCE hInst, int nCmdShow);

// ===================================
// INITIALIZATION
// ===================================

//////////////////////////////////////
// The WinMain function is the program entry point.
// Register the Window, Create it, enter the Message Loop.
// If either step fails, exit without creating the window
//////////////////////////////////////
#pragma argsused
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdParam, int nCmdShow)
{
  MSG  Msg;

  if (!hPrevInstance)
    if (!Register(hInst))
      return FALSE;

  MainWindow = Create(hInst, nCmdShow);
  if (!MainWindow)
    return FALSE;

  while (GetMessage(&Msg, NULL, 0, 0))
  {
     TranslateMessage(&Msg);
     DispatchMessage(&Msg);
  }

  return Msg.wParam;
}

//////////////////////////////////////
// Register the window
//////////////////////////////////////
BOOL Register(HINSTANCE hInst)
{
  /* You can use WNDCLASSEX and RegisterClassEx with WIN32 */

  WNDCLASS WndClass;

  WndClass.style         = CS_HREDRAW | CS_VREDRAW;
  WndClass.lpfnWndProc   = WndProc;
  WndClass.cbClsExtra    = 0;
  WndClass.cbWndExtra    = 0;
  WndClass.hInstance     = hInst;
  WndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  WndClass.lpszMenuName  = NULL;
  WndClass.lpszClassName = szAppName;

  return (RegisterClass(&WndClass) != 0);
}


//////////////////////////////////////
// Create the window
//////////////////////////////////////
HWND Create(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWindow = CreateWindowEx(0, szAppName, szAppName,
                    WS_OVERLAPPEDWINDOW, 
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL, NULL, hInstance, NULL);

  if (hWindow == NULL)
    return hWindow;

  ShowWindow(hWindow, nCmdShow);
  UpdateWindow(hWindow);

  return hWindow;
}

// =====================================
// IMPLEMENTATION
// =====================================
#define Window1_DefProc    DefWindowProc
void Window1_OnDestroy(HWND hwnd);
void Window1_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
//////////////////////////////////////
// The window proc is where messages get processed
//////////////////////////////////////


LRESULT CALLBACK EXPORT16 WndProc(HWND hWindow, UINT Message,
                                  WPARAM wParam, LPARAM lParam)
{
  switch(Message)
  {
    HANDLE_MSG(hWindow, WM_DESTROY, Window1_OnDestroy);
    HANDLE_MSG(hWindow, WM_LBUTTONDOWN, Window1_OnLButtonDown);
    default:
      return Window1_DefProc(hWindow, Message, wParam, lParam);
  }
}

class ISpeedDraw: public IUnknown
{
public:
   STDMETHOD(QueryInterface) (THIS_ REFIID, LPVOID*) PURE;
   STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
   STDMETHOD_(ULONG,Release) (THIS) PURE;
   STDMETHOD_(BOOL, BackgroundBlits) (THIS) PURE;
   STDMETHOD_(BOOL, Pause) (THIS) PURE;
   STDMETHOD_(VOID, Restore) (THIS) PURE;
   STDMETHOD_(VOID, Create) (THIS) PURE;
   STDMETHOD_(VOID, InitParams)(THIS_ HWND AHandle,
                               LPSTR BackGroundMapStr,
                               int TransColor,
                               LPSTR DllName) PURE;
   STDMETHOD_(VOID, Run) (THIS) PURE;
   STDMETHOD_(VOID, DestroyObjects) (THIS) PURE;
   STDMETHOD_(VOID, DoFlip)(THIS) PURE;
   STDMETHOD_(VOID, InitObjects) (THIS) PURE;
   STDMETHOD_(VOID, Move) (THIS_ int Value) PURE;
   STDMETHOD_(VOID, MakeActive) (THIS_ BOOL Value) PURE;
   STDMETHOD_(VOID, SetTimerOdd) (THIS_ BOOL Value) PURE;
   STDMETHOD_(VOID, SetBackOrigin) (THIS_ POINT Value) PURE;
};
typedef ISpeedDraw *PISpeedDraw;

PISpeedDraw P;

//////////////////////////////////////
// Handle WM_DESTROY message
//////////////////////////////////////
#pragma argsused
void Window1_OnDestroy(HWND hwnd)
{
  if (P)
    P->Release();
  CoUninitialize;
  PostQuitMessage(0);
}

#pragma argsused
void Window1_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
  HRESULT hr;

  CoInitialize(NULL);
  hr = CoCreateInstance(CLSID_ISPEEDDRAW, NULL, CLSCTX_INPROC_SERVER,
                         CLSID_ISPEEDDRAW, (VOID**) &P);
  if (SUCCEEDED(hr))
  {
    P->InitParams(hwnd, "..\\BACKGRD1.BMP", 254, "");
    P->Run();
  }
  else
    MessageBox(hwnd, "No Object", "Info", MB_OK);
}
