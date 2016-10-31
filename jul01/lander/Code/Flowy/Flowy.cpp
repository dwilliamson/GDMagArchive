/////////////////////////////////////////////////////////////////////////////////////
// Flowy.cpp
// This is the application shell for the Flowy simulator
// 
// Created:
//		JL 2/5/2001		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2001 Darwin 3D, LLC, All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

// Include files
#include <windows.h>            // Window defines
#include <math.h>				// Include for sqrt()
#include <stdio.h>				// Include for sqrt()
#include <gl\gl.h>              // OpenGL
#include <gl\glu.h>             // GLU library
#include "resource.h"           // About box and other resource identifiers.
#include "externs.h"
#include <mmsystem.h>
#include <time.h>
#include <commdlg.h>


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
static LPCTSTR lpszAppName = "Flowy";
static HINSTANCE hInstance;


// Declaration for Window procedures
// Defined in this file (Flowy.c)
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
BOOL APIENTRY ElemDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL BuildClasses(void);

int		g_SimRunning =	TRUE;

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
	wc.hIcon                = LoadIcon(hInstance,"APPICON");
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

void SetMenuOptions(HMENU hMenu, long id, int checked)
{
	if (checked)
		CheckMenuItem(hMenu,id,MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuItem(hMenu,id,MF_BYCOMMAND | MF_UNCHECKED);
}


///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadSimulation
// Purpose:		Load's the system settings ()
///////////////////////////////////////////////////////////////////////////////
void LoadSimFile(char *filename)
{
/// Local Variables ///////////////////////////////////////////////////////////
	FILE *fp;						// File Pointer
	char header[4];					// File header
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"rb");
	if (fp)
	{
		fread(header,sizeof(byte),4,fp);
		if (strncmp(header,"IMPR",4) == 0)
		{
			fread(&g_ElementCnt,sizeof(int),1,fp);
			fread(&g_Element,sizeof(t_Element),g_ElementCnt,fp);
		}
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadSimulation
// Purpose:		Load's the system settings (tide, floor, etc)
///////////////////////////////////////////////////////////////////////////////
void LoadSimulation()
{
/// Local Variables ///////////////////////////////////////////////////////////
	OPENFILENAME filestruct;
	char ext[80] = "Simulation Files *.sim\0*.sim\0All Files *.*\0*.*\0\0";
	char file[80] =  "";
///////////////////////////////////////////////////////////////////////////////
	memset(&filestruct,0,sizeof(OPENFILENAME));
	filestruct.lStructSize = sizeof(OPENFILENAME);
	filestruct.lpstrFilter = ext;
	filestruct.lpstrFile = file;
	filestruct.nMaxFile = 80; 
	if (GetOpenFileName(&filestruct))
	{
		LoadSimFile(filestruct.lpstrFile);
	}
}	

///////////////////////////////////////////////////////////////////////////////
// Procedure:	SaveSimulation
// Purpose:		Save's the system settings (tide, floor, etc)
///////////////////////////////////////////////////////////////////////////////
void SaveSimulation()
{
/// Local Variables ///////////////////////////////////////////////////////////
	OPENFILENAME filestruct;
	char ext[80] = "Simulation Files *.sim\0*.sim\0All Files *.*\0*.*\0\0";
	char file[80] =  "";
	const char header[] = "IMPR";	// File header
	FILE *fp;						// File Pointer
///////////////////////////////////////////////////////////////////////////////
	memset(&filestruct,0,sizeof(OPENFILENAME));
	filestruct.lStructSize = sizeof(OPENFILENAME);
	filestruct.lpstrFilter = ext;
	filestruct.lpstrFile = file;
	filestruct.nMaxFile = 80; 
	if (GetSaveFileName(&filestruct))
	{
		fp = fopen(filestruct.lpstrFile,"wb");
		if (fp)
		{
			fwrite(header,sizeof(byte),4,fp);
			fwrite(&g_ElementCnt,sizeof(int),1,fp);
			fwrite(&g_Element,sizeof(t_Element),g_ElementCnt,fp);
			fclose(fp);
		}
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
			HMENU hMenu = GetMenu(hWnd);
			
			// Set initial menu check state
			SetMenuOptions(hMenu, ID_SIM_RUNNING, g_SimRunning);
			SetMenuOptions(hMenu, ID_VIEW_DRAWFLOWELEMENTS, g_DrawInfluence);
			SetMenuOptions(hMenu, ID_VIEW_PARTICLES, g_DrawParticles);
			
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
			case 'S':		// Sim Running
				{
					g_SimRunning = !g_SimRunning;
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				}
			case VK_DELETE:		// Delete Element
				{
    				        DeleteElement();
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				}

			}
		}
		break;
		
		// A menu command
	case WM_COMMAND:
		{
			HMENU hMenu = GetMenu(hWnd);
			
			switch(LOWORD(wParam))
			{
			case ID_FILE_NEWSIMULATION:
				g_ElementCnt = 0;
				break;
				// Load a Simulation
			case ID_FILE_LOADSIMULATION:
				LoadSimulation();
				break;
				// Save a Simulation
			case ID_FILE_SAVESIMULATION:
				SaveSimulation();
				break;
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
			case ID_SIM_RUNNING:
				g_SimRunning = !g_SimRunning;
				if (g_SimRunning)
				{
					CheckMenuItem(hMenu,ID_SIM_RUNNING,MF_BYCOMMAND | MF_CHECKED);
				}
				else
					CheckMenuItem(hMenu,ID_SIM_RUNNING,MF_BYCOMMAND | MF_UNCHECKED);
				
				InvalidateRect(g_hViewWnd,NULL,FALSE);
				break;
			case ID_VIEW_PARTICLES:
				g_DrawParticles = !g_DrawParticles;
				if (g_DrawParticles)
				{
					CheckMenuItem(hMenu,ID_VIEW_PARTICLES,MF_BYCOMMAND | MF_CHECKED);
				}
				else
					CheckMenuItem(hMenu,ID_VIEW_PARTICLES,MF_BYCOMMAND | MF_UNCHECKED);
				
				InvalidateRect(g_hViewWnd,NULL,FALSE);
				break;

			case ID_VIEW_DRAWFLOWELEMENTS:
				g_DrawInfluence = !g_DrawInfluence;
				if (g_DrawInfluence)
				{
					CheckMenuItem(hMenu,ID_VIEW_DRAWFLOWELEMENTS,MF_BYCOMMAND | MF_CHECKED);
				}
				else
					CheckMenuItem(hMenu,ID_VIEW_DRAWFLOWELEMENTS,MF_BYCOMMAND | MF_UNCHECKED);
				InvalidateRect(g_hViewWnd,NULL,FALSE);
				break;
			case ID_ELEMENTS_ADDELEMENT:
				AddElement();
				break;
			case ID_EDIT_ELEMENT:
				if (g_Picked > -1)
					DialogBox (hInstance,
						MAKEINTRESOURCE(IDD_DIALOG_ELEMENT),
						hWnd,
						ElemDlgProc);		
				break;
			case ID_ELEMENTS_DELETEELEMENT:
				DeleteElement();
				break;
			}
			break;
		}
			
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

BOOL APIENTRY ElemDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char text[80];
	switch (message)
	{
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_ADDSTRING,0,(LONG)(LPCTSTR)"Uniform");
			SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_ADDSTRING,0,(LONG)(LPCTSTR)"Source");
			SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_ADDSTRING,0,(LONG)(LPCTSTR)"Vortex");
			SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_ADDSTRING,0,(LONG)(LPCTSTR)"Doublet");
			SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_SETCURSEL,g_Element[g_Picked].type,NULL);
                        if (g_Element[g_Picked].emitParticles)
			    SendDlgItemMessage(hDlg,IDC_EMITPART,BM_SETCHECK,BST_CHECKED,NULL);
                        else
			    SendDlgItemMessage(hDlg,IDC_EMITPART,BM_SETCHECK,BST_UNCHECKED,NULL);
			sprintf(text,"%.2f",g_Element[g_Picked].size);
			SetDlgItemText(hDlg, IDC_ELEMSIZE, text);
			sprintf(text,"%.2f",g_Element[g_Picked].strength);
			SetDlgItemText(hDlg, IDC_ELEMSTRENGTH, text);
			
		}
		break;
		// Process command messages
	case WM_COMMAND:      
		{
			// Validate and Make the changes
			if(LOWORD(wParam) == IDOK)
			{
				int sel;
				sel = SendDlgItemMessage(hDlg,IDC_ELEMTYPE,CB_GETCURSEL,0,0);
				g_Element[g_Picked].type = sel;
				GetDlgItemText(hDlg, IDC_ELEMSIZE, text, 80);
				g_Element[g_Picked].size = atof(text);
				GetDlgItemText(hDlg, IDC_ELEMSTRENGTH, text, 80);
				g_Element[g_Picked].strength = atof(text);
                                if (SendDlgItemMessage(hDlg,IDC_EMITPART,BM_GETCHECK,NULL,NULL) == BST_CHECKED)
                                    g_Element[g_Picked].emitParticles = true;
                                else
                                    g_Element[g_Picked].emitParticles = false;
				EndDialog(hDlg,TRUE);
			}
			else if(LOWORD(wParam) == IDCANCEL)
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

	// Initializes the particles setup
//	if(!InitParticles())
//		return FALSE;

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

	LoadSimFile("default.sim");		// Load Default sim 

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
			if (g_SimRunning) RenderWorld();
        }
    }

	return msg.wParam;
}

