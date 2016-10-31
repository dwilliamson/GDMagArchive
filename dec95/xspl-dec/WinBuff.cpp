/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Double Buffering

	Windows Version

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

#ifdef _WINDOWS

/***************************************************************************
	COffscreenBuffer constructor and destructor
*/

// This is here to keep it off the stack...
struct {
	BITMAPINFOHEADER Header;
	RGBQUAD          ColorTable[256];
} BufferInfo;

COffscreenBuffer::COffscreenBuffer(void) :
	pBits(0), OffscreenDC(0), OffscreenBitmap(0), TargetWindow(0)
{
	TargetWindow = GetActiveWindow();

	// Make the buffer the same size as the active window
	RECT ClientRect;
	GetClientRect(TargetWindow, &ClientRect);
	Width = ClientRect.right - ClientRect.left;
	Height = ClientRect.bottom - ClientRect.top;
	Stride = ((long)Width + 3) & (~3);

	#if !USE_WinG
		// Set up the header for a DIBSection
		BufferInfo.Header.biHeight = 1;
		BufferInfo.Header.biSize = sizeof(BITMAPINFOHEADER);
		BufferInfo.Header.biPlanes = 1;
		BufferInfo.Header.biBitCount = 8;
		BufferInfo.Header.biCompression = BI_RGB;
	#else
		// Set up the header for a WinGBitmap
		// (Making the following {...} clause part of an if statement)
		if (WinGRecommendDIBFormat((LPBITMAPINFO)&BufferInfo))
	#endif // USE_WinG
	{
		// Preserve sign on biHeight for appropriate orientation
		BufferInfo.Header.biWidth = Width;
		BufferInfo.Header.biHeight *= Height;

		// Grab the color entries from the current palette
		HDC hdcScreen = GetDC(TargetWindow);
		if (hdcScreen)
		{
			PALETTEENTRY Palette[256];
			GetSystemPaletteEntries(hdcScreen, 0, 256, Palette);

			ReleaseDC(TargetWindow, hdcScreen);

			// Convert the palette entries into RGBQUADs for the color table
			for (int i=0; i<256; ++i)
			{
				BufferInfo.ColorTable[i].rgbRed = Palette[i].peRed;
				BufferInfo.ColorTable[i].rgbGreen = Palette[i].peGreen;
				BufferInfo.ColorTable[i].rgbBlue = Palette[i].peBlue;
				BufferInfo.ColorTable[i].rgbReserved = 0;
			}
		}

		// Create the offscreen DC - either a CompatibleDC or a WinGDC
		#if !USE_WinG
			OffscreenDC = CreateCompatibleDC(GetDC(0));
		#else
			OffscreenDC = WinGCreateDC();
		#endif // USE_WinG

		if (OffscreenDC)
		{
			void *pVoidBits;

			// Create the offscreen bitmap
			// Either a DIBSection or a WinGBitmap
			#if !USE_WinG
				OffscreenBitmap = CreateDIBSection(OffscreenDC,
					(LPBITMAPINFO)&BufferInfo,
					DIB_RGB_COLORS, &pVoidBits, 0, 0);
			#else
				OffscreenBitmap = WinGCreateBitmap(OffscreenDC,
					(LPBITMAPINFO)&BufferInfo, &pVoidBits);
			#endif // USE_WinG

			pBits = (char unsigned *)pVoidBits;

			if (OffscreenBitmap)
			{
				// Adjust pBits and Stride for bottom-up DIBs
				if (BufferInfo.Header.biHeight > 0)
				{
					pBits = pBits + Stride * (Height - 1);
					Stride = -Stride;
				}

				// Prepare the WinGDC/WinGBitmap
				OriginalMonoBitmap = (HBITMAP)SelectObject(OffscreenDC,
					OffscreenBitmap);
			}
			else
			{
				// Clean up in case of error
				DeleteDC(OffscreenDC);
				OffscreenDC = 0;
			}
		}
	}
}

COffscreenBuffer::~COffscreenBuffer(void)
{
	// Delete the offscreen bitmap, selecting back in the original bitmap
	if (OffscreenDC && OffscreenBitmap)
	{
		SelectObject(OffscreenDC, OriginalMonoBitmap);
		DeleteObject(OffscreenBitmap);
	}

	// Delete the offscreen device context
	if (OffscreenDC)
		DeleteDC(OffscreenDC);
}

/***************************************************************************
	Buffer swapping
*/

void COffscreenBuffer::SwapBuffer(void) const
{
	// Get the DC of the active window
	// TODO: Since we know the target window is CS_OWNDC, we could always
	// TODO: just store the DC as part of COffscreenBuffer...
	HDC ActiveDC = GetDC(TargetWindow);

	// Perform the blt!
	// Use either BitBlt on a DIBSection or WinGBitBlt on a WinGDC
	#if !USE_WinG
		BitBlt(ActiveDC, 0, 0, Width, Height, OffscreenDC, 0, 0, SRCCOPY);
	#else
		WinGBitBlt(ActiveDC, 0, 0, Width, Height, OffscreenDC, 0, 0);
	#endif // USE_WinG

	ReleaseDC(TargetWindow, ActiveDC);
}

#endif // _WINDOWS
