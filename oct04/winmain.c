/*
 */

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "demo.h"

int FULLSCREEN=FALSE;

CHAR  szAppName[]="Infinite Texture Demo";
HDC   hDC;
HWND  hMainWin;
HGLRC hRC;
HINSTANCE hInst;

void fatal_error(char *s, char *file, int line)
{
   char buffer[512];
   sprintf(buffer, "%s %d: %s", file, line, s);
   OutputDebugString(buffer);
   assert(0);  // catch it in the debugger
   exit(1);
}

/* 
 * Need to change the pixel format / buffer of the
 * window to what we need for the opengl fullscreen
 */
BOOL ChangeResolution(DWORD w, DWORD h) {
  DEVMODE devMode;
  LONG    modeSwitch;
  LONG    i;
  CHAR    buf[256];

  i = 0;
  do { 
    modeSwitch = EnumDisplaySettings(NULL, i, &devMode); 
    i++; 
  }  while((  (devMode.dmBitsPerPel!=16) 
            ||(devMode.dmPelsWidth != w) 
            ||(devMode.dmPelsHeight != h) )
           && (modeSwitch) );
  /* Okay see if we found a mode */
  if (!modeSwitch) { 
    sprintf(buf, "%ldx%ldx65536 (16-bit color/Hicolor) is not supported", w, h);
    MessageBox(GetFocus(), buf, "ChangeResolution Error", MB_ICONERROR); 
    return FALSE; 
  } else { 
    modeSwitch = ChangeDisplaySettings(&devMode, 0); 
    if (modeSwitch!=DISP_CHANGE_SUCCESSFUL) { 
      //Might be running in Windows95, let's try without the hertz change 
      devMode.dmBitsPerPel = 16; 
      devMode.dmPelsWidth = w; 
      devMode.dmPelsHeight = h; 
      devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT; 
      modeSwitch = ChangeDisplaySettings(&devMode, 0); 
      if(modeSwitch != DISP_CHANGE_SUCCESSFUL) { 
        if(modeSwitch == DISP_CHANGE_RESTART) { 
          MessageBox(GetFocus(), "You must change your desktop to 16bit color (Hicolor) and then restart", "Win95/non-16bit color Init Error", MB_ICONERROR); 
        } else if(modeSwitch == DISP_CHANGE_BADMODE) {
          sprintf(buf, "The video mode %ldx%ldx65536 is not supported", w, h);
          MessageBox(GetFocus(), buf, "ChangeResolution Error", MB_ICONERROR); 
        } else if(modeSwitch == DISP_CHANGE_FAILED) {
          sprintf(buf, "Hardware failed to change to %ldx%ldx65536", w, h);
          MessageBox(GetFocus(), buf, "ChangeResolution Error", MB_ICONERROR); 
        }
        return FALSE; 
      } 
    } 
  }
  return TRUE;
}

BOOL SetupPixelFormat(HDC hdc) {
  PIXELFORMATDESCRIPTOR pfd;
  int                   pixelformat;

  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  pfd.dwLayerMask = PFD_MAIN_PLANE;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cAlphaBits = 0;
  pfd.cDepthBits = 16;
  pfd.cAccumBits = 0;
  pfd.cStencilBits = 0;

  /* Search for best matching pixel format */
  OutputDebugString("Choose pixel format\n");
  pixelformat = ChoosePixelFormat(hdc, &pfd);
  OutputDebugString("Choose pixel format done\n");

  if (pixelformat == 0) {
    MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
    return FALSE;
  }

  /* what did we get I wonder */
  DescribePixelFormat(hdc, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  /* set the format to closest match */
  SetPixelFormat(hdc, pixelformat, &pfd);

  return TRUE;
}

void redraw(int x, int y, int h, int w, int clear)
{
   RECT r;
   r.left = x;
   r.top = y;
   r.right = x+h;
   r.bottom = y+w;
   InvalidateRect(hMainWin, &r, clear);
}

void genMouse(int event, WPARAM wParam, LPARAM lParam)
{
   gameMouse(event, (int16) LOWORD(lParam), (int16) HIWORD(lParam), wParam & MK_SHIFT, wParam & MK_CONTROL);
}

static void updateFrame(void);
int WINAPI MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   RECT rect;
   LONG        lRet = 1;
  
   switch (uMsg) {
//      case WM_MOUSEMOVE:    genMouse(E_MOUSEMOVE,  wParam, lParam); break;
//      case WM_LBUTTONDOWN:  genMouse(E_LEFTDOWN,   wParam, lParam); break;
//      case WM_MBUTTONDOWN:  genMouse(E_MIDDLEDOWN, wParam, lParam); break;
//      case WM_RBUTTONDOWN:  genMouse(E_RIGHTDOWN,  wParam, lParam); break;
//      case WM_LBUTTONUP:    genMouse(E_LEFTUP,     wParam, lParam); break;
//      case WM_MBUTTONUP:    genMouse(E_MIDDLEUP,   wParam, lParam); break;
//      case WM_RBUTTONUP:    genMouse(E_RIGHTUP,    wParam, lParam); break;

      case WM_CREATE: {
         hMainWin = hWnd;
         hDC = GetDC(hWnd);
         if (!SetupPixelFormat(hDC)) {
            PostQuitMessage(0);
            break;
         }
         hRC = wglCreateContext(hDC);
         if (!hRC) {
            PostQuitMessage(0);
            break;
         }
         wglMakeCurrent(hDC, hRC);
         SelectObject (hDC, GetStockObject (SYSTEM_FONT)); 
         wglUseFontBitmaps(hDC, 32, 128-32, 32);
         GetClientRect(hWnd, &rect);
         screenInit(rect.right, rect.bottom);
         break;
      }

      case WM_PAINT: {
         PAINTSTRUCT ps;
         HDC hdc = BeginPaint(hWnd, &ps);
         SelectObject(hdc, GetStockObject(NULL_BRUSH));
         demoDraw();
         EndPaint(hWnd, &ps);
         return 0;
      }

      case WM_DESTROY:
         wglMakeCurrent(NULL, NULL) ; 
         if (hRC) { wglDeleteContext(hRC); hRC = 0; }
         if (hDC) { ReleaseDC(hWnd, hDC);  hDC = 0; }
         PostQuitMessage (0);
         break;

      case WM_CHAR:
         if (demoProcessCharacter(wParam))
            PostQuitMessage(0);
         break;

      case WM_ACTIVATE:
        if ( LOWORD(wParam)==WA_INACTIVE )
           wglMakeCurrent(hDC, NULL);
        else
           wglMakeCurrent(hDC, hRC);
        break;

      case WM_SIZE:
         GetClientRect(hWnd, &rect);
         demoResizeViewport(rect.right, rect.bottom);
         break;

      default:
         lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
         break;
   }
  
   return lRet;
}

