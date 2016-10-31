/////////////////////////////////////////////////////////////////////////////////////
// GLRender.c
// This is the application shell for the GLRender 
// 
// Created:
//		JL 9/5/99    (That's Jeff Lander, see)
//
///////////////////////////////////////////////////////////////////////////////

// Include files
#include <windows.h>            // Window defines
#include <gl\gl.h>              // OpenGL
#include <gl\glu.h>             // GLU library

#include "glrender.h"

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


// Declaration for Window procedures
// Defined in this file (GLRender.c)
LRESULT CALLBACK WndProcMain(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam);

// Declaration for Window procedure
// See ViewWnd.c
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

		case WM_KEYDOWN:
		        extern void handle_keydown(int);
			handle_keydown(wParam);
			break;

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}
