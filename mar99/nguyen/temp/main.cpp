// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : Main.cpp
// Description : source that contains the WinMain... 
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#define NAME "Sample"
#define TITLE "Sample"

#include "main.hpp"
#include "sample.hpp"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>



HWND					MainHwnd;
RECT					hwndRect;
int						NumEcranVisible;
BOOL					AppActive;




unsigned short *buffer=0;


// gestion E/S
char clavier[256];

typedef struct
{
	long deltax;
	long deltay;
	char move;
}souris;

souris mamouse;

static long speed_turn=30*3;
static long speed_move=60*3;
static float alook=0, awalk=0;
static long usewire=0;
static long usepvs=1;



long timercount=0;

// performance counter
LARGE_INTEGER start,stop;
unsigned long dep,arr,res;
// performance counter

/****************************************************************/
/*																*/
/*																*/
/*																*/
/*																*/
/****************************************************************/



/*__________________________________________________________*/ 
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
void print_text(long x, long y, char* texte)
{
	HDC handle;

	handle=GetDC(MainHwnd);
	TextOut(handle, x, y, texte, strlen(texte));
	ReleaseDC(MainHwnd,handle);  
}






/*__________________________________________________________*/ 
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
void print_infos(unsigned short value)
{
  char texte[80];

  //sprintf(texte,"%s - 1/z:%d      ", ptrnom, value);
  print_text(0,0,texte);
}








/*__________________________________________________________*/ 
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long lRet = 1;
//	long x,y;

    switch( message )
    {

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			break;


		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (wParam==27) 
			{
				PostQuitMessage( 0 );
			}
			clavier[wParam] = 1;

			if (wParam==VK_F1) SetScreenCamera();
	
			if (wParam==VK_F2) SetSpotCamera();

			if (wParam==VK_SPACE) ToggleCapture();
	
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			clavier[wParam] = 0;
			break;

		case WM_DESTROY:
			if (buffer!=NULL) free(buffer);
			PostQuitMessage( 0 );
			break;

		case WM_TIMER:
			timercount++;
			break;

		default:
			lRet = DefWindowProc(hWnd, message, wParam, lParam);
			break;
    }
    return lRet;
}
 /* END WindowProc */





/*__________________________________________________________*/ 
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
// User Function
static BOOL InitMainWindow( HINSTANCE hInstance, int nCmdShow )
{
    WNDCLASS            wc;
 
    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= WindowProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION);
    wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground	= (HBRUSH)GetStockObject( GRAY_BRUSH );
    wc.lpszMenuName		= NULL;
    wc.lpszClassName	= NAME;
    RegisterClass( &wc );
	MainHwnd = CreateWindow(	NAME, 
								TITLE,
								WS_OVERLAPPEDWINDOW,
								0,
								30,
								400, 
								300,
								NULL, NULL,
								hInstance,
								NULL
							 );

    if( MainHwnd==NULL )
    {
        return FALSE;
    }

    ShowWindow( MainHwnd, nCmdShow );
    UpdateWindow( MainHwnd );

    return TRUE;
}







/*__________________________________________________________*/
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
void set_window(short width,short height)
{
  HDC handle;
  
  handle=GetDC(MainHwnd);
  
  SetWindowExtEx(handle,width,height,NULL);

  SetWindowPos(
    MainHwnd,
    HWND_TOP,	// placement-order handle
    0,				// horizontal position
    0,				// vertical position
    width,			// width
    height,			// height
    SWP_DRAWFRAME 	// window-positioning flags
   );

  ReleaseDC(MainHwnd,handle);
}






/*__________________________________________________________*/
/*                                                          */
/*                                                          */
/*                                                          */
/*__________________________________________________________*/
void print_infos(void)
{
/*	print_text(0, 240, "ARROWS to MOVE");
	print_text(0, 255, "Pg UP/Pg DOWN - UP/DOWN");
	print_text(0, 270, "Insert - Wireframe");*/
}



/*__________________________________________________________*/
/*                                                          */
/*__________________________________________________________*/
void print_percent(long p)
{
	char texte[256];

	sprintf(texte, "%d pct", p);
	print_text(0, 250, texte);
}





/*__________________________________________________________*/
/*                                                          */
/*__________________________________________________________*/
void ReadKeyboard(void)
{
	long value=20;

	if (clavier[VK_SHIFT]==1)
	{
		if (clavier[VK_LEFT] ==1)	MeshControl(Mesh::XLESS, (float) value);
		if (clavier[VK_RIGHT]==1)	MeshControl(Mesh::XPLUS, (float)value);
		if (clavier[VK_PRIOR]==1)	MeshControl(Mesh::YPLUS, (float)value);
		if (clavier[VK_NEXT] ==1)	MeshControl(Mesh::YLESS, (float)value);
		if (clavier[VK_UP]  ==1)	MeshControl(Mesh::ZPLUS, (float)value);
		if (clavier[VK_DOWN]==1)	MeshControl(Mesh::ZLESS, (float)value);
	}
	if (clavier[VK_CONTROL]==1)
	{
		if (clavier[VK_LEFT] ==1)	CameraControl(Camera::LEFT, value);
		if (clavier[VK_RIGHT]==1)	CameraControl(Camera::RIGHT, value);
		if (clavier[VK_PRIOR]==1)	CameraControl(Camera::DOWN, value);
		if (clavier[VK_NEXT] ==1)	CameraControl(Camera::UP, value);
		if (clavier[VK_UP]  ==1)	CameraControl(Camera::FORWARD, value);
		if (clavier[VK_DOWN]==1)	CameraControl(Camera::BACKWARD, value);
		if (clavier[VK_END] ==1)	CameraControl(Camera::ROLL_RIGHT, value);
		if (clavier[VK_HOME]==1)	CameraControl(Camera::ROLL_LEFT, value);
	}
}





/*__________________________________________________________*/
/*                                                          */
/* WinMain - initialisation, boucle principale des messages */
/*                                                          */
/*__________________________________________________________*/
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR lpCmdLine, int nCmdShow)
{  
	MSG         msg;

    lpCmdLine = lpCmdLine;
    hPrevInstance = hPrevInstance;
	if (hPrevInstance!=NULL)
	{
		exit(0);
	}

	if( !InitMainWindow( hInstance, nCmdShow ) )
    {
        return FALSE;
    }

	InitScene();

	while(1)
	{
		ReadKeyboard();

		InnerLoop();

		while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage (&msg, NULL, 0, 0))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
			else
			{
				goto fin;
			}
		}
	}
fin:
    return msg.wParam;
}