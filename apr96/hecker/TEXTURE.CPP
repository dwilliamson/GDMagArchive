/*----------------------------------------------------------------------------

Texture Test Program - a cheesy test harness for texture mapping

by Chris Hecker (checker@netcom.com) for my Game Developer Magazine articles
See files.txt for back-issue info.

NOTE: This is a hacked test program, not a nice example of Windows programming.
The texture mappers are the only part of this you should look at.

*/

/*

This material is Copyright 1995, 1996 Chris Hecker, All Rights Reserved.
It's for you to read and learn from, not to put in your own articles
or books or on your website, etc.  Thank you.

Chris Hecker
checker@netcom.com

*/

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <stdlib.h>
#include <strstrea.h>
#include <fstream.h>
#include <math.h>
#include <assert.h>

#include "texture.h"
#include "dumb3d.hpp"
#include "mappers.h"

#pragma warning (disable:4244) // conversion from float to int

/*----------------------------------------------------------------------------

Globals and declarations

*/

char szAppName[200] = "Cheesy Texture Test - ";
char *pMapperType;

HINSTANCE hInstApp;
HWND	  hwndApp;
HPALETTE  hpalApp;

LONG FAR PASCAL AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LONG AppCommand (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void AppExit(void);
BOOL AppIdle(void);

void TestMapper( void );
void Draw5000PixelTriangle( void );

int Timing = 0;

real const MinimumViewLength = 280.0;
real ViewLength = MinimumViewLength + 40.0;
vector_4 View(0,0,-1);
vector_4 Up(0,1,0);
vector_4 Right(1,0,0);

point_4 aVertices[5] =
{
	point_4(-100, 100,0),
	point_4( 100, 100,0),
	point_4( 100,-100,0),
	point_4(-100,-100,0),
	point_4(-100, 100,0),
};
point_4 aScreenVertices[5];

struct info
{
	BITMAPINFOHEADER Header;
	RGBQUAD aColors[256];

};

info BufferInfo =
{
	sizeof(BITMAPINFOHEADER),
	300,300,
	1,8,
};

info TextureInfo;
char unsigned *pTextureBits;
HBITMAP TextureBitmap;

HBITMAP OldBitmap;
HDC BufferDC;
HBITMAP BufferBitmap;
char unsigned *pBufferBits;

dib_info Texture;
dib_info Buffer;

struct
{
	WORD Version;
	WORD NumberOfEntries;
	PALETTEENTRY aEntries[256];
} LogicalPalette =
{
	0x300,
	256
};

struct
{
	char unsigned Red, Green, Blue;
} aStaticColors[20] =
{
	{ 0,0,0 },
	{ 128,0,0 },
	{ 0,128,0 },
	{ 128,128,0 },
	{ 0,0,128 },
	{ 128,0,128 },
	{ 0,128,128 },
	{ 192,192,192 },
	{ 192,220,192 },
	{ 166,202,240 },

	{ 255,251,240 },
	{ 160,160,164 },
	{ 128,128,128 },
	{ 255,0,0 },
	{ 0,255,0 },
	{ 255,255,0 },
	{ 0,0,255 },
	{ 255,0,255 },
	{ 0,255,255 },
	{ 255,255,255 },
};
/*----------------------------------------------------------------------------

dynamic menu structures - here's where you can add your own mappers and views

*/

struct mapper_entry;

typedef void mapper( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture );

typedef void mapper_init( mapper_entry &Entry );
typedef void mapper_setup( mapper_entry &Entry );

mapper_init Init_fl_fl;
mapper_setup Setup_fl_fl;
mapper_init Init_i_fl;
mapper_setup Setup_i_fl;
mapper_init Init_fx_fl;
mapper_setup Setup_fx_fl;

struct mapper_entry
{
	mapper *pMapper;
	mapper_init *pInit;
	mapper_setup *pSetup;
	char const *pDescription;

	POINT3D aVertices[5];
	
} aMappers[] =
{
	{TextureMapTriangle_div_fl_fl, Init_fl_fl, Setup_fl_fl, "Div, fl, fl"},
	{TextureMapTriangle_div_i_fl, Init_i_fl, Setup_i_fl, "Div, int, fl"},
	{TextureMapTriangle_div_fx_fl, Init_fx_fl, Setup_fx_fl, "Div, fix, fl"},
	{TextureMapTriangle_suba_fx_fl, Init_fx_fl, Setup_fx_fl, "SubAff, fix, fl"},
	{TextureMapTriangle_suba_fx_fl_asm, Init_fx_fl, Setup_fx_fl, "SubAff, fix, fl - Asm"},
};
int const NumberOfMappers = sizeof(aMappers) / sizeof(aMappers[0]);

int MapperIndex = 0;

struct view_entry
{
	view_entry( char const *p, real L, vector_4 V, vector_4 U, vector_4 R ) :
		pDescription(p),Length(L),View(V),Up(U),Right(R) { }
		
	char const *pDescription;
	real Length;
	vector_4 View, Up, Right;
} aViews[] =
{
	view_entry("&Wall",ViewLength,vector_4(0.939693,0,-0.34202),vector_4(0,1,0),vector_4(0.34202,0,0.939693)),
	view_entry("&Floor",ViewLength,vector_4(0,0.939694,-0.34202),vector_4(0,0.342019,0.939693),vector_4(1,0,0)),
	view_entry("Wol&berg",ViewLength,vector_4(0.655004,0.529593,-0.538986),vector_4(0.456389,0.291221,0.840775),vector_4(0.602231,-0.796697,-0.0509494)),

	// you can add your own views here, like I did, just break in the
	// debugger and dump out he ViewLength, View, Up, and Right variables
	// and put them another field

//	view_entry("&Casey",320.0,vector_4(0.0348996,6.99192e-007,-0.999393),vector_4(0.000000,1.00000,1.22685e-006),vector_4(0.999391,-4.28163e-008,0.0348995)),
//	view_entry("&Bad",320.0,vector_4(0.883587,0.0532488,-0.465236),vector_4(0.250669,-0.892966,0.373867),vector_4(-0.395530,-0.446964,-0.802359)),

};
int const NumberOfViews = sizeof(aViews) / sizeof(aViews[0]);




/*----------------------------------------------------------------------------*\
|	AppAbout( hDlg, uiMessage, wParam, lParam ) 							   |
|																			   |
|	Description:															   |
|		This function handles messages belonging to the "About" dialog box.    |
|		The only message that it looks for is WM_COMMAND, indicating the use   |
|		has pressed the "OK" button.  When this happens, it takes down		   |
|		the dialog box. 													   |
|																			   |
|	Arguments:																   |
|		hDlg			window handle of about dialog window				   |
|		uiMessage		message number										   |
|		wParam			message-dependent									   |
|		lParam			message-dependent									   |
|																			   |
|	Returns:																   |
|		TRUE if message has been processed, else FALSE						   |
|																			   |
\*----------------------------------------------------------------------------*/
BOOL FAR PASCAL  AppAbout(HWND hwnd,UINT msg,WPARAM wParam,
			LPARAM /* lParam */ )
{
	switch (msg)
	{
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hwnd,TRUE);
			}
			break;

		case WM_INITDIALOG:
			return TRUE;
	}
	return FALSE;
}

