/////////////////////////////////////////////////////////////////////////////////////
// ToonTown.c
// This is the application shell for the ToonTown simulator
// 
// The code base was pulled from the OpenGL Super Bible.
// Great book that I highly recommend
//
// Created:
//		JL 9/5/99		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

// Include files
#include <windows.h>            // Window defines
#include <math.h>				// Include for sqrt()
#include <gl\gl.h>              // OpenGL
#include <gl\glu.h>             // GLU library
#include "resource.h"           // About box and other resource identifiers.
#include "externs.h"
#include <mmsystem.h>
#include <time.h>


///////////////////////////////////////////////////////////////////////////
// Declaration of shared handles, etc. These are declared as externs in the
// header file externs.h
HWND	 g_hViewWnd = NULL;
HWND     g_hMainWnd = NULL;

HPALETTE hPalette = NULL;

// Class Names for all the window classes in this application
static LPCTSTR lpszMainWndClass = "MainClass";
static LPCTSTR lpszViewWndClass = "ViewClass";


// Application name and instance storeage
static LPCTSTR lpszAppName = "GL ToonTown";
static HINSTANCE hInstance;


// Declaration for Window procedures
// Defined in this file (ToonTown.c)
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


// Dialog procedure for about box
BOOL APIENTRY AboutDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL BuildClasses(void);

float	g_LastTime	=	0;
int		g_SimRunning =	TRUE;
long	g_TimeIterations = 10;
int		g_UseFixedTimeStep = FALSE;
float	g_MaxTimeStep = 0.01f;

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
	
	// No need for background brush for this window
	wc.hbrBackground        = NULL;          
	
	wc.lpszMenuName         = MAKEINTRESOURCE(IDR_MENU);
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

