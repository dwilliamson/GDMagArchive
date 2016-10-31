/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Double-Buffered Game Window

	Macintosh Version

	The CXSplatWindow constructor creates a standard document window and
	attaches to it a pointer to the CXSplatWindow via SetWRefCon. It also
	creates a COffscreenBuffer to go with it.

	Calling the CXSplatWindow destructor destroys the allocated window.
	"Manually" destroying the window DOES NOT destroy the CXSplatWindow or
	the associated OffscreenBuffer.

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"
#include <string.h>

#ifdef _MACINTOSH

/*****************************************************************************
	Constructor and Destructor
*/

CXSplatWindow::CXSplatWindow(const char *Caption, int WindowWidth,
	int WindowHeight, unsigned char const *Colors) :
	WinPalette(0), Window(0), pOffscreenBuffer(0),
	IsActiveFlag(0), MouseDownFlag(0)
{
	// Determine the screen size
	int ScreenWidth = qd.screenBits.bounds.right - qd.screenBits.bounds.left;
	int ScreenHeight = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top;
	
	// Given a zero or negative dimension, fill the screen
	if (WindowWidth <= 0)
		WindowWidth = ScreenWidth;
	if (WindowHeight <= 0)
		WindowHeight = ScreenHeight;

	// Create the window centered on the screen
	int Left = (ScreenWidth - WindowWidth) / 2;
	int Top = (ScreenHeight - WindowHeight) / 2;

	Rect WindowRect = {Top, Left, Top + WindowHeight, Left + WindowWidth};

	// Convert the caption to a pascal string
	// TODO: This doesn't do well with non-ASCII character sets...
	char unsigned PascalCaption[256];
	strcpy((char *)&PascalCaption[1], Caption);
	PascalCaption[0] = (char unsigned)strlen(Caption);

	// Create a new color document window with these dimensions
	Window = NewCWindow(0, &WindowRect, PascalCaption, TRUE, noGrowDocProc,
		WindowPtr(-1), TRUE, 0);
	if (Window)
	{
		// Store a pointer back to the window
		SetWRefCon(Window, (long)this);
		
		// Set up the palette from the given Colors
		// Create a palette for the window and make sure it will give
		// a 1:1 color mapping with pmExplicit | pmAnimated.
		WinPalette = NewPalette(256, 0, pmExplicit | pmAnimated, 0);
		if (WinPalette)
		{
			// To maintain compatibility with our Windows cousin, we'll
			// include the Windows static colors in the palette
			// Of course, color 0 must be white and color 255 black
			char unsigned const WindowsColorsLow[] =
				{ 0xFF,0xFF,0xFF,
				  0x80,0x00,0x00,  0x00,0x80,0x00,  0x80,0x80,0x00,
				  0x00,0x00,0x80,  0x80,0x00,0x80,  0x00,0x80,0x80,
				  0xC0,0xC0,0xC0,  0xC0,0xDC,0xC0,  0xA6,0xCA,0xF0 };
			char unsigned const WindowsColorsHigh[] =
				{ 0xFF,0xFB,0xF0,  0xA0,0xA0,0xA4,  0x80,0x80,0x80,
				  0xFF,0x00,0x00,  0x00,0xFF,0x00,  0xFF,0xFF,0x00,
				  0x00,0x00,0xFF,  0xFF,0x00,0xFF,  0x00,0xFF,0xFF,
				  0x00,0x00,0x00 };

			// We'll create our palette from these "static" colors, filling
			// in whatever gaps are left with colors from the requested
			// colors, or a gray wash as before if there are none.
			// Count tells us where to find the appropriate RGB
			// triplet in the requested color array
			int Count = 0;
			RGBColor rgb;
			int i;
			for (i=0; i<10; ++i)
			{
				// Fill in the first ten entries
				rgb.red = (long)WindowsColorsLow[Count++] << 8;
				rgb.green = (long)WindowsColorsLow[Count++] << 8;
				rgb.blue = (long)WindowsColorsLow[Count++] << 8;
			
				SetEntryColor(WinPalette, i, &rgb);
			}

			if (Colors)
			{
				// Fill in the middle entries with the requested colors
				for (; i<246; ++i)
				{
					rgb.red = (long)Colors[Count++] << 8;
					rgb.green = (long)Colors[Count++] << 8;
					rgb.blue = (long)Colors[Count++] << 8;
			
					SetEntryColor(WinPalette, i, &rgb);
				}
			}
			else
			{
				// Fill in the middle entries with a grey wash
				for (; i<246; ++i)
				{
					rgb.red = rgb.green = rgb.blue = (long)i << 8;
			
					SetEntryColor(WinPalette, i, &rgb);
				}
			}

			Count = 0;
			for (; i<256; ++i)
			{
				// Fill in the remaining static entries
				rgb.red = (long)WindowsColorsHigh[Count++] << 8;
				rgb.green = (long)WindowsColorsHigh[Count++] << 8;
				rgb.blue = (long)WindowsColorsHigh[Count++] << 8;
			
				SetEntryColor(WinPalette, i, &rgb);
			}

			SetPalette(Window, WinPalette, FALSE);
		}
		
		// Initialize the Window
		SetPort(Window);
		ForeColor(blackColor);
		BackColor(whiteColor);
		PenNormal();
		
		// Set up the offscreen environment. Note that because of the
		// way COffscreenBuffer was originally defined, the window
		// must be visible and active
		pOffscreenBuffer = new COffscreenBuffer;
	}
}

CXSplatWindow::~CXSplatWindow(void)
{
	if (Window)
	{
		// Reset the pointer back here to avoid confusion
		SetWRefCon(Window, 0);
		DisposeWindow(Window);
		Window = 0;
	}

	if (pOffscreenBuffer)
		delete pOffscreenBuffer;

	if (WinPalette)
		DisposePalette(WinPalette);
}

#endif // _MACINTOSH