/*----------------------------------------------------------------------------*\
|	AppInit( hInst, hPrev)													   |
|																			   |
|	Description:															   |
|		This is called when the application is first loaded into			   |
|		memory.  It performs all initialization that doesn't need to be done   |
|		once per instance.													   |
|																			   |
|	Arguments:																   |
|		hInstance		instance handle of current instance 				   |
|		hPrev			instance handle of previous instance				   |
|																			   |
|	Returns:																   |
|		TRUE if successful, FALSE if not									   |
|																			   |
\*----------------------------------------------------------------------------*/
BOOL AppInit(HINSTANCE hInst,HINSTANCE hPrev,int sw,LPSTR szCmdLine)
{
	WNDCLASS cls;

	/* Save instance handle for DialogBoxs */
	hInstApp = hInst;

	if (!hPrev)
	{
		/*
		 *	Register a class for the main application window
		 */
		cls.hCursor 	   = LoadCursor(NULL,IDC_ARROW);
		cls.hIcon		   = LoadIcon(hInst,"AppIcon");
		cls.lpszMenuName   = "AppMenu";
		cls.lpszClassName  = szAppName;
		cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
		cls.hInstance	   = hInst;
		cls.style		   = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		cls.lpfnWndProc    = (WNDPROC)AppWndProc;
		cls.cbWndExtra	   = 0;
		cls.cbClsExtra	   = 0;

		if (!RegisterClass(&cls))
			return FALSE;
	}

	// init palette and color table

	HDC Screen = GetDC(0);

	if(GetDeviceCaps(Screen,RASTERCAPS) & RC_PALETTE)
	{
		GetSystemPaletteEntries(Screen,0,10,LogicalPalette.aEntries);
		GetSystemPaletteEntries(Screen,246,10,LogicalPalette.aEntries+246);
	}
	else
	{
		for(int Counter = 0;Counter < 10;Counter++)
		{
			LogicalPalette.aEntries[Counter].peRed =
				aStaticColors[Counter].Red;
			LogicalPalette.aEntries[Counter].peGreen =
				aStaticColors[Counter].Green;
			LogicalPalette.aEntries[Counter].peBlue =
				aStaticColors[Counter].Blue;

			LogicalPalette.aEntries[Counter+246].peRed =
				aStaticColors[Counter+10].Red;
			LogicalPalette.aEntries[Counter+246].peGreen =
				aStaticColors[Counter+10].Green;
			LogicalPalette.aEntries[Counter+246].peBlue =
				aStaticColors[Counter+10].Blue;
		}
	}

	ReleaseDC(0,Screen);

	for(int Counter = 0;Counter < 10;Counter++)
	{
		BufferInfo.aColors[Counter].rgbRed =
			LogicalPalette.aEntries[Counter].peRed;
		BufferInfo.aColors[Counter+246].rgbRed =
			LogicalPalette.aEntries[Counter+246].peRed;

		BufferInfo.aColors[Counter].rgbGreen =
			LogicalPalette.aEntries[Counter].peGreen;
		BufferInfo.aColors[Counter+246].rgbGreen =
			LogicalPalette.aEntries[Counter+246].peGreen;

		BufferInfo.aColors[Counter].rgbBlue =
			LogicalPalette.aEntries[Counter].peBlue;
		BufferInfo.aColors[Counter+246].rgbBlue =
			LogicalPalette.aEntries[Counter+246].peBlue;

		BufferInfo.aColors[Counter].rgbReserved =
			LogicalPalette.aEntries[Counter].peFlags = 0;
		BufferInfo.aColors[Counter+246].rgbReserved =
			LogicalPalette.aEntries[Counter+246].peFlags = 0;
	}
		
	for(Counter = 10;Counter < 246;Counter++)
	{
		BufferInfo.aColors[Counter].rgbRed =
				LogicalPalette.aEntries[Counter].peRed = Counter;

		BufferInfo.aColors[Counter].rgbGreen =
				LogicalPalette.aEntries[Counter].peGreen = Counter;

		BufferInfo.aColors[Counter].rgbBlue =
				LogicalPalette.aEntries[Counter].peBlue = Counter;

		BufferInfo.aColors[Counter].rgbReserved = 0;
		LogicalPalette.aEntries[Counter].peFlags = PC_RESERVED;
	}

	hpalApp = CreatePalette((LOGPALETTE *)&LogicalPalette);

	// create bitmaps

	BufferDC = CreateCompatibleDC(0);
	BufferBitmap = CreateDIBSection(BufferDC,(BITMAPINFO *)&BufferInfo,
						DIB_RGB_COLORS,(void **)&pBufferBits,0,0);

	TextureInfo = BufferInfo;
	TextureInfo.Header.biWidth = 102;
	TextureInfo.Header.biHeight = 102;
						
	TextureBitmap = CreateDIBSection(BufferDC,(BITMAPINFO *)&TextureInfo,
						DIB_RGB_COLORS,(void **)&pTextureBits,0,0);

	assert((TextureInfo.Header.biHeight > 0)
		&& (BufferInfo.Header.biHeight > 0));

	// set up two bottom-up DIBs for rendering

	Texture.Width = TextureInfo.Header.biWidth;
	Texture.Height = TextureInfo.Header.biHeight;
	Texture.DeltaScan = (TextureInfo.Header.biWidth + 3) & ~3;
	Texture.pBits = pTextureBits + Texture.DeltaScan *
						(TextureInfo.Header.biHeight - 1);
	Texture.DeltaScan *= -1;

	Buffer.Width = BufferInfo.Header.biWidth;
	Buffer.Height = BufferInfo.Header.biHeight;
	Buffer.DeltaScan = (BufferInfo.Header.biWidth + 3) & ~3;
	Buffer.pBits = pBufferBits + Buffer.DeltaScan *
						(BufferInfo.Header.biHeight - 1);
	Buffer.DeltaScan *= -1;

	// draw checker board texture

	OldBitmap = SelectBitmap(BufferDC,TextureBitmap);

	HBRUSH Red = CreateSolidBrush(RGB(255,0,0));
	HBRUSH Blue = CreateSolidBrush(RGB(50,50,255));

	for(int X = 0;X < 100;X += 10)
	{
		for(int Y = 0;Y < 100;Y += 10)
		{
			if(((X + Y)/10)%2)
			{
				SelectObject(BufferDC,Blue);
			}
			else
			{
				SelectObject(BufferDC,Red);
			}

			PatBlt(BufferDC,X+1,Y+1,10,10,PATCOPY);
		}
	}

	// draw two outlines so we can see when we fetch invalid texture coordinates

	HPEN Pen = CreatePen(PS_SOLID,0,RGB(0,255,0));
	Pen = SelectPen(BufferDC,Pen);
	MoveToEx(BufferDC,0,0,0);
	LineTo(BufferDC,101,0);
	LineTo(BufferDC,101,101);
	LineTo(BufferDC,0,101);
	LineTo(BufferDC,0,0);

	DeleteObject(SelectObject(BufferDC,Pen));
	MoveToEx(BufferDC,1,1,0);
	LineTo(BufferDC,100,1);
	LineTo(BufferDC,100,100);
	LineTo(BufferDC,1,100);
	LineTo(BufferDC,1,1);

	SelectBitmap(BufferDC,BufferBitmap);

	// setup window size

	DWORD Style = WS_OVERLAPPEDWINDOW;
	RECT WindowRect = { 0, 0 };
	WindowRect.right = BufferInfo.Header.biWidth;
	WindowRect.bottom = BufferInfo.Header.biHeight;

	AdjustWindowRect(&WindowRect,Style,TRUE);
	
	hwndApp = CreateWindow(szAppName,szAppName,Style,
					CW_USEDEFAULT,0,
					WindowRect.right-WindowRect.left,
					WindowRect.bottom-WindowRect.top,
					0,0,hInst,0);

	// init dynamic menus
					
    HMENU hmenu = GetSubMenu(GetMenu(hwndApp),1);
    DeleteMenu(hmenu,MENU_MAPPER,MF_BYCOMMAND);

    for(int i=0; i<NumberOfMappers; i++)
    {
    	(*aMappers[i].pInit)(aMappers[i]);

    	char aBuffer[100];
    	ostrstream Out(aBuffer,sizeof(aBuffer));
    	Out<<aMappers[i].pDescription<<"\t"<<i<<ends;
        AppendMenu(hmenu,0,MENU_MAPPER+i,aBuffer);
    }

    hmenu = GetSubMenu(GetMenu(hwndApp),2);
    DeleteMenu(hmenu,MENU_VIEW,MF_BYCOMMAND);

    for(i=0; i<NumberOfViews; i++)
    {
        AppendMenu(hmenu,0,MENU_VIEW+i,aViews[i].pDescription);
    }

    pMapperType = szAppName + strlen(szAppName);

    SendMessage(hwndApp,WM_COMMAND,MENU_MAPPER,0);

	ShowWindow(hwndApp,sw);

	return TRUE;
}


