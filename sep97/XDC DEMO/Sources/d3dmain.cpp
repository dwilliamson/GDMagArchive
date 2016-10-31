/**************************************************************************

Mixed Rendering

 **************************************************************************/
/***************************************************************
*
*       This program has been developed by Intel Corporation.  
*		You have Intel's permission to incorporate this code 
*       into your product, royalty free.  Intel has various 
*	    intellectual property rights which it may assert under
*       certain circumstances, such as if another manufacturer's
*       processor mis-identifies itself as being "GenuineIntel"
*		when the CPUID instruction is executed.
*
*       Intel specifically disclaims all warranties, express or
*       implied, and all liability, including consequential and
*		other indirect damages, for the use of this code, 
*		including liability for infringement of any proprietary
*		rights, and including the warranties of merchantability
*		and fitness for a particular purpose.  Intel does not 
*		assume any responsibility for any errors which may 
*		appear in this code nor any responsibility to update it.
*
*  * Other brands and names are the property of their respective
*    owners.
*
*  Copyright (c) 1995, Intel Corporation.  All rights reserved.
***************************************************************/

/*
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: d3dmain.cpp
 *
 *  Each of the Direct3D samples must be linked with this file.  It contains
 *  the code which allows them to run in the Windows environment.
 *
 *  A window is created using d3dmain.res which allows the user to select the
 *  Direct3D driver to use and change the render options.  The D3DApp
 *  collection of functions is used to initialize DirectDraw, Direct3D and
 *  keep surfaces and D3D devices available for rendering.
 *
 *  Frame rate and a screen mode information buffer is Blt'ed to the screen
 *  by functions in stats.cpp.
 *
 *  Each sample is executed through the functions: InitScene, InitView,
 *  RenderScene, ReleaseView, ReleaseScene and OverrideDefaults, as described
 *  in d3ddemo.h.  Samples can also read mouse and keyboard input via
 *  SetMouseCallback and SetKeyboardCallback.
 */ 

#include "d3dmain.h"
#include "main.h"
#include "ddutil.h"
#include "d3dappi.h"
/*
 * GLOBAL VARIABLES
 */
 
// These surfaces are used as the destination for the SW rendering
LPDIRECTDRAW					  lpDD_SW                 = NULL;
HANDLE SW_Win;


int compositeMethod  =  TEXTUREMAP_COMPOSITE ;

// Texture generated from the SW buffer after each frame 
    
    LPDIRECT3DTEXTURE           lpActiveTexture = NULL;
	LPDIRECTDRAWSURFACE     lpDDTextureSurfaceSPOT  = NULL;
    LPDIRECT3DTEXTURE           lpSPOTTexture = NULL;
    D3DTEXTUREHANDLE            SPOTTextureHandle;

	LPDIRECTDRAWSURFACE  lpDDActiveSurfaceSystem  = NULL;
	LPDIRECTDRAWSURFACE  lpDDDevelopSurfaceSystem = NULL;
	LPDIRECTDRAWSURFACE  lpDDTempSurfaceSystem    = NULL;
	LPDIRECTDRAWSURFACE  lpDDSurfaceSystem        = NULL;
    
	LPDIRECTDRAWSURFACE  lpDDZBufferSystem        = NULL;


HANDLE eventHandle;
HANDLE compositeHandle;
HANDLE th;
DWORD dwThreadId;
int countHW=0, countSW=0;



D3DAppInfo* d3dapp;	    /* Pointer to read only collection of DD and D3D
			       objects maintained by D3DApp */
d3dmainglobals myglobs;	    /* collection of global variables */

BOOL(*MouseHandler)(UINT, WPARAM, LPARAM);    /* sample's function which traps
						 mouse input */
BOOL(*KeyboardHandler)(UINT, WPARAM, LPARAM); /* sample's functions which traps
                                                 keyboard input */

/*
 *  INTERNAL FUNCTION PROTOTYPES
 */
static BOOL AppInit(HINSTANCE hInstance, LPSTR lpCmdLine);
static BOOL CreateD3DApp(LPSTR lpCmdLine);
static BOOL BeforeDeviceDestroyed(LPVOID lpContext);
static BOOL AfterDeviceCreated(int w, int h, LPDIRECT3DVIEWPORT* lpViewport,
			       LPVOID lpContext);
void CleanUpAndPostQuit(void);
static void InitGlobals(void);
static BOOL AppPause(BOOL f);
void ReportD3DAppError(void);
static BOOL RenderLoop(void);
static BOOL RestoreSurfaces();
long FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam,
			   LPARAM lParam );

/****************************************************************************/
/*                            WinMain                                       */
/****************************************************************************/
/*
 * Initializes the application then enters a message loop which calls sample's
 * RenderScene until a quit message is received.
 */
int PASCAL
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
    int failcount = 0; /* number of times RenderLoop has failed */
    MSG	msg;
    HACCEL hAccelApp;

    hPrevInstance;
    /*
     * Create the window and initialize all objects needed to begin rendering
     */
    if(!AppInit(hInstance, lpCmdLine))
    	return FALSE;
    hAccelApp = LoadAccelerators(hInstance, "AppAccel");    

    while (!myglobs.bQuit) {
	/* 
	 * Monitor the message queue until there are no pressing
	 * messages
	 */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
		CleanUpAndPostQuit();
                break;
	    }
            if (!myglobs.hWndMain || !TranslateAccelerator(myglobs.hWndMain,
							   hAccelApp, &msg)) {
                TranslateMessage(&msg); 
                DispatchMessage(&msg);
            }
	/* 
	 * If the app is not minimized, not about to quit, not paused, either the
	 * active fullscreen app or in a window and D3D has been initialized,
	 * we can render
	 */
        } else if (d3dapp->bRenderingIsOK && !d3dapp->bMinimized 
		   && !d3dapp->bPaused && !myglobs.bQuit
		   && (d3dapp->bAppActive || !d3dapp->bFullscreen)) {
	    /*
	     * If were are not in single step mode or if we are and the
	     * bDrawAFrame flag is set, render one frame
	     */
            if (!(myglobs.bSingleStepMode && !myglobs.bDrawAFrame)) {
		/* 
		 * Attempt to render a frame, if it fails, take a note.  If
		 * rendering fails more than twice, abort execution.
		 */
		if (!RenderLoop()) {
		    ++failcount;
		    if (failcount == 3) {
			Msg("Rendering has failed too many times.  Aborting execution.\n");
			CleanUpAndPostQuit();
			break;
		    }
		}
	    }
	    /*
	     * Reset the bDrawAFrame flag if we are in single step mode
	     */
            if (myglobs.bSingleStepMode)
                myglobs.bDrawAFrame = FALSE;
        } else {
            WaitMessage();
	}
    }

    DestroyWindow(myglobs.hWndMain);
    return msg.wParam;
}

