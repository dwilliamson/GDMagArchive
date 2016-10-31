/////////////////////////////////////////////////////////////////////////////////////
// GLRender.c
// This is the application shell for the GLRender 
// 
// Created:
//		JL 9/5/99    (That's Jeff Lander, see)
//
///////////////////////////////////////////////////////////////////////////////

// Include files
#include <stdio.h>
#include <fstream.h>
#include <strstrea.h>
#include <iomanip.h>
#include <math.h>

#include <windows.h>            // Window defines
#include <gl/gl.h>              // OpenGL
#include <gl/glu.h>             // GLU library
#include <assert.h>

#include "app_config.h"
#include "win32_opengl.h"

bool app_is_active = true;

///////////////////////////////////////////////////////////////////////////
// Declaration of shared handles, etc. These are declared as externs in the
// header file externs.h
HWND	 g_hViewWnd = NULL;
HWND     g_hMainWnd = NULL;

// Class Names for all the window classes in this application
LPCTSTR lpszMainWndClass = "MainClass";
LPCTSTR lpszViewWndClass = "ViewClass";

HWND AppWindow;

int			g_ScreenHeight,g_ScreenWidth;

HDC			g_hDC = 0;	// Keep the Device Context
HPALETTE	hPalette = NULL;
HGLRC  g_hRC;


// Application name and instance storeage
HINSTANCE hInstance;


// Declaration for Window procedure
// See ViewWnd.c
LRESULT CALLBACK WndProcMain(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
                            LPARAM  lParam);
LRESULT CALLBACK WndProcView(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam);


/////////////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Create the window classes. Returns FALSE if any errors occur
BOOL BuildClasses(void)
{
	WNDCLASS        wc;                // Windows class structure

	// Register window style for the main window
	wc.style                = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc          = (WNDPROC) WndProcMain;
	wc.cbClsExtra           = 0;
	wc.cbWndExtra           = 0;
	wc.hInstance            = hInstance;
	wc.hIcon                = LoadIcon(NULL,"APPICON");
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	
	
	wc.lpszMenuName         = NULL; 
	wc.lpszClassName        = lpszMainWndClass;

	// Register the window class
	if(RegisterClass(&wc) == 0)
		return FALSE;


	// Register window style for the main view window
	// This is a window OpenGL will render in
	wc.style                = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc          = (WNDPROC) WndProcView;

	// May want to change the cursor
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);
	
	// No need for background brush for this window
	// Delete me later
	wc.hbrBackground        = NULL;
	wc.lpszClassName        = lpszViewWndClass;

	// Register the window class
	if(RegisterClass(&wc) == 0)
		return FALSE;

	return TRUE;
}

// Select the pixel format for a given device context.
void SetDCDepthPixelFormat(HDC hDC)
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size of this structure
		1,                                                              // Version of this structure    
		PFD_DRAW_TO_WINDOW |                    // Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |					// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,                       // Double buffered
		PFD_TYPE_RGBA,                          // RGBA Color mode
		24,                                     // Want 24bit color 
		0,0,0,0,0,0,                            // Not used to select mode
		0,0,                                    // Not used to select mode
		0,0,0,0,0,                              // Not used to select mode
		// Try to get away with smaller depth buffer to take advantage
		// of low end PC accelerator cards
		32,                                     // Size of depth buffer
		0,                                      // Not used to select mode
		0,                                      // Not used to select mode
		PFD_MAIN_PLANE,                         // Draw in main plane
		0,                                      // Not used to select mode
		0,0,0 };                                // Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	// Set the pixel format for the device context
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