float GetTime( void )
{
    static long StartMilliseconds;
	long CurrentMilliseconds;
    if(!StartMilliseconds)
    {
        // yes, the first time through will be a 0 timestep
        StartMilliseconds = timeGetTime();
    }

    CurrentMilliseconds = timeGetTime();
    return (float)(CurrentMilliseconds - StartMilliseconds) / 1000.0f;
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	RunSim
// Purpose:		Actual simulation loop
// Notes:		Allows you to adjust the rate of simulation or to change it
//				to fixed time steps or actual timesteps.
///////////////////////////////////////////////////////////////////////////////		
void RunSim()
{
/// Local Variables ///////////////////////////////////////////////////////////
	float Time;
	float DeltaTime;
///////////////////////////////////////////////////////////////////////////////

    if (g_UseFixedTimeStep)
		Time = g_LastTime + (g_MaxTimeStep * g_TimeIterations);
	else
		Time = GetTime();

	if (g_SimRunning)
	{
		while(g_LastTime < Time)
		{
			DeltaTime = Time - g_LastTime;
			if(DeltaTime > g_MaxTimeStep)
			{
				DeltaTime = g_MaxTimeStep;
			}

	 		Simulate(DeltaTime,g_SimRunning);
			g_LastTime += DeltaTime;
		}
		g_LastTime = Time;
	}
	else
	{
		DeltaTime = 0;
		Simulate(DeltaTime,g_SimRunning);
	}
	RenderWorld();
}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Entry point of all Windows programs
int APIENTRY WinMain(   HINSTANCE       hInst,
						HINSTANCE       hPrevInstance,
						LPSTR           lpCmdLine,
						int             nCmdShow)
{
	MSG             msg;            // Windows message structure

	// Variable is scoped to this file
	hInstance = hInst;


	// Create the window classes for the main window and the
	// children
	if(!BuildClasses())
		return FALSE;

	// Initializes the world geography
	if(!InitGame())
		return FALSE;

	// Create the main application window
	g_hMainWnd = CreateWindow(
					lpszMainWndClass,
					lpszAppName,
					WS_OVERLAPPEDWINDOW,
					0, 0,               // Size and dimensions of window
					640, 480,
					NULL,
					NULL,
					hInstance,
					NULL);

	// Display the window
	ShowWindow(g_hMainWnd,SW_SHOW);
	UpdateWindow(g_hMainWnd);

	// Process application messages until the application closes
    for (;;)
    {
        if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
			RunSim();
        }
    }

	return msg.wParam;
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
			HANDLE hMenu = GetMenu(hWnd);

			// Set initial menu check state
			CheckMenuItem(hMenu,ID_USE_FRICTION,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(hMenu,ID_USE_GRAVITY,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(hMenu,ID_VIEW_SPRINGS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(hMenu,ID_VIEW_CVS,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(hMenu,ID_VIEW_MESH,MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(hMenu,ID_VIEW_VERTEXINFLUENCES,MF_BYCOMMAND | MF_UNCHECKED);		
			
			// Create the child windows
			// View Window
			g_hViewWnd = CreateWindow(
					lpszViewWndClass,
					NULL,
					WS_DLGFRAME | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS| WS_VISIBLE,
					0, 0,               
					10, 10,
					hWnd,
					NULL,
					hInstance,
					NULL);

			g_LastTime = GetTime();
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

			GetClientRect(g_hMainWnd,&clientRect);

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

		// Catch and handle the arrow keys for movement
		case WM_KEYDOWN:
			{
			switch(wParam)
				{
				case 'G':		// Toggle Gravity
					{
					g_UseGravity = !g_UseGravity;
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case 'S':		// Toggle Springs
					{
					g_DrawSprings = !g_DrawSprings;
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}

				case VK_UP:		// Up arrow, move forward
					{

					if (g_POV.rot.x > 0.0f) g_POV.rot.x -= 1.0;			
					// Invalidate the view window (compass doesn't change)
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}

				case VK_DOWN: // Down arrow, move backward
					{

					if (g_POV.rot.x < 90.0f) g_POV.rot.x += 1.0;			
					// Invalidate the view window (compass doesn't change)
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}

				case VK_LEFT:		// Left arrow, turn left
					{
					g_POV.rot.y += 1.0;			
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
		
				case VK_RIGHT:		// Right Arrow, turn right
					{
					g_POV.rot.y -= 1.0;			
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case VK_PRIOR:		// PGUP Zoom Out
					{

					if (g_POV.trans.z > 1.0f) g_POV.trans.z -= 1.0;			
					// Invalidate the view window (compass doesn't change)
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case VK_NEXT:		// PGDN Zoom In
					{

					if (g_POV.trans.z < 9.0f) g_POV.trans.z += 1.0;			
					// Invalidate the view window (compass doesn't change)
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				}
			}
			break;

   		// A menu command
		case WM_COMMAND:
			{
			HANDLE hMenu = GetMenu(hWnd);

			switch(LOWORD(wParam))
				{
				// Exit the program
				case ID_FILE_EXIT:
					DestroyWindow(hWnd);
					break;

				// Display the about box
				case ID_HELP_ABOUT:
					DialogBox (hInstance,
						MAKEINTRESOURCE(IDD_DIALOG_ABOUT),
						hWnd,
						AboutDlgProc);
					break;

				// Turn On/Off Friction
				case ID_USE_FRICTION:
					g_UseFriction = !g_UseFriction;
					if (g_UseFriction)
					{
						CheckMenuItem(hMenu,ID_USE_FRICTION,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_USE_FRICTION,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				// Turn On/Off Friction
				case ID_USE_GRAVITY:
					g_UseGravity = !g_UseGravity;
					if (g_UseGravity)
					{
						CheckMenuItem(hMenu,ID_USE_GRAVITY,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_USE_GRAVITY,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_VIEW_SPRINGS:
					g_DrawSprings = !g_DrawSprings;
					if (g_DrawSprings)
					{
						CheckMenuItem(hMenu,ID_VIEW_SPRINGS,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_VIEW_SPRINGS,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_VIEW_CVS:
					g_DrawCVs = !g_DrawCVs;
					if (g_DrawCVs)
					{
						CheckMenuItem(hMenu,ID_VIEW_CVS,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_VIEW_CVS,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_VIEW_MESH:
					g_DrawMesh = !g_DrawMesh;
					if (g_DrawMesh)
					{
						CheckMenuItem(hMenu,ID_VIEW_MESH,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_VIEW_MESH,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_VIEW_VERTEXINFLUENCES:
					g_DrawInfluence = !g_DrawInfluence;
					if (g_DrawInfluence)
					{
						CheckMenuItem(hMenu,ID_VIEW_VERTEXINFLUENCES,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_VIEW_VERTEXINFLUENCES,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				}
			}
			break;


	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}



///////////////////////////////////////////////////////////////////////////
// Dialog procedure 
BOOL APIENTRY AboutDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
		// Process command messages
	    case WM_COMMAND:      
			{
			// Validate and Make the changes
			if(LOWORD(wParam) == IDOK)
				EndDialog(hDlg,TRUE);
		    }
			break;

		// Closed from sysbox
		case WM_CLOSE:
			EndDialog(hDlg,TRUE);
			break;
	}
	return FALSE;
}