/**********************************************************************************/
/* This is a separate thread and continues in an infinite loop until process is ready
/* to exit (and will be killed by the main thread .
/* For each frame the following operations are executed :
   1. Clean the Develop and the Z-buffer (using DDraw Blt() )
   2. render the SW scene to the develop buffer
   3. synchornize with the HW thread :
      a:  SIGNAL that SW is ready
	  b:  WAIT   for Composite event  (The HW actually only switched the buffers now)
	  c:  RESET  Composite event 
*/
/**********************************************************************************/
void DrawFrameSW(void)
{
  HRESULT ddrval;
  BOOL bResult;
  DDBLTFX ddbltfx;
 
 while (TRUE){
	
	// Erase the system memory buffer
	memset(&ddbltfx, 0, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillColor = 0;
    if(compositeMethod  ==  TEXTUREMAP_COMPOSITE )
	   ddrval = lpDDDevelopSurfaceSystem->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
    else	// compositeMethod  == BLT_COMPOSITE
	   ddrval = lpDDSurfaceSystem->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
       if (ddrval != DD_OK)
       Msg("Clearing the SW Develop buffer failed.\0", ddrval);
    // Erase the associated Z-buffer
	memset(&ddbltfx, 0, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillDepth = 0x7fff;
    ddrval = lpDDZBufferSystem->Blt(NULL, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &ddbltfx);
     if (ddrval != DD_OK)
       Msg("Clearing the SW Zbuffer failed.\0", ddrval);
    
		// Start to render the SW scene
	countSW++;

	// render the SW scene
    if(compositeMethod  ==  TEXTUREMAP_COMPOSITE )
	  bResult = RenderSWScene(lpDDDevelopSurfaceSystem );  
    else	// compositeMethod  == BLT_COMPOSITE
	  bResult = RenderSWScene(lpDDSurfaceSystem);  
    if (bResult == FALSE)
	   Msg("Call to Software Render  Scene failed.\0", 0);
     // SIGNAL that SW is ready
	 SetEvent(eventHandle); 
     // Suspend SW thread - until SW output is consumed into composite 
	 // the 'consuming' is dependent of the Blit method .
	 WaitForSingleObject(compositeHandle,1000);
	 ResetEvent(compositeHandle);

 };  // loop indefinitely

}

/****************************************************************************/
/*             D3DApp Initialization and callback functions                 */
/****************************************************************************/
/*
 * AppInit
 * Creates the window and initializes all objects necessary to begin rendering
 */
static BOOL
AppInit(HINSTANCE hInstance, LPSTR lpCmdLine)
{
    WNDCLASS wc;
    HMENU hmenu;
    int i;

    /*
     * Initialize the global variables
     */
    InitGlobals();
    myglobs.hInstApp = hInstance;
    myglobs.lpCmdLine = lpCmdLine;

    /*
     * Enumerate the DD drivers
     */
    if (!D3DAppEnumerateDDDevices(&myglobs.NumDDDrivers, &myglobs.DDDriver[0])) {
	ReportD3DAppError();
	return FALSE;
    }
    /*
     * Choose the last device enumerated which will probably be secondary 3d hardware.
     */
    myglobs.CurrDDDriver = myglobs.NumDDDrivers - 1;

    /*
     * Register the window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, "AppIcon");
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = "AppMenu";
    wc.lpszClassName = "Example";
    if (!RegisterClass(&wc))
        return FALSE;
    /*
     * Create a window with some default settings that may change
     */
    myglobs.hWndMain = CreateWindowEx(
         WS_EX_APPWINDOW,
	 "Example",
	 "D3D Example",
	 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
	 WS_THICKFRAME | WS_MINIMIZEBOX,
         CW_USEDEFAULT, CW_USEDEFAULT,
	 START_WIN_SIZE, START_WIN_SIZE,
         NULL,				    /* parent window */
	 NULL,				    /* menu handle */
	 hInstance,			    /* program handle */
	 NULL);				    /* create parms */	
    if (!myglobs.hWndMain){
    	Msg("CreateWindowEx failed");
    	return FALSE;
    }
    /*
     * Display the window
     */
    ShowWindow(myglobs.hWndMain, SW_SHOWNORMAL);
    UpdateWindow(myglobs.hWndMain);

#if 0
	// DEBUG - helps with fullscreen exclusive debugs!
	ShowWindow(myglobs.hWndMain, SW_HIDE);
#endif


#if 1   // I don't want users to be able to change drivers
    /*
     * Add the list of DD drivers D3DApp found to the file menu
     */
    for (i = 0; i < myglobs.NumDDDrivers; i++) {
        InsertMenu(hmenu,  6 + i, MF_BYPOSITION | MF_STRING,
		   MENU_FIRST_DDDRIVER + i, myglobs.DDDriver[i].Name);
    }
#endif
    /* 
     * Have the example initialize it components which remain constant
     * throughout execution
     */
    if (!InitScene())
        return FALSE;



    /*
     * Call D3DApp to initialize all DD and D3D objects necessary to render.
     * D3DApp will call the device creation callback which will initialize the
     * viewport and the sample's execute buffers.
     */
    if (!CreateD3DApp(lpCmdLine))
	return FALSE;

#if 0
	// DEBUG - helps with fullscreen exclusive debugs!
	ShowWindow(myglobs.hWndMain, SW_HIDE);
#endif

	// Initialize theSW stuff NOW
	{
    HRESULT ddrval;
	DDSURFACEDESC ddsd;
    char *error_str;

	ddrval = DirectDrawCreate(NULL, &lpDD_SW, NULL);
    if (ddrval != DD_OK)
    {
      error_str = D3DAppErrorToString(ddrval);
      Msg(error_str, ddrval);
      return(FALSE);
    }
   	// Set the cooperation level to normal for HEL
	// (second window is not necessary and part of the development legacy)
	
	ddrval = lpDD_SW->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (ddrval != DD_OK)
	{
	  error_str = D3DAppErrorToString(ddrval);
      Msg(error_str, ddrval);
	  return(FALSE);
	}

#define sizeX	256
#define sizeY	256
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize  = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH ;
	ddsd.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY; 
    ddsd.dwWidth  = sizeX;
	ddsd.dwHeight = sizeY;
	 //active and develop buffers for the texture map composite method
	ddrval = lpDD_SW->CreateSurface(&ddsd,  &lpDDActiveSurfaceSystem, NULL);
    ddrval = lpDD_SW->CreateSurface(&ddsd,  &lpDDDevelopSurfaceSystem, NULL);
	// single system buf for blt coposite method
	ddrval = lpDD_SW->CreateSurface(&ddsd,  &lpDDSurfaceSystem, NULL);
    if (ddrval != DD_OK)
	{
        error_str = D3DAppErrorToString(ddrval);
        Msg(error_str, ddrval);
	  if (ddrval == DDERR_OUTOFMEMORY || ddrval == DDERR_OUTOFVIDEOMEMORY)
	  {
        Msg("Not enough memory to create the system surface (SW).\0", ddrval);
		return(FALSE);
      }
	  else
	  {
		Msg("CreateSurface for system surface failed (SW).\0", ddrval);
		return(FALSE);
      }
    }
	// set the color key of this surface to black for the Blits
	myDDSetColorKey(lpDDSurfaceSystem, RGB(0,0,0));

	// create the Z buffer for the SW thread
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize  = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth  = sizeX;
	ddsd.dwHeight = sizeY;
    ddsd.dwZBufferBitDepth = 16;

    ddrval = lpDD_SW->CreateSurface(&ddsd, &lpDDZBufferSystem, NULL);
    if (ddrval != DD_OK)
	{
      if (ddrval == DDERR_OUTOFMEMORY || ddrval == DDERR_OUTOFVIDEOMEMORY)
	  {
        Msg("Not enough memory to create the Z-Buffer (SW).\0", ddrval);
		return(FALSE);
      }
	  else
	  {
		Msg("CreateSurface for Z-Buffer failed (SW).\0", ddrval);
		return(FALSE);
      }
    }
	/* 
	    Now we create the surface who'll hold the texture generated 
	     by the SW thread in Frame N-1, this texture will be used in 
		 Frame N to render two triangles by the HW thread.
    */
    ddrval      = createSwTexture();
	if (ddrval != TRUE) return(FALSE);
	// set color key of buffer in video memory to black for
	// transparent texture mapping
    myDDSetColorKey(lpDDTextureSurfaceSPOT, RGB(0,0,0));  
	
    // init the SW scene 
	ddrval = InitSwScene();
	if (ddrval != TRUE)
	{
	  Msg("init of SW data struct failed.\0", ddrval);
	  return(FALSE);
	}
  } // end of SW rendering initialization
    
	// Create SW Thread
	th = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)DrawFrameSW,
		NULL, CREATE_SUSPENDED, &dwThreadId);
	SetThreadPriority(th, THREAD_PRIORITY_HIGHEST);
	eventHandle = CreateEvent(NULL, TRUE, FALSE, "SW is Ready\0");
    // setup synch events and resume SW thread
	compositeHandle = CreateEvent(NULL, TRUE, FALSE, "Composite is Ready\0");
	ResumeThread(th); // Start SW Thread going...
	return TRUE;
}

