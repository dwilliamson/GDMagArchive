/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Double-Buffered Game Window

	Windows Version

	The CXSplatWindow constructor creates a standard overlapped window and
	attaches to it a pointer to the CXSplatWindow via SetWindowLong. It also
	creates a COffscreenBuffer to go with it.

	Calling the CXSplatWindow destructor destroys the allocated window.
	"Manually" destroying the window DOES NOT destroy the CXSplatWindow or
	the associated OffscreenBuffer.

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

#ifdef _WINDOWS

/****************************************************************************
	Constants
*/

static char XSplatWindowClassName[] = "XSPLAT";

// For now, don't allow the window to be resized
static const DWORD XSplatWindowStyle = WS_OVERLAPPEDWINDOW &
	~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

/****************************************************************************
	Internal Functions
*/

LONG PASCAL XSplatWindowProc(HWND, UINT, WPARAM, LPARAM);

/*****************************************************************************
	Constructor and Destructor
*/

CXSplatWindow::CXSplatWindow(const char *Caption, int WindowWidth,
	int WindowHeight, unsigned char const *Colors) :
	Palette(0), Window(0), pOffscreenBuffer(0),
	IsActiveFlag(0), MouseDownFlag(0)
{
	extern HINSTANCE ghInstance;
	assert(ghInstance);

	// Register the XSplatWindow class if it isn't already registered
	int Success = 1;
	WNDCLASS ClassInfo;
	if (!GetClassInfo(ghInstance, XSplatWindowClassName, &ClassInfo))
	{
		ClassInfo.hCursor       = LoadCursor(0, IDC_ARROW);
		ClassInfo.hIcon         = LoadIcon(ghInstance, IDI_APPLICATION);
		ClassInfo.lpszMenuName  = 0;
		ClassInfo.lpszClassName = XSplatWindowClassName;
		ClassInfo.hbrBackground = HBRUSH(GetStockObject(WHITE_BRUSH));
		ClassInfo.hInstance     = ghInstance;
		ClassInfo.style         = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
		ClassInfo.lpfnWndProc   = WNDPROC(XSplatWindowProc);
		ClassInfo.cbWndExtra    = 0;
		ClassInfo.cbClsExtra    = 0;
		
		Success = RegisterClass(&ClassInfo);
	}

	if (Success)
	{
		// Determine screen dimensions
		int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		// Given a zero or negative dimension, fill the screen
		if (WindowWidth <= 0)
			WindowWidth = ScreenWidth;
		if (WindowHeight <= 0)
			WindowHeight = ScreenHeight;

		// Determine the window size for the requested client area
		RECT WindowRect = { 0, 0, WindowWidth, WindowHeight };
		AdjustWindowRect(&WindowRect, XSplatWindowStyle, 0);

		// Make sure it's not larger than the screen
		int WindowWidth = WindowRect.right - WindowRect.left;
		if (WindowWidth > ScreenWidth) WindowWidth = ScreenWidth;

		int WindowHeight = WindowRect.bottom - WindowRect.top;
		if (WindowHeight > ScreenHeight) WindowHeight = ScreenHeight;

		// Create the window centered on the screen
		int Left = (ScreenWidth - WindowWidth) / 2;
		int Top = (ScreenHeight - WindowHeight) / 2;

		Window = CreateWindow(XSplatWindowClassName, Caption, XSplatWindowStyle,
		                      Left, Top, WindowWidth, WindowHeight,
		                      0, 0, ghInstance, 0);

		if (Window)
		{
			// Store a pointer back to this object in GWL_USERDATA
			SetWindowLong(Window, GWL_USERDATA, (long)this);

			// TODO: Set up the palette from the given Colors
			// TODO: We'll do this in a later article... For now, create a gray wash
			struct
			{
				WORD Version;
				WORD NumberOfEntries;
				PALETTEENTRY aEntries[256];
			} LogPalette =
			{
				0x300, // Palette version
				256    // Number of colors
			};

			for (int i=0; i<256; ++i)
			{
				LogPalette.aEntries[i].peRed = LogPalette.aEntries[i].peGreen =
					LogPalette.aEntries[i].peBlue = i;

				LogPalette.aEntries[i].peFlags = 0;
			}
			Palette = CreatePalette((LOGPALETTE *)&LogPalette);

			// Initialize the Window's DC as necessary
			// Since it's a CS_OWNDC, these settings will last forever
			HDC hdc = GetDC(Window);
			if (hdc)
			{
				SetMapMode(hdc, MM_TEXT);
				SelectPalette(hdc, Palette, FALSE);
				RealizePalette(hdc);

				// This isn't really necessary, but...
				ReleaseDC(Window, hdc);
			}

			// All ready to go, show the window
			ShowWindow(Window, SW_NORMAL);

			// Set up the offscreen environment. Note that because of the
			// way COffscreenBuffer was originally defined, the window
			// must be visible and active
			assert(GetActiveWindow() == Window);
			pOffscreenBuffer = new COffscreenBuffer;
		}
	}
}