/*----------------------------------------------------------------------------*\
|	AppExit()									   |
|																			   |
|	Description:															   |
|	app is just about to exit, cleanup					   |
|																			   |
\*----------------------------------------------------------------------------*/
void AppExit()
{
	SelectObject(BufferDC,OldBitmap);
	DeleteObject(TextureBitmap);
	DeleteObject(BufferBitmap);
	DeleteDC(BufferDC);
	DeleteObject(hpalApp);
}

/*----------------------------------------------------------------------------*\
|	WinMain( hInst, hPrev, lpszCmdLine, cmdShow )							   |
|																			   |
|	Description:															   |
|		The main procedure for the App.  After initializing, it just goes	   |
|		into a message-processing loop until it gets a WM_QUIT message		   |
|		(meaning the app was closed).										   |
|																			   |
|	Arguments:																   |
|		hInst			instance handle of this instance of the app 		   |
|		hPrev			instance handle of previous instance, NULL if first    |
|		szCmdLine		->null-terminated command line						   |
|		cmdShow 		specifies how the window is initially displayed 	   |
|																			   |
|	Returns:																   |
|		The exit code as specified in the WM_QUIT message.					   |
|																			   |
\*----------------------------------------------------------------------------*/
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	MSG 	msg;

	/* Call initialization procedure */
	if (!AppInit(hInst,hPrev,sw,szCmdLine))
	return FALSE;

	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	AppExit();
	return msg.wParam;
}