int translate_ascii_code(WPARAM wParam) {
    switch (wParam) {
    // Hacky keybindings so that we don't have to define a bunch of
    // OS-independent constants for these, like we would if we
    // were writing a real game input system.
    case VK_LEFT:
    case VK_PRIOR:
        return '!';
    case VK_RIGHT:
    case VK_NEXT:
        return '@';
    case VK_UP:
        return '#';
    case VK_DOWN:
        return '$';
    default:
        return wParam;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Window procedure, handles all top messages for this program
LRESULT CALLBACK WndProcMain(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam)
{
	switch (message)
		{
		// Window creation, setup here
		case WM_CREATE:
			{
			// Create the child windows
			// OpenGL View Window
			g_hViewWnd = CreateWindow(
					lpszViewWndClass,
					NULL,
					WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS| WS_VISIBLE,
					0, 0,               
					10, 10,
					hWnd,
					NULL,
					hInstance,
					NULL);

			}
			break;

		// Window is being destroyed, cleanup
		case WM_DESTROY:

			// Tell the application to terminate after the window
			// is gone.
			PostQuitMessage(0);
			break;

		// Window is resized.
		// This is really only going to be called once. Why resize here
		// instead of just creating the windows where we want them? Because
		// changes to the GUI may change the size of borders, etc. This
		// code should continue to work regardless of any changes to the
		// GUI. 
		case WM_SIZE:
			{
			RECT clientRect;
			int width,height;

			GetClientRect(AppWindow,&clientRect);

			// Position the viewing window
			width = clientRect.right - clientRect.left;
			height = (clientRect.bottom*2)/5;
			MoveWindow(g_hViewWnd,0,0,
					width,clientRect.bottom-clientRect.top,TRUE);

			}
			break;

		// Windows is telling the application that it may modify
		// the system palette.  This message in essance asks the 
		// application for a new palette.
		case WM_QUERYNEWPALETTE:
			// Pass the message to the OpenGL Windows, none of the other
			// Windows use anything outside the standard set of colors
			PostMessage(g_hViewWnd,message,wParam,lParam);
			break;

	
		// This window may set the palette, even though it is not the 
		// currently active window.
		case WM_PALETTECHANGED:
			// Pass the message to the OpenGL Windows, none of the other
			// Windows use anything outside the standard 16 colors
			PostMessage(g_hViewWnd,message,wParam,lParam);
			break;

		case WM_KEYDOWN: {
            extern void handle_keydown(int);
            int ascii_code;
            ascii_code = translate_ascii_code(wParam);
			handle_keydown(ascii_code);
			break;
        }
		case WM_KEYUP: {
            extern void handle_keyup(int);
            int ascii_code;
            ascii_code = translate_ascii_code(wParam);
			handle_keyup(ascii_code);
			break;
        }

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));
	}

    return (0L);
}

LRESULT CALLBACK WndProcView(HWND    hWnd,
			     UINT    message,
			     WPARAM  wParam,
			     LPARAM  lParam)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int tx,ty;
/////////////////////////////////////////////////////////////////////////////////////
	
	switch (message) {
      case WM_ACTIVATEAPP:
        if (wParam) app_is_active = true;
        else app_is_active = false;
	  // Window creation, setup here
      case WM_CREATE:
          g_hDC = GetDC(hWnd);

			// Set the pixel format
          SetDCDepthPixelFormat(g_hDC);

          // Create the rendering context and make it current
          g_hRC = wglCreateContext(g_hDC);
          assert(g_hRC != NULL);

          BOOL succ;
          succ = wglMakeCurrent(g_hDC, g_hRC);
          assert(succ);

          break;

      // Window is being destroyed, cleanup
      case WM_DESTROY:
        // Cleanup...
        // Deselect the current rendering context and delete it
        wglMakeCurrent(g_hDC,NULL);
        wglDeleteContext(g_hRC);

        // Destroy the palette if it was created 
        if(hPalette != NULL)
            DeleteObject(hPalette);

			// Release the device context
        ReleaseDC(hWnd,g_hDC);
        break;

	  // Window is resized. Setup the viewing transformation
      case WM_SIZE: {
          int nWidth,nHeight;
          double dAspect;

          nWidth = LOWORD(lParam);  // width of client area 
          nHeight = HIWORD(lParam); // height of client area 

          g_ScreenWidth = nWidth;
          g_ScreenHeight = nHeight;

          if(nHeight == 0)		  // Don't allow divide by zero
              nHeight = 1;

          dAspect = (double)nWidth/(double)nHeight;

          // Make this rendering context current
          wglMakeCurrent(g_hDC, g_hRC);

          // Set the viewport to be the entire window
          glViewport(0, 0, nWidth, nHeight);
	
          // Setup Perspective
          glMatrixMode(GL_PROJECTION);
          glLoadIdentity();

          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();

          break;
      }

      case WM_PAINT: 
      {
        // Validate the newly painted client area
        ValidateRect(hWnd,NULL);
        break;
      }

      // Windows is telling the application that it may modify
      // the system palette.  This message in essance asks the 
      // application for a new palette.
      case WM_QUERYNEWPALETTE:
        // If the palette was created.
        if(hPalette) {
            int nRet;

            // Selects the palette into the current device context
            SelectPalette(g_hDC, hPalette, FALSE);

            // Map entries from the currently selected palette to
            // the system palette.  The return value is the number 
            // of palette entries modified.
            nRet = RealizePalette(g_hDC);

            // Repaint, forces remap of palette in current window
            InvalidateRect(hWnd,NULL,FALSE);

            return nRet;
        }
        break;

	
      // This window may set the palette, even though it is not the 
	  // currently active window.
	  case WM_PALETTECHANGED:
          // Don't do anything if the palette does not exist, or if
          // this is the window that changed the palette.
          if((hPalette != NULL) && ((HWND)wParam != hWnd))
              {
                  // Select the palette into the device context
                  SelectPalette(g_hDC,hPalette,FALSE);

				// Map entries to system palette
                  RealizePalette(g_hDC);
				
                  // Remap the current colors to the newly realized palette
                  UpdateColors(g_hDC);
                  return 0;
              }
          break;


		// Handle Right Mouse Button Press
      case WM_RBUTTONDOWN:
        break;

		// Handle Right Mouse Button Release
      case WM_RBUTTONUP:
			break;

      default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return 0L;
}