CXSplatWindow::~CXSplatWindow(void)
{
	if (pOffscreenBuffer)
		delete pOffscreenBuffer;

	if (Window)
	{
		// Avoid a bad delete/WM_DESTROY loop
		// Reset the pointer back here to avoid confusion
		SetWindowLong(Window, GWL_USERDATA, 0);
		DestroyWindow(Window);
		Window = 0;
	}

	if (Palette)
		DeleteObject(Palette);
}

/****************************************************************************
	The Window Procedure

	This procedure serves as the message dispatcher between the system
	and the CXSplatWindow. All it does is map Windows messages onto
	CXSplatWindow calls and process other messages according to default
	behaviors.
*/

extern int GameRunningFlag;

LONG PASCAL XSplatWindowProc(HWND Window, UINT Message, WPARAM wParam,
	LPARAM lParam)
{
	// You don't really need the pXSplatWindow to process all
	// messages -- most just go through DefWindowProc anyway. If you're
	// concerned, move this into the messages that really care.
	CXSplatWindow *pXSplatWindow =
		(CXSplatWindow *)GetWindowLong(Window, GWL_USERDATA);

	switch(Message)
	{
		case WM_ACTIVATE:
			// Re-realize the palette on activate
			if (pXSplatWindow)
			{
				HDC hdc = GetDC(Window);
				SelectPalette(hdc, pXSplatWindow->Palette, FALSE);
				ReleaseDC(Window, hdc);

				if (LOWORD(wParam) != WA_INACTIVE)
					pXSplatWindow->Activate();
				else
					pXSplatWindow->Deactivate();
			}
			break;

		case WM_CHAR:
			pXSplatWindow->KeyDown((char)wParam, LOWORD(lParam));
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			if (pXSplatWindow)
			{
				// Avoid a bad delete/WM_DESTROY loop
				// Reset the pointer back here to avoid confusion
				pXSplatWindow->Window = 0;
			}
			GameRunningFlag = 0;
			break;

		case WM_LBUTTONDOWN:
			SetCapture(Window);
			if (pXSplatWindow)
				pXSplatWindow->MouseDown(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_LBUTTONUP:
			ReleaseCapture();
			if (pXSplatWindow)
				pXSplatWindow->MouseUp(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_MOUSEMOVE:
			if (pXSplatWindow)
				pXSplatWindow->MouseMove(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_PAINT:
			// Since we've always got a complete picture of the game around
			// in the offscreen buffer, default paint handling poses no
			// problem: copy the offscreen buffer, if any, to the screen

			PAINTSTRUCT Paint;
			HDC hdc;
			hdc = BeginPaint(Window, &Paint);
				
			if (pXSplatWindow)
			{
				COffscreenBuffer *pOffscreen = pXSplatWindow->pOffscreenBuffer;
				if (pOffscreen)
				{
					// Lock and Unlock are no-ops on Windows, but include them
					// for good measure.
					pOffscreen->Lock();
					pOffscreen->SwapBuffer();
					pOffscreen->Unlock();
				}
			}

			EndPaint(Window, &Paint);
			break;
			
		case WM_SIZE:
			// TODO: If you want a resizable game window, you could
			// TODO: process this message and resize the associated
			// TODO: COffscreenBuffer.
			break;

		case WM_GETMINMAXINFO:
			// TODO: If you want to restrict the size of your game,
			// TODO: you could process this message.
			break;
		
		case WM_PALETTECHANGED:
			// Ignore palette changed messages for this window
			if ((HWND)wParam == Window)
				break;
				
			// For others, re-realize palette
		case WM_QUERYNEWPALETTE:
			if (pXSplatWindow)
			{
				HDC hdc = GetDC(Window);
				
				SelectPalette(hdc, pXSplatWindow->Palette, FALSE);
				BOOL Redraw = RealizePalette(hdc);
				ReleaseDC(Window, hdc);
					
				if (Redraw)
					InvalidateRect(Window, 0, FALSE);
					
				return Redraw;
			}
			break;
	}
	
	return DefWindowProc(Window, Message, wParam, lParam);
}

#endif // _WINDOWS