/*----------------------------------------------------------------------------*\
|	AppPaint(hwnd, hdc) 													   |
|																			   |
|	Description:															   |
|		The paint function.  Right now this does nothing.					   |
|																			   |
|	Arguments:																   |
|		hwnd			 window painting into								   |
|		hdc 			 display context to paint to						   |
|																			   |
|	Returns:																   |
|		nothing 															   |
|																			   |
\*----------------------------------------------------------------------------*/
AppPaint (HWND hwnd, HDC hdc)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	PatBlt(BufferDC,0,0,BufferInfo.Header.biWidth,BufferInfo.Header.biHeight,
		WHITENESS);

	int Counter;

	real ViewDistance = 25.0;
	matrix_4x4 ViewTransform(PointFrom(-View * ViewLength),Right,Up,View);

	SetViewDistance(ViewTransform,ViewDistance);

	ViewTransform.ConcatenateXScale(10);
	ViewTransform.ConcatenateYScale(-10);
	ViewTransform.ConcatenateXTranslation(BufferInfo.Header.biWidth/2.0);
	ViewTransform.ConcatenateYTranslation(BufferInfo.Header.biHeight/2.0);

	for(Counter = 0;Counter < 5;Counter++)
	{
		aScreenVertices[Counter] = ViewTransform * aVertices[Counter];
		aScreenVertices[Counter].Homogenize();
	}

	MoveToEx(BufferDC,aScreenVertices[0].GetX(),aScreenVertices[0].GetY(),0);

	// turn this on if you want to see which vertices are which
	
#if 0
	for(Counter = 1;Counter < 5;Counter++)
	{
		// draw the edges
		LineTo(BufferDC,aScreenVertices[Counter].GetX(),
			aScreenVertices[Counter].GetY());

		// label the vertices
		TextOut(BufferDC,aScreenVertices[Counter].GetX(),
			aScreenVertices[Counter].GetY(),"ABCDA"+Counter,1);
	}
