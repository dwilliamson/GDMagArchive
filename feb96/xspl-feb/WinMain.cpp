/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Platform Setup and Message Dispatch

	Windows Version

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

#ifdef _WINDOWS

HINSTANCE ghInstance;

/***************************************************************************
	ClearSystemPalette

	This will force the Palette Manager to start realization of our
	game palette at entry 0.
*/

void ClearSystemPalette(void)
{
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} Palette = { 0x300, 256 };

	// Create an all-black, PC_NOCOLLAPSE palette
	for(int Count = 0; Count < 256; Count++)
	{
		Palette.aEntries[Count].peRed = 0;
		Palette.aEntries[Count].peGreen = 0;
		Palette.aEntries[Count].peBlue = 0;
		Palette.aEntries[Count].peFlags = PC_NOCOLLAPSE;
	}

	// Reset the Palette Manager using this palette
	HDC ScreenDC = GetDC(0);
	if (ScreenDC)
	{
		HPALETTE BlackPalette = CreatePalette((LOGPALETTE *)&Palette);
		if (BlackPalette)
		{
			// Select and realize the all-black palette
			HPALETTE ScreenPalette = SelectPalette(ScreenDC, BlackPalette, FALSE);
			RealizePalette(ScreenDC);

			// Re-select the old palette
			if (ScreenPalette)
				SelectPalette(ScreenDC, ScreenPalette, FALSE);

			DeleteObject(BlackPalette);
		}

		ReleaseDC(0, ScreenDC);
	}
}

/***************************************************************************
	DispatchEvents

	All this is is a simple PeekMessage loop that pulls messages from the
	queue while they're there, then sends control back to the caller.
*/

void DispatchEvents(void)
{
	MSG Message;
	
	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
}

/****************************************************************************
	WinMain

	This just jumps straight into the XSplatMain function, but it could
	handle platform-specific initialization or system checks.
*/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	ghInstance = hInstance;

	ClearSystemPalette();

	XSplatMain();

	return 0;
}

#endif // _WINDOWS