/*
 * CreateD3DApp
 * Create all DirectDraw and Direct3D objects necessary to begin rendering.
 * Add the list of D3D drivers to the file menu.
 */
static BOOL
CreateD3DApp(LPSTR lpCmdLine)
{
    HMENU hmenu;
    int i;
    LPSTR option;
    BOOL bOnlySystemMemory, bOnlyEmulation;
    DWORD flags;
    Defaults defaults;

    /*
     * Give D3DApp a list of textures to keep track of.
     */
    D3DAppAddTexture("checker.ppm");
    D3DAppAddTexture("tex2.ppm");
    D3DAppAddTexture("tex7.ppm");
    D3DAppAddTexture("win95.ppm");
    /*
     * Parse the command line in seach of one of the following options:
     *     systemmemory  All surfaces should be created in system memory.
     *                   Hardware DD and D3D devices are disabled, but
     *                   debugging during the Win16 lock becomes possible.
     *     emulation     Do not use hardware DD or D3D devices.
     */
    bOnlySystemMemory = FALSE;
    bOnlyEmulation = FALSE;
#if 0		// I don't want user to be able to change this
    option = strtok(lpCmdLine, " -");
    while(option != NULL )   {
        if (!lstrcmp(option, "systemmemory")) {
            bOnlySystemMemory = TRUE;
        } else if (!lstrcmp(option, "emulation")) {
	    bOnlyEmulation = TRUE;
	} else {
            Msg("Invalid command line options given.\nLegal options: -systemmemory, -emulation\n");
            return FALSE;
        }
        option = strtok(NULL, " -");
    }
#endif
    /*
     * Set the flags to pass to the D3DApp creation based on command line
     */
    flags = ((bOnlySystemMemory) ? D3DAPP_ONLYSYSTEMMEMORY : 0) | 
	    ((bOnlyEmulation) ? (D3DAPP_ONLYD3DEMULATION |
				 D3DAPP_ONLYDDEMULATION) : 0);
    /*
     * Create all the DirectDraw and D3D objects neccesary to render.  The
     * AfterDeviceCreated callback function is called by D3DApp to create the
     * viewport and the example's execute buffers.
     */
    if (!D3DAppCreateFromHWND(flags, myglobs.hWndMain,
	    myglobs.DDDriver[myglobs.CurrDDDriver].bIsPrimary ? NULL : &myglobs.DDDriver[myglobs.CurrDDDriver].Guid,
	    AfterDeviceCreated, NULL, BeforeDeviceDestroyed, NULL, &d3dapp)) {
	ReportD3DAppError();
	return FALSE;
    }
#if 0
	// DEBUG - helps with fullscreen exclusive debugs!
	ShowWindow(myglobs.hWndMain, SW_HIDE);
#endif
    /*
     * Add the the list of display modes D3DApp found to the mode menu
     */
    hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 4);
    for (i = 0; i < d3dapp->NumModes; i++) {
        char ach[80];
        wsprintf(ach,"%dx%dx%d", d3dapp->Mode[i].w, d3dapp->Mode[i].h,
		 d3dapp->Mode[i].bpp);
		if ((d3dapp->Mode[i].w == 640) && (d3dapp->Mode[i].h == 480) && (d3dapp->Mode[i].bpp == 16))
			AppendMenu(hmenu, MF_STRING, MENU_FIRST_MODE+i, ach);
		//else
			//AppendMenu(hmenu, MF_STRING | MF_DISABLED | MF_GRAYED, MENU_FIRST_MODE+i, ach);
    }
    /*
     * Add the list of D3D drivers D3DApp foudn to the file menu
     */
    hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 0);
    for (i = 0; i < d3dapp->NumDrivers; i++) {
		if (strstr(myglobs.DDDriver[i].Name,"HAL") == NULL)
		    InsertMenu(hmenu,  7 + myglobs.NumDDDrivers + i, MF_BYPOSITION | MF_STRING | MF_GRAYED,
			   MENU_FIRST_DRIVER + i, d3dapp->Driver[i].Name);
		else
			InsertMenu(hmenu,  7 + myglobs.NumDDDrivers + i, MF_BYPOSITION | MF_STRING,
			   MENU_FIRST_DRIVER + i, d3dapp->Driver[i].Name);
    }
    /*
     * Allow the sample to override the default render state and other
     * settings
     */
    if (!D3DAppGetRenderState(&defaults.rs)) {
	ReportD3DAppError();
	return FALSE;
    }
    lstrcpy(defaults.Name, "D3D Example");
    defaults.bTexturesDisabled = FALSE;
    defaults.bResizingDisabled = myglobs.bResizingDisabled;
    defaults.bClearsOn = myglobs.bClearsOn;
    OverrideDefaults(&defaults);
    myglobs.bClearsOn = defaults.bClearsOn;
    myglobs.bResizingDisabled = defaults.bResizingDisabled;
    /*
     * Apply any changes to the render state
     */
    memcpy(&myglobs.rstate, &defaults.rs, sizeof(D3DAppRenderState));
    if (!D3DAppSetRenderState(&myglobs.rstate)) {
	ReportD3DAppError();
	return FALSE;
    }
    /*
     * If I should begin with textures off, disable them and re-create
     * the view.
     */
    if (defaults.bTexturesDisabled) {
	if (!D3DAppDisableTextures(defaults.bTexturesDisabled)) {
	    ReportD3DAppError();
	    return FALSE;
	}
	/*
	 * Release all objects (ie execute buffers) created by InitView
	 */
	ReleaseView(d3dapp->lpD3DViewport);
	/*
	 * Create the sample's execute buffers via InitView
	 */
	if (!InitView(d3dapp->lpDD, d3dapp->lpD3D, d3dapp->lpD3DDevice,
		      d3dapp->lpD3DViewport, d3dapp->NumUsableTextures,
		      d3dapp->TextureHandle)) {
	    Msg("InitView failed.\n");
	    CleanUpAndPostQuit();
	    return FALSE;
	}
    }

    SetWindowText(myglobs.hWndMain, defaults.Name);

    return TRUE;
}

