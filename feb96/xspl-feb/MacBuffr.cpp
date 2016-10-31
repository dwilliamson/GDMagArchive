/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Double Buffering

	Macintosh Version

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

#ifdef _MACINTOSH

/***************************************************************************
	COffscreenBuffer constructor and destructor
*/

COffscreenBuffer::COffscreenBuffer(void)
{
	// Use the current GDevice and GrafPort to make a GWorld
	CGrafPtr CurrentPort;
	GDHandle CurrentDevice;
	GetGWorld(&CurrentPort, &CurrentDevice);

	// Get the color table from the current port
	PixMapHandle CurrentPixMap = CurrentPort->portPixMap;
	HLock((Handle)CurrentPixMap);
	CTabHandle ColorTable = (*CurrentPixMap)->pmTable;

	// Create a new GWorld with this information
	NewGWorld(&OffscreenGWorld, 8, &CurrentPort->portRect, ColorTable,
		CurrentDevice, noNewDevice);

	// Store data that doesn't change
	Width = CurrentPort->portRect.right - CurrentPort->portRect.left;
	Height = CurrentPort->portRect.bottom - CurrentPort->portRect.top;

	// Release the current PixMap
	HUnlock((Handle)CurrentPixMap);
}

COffscreenBuffer::~COffscreenBuffer(void)
{
	// Free the allocated GWorld
	if (OffscreenGWorld)
		DisposeGWorld(OffscreenGWorld);
}

/***************************************************************************
	Buffer Lock/Unlock
*/

void COffscreenBuffer::Lock(void)
{
	PixMapHandle OffscreenPixMap = GetGWorldPixMap(OffscreenGWorld);
	if (OffscreenPixMap)
	{
		// Lock the PixMap memory and pull some info off the PixMap structure
		LockPixels(OffscreenPixMap);
		Stride = (*OffscreenPixMap)->rowBytes & 0x3FFF;
		pBits = (char unsigned *)GetPixBaseAddr(OffscreenPixMap);
	}
}

void COffscreenBuffer::Unlock(void)
{
	PixMapHandle OffscreenPixMap = GetGWorldPixMap(OffscreenGWorld);
	if (OffscreenPixMap)
	{
		// Unlock the PixMap memory and reset Stride and pBits
		UnlockPixels(OffscreenPixMap);
		Stride = 0;
		pBits = 0;
	}
}

/***************************************************************************
	Buffer swapping
*/

void COffscreenBuffer::SwapRect(int Left, int Top, int Right, int Bottom) const
{
	// Copy bits from the offscreen GWorld to the active GrafPort
	// CopyBits uses rectangles to indicate the copy rectangles
	// Mac rects are not bottom-right inclusive
	Rect CopyRect;
	CopyRect.left = Left;
	CopyRect.top = Top;
	CopyRect.right = Right;
	CopyRect.bottom = Bottom;

	// Note: The offscreen GWorld should be locked!
	CopyBits(&((GrafPtr)OffscreenGWorld)->portBits,
		&((GrafPtr)qd.thePort)->portBits,
		&CopyRect, &CopyRect,
		srcCopy, qd.thePort->visRgn);
}

#endif // _MACINTOSH