#endif

	// call the mapper setup and the draw the polygon

	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices+2,
									Texture);

#if 0
	// XOR test with last mapper
	(*aMappers[NumberOfMappers-1].pSetup)(aMappers[NumberOfMappers-1]);
	(*aMappers[NumberOfMappers-1].pMapper)(Buffer,
					aMappers[NumberOfMappers-1].aVertices,Texture);
	(*aMappers[NumberOfMappers-1].pMapper)(Buffer,
					aMappers[NumberOfMappers-1].aVertices+2,Texture);
#endif


	// update the display

	BitBlt(hdc,0,0,rc.right,rc.bottom,BufferDC,0,0,SRCCOPY);

	return TRUE;
}

/*----------------------------------------------------------------------------*\
|	AppWndProc( hwnd, uiMessage, wParam, lParam )							   |
|																			   |
|	Description:															   |
|		The window proc for the app's main (tiled) window.	This processes all |
|		of the parent window's messages.									   |
|																			   |
\*----------------------------------------------------------------------------*/
LONG FAR PASCAL AppWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	BOOL f;

	static int LastX, LastY;

	switch (msg)
	{
		case WM_CREATE:
		break;

		case WM_KEYDOWN:
		{
			real const Step = RadiansFrom(0.5); // 2.0);

			switch(wParam)
			{
				case VK_PRIOR:
				case VK_NEXT:
				{
					// rotate around the view vector
					real Radians = (wParam == VK_PRIOR) ? -Step : Step;
														
					matrix_4x4 Rotation(View,Radians);
					Up = Rotation * Up;
					Right = Rotation * Right;
					break;
				}
				case VK_LEFT:
				case VK_RIGHT:
				{
					// rotate around the up vector
					real Radians = (wParam == VK_LEFT) ? -Step : Step;
														
					matrix_4x4 Rotation(Up,Radians);
					Right = Rotation * Right;
					View = Rotation * View;
					break;
				}
				case VK_UP:
				case VK_DOWN:
				{
					// rotate around the right vector
					real Radians = (wParam == VK_DOWN) ? -Step : Step;

					matrix_4x4 Rotation(Right,Radians);
					Up = Rotation * Up;
					View = Rotation * View;
					break;
				}
				case 'I':
				{
					ViewLength *= 0.9;
					if(ViewLength < MinimumViewLength)
					{
						ViewLength = MinimumViewLength;
					}
					break;
				}
				case 'O':
				{
					ViewLength *= 1.1;
					break;
				}
				case 'A':
				{
					SendMessage(hwnd,WM_COMMAND,MENU_ANIMATE,0);
					break;
				}
				case 'P':
				{
					SendMessage(hwnd,WM_COMMAND,MENU_PIXELSPERSEC,0);
					break;
				}
				case 'T':
				{
					if(GetKeyState(VK_CONTROL) & 0x80000000)
					{
						SendMessage(hwnd,WM_COMMAND,MENU_TEST,0);
						return 0; // don't repaint
					}
					else
					{
						LARGE_INTEGER StartTime, EndTime, Frequency;
						int const Iterations = 500;

						QueryPerformanceFrequency(&Frequency);
			
						QueryPerformanceCounter(&StartTime);
			
						for(int Counter = 0;Counter < Iterations;Counter++)
						{
							(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
						}
			
						QueryPerformanceCounter(&EndTime);
			
						char aBuffer[100];
						ostrstream Out(aBuffer,sizeof(aBuffer));

						double SecondsPerIteration =
							(double(EndTime.LowPart - StartTime.LowPart) /
									double(Frequency.LowPart)) /
										double(Iterations);

						double CyclesPerSecond = 133000000.0;
			
						Out<<"Microseconds: "<<SecondsPerIteration / 1000000.0
							<<" Cycles: "<<SecondsPerIteration * CyclesPerSecond<<ends;
			
						MessageBox(0,aBuffer,"Timing",MB_OK);

						// SendMessage(hwnd,WM_COMMAND,MENU_TIME,0);
					}
					break;
				}
				default:
				{
					if((wParam >= '0') && (wParam < ('0' + NumberOfMappers)))
					{
						SendMessage(hwnd,WM_COMMAND,MENU_MAPPER+wParam-'0',0);
					}
					break;
				}
						
			}
			InvalidateRect(hwnd,0,FALSE);
			UpdateWindow(hwnd);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			LastX = (short)LOWORD(lParam);
			LastY = (short)HIWORD(lParam);

			SetCapture(hwnd);
			break;
		}

		case WM_MOUSEMOVE:
		{
			if(wParam & MK_LBUTTON)
			{
				int X = (short)LOWORD(lParam);
				int Y = (short)HIWORD(lParam);
				int DeltaX = LastX - X;
				int DeltaY = LastY - Y;

				if(DeltaX)
				{
					// rotate around the up vector
					real Radians = RadiansFrom(DeltaX);
														
					matrix_4x4 Rotation(Up.Normalize(),Radians);
					Right = Rotation * Right;
					View = Rotation * View;
				}

				if(DeltaY)
				{
					// rotate around the right vector
					real Radians = RadiansFrom(DeltaY);

					matrix_4x4 Rotation(Right.Normalize(),-Radians);
					Up = Rotation * Up;
					View = Rotation * View;
				}

				LastX = X;
				LastY = Y;
				
				InvalidateRect(hwnd,0,FALSE);
				UpdateWindow(hwnd);
			}
			break;
		}

		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			break;
		}

		case WM_ERASEBKGND:
			break;

		case WM_INITMENU:
			break;

		case WM_COMMAND:
			return AppCommand(hwnd,msg,wParam,lParam);

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_CLOSE:
			break;

		case WM_PALETTECHANGED:
			if ((HWND)wParam == hwnd)
			break;

		// fall through to WM_QUERYNEWPALETTE

		case WM_QUERYNEWPALETTE:
			hdc = GetDC(hwnd);

			if (hpalApp)
				SelectPalette(hdc, hpalApp, FALSE);

			f = RealizePalette(hdc);
			ReleaseDC(hwnd,hdc);

			if (f)
				InvalidateRect(hwnd,NULL,FALSE);

			return f;

		case WM_PAINT:
			hdc = BeginPaint(hwnd,&ps);
			if (hpalApp)
				SelectPalette(hdc, hpalApp, FALSE);
			f = RealizePalette(hdc);
			AppPaint (hwnd,hdc);
			EndPaint(hwnd,&ps);
			return 0L;

		case WM_SIZE:
			break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

/*----------------------------------------------------------------------------*\
|	AppCommand(hwnd, msg, wParam, lParam )					   |
|																			   |
|	Description:															   |
|	handles WM_COMMAND messages for the main window (hwndApp)		   |
|		of the parent window's messages.									   |
|																			   |
\*----------------------------------------------------------------------------*/
LONG AppCommand (HWND hwnd,UINT /* msg */,WPARAM wParam,LPARAM /* lParam */)
{
	switch(wParam)
	{
		case MENU_ABOUT:
			DialogBox(hInstApp,"AppAbout",hwnd,AppAbout);
			break;

		case MENU_EXIT:
			PostMessage(hwnd,WM_CLOSE,0,0L);
			break;

		case MENU_TEST:
			TestMapper();
			break;

		case MENU_ANIMATE:
		{
			HDC Display = GetDC(hwnd);
			SelectPalette(Display,hpalApp,FALSE);
			RealizePalette(Display);

			for(int Counter = 0;Counter < 100;Counter++)
			{
				matrix_4x4 Rotation(Up,RadiansFrom(0.5));
				Right = Rotation * Right;
				View = Rotation * View;
				AppPaint(hwnd,Display);
			}
			ReleaseDC(hwnd,Display);
			break;
		}
		case MENU_PIXELSPERSEC:
		{
			HDC Display = GetDC(hwnd);

			SelectPalette(Display,hpalApp,FALSE);
			RealizePalette(Display);

			PatBlt(BufferDC,0,0,BufferInfo.Header.biWidth,
				BufferInfo.Header.biHeight,
				WHITENESS);

			LARGE_INTEGER StartTime, EndTime, Frequency;
			int const Iterations = 100;

			int const Pixels = 5000;

			QueryPerformanceFrequency(&Frequency);

			QueryPerformanceCounter(&StartTime);

			for(int Counter = 0;Counter < Iterations;Counter++)
			{
				Draw5000PixelTriangle();
			}

			QueryPerformanceCounter(&EndTime);

			BitBlt(Display,0,0,Buffer.Width,Buffer.Height,BufferDC,0,0,SRCCOPY);

			Timing = 0;

			char aBuffer[100];
			ostrstream Out(aBuffer,sizeof(aBuffer));

			Out<<"Pixels/Sec ("<<Pixels<<" pixel triangles): "
				<<(double(Iterations) * Pixels) /
					(double(EndTime.LowPart - StartTime.LowPart) /
						double(Frequency.LowPart))<<ends;

			MessageBox(0,aBuffer,"Pixels Per Second",MB_OK);
			
			ReleaseDC(hwnd,Display);
			break;
		}
		case MENU_TIME:
		{
			HDC Display = GetDC(hwnd);

			SelectPalette(Display,hpalApp,FALSE);
			RealizePalette(Display);

			LARGE_INTEGER StartTime, EndTime, Frequency;
			int const Iterations = 25;

			QueryPerformanceFrequency(&Frequency);

			QueryPerformanceCounter(&StartTime);

			for(int Counter = 0;Counter < Iterations;Counter++)
			{
				(*aMappers[MapperIndex].pMapper)(Buffer,
						aMappers[MapperIndex].aVertices,Texture);
				(*aMappers[MapperIndex].pMapper)(Buffer,
						aMappers[MapperIndex].aVertices+2,Texture);
			}

			QueryPerformanceCounter(&EndTime);

			Timing = 0;

			char aBuffer[100];
			ostrstream Out(aBuffer,sizeof(aBuffer));

			Out<<"FPS: "<<double(Iterations) /
				(double(EndTime.LowPart - StartTime.LowPart) /
					double(Frequency.LowPart))<<ends;

			MessageBox(0,aBuffer,"Frames Per Second",MB_OK);
			
			ReleaseDC(hwnd,Display);
			break;
		}
		default:
		{
			if((wParam >= MENU_MAPPER) &&
				(wParam < (MENU_MAPPER + NumberOfMappers)))
			{
				ostrstream Out(pMapperType,
							sizeof(szAppName) - (szAppName - pMapperType));

				Out<<aMappers[wParam-MENU_MAPPER].pDescription<<ends;

				SetWindowText(hwnd,szAppName);

				CheckMenuItem(GetMenu(hwnd),MENU_MAPPER+MapperIndex,
					MF_UNCHECKED);
				MapperIndex = wParam - MENU_MAPPER;
				CheckMenuItem(GetMenu(hwnd),MENU_MAPPER+MapperIndex,
					MF_CHECKED);
			}
			else
			if((wParam >= MENU_VIEW) &&
				(wParam < (MENU_VIEW + NumberOfViews)))
			{
				ViewLength = aViews[wParam - MENU_VIEW].Length;
				View = aViews[wParam - MENU_VIEW].View;
				Up = aViews[wParam - MENU_VIEW].Up;
				Right = aViews[wParam - MENU_VIEW].Right;
			}

			InvalidateRect(hwnd,0,FALSE);
			UpdateWindow(hwnd);
		}
	}
	return 0L;
}
    
extern "C" void __cdecl _assert( void *pExpression, void *pFile,
	unsigned LineNumber )
{
	char aBuffer[500];
	wsprintf(aBuffer,"Assertion: %s\nFile: %s, Line: %d\n" \
					"Hit Abort to exit, Retry to debug, Ignore to continue",
					pExpression,pFile,LineNumber);

	int Hit = MessageBox(hwndApp,aBuffer,"Assert!",MB_ABORTRETRYIGNORE |
					MB_ICONHAND);

	if(Hit == IDABORT)
	{
		exit(0);
	}
	else
	if(Hit == IDRETRY)
	{
		DebugBreak();
	}
}

/*----------------------------------------------------------------------------

test functions

*/

void TestMapper( void )
{
	PatBlt(BufferDC,0,0,BufferInfo.Header.biWidth,BufferInfo.Header.biHeight,
		WHITENESS);

	for(int Counter = 0;Counter < 4;Counter++)
	{
		aScreenVertices[Counter].SetW(1);
	}

	float XOffset = 10, YOffset = 10;

	aScreenVertices[0].SetX((XOffset - 0.5));
	aScreenVertices[0].SetY((YOffset - 0.5));
	aScreenVertices[1].SetX((XOffset + 99.5));
	aScreenVertices[1].SetY((YOffset - 0.5));
	aScreenVertices[2].SetX((XOffset + 99.5));
	aScreenVertices[2].SetY((YOffset + 99.5));
	aScreenVertices[3].SetX((XOffset - 0.5));
	aScreenVertices[3].SetY((YOffset + 99.5));

	aScreenVertices[4] = aScreenVertices[0];
	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices+2,
									Texture);

	XOffset = 120, YOffset = 10;
	aScreenVertices[0].SetX((XOffset));
	aScreenVertices[0].SetY((YOffset));
	aScreenVertices[1].SetX((XOffset + 100));
	aScreenVertices[1].SetY((YOffset));
	aScreenVertices[2].SetX((XOffset + 100));
	aScreenVertices[2].SetY((YOffset + 100));
	aScreenVertices[3].SetX((XOffset));
	aScreenVertices[3].SetY((YOffset + 100));

	aScreenVertices[4] = aScreenVertices[0];
	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices+2,
									Texture);

	XOffset = 10, YOffset = 120;
	aScreenVertices[2].SetX((XOffset));
	aScreenVertices[2].SetY((YOffset));
	aScreenVertices[3].SetX((XOffset + 100));
	aScreenVertices[3].SetY((YOffset));
	aScreenVertices[0].SetX((XOffset + 100));
	aScreenVertices[0].SetY((YOffset + 100));
	aScreenVertices[1].SetX((XOffset));
	aScreenVertices[1].SetY((YOffset + 100));

	aScreenVertices[4] = aScreenVertices[0];
	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices+2,
									Texture);


	XOffset = 120, YOffset = 120;
	aScreenVertices[2].SetX((XOffset + 100));
	aScreenVertices[2].SetY((YOffset));
	aScreenVertices[0].SetX((XOffset + 100));
	aScreenVertices[0].SetY((YOffset + 100));
	aScreenVertices[0].SetW(2);
	aScreenVertices[1].SetX((XOffset));
	aScreenVertices[1].SetY((YOffset + 100));

	aScreenVertices[4] = aScreenVertices[0];
	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);

	HDC hdc = GetDC(hwndApp);	
	BitBlt(hdc,0,0,Buffer.Width,Buffer.Height,BufferDC,0,0,SRCCOPY);
	ReleaseDC(hwndApp,hdc);
}