/*
 * DestroyD3DApp
 *
 * Destroy D3DApp and changes to menu
 */
static void
DestroyD3DApp(void)
{
    HMENU hmenu;
    int i;

    /*
     * Remove the list of display modes
     */
    hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 4);
    for (i = 0; i < d3dapp->NumModes; i++) {
        DeleteMenu(hmenu, MENU_FIRST_MODE + i, MF_BYCOMMAND);
    }
    /*
     * Remove the list of D3D drivers
     */
    hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 0);
    for (i = 0; i < d3dapp->NumDrivers; i++) {
        DeleteMenu(hmenu, MENU_FIRST_DRIVER + i, MF_BYCOMMAND);
    }

    D3DAppDestroy();
}

/*
 * AfterDeviceCreated
 * D3DApp will call this function immediately after the D3D device has been
 * created (or re-created).  D3DApp expects the D3D viewport to be created and
 * returned.  The sample's execute buffers are also created (or re-created)
 * here.
 */
static BOOL
AfterDeviceCreated(int w, int h, LPDIRECT3DVIEWPORT* lplpViewport, LPVOID lpContext)
{
    HMENU hmenu;
    int i;
    char ach[20];
    LPDIRECT3DVIEWPORT lpD3DViewport;
    HRESULT rval;

    /*
     * Create the D3D viewport object
     */
    rval = d3dapp->lpD3D->CreateViewport(&lpD3DViewport, NULL);
    if (rval != D3D_OK) {
    	Msg("Create D3D viewport failed.\n%s", D3DAppErrorToString(rval));
	CleanUpAndPostQuit();
	return FALSE;
    }
    /*
     * Add the viewport to the D3D device
     */
    rval = d3dapp->lpD3DDevice->AddViewport(lpD3DViewport);
    if (rval != D3D_OK) {
    	Msg("Add D3D viewport failed.\n%s", D3DAppErrorToString(rval));
	CleanUpAndPostQuit();
	return FALSE;
    }
    /*
     * Setup the viewport for a reasonable viewing area
     */
    D3DVIEWPORT viewData;
    memset(&viewData, 0, sizeof(D3DVIEWPORT));
    viewData.dwSize = sizeof(D3DVIEWPORT);
    viewData.dwX = viewData.dwY = 0;
    viewData.dwWidth = w;
    viewData.dwHeight = h;
    viewData.dvScaleX = viewData.dwWidth / (float)2.0;
    viewData.dvScaleY = viewData.dwHeight / (float)2.0;
    viewData.dvMaxX = (float)D3DDivide(D3DVAL(viewData.dwWidth),
				       D3DVAL(2 * viewData.dvScaleX));
    viewData.dvMaxY = (float)D3DDivide(D3DVAL(viewData.dwHeight),
	                               D3DVAL(2 * viewData.dvScaleY));
    rval = lpD3DViewport->SetViewport(&viewData);
    if (rval != D3D_OK) {
    	Msg("SetViewport failed.\n%s", D3DAppErrorToString(rval));
	CleanUpAndPostQuit();
	return FALSE;
    }
    /*
     * Return the viewport to D3DApp so it can use it
     */
    *lplpViewport = lpD3DViewport;
    /*
     * Create the sample's execute buffers via InitView
     */
    if (!InitView(d3dapp->lpDD, d3dapp->lpD3D, d3dapp->lpD3DDevice,
		  lpD3DViewport, d3dapp->NumUsableTextures,
		  d3dapp->TextureHandle)) {
	Msg("InitView failed.\n");
	CleanUpAndPostQuit();
	return FALSE;
    }
    /*
     * Add the list of texture formats found by D3DApp to the texture menu
     */
    hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 3);
    for (i = 0; i < d3dapp->NumTextureFormats; i++) {
	if (d3dapp->TextureFormat[i].bPalettized) {
	    wsprintf(ach, "%d-bit Palettized",
		     d3dapp->TextureFormat[i].IndexBPP);
	} else {
	    wsprintf(ach, "%d%d%d RGB", d3dapp->TextureFormat[i].RedBPP,
		     d3dapp->TextureFormat[i].GreenBPP,
		     d3dapp->TextureFormat[i].BlueBPP);
	}
        AppendMenu(hmenu, MF_STRING, MENU_FIRST_FORMAT + i, ach);
    }
    /*
     * Create and initialize the surfaces containing the frame rate and
     * window information
     */
    InitFontAndTextBuffers();

    return TRUE;
}

/*
 * BeforeDeviceDestroyed
 * D3DApp will call this function before the current D3D device is destroyed
 * to give the app the opportunity to destroy objects it has created with the
 * DD or D3D objects.
 */
static BOOL
BeforeDeviceDestroyed(LPVOID lpContext)
{
    HMENU hmenu;
    int i;
    /*
     * Release the font object and buffers containing stats
     */
    ReleaseFontAndTextBuffers();

    if (d3dapp && d3dapp->lpD3DViewport) {
	/*
	 * Release all objects (ie execute buffers) created by InitView
	 */
	ReleaseView(d3dapp->lpD3DViewport);
	/*
	 * Since we created the viewport it is our responsibility to release
	 * it.  Use D3DApp's pointer to it since we didn't save one.
	 */
	d3dapp->lpD3DViewport->Release();
	d3dapp->lpD3DViewport = NULL;
    }
    /*
     * Delete the list of texture formats from the texture menu because
     * they are likely to change
     */
    if (myglobs.hWndMain) {
	hmenu = GetSubMenu(GetMenu(myglobs.hWndMain), 3);
	if (hmenu) {
	    for (i = 0; i < d3dapp->NumTextureFormats; i++) {
		DeleteMenu(hmenu, MENU_FIRST_FORMAT + i, MF_BYCOMMAND);
	    }
	}
    }
    return TRUE;
}

/****************************************************************************/
/*                            Rendering loop                                */
/****************************************************************************/
/*
 * RenderLoop
 * Render the next frame and update the window
 */
