/////////////////////////////////////////////////////////////////////////////////////
// Ropes.cpp
// This is the application shell for the Rope simulator
// 
// Created:
//		JL 7/10/02		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2002 Darwin 3D, LLC., All Rights Reserved.
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
static LPCTSTR lpszAppName = "RopeSim";
static HINSTANCE hInstance;


// Declaration for Window procedures
// Defined in this file (Ropes.c)
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
int		g_SimRunning =	FALSE;
long	g_TimeIterations = 2;
int		g_UseFixedTimeStep = TRUE;
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

void SetMenuOptions(HMENU hMenu, long id, int checked)
{
	if (checked)
		CheckMenuItem(hMenu,id,MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuItem(hMenu,id,MF_BYCOMMAND | MF_UNCHECKED);
}


///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadSimulation
// Purpose:		Load's the system settings
///////////////////////////////////////////////////////////////////////////////
void LoadSimFile(char *filename)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	FILE *fp;						// File Pointer
	char header[4];					// File header
///////////////////////////////////////////////////////////////////////////////
	fp = fopen(filename,"rb");
	if (fp)
	{
		fread(header,sizeof(byte),4,fp);
		if (strncmp(header,"ROPE",4) == 0)
		{
			fread(&g_ParticleCount,sizeof(int),1,fp);
			fread(&g_Time,sizeof(DWORD),1,fp);
			fread(&g_Hour,sizeof(float),1,fp);
			for (loop = 0; loop < GROUND_POINTS; loop++)
			{
				fread(&g_Ground[loop],sizeof(float),3,fp);
			}
			fread(&g_DrawSprings,sizeof(int),1,fp);
			fread(&g_DrawCVs,sizeof(int),1,fp);
			fread(&vGravity,sizeof(CVector),1,fp);
			fread(&g_Kd,sizeof(float),1,fp);
			fread(&g_Kr,sizeof(float),1,fp);
			fread(&g_Ksh,sizeof(float),1,fp);
			fread(&g_Ksd,sizeof(float),1,fp);
			fread(&g_YoungModulus,sizeof(float),1,fp);
			fread(&g_FractureThreshold,sizeof(float),1,fp);
			fread(g_GameSys[0],sizeof(t_Particle),g_ParticleCount,fp);
			fread(g_GameSys[1],sizeof(t_Particle),g_ParticleCount,fp);
			fread(g_GameSys[2],sizeof(t_Particle),g_ParticleCount,fp);
			fread(&g_SpringCnt,sizeof(int),1,fp);
			g_Spring = (t_Spring *)malloc(sizeof(t_Spring) * g_SpringCnt);
			fread(g_Spring,sizeof(t_Spring),g_SpringCnt,fp);
		}
		fclose(fp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadSimulation
// Purpose:		Load's the system settings
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
// Purpose:		Save's the system settings
///////////////////////////////////////////////////////////////////////////////
void SaveSimulation()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	OPENFILENAME filestruct;
	char ext[80] = "Simulation Files *.sim\0*.sim\0All Files *.*\0*.*\0\0";
	char file[80] =  "";
	const char header[] = "ROPE";	// File header
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
			fwrite(&g_ParticleCount,sizeof(int),1,fp);
			fwrite(&g_Time,sizeof(DWORD),1,fp);
			fwrite(&g_Hour,sizeof(float),1,fp);
			for (loop = 0; loop < GROUND_POINTS; loop++)
			{
				fwrite(&g_Ground[loop],sizeof(float),3,fp);
			}
			fwrite(&g_DrawSprings,sizeof(int),1,fp);
			fwrite(&g_DrawCVs,sizeof(int),1,fp);
			fwrite(&vGravity,sizeof(CVector),1,fp);
			fwrite(&g_Kd,sizeof(float),1,fp);
			fwrite(&g_Kr,sizeof(float),1,fp);
			fwrite(&g_Ksh,sizeof(float),1,fp);
			fwrite(&g_Ksd,sizeof(float),1,fp);
			fwrite(&g_YoungModulus,sizeof(float),1,fp);
			fwrite(&g_FractureThreshold,sizeof(float),1,fp);
			fwrite(g_GameSys[0],sizeof(t_Particle),g_ParticleCount,fp);
			fwrite(g_GameSys[1],sizeof(t_Particle),g_ParticleCount,fp);
			fwrite(g_GameSys[2],sizeof(t_Particle),g_ParticleCount,fp);
			fwrite(&g_SpringCnt,sizeof(int),1,fp);
			fwrite(g_Spring,sizeof(t_Spring),g_SpringCnt,fp);
			fclose(fp);
		}
	}
}