void Draw5000PixelTriangle( void )
{
	for(int Counter = 0;Counter < 4;Counter++)
	{
		aScreenVertices[Counter].SetW(1);
	}

	float XOffset = 120, YOffset = 120;
	aScreenVertices[2].SetX((XOffset + 100));
	aScreenVertices[2].SetY((YOffset));
	aScreenVertices[0].SetX((XOffset + 100));
	aScreenVertices[0].SetY((YOffset + 100));
	aScreenVertices[0].SetW(2);
	aScreenVertices[1].SetX((XOffset));
	aScreenVertices[1].SetY((YOffset + 100));

	aScreenVertices[4] = aScreenVertices[0];
	(*aMappers[MapperIndex].pSetup)(aMappers[MapperIndex]);
	(*aMappers[MapperIndex].pMapper)(Buffer,aMappers[MapperIndex].aVertices,
									Texture);
}

/*----------------------------------------------------------------------------

init and setup functions

*/

void Init_fl_fl( mapper_entry &Entry )
{
	Entry.aVertices[0].flfl.U = Entry.aVertices[4].flfl.U = 0.5;
	Entry.aVertices[0].flfl.V = Entry.aVertices[4].flfl.V = 0.5;

	Entry.aVertices[1].flfl.U = 100.5;
	Entry.aVertices[1].flfl.V = 0.5;

	Entry.aVertices[2].flfl.U = 100.5;
	Entry.aVertices[2].flfl.V = 100.5;

	Entry.aVertices[3].flfl.U = 0.5;
	Entry.aVertices[3].flfl.V = 100.5;
}