static BOOL
RenderLoop()
{
    D3DRECT extents[D3DAPP_MAXCLEARRECTS];
    int count;
    RECT dRect, sRect;
    HRESULT ddrval;  

	/*
     * If all the DD and D3D objects have been initialized we can render
     */
    if (d3dapp->bRenderingIsOK) {
	/*
	 * Restore any lost surfaces
	 */
        if (!RestoreSurfaces()) {
	    /*
	     * Restoring surfaces sometimes fails because the surfaces cannot
	     * yet be restored.  If this is the case, the error will show up
	     * somewhere else and we should return success here to prevent
	     * unnecessary error's being reported.
	     */
	    return TRUE;
	}
	/*
	 * Calculate the frame rate
	 */
        if (!CalculateFrameRate())
	    return FALSE;

        /*
         * Clear the back buffer and Z buffer if enabled.  If bResized is set,
	 * clear the entire window.
         */

		if (myglobs.bClearsOn) {
	    if (!D3DAppClearBackBuffer(myglobs.bResized ?
				      D3DAPP_CLEARALL : NULL)) {
		ReportD3DAppError();
		return FALSE;
	    }
	}
	/*
	 * Call the sample's RenderScene to render this frame
	 */
		if (!RenderScene(d3dapp->lpD3DDevice, d3dapp->lpD3DViewport,
			     &extents[0])) {
		Msg("RenderScene failed.\n");
		return FALSE;
	    }
	   count = 1;

#ifdef SYNCH_WITH_SW
		  countHW++;  // just for synchronization debugging
#endif		
 
#ifdef DEBUG_SYNCH_WITH_SW  // debug - synch
      //test to make sure counters are same for HW & SW
if (countSW != countHW)
  {
	  Msg("countSW != countHW !\0",0);
	  __asm int 3;
  }
#endif
	/*
	 * Blt the frame rate and window stat text to the back buffer
	 */
        if (!DisplayFrameRate(&count, &extents[1]))
	    return FALSE;
	/*
	 * Give D3DApp the extents so it can keep track of dirty sections of
	 * the back and front buffers
	 */
	if (!D3DAppRenderExtents(count, extents, myglobs.bResized ?
				 D3DAPP_CLEARALL : NULL)) {
	    ReportD3DAppError();
	    return FALSE;
	}

   if	(compositeMethod  == BLT_COMPOSITE) {
    // Insert composite step here!!! and include synch events
	if (d3dappi.bFullscreen)
    {
	SetRect(&dRect, 200, 180, 200+255,180+255);
    SetRect(&sRect, 0, 0, 255, 255);
	} 
	else
    {
	SetRect(&dRect, 50, 30, 180, 150);
    SetRect(&sRect, 0, 0, 255, 255);
	}
    ddrval = d3dappi.lpBackBuffer->Blt(&dRect, lpDDSurfaceSystem, &sRect,
																DDBLT_KEYSRC | DDBLT_ASYNC, NULL);  
    if (ddrval != D3D_OK)
    {
	  Msg("BLT Failed.\0", ddrval);
      return(FALSE);
     }

  // Resume the SW Thread - that is, the main thread has consumed the
  // output of the SW rendering.  Use of double buffering would eliminate
  // this and then allow SW to go ahead to the next frame early
	SetEvent(compositeHandle);
  }
    /*
	 * Blt or flip the back buffer to the front buffer.  Don't report an
	 * error if this fails.
	 */
	D3DAppShowBackBuffer(myglobs.bResized ? D3DAPP_SHOWALL : NULL);
	/*
	 * Reset the resize flag
	 */
        myglobs.bResized = FALSE;
    }
    return TRUE;
}






/*
 * AppPause
 * Pause and unpause the application
 */
static BOOL
AppPause(BOOL f)
{
    /*
     * Flip to the GDI surface and halt rendering
     */
    if (!D3DAppPause(f))
	return FALSE;
    /*
     * When returning from a pause, reset the frame rate count
     */
    if (!f) {
        ResetFrameRate();
    }
    return TRUE;
}

/*
 * RestoreSurfaces
 * Restores any lost surfaces.  Returns TRUE if all surfaces are not lost and
 * FALSE if one or more surfaces is lost and can not be restored at the
 * moment.
 */
static BOOL
RestoreSurfaces()
{
    HRESULT d3drval;

    /*
     * Have D3DApp check all the surfaces it's in charge of
     */
    if (!D3DAppCheckForLostSurfaces()) {
            return FALSE;
    }
    /*
     * Check frame rate and info surfaces and re-write them if they
     * were lost.
     */
    if (myglobs.lpFrameRateBuffer->IsLost() == DDERR_SURFACELOST) {
        d3drval = myglobs.lpFrameRateBuffer->Restore();
        if (d3drval != DD_OK) {
            return FALSE;
        }
	if (!WriteFrameRateBuffer(0.0f, 0))
	    return FALSE;
    }
    if (myglobs.lpInfoBuffer->IsLost() == DDERR_SURFACELOST) {
        d3drval = myglobs.lpInfoBuffer->Restore();
        if (d3drval != DD_OK) {
            return FALSE;
        }
	if (!WriteInfoBuffer())
	    return FALSE;
    }
    return TRUE;
}


/*************************************************************************
  Windows message handlers
 *************************************************************************/
/*
 * AppAbout
 * About box message handler
 */
BOOL
FAR PASCAL AppAbout(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK)
        EndDialog(hwnd, TRUE);
      break;

    case WM_INITDIALOG:
      return TRUE;
  }
  return FALSE;
}

/*
 * WindowProc
 * Main window message handler.
 */