int unsigned GetCommandLineArgInt(char const *pArgument,
                                  char const *pCommandLine,
                                  int unsigned DefaultValue) {
    assert(pArgument && pCommandLine);
    int unsigned Value = DefaultValue;
    char const *pStart;
    if((pStart = strstr(pCommandLine,pArgument)) != 0)
    {
        char aFormat[1024];
        ostrstream Out(aFormat,sizeof(aFormat));
        Out<<pArgument<<"=%d"<<ends;
        sscanf(pStart,aFormat,&Value);
    }
    return Value;
}

bool GetCommandLineArgString(char const *pArgument,
                             char const *pCommandLine,
                             char *pStringBuffer) {
    assert(pArgument && pCommandLine && pStringBuffer);

    bool ReturnValue = false;
    char const *pStart;
    if((pStart = strstr(pCommandLine,pArgument)) != 0) {
        char aFormat[1024];
        ostrstream Out(aFormat,sizeof(aFormat));
        Out<<pArgument<<"=%s"<<ends;
        int Fields = sscanf(pStart,aFormat,pStringBuffer);

        if (Fields && (Fields != EOF)) {
            ReturnValue = true;
        }
    }

    return ReturnValue;
}

int AppPaint(HWND, HDC) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    extern void draw_scene();
    draw_scene();
    SwapBuffers(g_hDC);

    Sleep(20); // Let us not eat all the CPU under any circumstances!
    return TRUE;
}

int PASCAL WinMain(HINSTANCE hinst, HINSTANCE,
                   LPSTR pCommandLine, int ShowMode) {
    hInstance = hinst;

    // get the user's specified display parameters, or defaults
    // the shortened names work because of the default value
    
    g_ScreenWidth = GetCommandLineArgInt("width", 
                                           pCommandLine, APP_DEFAULT_WIDTH);

    g_ScreenHeight = GetCommandLineArgInt("height", 
                                            pCommandLine, APP_DEFAULT_HEIGHT);


    // Create the window classes for the main window and the
    // children
    if(!BuildClasses()) return FALSE;

    // Create the main application window
    AppWindow = CreateWindow(
			     lpszMainWndClass,
			     APP_NAME,
			     WS_OVERLAPPEDWINDOW,
			     0, 0,               // Size and dimensions of window
			     g_ScreenWidth, g_ScreenHeight,
			     NULL,
			     NULL,
			     hInstance,
			     NULL);

    assert(g_hDC != 0);

    // Display the window
    ShowWindow(AppWindow,SW_SHOW);
    UpdateWindow(AppWindow);

    // Set the viewport to be the entire window
    wglMakeCurrent(g_hDC, g_hRC);
    glViewport(0, 0, g_ScreenWidth, g_ScreenHeight);

    MSG msg;
    for (;;) {
        if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            if (app_is_active) {
                AppPaint(AppWindow,g_hDC);
            } else {
                WaitMessage();
            }
        }
    }

    return msg.wParam;
}