void Setup_fl_fl( mapper_entry &Entry )
{
	for(int Counter = 0;Counter < 5;Counter++)
	{
		Entry.aVertices[Counter].flfl.X = aScreenVertices[Counter].GetX();
		Entry.aVertices[Counter].flfl.Y = aScreenVertices[Counter].GetY();
		Entry.aVertices[Counter].flfl.Z = aScreenVertices[Counter].GetW();
	}
}

void Init_i_fl( mapper_entry &Entry )
{
	Entry.aVertices[0].ifl.U = Entry.aVertices[4].ifl.U = 0.5;
	Entry.aVertices[0].ifl.V = Entry.aVertices[4].ifl.V = 0.5;

	Entry.aVertices[1].ifl.U = 100.5;
	Entry.aVertices[1].ifl.V = 0.5;

	Entry.aVertices[2].ifl.U = 100.5;
	Entry.aVertices[2].ifl.V = 100.5;

	Entry.aVertices[3].ifl.U = 0.5;
	Entry.aVertices[3].ifl.V = 100.5;
}

void Setup_i_fl( mapper_entry &Entry )
{
	for(int Counter = 0;Counter < 5;Counter++)
	{
		Entry.aVertices[Counter].ifl.X = aScreenVertices[Counter].GetX();
		Entry.aVertices[Counter].ifl.Y = aScreenVertices[Counter].GetY();
		Entry.aVertices[Counter].ifl.Z = aScreenVertices[Counter].GetW();
	}
}

void Init_fx_fl( mapper_entry &Entry )
{
	Entry.aVertices[0].fxfl.U = Entry.aVertices[4].fxfl.U = 0.5;
	Entry.aVertices[0].fxfl.V = Entry.aVertices[4].fxfl.V = 0.5;

	Entry.aVertices[1].fxfl.U = 100.5;
	Entry.aVertices[1].fxfl.V = 0.5;

	Entry.aVertices[2].fxfl.U = 100.5;
	Entry.aVertices[2].fxfl.V = 100.5;

	Entry.aVertices[3].fxfl.U = 0.5;
	Entry.aVertices[3].fxfl.V = 100.5;
}

void Setup_fx_fl( mapper_entry &Entry )
{
	for(int Counter = 0;Counter < 5;Counter++)
	{
		Entry.aVertices[Counter].fxfl.X = aScreenVertices[Counter].GetX()*16.0;
		Entry.aVertices[Counter].fxfl.Y = aScreenVertices[Counter].GetY()*16.0;
		Entry.aVertices[Counter].fxfl.Z = aScreenVertices[Counter].GetW()*16.0;
	}
}