long
FAR PASCAL WindowProc(HWND hWnd, UINT message, WPARAM wParam,
			   LPARAM lParam )
{
    int i;
    BOOL bStop;
    LRESULT lresult;

    /*
     * Give D3DApp an opportunity to process any messages it MUST see in order
     * to perform it's function.
     */
    if (!D3DAppWindowProc(&bStop, &lresult, hWnd, message, wParam, lParam)) {
	ReportD3DAppError();
	CleanUpAndPostQuit();
	return 0;
    }

    /*
     * Prevent resizing through this message
     */
    if (message == WM_GETMINMAXINFO && myglobs.bResizingDisabled && !d3dapp->bFullscreen && !d3dapp->bMinimized) {
	RECT rc;
	GetWindowRect(hWnd, &rc);
	((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = START_WIN_SIZE;
	((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = START_WIN_SIZE;
	((LPMINMAXINFO)lParam)->ptMinTrackSize.x = START_WIN_SIZE;
	((LPMINMAXINFO)lParam)->ptMinTrackSize.y = START_WIN_SIZE;
        return 0;
    }

    /* 
     * If bStop is set by D3DApp, the app should not process the message but
     * return lresult.
     */
    if (bStop)
	return lresult;

    if (!d3dapp || !d3dapp->bRenderingIsOK) {
	return DefWindowProc(hWnd, message, wParam, lParam);
    }

    switch( message ) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
	    /*
	     * Call the sample's MouseHandler if available
	     */
            if (!MouseHandler)
                break;
            if ((MouseHandler)(message, wParam, lParam))
                return 1;
            break;
        case WM_KEYDOWN:
	    /*
	     * Call the sample's keyboard handler if available
	     */
            if (!KeyboardHandler)
                break;
            if ((KeyboardHandler)(message, wParam, lParam))
                return 1;
            break;
	/*
	 * Pause and unpause the app when entering/leaving the menu
	 */
        case WM_ENTERMENULOOP:
            AppPause(TRUE);
            break;
        case WM_EXITMENULOOP:
            AppPause(FALSE);
            break;
    	case WM_DESTROY:
            myglobs.hWndMain = NULL;
            CleanUpAndPostQuit();
            break;
        case WM_INITMENUPOPUP:
	    /*
             * Check and enable the appropriate menu items
	     */
	    if (d3dapp->ThisDriver.bDoesZBuffer) {
		EnableMenuItem((HMENU)wParam, MENU_ZBUFFER, MF_ENABLED);
		CheckMenuItem((HMENU)wParam, MENU_ZBUFFER, myglobs.rstate.bZBufferOn ? MF_CHECKED : MF_UNCHECKED);
	    } else {
		EnableMenuItem((HMENU)wParam, MENU_ZBUFFER, MF_GRAYED);
		CheckMenuItem((HMENU)wParam, MENU_ZBUFFER, MF_UNCHECKED);
	    }
            CheckMenuItem((HMENU)wParam, MENU_STEP, (myglobs.bSingleStepMode) ? MF_CHECKED : MF_UNCHECKED);
            EnableMenuItem((HMENU)wParam, MENU_GO, (myglobs.bSingleStepMode) ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem((HMENU)wParam, MENU_FLAT, (myglobs.rstate.ShadeMode == D3DSHADE_FLAT) ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_GOURAUD, (myglobs.rstate.ShadeMode == D3DSHADE_GOURAUD) ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_PHONG, (myglobs.rstate.ShadeMode == D3DSHADE_PHONG) ? MF_CHECKED : MF_UNCHECKED);
	    EnableMenuItem((HMENU)wParam, MENU_PHONG, MF_GRAYED);
            CheckMenuItem((HMENU)wParam, MENU_CLEARS, myglobs.bClearsOn ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_POINT, (myglobs.rstate.FillMode == D3DFILL_POINT) ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_WIREFRAME, (myglobs.rstate.FillMode == D3DFILL_WIREFRAME) ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_SOLID, (myglobs.rstate.FillMode == D3DFILL_SOLID) ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_DITHERING, myglobs.rstate.bDithering ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_SPECULAR, myglobs.rstate.bSpecular ? MF_CHECKED : MF_UNCHECKED);
            EnableMenuItem((HMENU)wParam, MENU_SPECULAR, MF_ENABLED);
            CheckMenuItem((HMENU)wParam, MENU_FOG, myglobs.rstate.bFogEnabled ? MF_CHECKED : MF_UNCHECKED);
            CheckMenuItem((HMENU)wParam, MENU_ANTIALIAS, myglobs.rstate.bAntialiasing ? MF_CHECKED : MF_UNCHECKED);
	    if (d3dapp->ThisDriver.bDoesTextures) {
		CheckMenuItem((HMENU)wParam, MENU_TEXTURE_TOGGLE, (!d3dapp->bTexturesDisabled) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_TEXTURE_TOGGLE, MF_ENABLED);
		EnableMenuItem((HMENU)wParam, MENU_TEXTURE_SWAP, (d3dapp->bTexturesDisabled) ? MF_GRAYED : MF_ENABLED);
		CheckMenuItem((HMENU)wParam, MENU_PERSPCORRECT, myglobs.rstate.bPerspCorrect ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_PERSPCORRECT, (d3dapp->bTexturesDisabled) ? MF_GRAYED : MF_ENABLED);
		CheckMenuItem((HMENU)wParam, MENU_POINT_FILTER, (myglobs.rstate.TextureFilter == D3DFILTER_NEAREST) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_POINT_FILTER, (d3dapp->bTexturesDisabled) ? MF_GRAYED : MF_ENABLED);
		CheckMenuItem((HMENU)wParam, MENU_LINEAR_FILTER, (myglobs.rstate.TextureFilter == D3DFILTER_LINEAR) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_LINEAR_FILTER, (d3dapp->bTexturesDisabled) ? MF_GRAYED : MF_ENABLED);
		for (i = 0; i < d3dapp->NumTextureFormats; i++) {
		    CheckMenuItem((HMENU)wParam, MENU_FIRST_FORMAT + i, (i == d3dapp->CurrTextureFormat) ? MF_CHECKED : MF_UNCHECKED);
		    EnableMenuItem((HMENU)wParam, MENU_FIRST_FORMAT + i, (d3dapp->bTexturesDisabled) ? MF_GRAYED : MF_ENABLED);
		}
	    } else {
		EnableMenuItem((HMENU)wParam, MENU_TEXTURE_SWAP, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_TEXTURE_TOGGLE, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_POINT_FILTER, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_LINEAR_FILTER, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_PERSPCORRECT, MF_GRAYED);
	    }
	    if (d3dapp->bIsPrimary) {
		CheckMenuItem((HMENU)wParam, MENU_FULLSCREEN, d3dapp->bFullscreen ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_FULLSCREEN, d3dapp->bFullscreen && !d3dapp->ThisDriver.bCanDoWindow ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem((HMENU)wParam, MENU_NEXT_MODE, (!d3dapp->bFullscreen) ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem((HMENU)wParam, MENU_PREVIOUS_MODE, (!d3dapp->bFullscreen) ? MF_GRAYED : MF_ENABLED);
	    } else {
		EnableMenuItem((HMENU)wParam, MENU_FULLSCREEN, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_NEXT_MODE, MF_GRAYED);
		EnableMenuItem((HMENU)wParam, MENU_PREVIOUS_MODE, MF_GRAYED);
	    }
	    for (i = 0; i < d3dapp->NumModes; i++) {
		CheckMenuItem((HMENU)wParam, MENU_FIRST_MODE + i, (i == d3dapp->CurrMode) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem((HMENU)wParam, MENU_FIRST_MODE + i, (d3dapp->Mode[i].bThisDriverCanDo) ? MF_ENABLED : MF_GRAYED);
	    }
            for (i = 0; i < d3dapp->NumDrivers; i++) {
                CheckMenuItem((HMENU)wParam, MENU_FIRST_DRIVER + i, (i == d3dapp->CurrDriver) ? MF_CHECKED : MF_UNCHECKED);
            }
            for (i = 0; i < myglobs.NumDDDrivers; i++) {
                CheckMenuItem((HMENU)wParam, MENU_FIRST_DDDRIVER + i, (i == myglobs.CurrDDDriver) ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
	case WM_GETMINMAXINFO:
	    /*
	     * Some samples don't like being resized, such as those which use
	     * screen coordinates (TLVERTEXs).
	     */
	    if (myglobs.bResizingDisabled) {
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = START_WIN_SIZE;
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = START_WIN_SIZE;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = START_WIN_SIZE;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = START_WIN_SIZE;
		return 0;
	    }
	    break;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case MENU_ABOUT:
                    AppPause(TRUE);
                    DialogBox(myglobs.hInstApp, "AppAbout", myglobs.hWndMain, (DLGPROC)AppAbout);
                    AppPause(FALSE);
                    break;
                case MENU_EXIT:
	            CleanUpAndPostQuit();
        	    break;
                case MENU_STEP:
		    /*
		     * Begin single step more or draw a frame if in single
		     * step mode
		     */
                    if (!myglobs.bSingleStepMode) {
                        myglobs.bSingleStepMode = TRUE;
                        myglobs.bDrawAFrame = TRUE;
                    } else if (!myglobs.bDrawAFrame) {
                        myglobs.bDrawAFrame = TRUE;
                    }
                    break;
                case MENU_GO:
		    /*
		     * Exit single step mode
		     */
                    if (myglobs.bSingleStepMode) {
                        myglobs.bSingleStepMode = FALSE;
                        ResetFrameRate();
                    }
                    break;
                case MENU_STATS:
		    /*
		     * Toggle output of frame rate and window info
		     */
                    if ((myglobs.bShowFrameRate) && (myglobs.bShowInfo)) {
                        myglobs.bShowFrameRate = FALSE;
                        myglobs.bShowInfo = FALSE;
                        break;
                    }
                    if ((!myglobs.bShowFrameRate) && (!myglobs.bShowInfo)) {
                        myglobs.bShowFrameRate = TRUE;
                        break;
                    }
                    myglobs.bShowInfo = TRUE;
                    break;
        	case MENU_FULLSCREEN:
		    if (d3dapp->bFullscreen) {
			/*
			 * Return to a windowed mode.  Let D3DApp decide which
			 * D3D driver to use in case this one cannot render to
			 * the Windows display depth
			 */
			if (!D3DAppWindow(D3DAPP_YOUDECIDE, D3DAPP_YOUDECIDE)) {
			    ReportD3DAppError();
			    CleanUpAndPostQuit();
			    break;
			}
		    } else {
			/*
			 * Enter the current fullscreen mode.  D3DApp may
			 * resort to another mode if this driver cannot do
			 * the currently selected mode.
			 */
			if (!D3DAppFullscreen(d3dapp->CurrMode)) {
			    ReportD3DAppError();
			    CleanUpAndPostQuit();
			    break;
			}
		    }
            	    break;
		/*
		 * Texture filter method selection
		 */
                case MENU_POINT_FILTER:
                    if (myglobs.rstate.TextureFilter == D3DFILTER_NEAREST)
                        break;
                    myglobs.rstate.TextureFilter = D3DFILTER_NEAREST;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_LINEAR_FILTER:
                    if (myglobs.rstate.TextureFilter == D3DFILTER_LINEAR)
                        break;
                    myglobs.rstate.TextureFilter = D3DFILTER_LINEAR;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
		/* 
		 * Shading selection
		 */
                case MENU_FLAT:
                    if (myglobs.rstate.ShadeMode == D3DSHADE_FLAT)
                        break;
                    myglobs.rstate.ShadeMode = D3DSHADE_FLAT;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_GOURAUD:
                    if (myglobs.rstate.ShadeMode == D3DSHADE_GOURAUD)
                        break;
                    myglobs.rstate.ShadeMode = D3DSHADE_GOURAUD;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
                case MENU_PHONG:
                    if (myglobs.rstate.ShadeMode == D3DSHADE_PHONG)
                        break;
                    myglobs.rstate.ShadeMode = D3DSHADE_PHONG;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
		/*
		 * Fill mode selection
		 */
                case MENU_POINT:
                    if (myglobs.rstate.FillMode == D3DFILL_POINT)
                        break;
                    myglobs.rstate.FillMode = D3DFILL_POINT;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
                case MENU_WIREFRAME:
                    if (myglobs.rstate.FillMode == D3DFILL_WIREFRAME)
                        break;
                    myglobs.rstate.FillMode = D3DFILL_WIREFRAME;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
                case MENU_SOLID:
                    if (myglobs.rstate.FillMode == D3DFILL_SOLID)
                        break;
                    myglobs.rstate.FillMode = D3DFILL_SOLID;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
                case MENU_PERSPCORRECT:
		    /*
		     * Toggle perspective correction
		     */
                    myglobs.rstate.bPerspCorrect =
			!myglobs.rstate.bPerspCorrect;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
                    break;
                case MENU_CLEARS:
		    /*
		     * Toggle the clearing the the back buffer and Z-buffer
		     * and set the resized flag to clear the entire window
		     */
                    myglobs.bClearsOn = !myglobs.bClearsOn;
                    if (myglobs.bClearsOn)
                        myglobs.bResized = TRUE;
                    break;
                case MENU_ZBUFFER:
		    /*
		     * Toggle the use of a Z-buffer
		     */
                    myglobs.rstate.bZBufferOn = !myglobs.rstate.bZBufferOn;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_DITHERING:
		    /*
		     * Toggle dithering
		     */
                    myglobs.rstate.bDithering = !myglobs.rstate.bDithering;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_SPECULAR:
		    /*
		     * Toggle specular highlights
		     */
                    myglobs.rstate.bSpecular = !myglobs.rstate.bSpecular;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_FOG:
		    /*
		     * Toggle fog
		     */
                    myglobs.rstate.bFogEnabled = !myglobs.rstate.bFogEnabled;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_ANTIALIAS:
		    /*
		     * Toggle anti-aliasing
		     */
                    myglobs.rstate.bAntialiasing =
			!myglobs.rstate.bAntialiasing;
                    if (!D3DAppSetRenderState(&myglobs.rstate)) {
			ReportD3DAppError();
			break;
		    }
		    break;
                case MENU_TEXTURE_TOGGLE:
		    /*
		     * Release the sample's execute buffers, toggle the
		     * texture disabled state and recreate them.
		     */
		    ReleaseView(d3dapp->lpD3DViewport);
		    D3DAppDisableTextures(!d3dapp->bTexturesDisabled);
		    {
			if (!InitView(d3dapp->lpDD, d3dapp->lpD3D,
				      d3dapp->lpD3DDevice,
				      d3dapp->lpD3DViewport,
				      d3dapp->NumUsableTextures,
				      d3dapp->TextureHandle)) {
			    Msg("InitView failed.\n");
			    CleanUpAndPostQuit();
			    break;
			}
		    }
		    myglobs.bResized = TRUE;
		    break;
		case MENU_TEXTURE_SWAP:
		    /*
		     * Swap textures using the load command
		     */
		    if (!D3DAppSwapTextures()) {
			ReportD3DAppError();
			break;
		    }
		    /*
		     * Just in case we have a texture background
		     */
		    myglobs.bResized = TRUE;
		    break;
		
		case  MENU_SW_TOGGLE_LIGHT  :
		case  MENU_SW_COS_POWER       :
		case  MENU_SW_LIGHT_MODE       :
		case  MENU_SW_TOGGLE_LIGHT_EFFECT  :
		case  MENU_SW_TOGGLE_RESOLUTION :
		case  MENU_SW_OCTAVE_NUM  :
		case  MENU_SW_MARBLE_METHOD  :
		case  MENU_SW_TOGGLE_TEXTURE  :
		case  MENU_SW_TOGGLE_ZBUF  :
		case  MENU_SW_SCALE_UP        :
		case  MENU_SW_SCALE_DOWN  :
				if (KeyboardHandler && (KeyboardHandler)(message, wParam, lParam))
							return 1;
				break;
		case MENU_SW_TOGGLE_COMPOSITE  :     
				    compositeMethod = (compositeMethod + 1 ) % 2 ;
			         break;
		case MENU_NEXT_MODE:
		    /*
		     * Enter the next usable fullscreen mode
		     */
		    i = d3dapp->CurrMode;
		    do {
			++i;
			if (i >= d3dapp->NumModes)
			    i = 0;
			if (!d3dapp->Mode[i].bThisDriverCanDo)
			    continue;
			else {
			    if (!D3DAppFullscreen(i)) {
				ReportD3DAppError();
				CleanUpAndPostQuit();
			    }
			    break;
			}
		    } while(i != d3dapp->CurrMode);
		    break;
		case MENU_PREVIOUS_MODE:
		    /*
		     * Enter the previous usable fullscreen mode
		     */
		    i = d3dapp->CurrMode;
		    do {
			--i;
			if (i < 0)
			    i = d3dapp->NumModes - 1;
			if (!d3dapp->Mode[i].bThisDriverCanDo)
			    continue;
			else {
			    if (!D3DAppFullscreen(i)) {
				ReportD3DAppError();
				CleanUpAndPostQuit();
			    }
			    break;
			}
		    } while(i != d3dapp->CurrMode);
                    break;
            }
            if (   LOWORD(wParam) >= MENU_FIRST_FORMAT
		&& LOWORD(wParam) < MENU_FIRST_FORMAT +
		    D3DAPP_MAXTEXTUREFORMATS
		&& d3dapp->CurrTextureFormat !=
		    LOWORD(wParam) - MENU_FIRST_FORMAT) {
		/*
		 * Release the sample's execute buffers, change the texture
		 * format and recreate the view.
		 */
		ReleaseView(d3dapp->lpD3DViewport);
		if (!D3DAppChangeTextureFormat(LOWORD(wParam) -
  					       MENU_FIRST_FORMAT)) {
		    ReportD3DAppError();
		    CleanUpAndPostQuit();
		}
		{
		    if (!InitView(d3dapp->lpDD, d3dapp->lpD3D,
				  d3dapp->lpD3DDevice, d3dapp->lpD3DViewport,
				  d3dapp->NumUsableTextures,
				  d3dapp->TextureHandle)) {
			Msg("InitView failed.\n");
			CleanUpAndPostQuit();
			break;
		    }
		}
                ResetFrameRate();
            }
            if (   LOWORD(wParam) >= MENU_FIRST_DRIVER
		&& LOWORD(wParam) < MENU_FIRST_DRIVER + D3DAPP_MAXD3DDRIVERS
		&& d3dapp->CurrDriver != LOWORD(wParam) - MENU_FIRST_DRIVER) {
		/*
		 * Change the D3D driver
		 */
		if (!D3DAppChangeDriver(LOWORD(wParam) - MENU_FIRST_DRIVER,
					NULL)) {
		    ReportD3DAppError();
		    CleanUpAndPostQuit();
		}
            }
            if (   LOWORD(wParam) >= MENU_FIRST_MODE
		&& LOWORD(wParam) < MENU_FIRST_MODE+100) {
		/*
		 * Switch to the selected fullscreen mode
		 */
		if (!D3DAppFullscreen(LOWORD(wParam) - MENU_FIRST_MODE)) {
		    ReportD3DAppError();
		    CleanUpAndPostQuit();
		}
            }
            if (   LOWORD(wParam) >= MENU_FIRST_DDDRIVER
		&& LOWORD(wParam) < MENU_FIRST_DDDRIVER + D3DAPP_MAXDDDRIVERS
		&& myglobs.CurrDDDriver != LOWORD(wParam) - MENU_FIRST_DDDRIVER) {
		/*
		 * Change the DD driver
		 */
		DestroyD3DApp();
		myglobs.CurrDDDriver = LOWORD(wParam) - MENU_FIRST_DDDRIVER;
		SetWindowPos(myglobs.hWndMain, HWND_NOTOPMOST, 0, 0, START_WIN_SIZE, START_WIN_SIZE, SWP_NOMOVE | SWP_SHOWWINDOW);
		if (!CreateD3DApp(myglobs.lpCmdLine)) {
		    ReportD3DAppError();
		    CleanUpAndPostQuit();
		}
            }
	    /*
	     * Whenever we receive a command in single step mode, draw a frame
	     */
	    if (myglobs.bSingleStepMode)
		myglobs.bDrawAFrame = TRUE;
            return 0L;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


/****************************************************************************/
/*                 Setting up callbacks for examples                        */
/****************************************************************************/
 /*
  * SetMouseCallback
  * Called in an example to set a callback function for all WM messages
  * dealing with the mouse.  The handler should return whether or not
  * it handled the message.
  */
BOOL
SetMouseCallback(BOOL(*Handler)(UINT, WPARAM, LPARAM))
{
    MouseHandler = Handler;
    return TRUE;
}
 
/*
 * SetKeyboardCallback
 * Called in an example to set a callback function for all WM messages
 * dealing with the keyboard. The handler should return whether or not it
 * handled the message.
 */
BOOL
SetKeyboardCallback(BOOL(*Handler)(UINT, WPARAM, LPARAM)) {
    
    KeyboardHandler = Handler;
    return TRUE;
}

/****************************************************************************/
/*          Initialization, error reporting and release functions.          */
/****************************************************************************/
/*
 * InitGlobals
 * Called once at program initialization to initialize global variables.
 */
static void
InitGlobals(void)
{
    d3dapp = NULL;
    memset(&myglobs.rstate, 0, sizeof(myglobs.rstate));
    memset(&myglobs, 0, sizeof(myglobs));
    myglobs.bClearsOn = TRUE;
    myglobs.bShowFrameRate = TRUE;
    myglobs.bShowInfo = TRUE;
    MouseHandler = NULL;
    KeyboardHandler = NULL;
}

/*
 * CleanUpAndPostQuit
 * Release all D3D objects, post a quit message and set the bQuit flag
 */
void
CleanUpAndPostQuit(void)
{
    if (myglobs.bQuit)
	return;
    DestroyD3DApp();
    ReleaseScene();
    myglobs.bQuit = TRUE;
    PostQuitMessage( 0 );
}

/*
 * ReportD3DAppError
 * Reports an error during a d3d app call.
 */
void
ReportD3DAppError(void)
{
    Msg("%s", D3DAppLastErrorString());
}

/* Msg
 * Message output for error notification.
 */
void __cdecl
Msg( LPSTR fmt, ... )
{
    char buff[256];

    wvsprintf(buff, fmt, (char *)(&fmt+1));
    lstrcat(buff, "\r\n");
    AppPause(TRUE);
    if (d3dapp && d3dapp->bFullscreen)
	SetWindowPos(myglobs.hWndMain, HWND_NOTOPMOST, 0, 0, 0, 0,
		     SWP_NOSIZE | SWP_NOMOVE);
    MessageBox( NULL, buff, "D3D Example Message", MB_OK );
    if (d3dapp && d3dapp->bFullscreen)
	SetWindowPos(myglobs.hWndMain, HWND_TOPMOST, 0, 0, 0, 0,
		     SWP_NOSIZE | SWP_NOMOVE);
    AppPause(FALSE);
}