#define MAX_FPS 120

HWND hCameraWnd;

char szCameraClass[] = "Infinite Texture";

#ifdef SCREENSHOT
static int width=512;
static int wheight=512;
#else
static int width=800;
static int wheight=600;
#endif

static DWORD     thisTime;
static DWORD     lastTime;
static float     elapsedTime;

static void updateFrame(void)
{
   lastTime = thisTime;
   do {
      thisTime = timeGetTime();
      elapsedTime = ((float)(unsigned)(thisTime - lastTime) / 1000);
   } while (elapsedTime < 1.0/MAX_FPS);

   wglMakeCurrent(hDC, hRC);

   if (demoRunLoopmode(elapsedTime))
      PostQuitMessage(0);
}

void resizeCamera(int x, int y)
{
   MoveWindow(hCameraWnd, 0,0, x,y, TRUE);
}
    
/* 
 * Core looping routine, Alt-F4 (standard windows quit key, y'know) to exit.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  MSG       msg;
  WNDCLASSEX  wndclass;
  HWND      hWnd;

  hInst = hInstance;
  
  /* Register the frame class */
  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = CS_OWNDC;
  wndclass.lpfnWndProc   = (WNDPROC)MainWndProc;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = 0;
  wndclass.hInstance     = hInstance;
  wndclass.hIcon         = LoadIcon(hInstance, szAppName);
  wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
  wndclass.hbrBackground = GetStockObject(LTGRAY_BRUSH);
  wndclass.lpszMenuName  = szAppName;
  wndclass.lpszClassName = szAppName;
  wndclass.hIconSm       = LoadIcon(hInstance, szAppName);

  if (!RegisterClassEx(&wndclass))
    return FALSE;

  if (lpCmdLine && strstr(lpCmdLine, "+full"))
     FULLSCREEN = TRUE;

//  if (lpCmdLine && strstr(lpCmdLine, "+laptop")) {

  if ( FULLSCREEN ) {
    if (!ChangeResolution(width, wheight)) return 0;
  }

  /* Create the frame */
  /* Use the WS_POPUP window style to bring up a window that is
  * exactly WIDTHxHEIGHT.
  */
  if ( FULLSCREEN ) {
    hWnd = CreateWindow(szAppName, szAppName, 
                         WS_POPUP |  WS_CLIPSIBLINGS,
                        0, 0, width, wheight,
                        NULL, NULL, hInstance, NULL);
  } else {
    LONG  h;
    LONG  w;

    h = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXDLGFRAME)*2 + wheight + 2;
    w = GetSystemMetrics(SM_CXDLGFRAME)*2 + width + 2;

    // Same window settings as QuakeII
    // WS_BORDER WS_DLGFRAME WS_CLIPSIBLINGS
    hWnd = CreateWindow(szAppName, szAppName, 
                        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
                        16, 16, w, h,
                        NULL, NULL, hInstance, NULL);
  }

  /* Make sure window was created */
  if (!hWnd)
    return FALSE;
  
  /* Show and update main window */
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  InvalidateRect(hWnd, NULL, TRUE);
  demoInit();

  /* Animation loop */
  /* This ensures a fixed animation rate, frame rate will
   * vary by videocard performance
   */
  thisTime = timeGetTime();
  while (1) {
    /* Process all pending messages */
    /* If we dont lower the priority here, the keyboard / mouse events
     * wont get processed, and the computer will be hung
     */

    while (force_draw == FALSE && PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {
      if (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      } else { /* WM_QUIT message */
        ChangeDisplaySettings(NULL, 0); 
        return msg.wParam;
      }
    }

    force_draw = FALSE;

    updateFrame();
  }
}