BOOL APIENTRY SimPropDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char text[80];
	switch (message)
	{
		case WM_INITDIALOG:
			{
				sprintf(text,"%.2f",vGravity.x);
				SetDlgItemText(hDlg, IDC_GRAVX, text);
				sprintf(text,"%.2f",vGravity.y);
				SetDlgItemText(hDlg, IDC_GRAVY, text);
				sprintf(text,"%.2f",vGravity.z);
				SetDlgItemText(hDlg, IDC_GRAVZ, text);
				sprintf(text,"%.2f",g_Kr);
				SetDlgItemText(hDlg, IDC_COEFREST, text);
				sprintf(text,"%.2f",g_Kd);
				SetDlgItemText(hDlg, IDC_Damping, text);
				sprintf(text,"%.2f",g_Ksh);
				SetDlgItemText(hDlg, IDC_SPRINGCONST, text);
				sprintf(text,"%.2f",g_Ksd);
				SetDlgItemText(hDlg, IDC_SPRINGDAMP, text);
				sprintf(text,"%.2f",g_YoungModulus);
				SetDlgItemText(hDlg, IDC_YOUNG, text);
				sprintf(text,"%.2f",g_FractureThreshold);
				SetDlgItemText(hDlg, IDC_FRACTURE, text);
			}
			break;
		// Process command messages
	    case WM_COMMAND:      
			{
				// Validate and Make the changes
				if(LOWORD(wParam) == IDOK)
				{
					GetDlgItemText(hDlg, IDC_GRAVX, text, 80);
					vGravity.x = (float)atof(text);
					GetDlgItemText(hDlg, IDC_GRAVY, text, 80);
					vGravity.y = (float)atof(text);
					GetDlgItemText(hDlg, IDC_GRAVZ, text, 80);
					vGravity.z = (float)atof(text);

					GetDlgItemText(hDlg, IDC_COEFREST, text, 80);
					g_Kr = (float)atof(text);
					GetDlgItemText(hDlg, IDC_Damping, text, 80);
					g_Kd = (float)atof(text);

					GetDlgItemText(hDlg, IDC_SPRINGCONST, text, 80);
					g_Ksh = (float)atof(text);
					GetDlgItemText(hDlg, IDC_Damping, text, 80);
					g_Ksd = (float)atof(text);
					GetDlgItemText(hDlg, IDC_YOUNG, text, 80);
					g_YoungModulus = (float)atof(text);
					GetDlgItemText(hDlg, IDC_FRACTURE, text, 80);
					g_FractureThreshold = (float)atof(text);
					ResetSim();
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
			SetMenuOptions(hMenu, ID_USE_FRICTION, g_UseFriction);
			SetMenuOptions(hMenu, ID_USE_GRAVITY, g_UseGravity);
			SetMenuOptions(hMenu, ID_VIEW_SPRINGS, g_DrawSprings);
			SetMenuOptions(hMenu, ID_VIEW_CVS, g_DrawCVs);

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
				case 'S':		// Sim Running
					{
					g_SimRunning = !g_SimRunning;
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case 'R':		// Sim Running
					{
					ResetSim();
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case 'P':		// Sim Properties
					{
					DialogBox (hInstance,
						MAKEINTRESOURCE(IDD_SIMPROP),
						hWnd,
						SimPropDlgProc);
					break;
					}
				case ' ':
					{
					g_Pick[0] = -1;
					g_Pick[1] = -1;
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
					}
				case 13:		// ENTER
					{
						if (g_Pick[0] > -1 && g_Pick[1] > -1)
						{
							AddSpring(g_Pick[0],g_Pick[1]);
						}
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
				case ID_FILE_NEWSIM:
					g_ParticleCount = 0;
					g_SpringCnt = 0;
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
				case ID_PHYSICS_GRIDSNAP:
					g_GridSnap = !g_GridSnap;
					if (g_GridSnap)
					{
						CheckMenuItem(hMenu,ID_PHYSICS_GRIDSNAP,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_PHYSICS_GRIDSNAP,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_PHYSICS_SIMRUNNING:
					g_SimRunning = !g_SimRunning;
					if (g_SimRunning)
					{
						CheckMenuItem(hMenu,ID_PHYSICS_SIMRUNNING,MF_BYCOMMAND | MF_CHECKED);
					}
					else
						CheckMenuItem(hMenu,ID_PHYSICS_SIMRUNNING,MF_BYCOMMAND | MF_UNCHECKED);

					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_PHYSICS_RESET:
					ResetSim();
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
				case ID_PHYSICS_ADDSPRING:
					if (g_Pick[0] > -1 && g_Pick[1] > -1)
					{
						AddSpring(g_Pick[0],g_Pick[1]);
					}
					InvalidateRect(g_hViewWnd,NULL,FALSE);
					break;
			
				case ID_PHYSICS_SIMPROPERTIES:
					DialogBox (hInstance,
						MAKEINTRESOURCE(IDD_SIMPROP),
						hWnd,
						SimPropDlgProc);
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
	if(!InitSim())
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

	LoadSimFile("default.sim");		// Load Default sim 
	g_Time = (unsigned long)GetTime();

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

