/*****************************************************************************
	Full-Screen Access Demo

	This code is Copyright (c) 1996 by Jon Blossom. All Rights Reserved.
 ****************************************************************************/

#include <string.h>

//****************************************************************************
// The simple FullScreen API

int BeginFullScreen(int Width, int Height, int Depth);
void EndFullScreen(void);

int OffscreenLock(void);
char unsigned *GetOffscreenBits(void);
long GetOffscreenStride(void);
int OffscreenUnlock(void);

void SwapBuffer(void);
void SwapRect(int Left, int Top, int Right, int Bottom);

// This is stolen from XSplat
long unsigned GetMillisecondTime(void);

//****************************************************************************
// The cheesy demo
// All this does is set us up for full-screen access, draw a wash
// offscreen, and swap it to the screen.

void DemoMain(void)
{
	if (BeginFullScreen(640, 480, 8))
	{
		// Go for 10 seconds
		char unsigned Color = 0;
		long unsigned Time = GetMillisecondTime();
		while (GetMillisecondTime() - Time < 10000)
		{
			if (OffscreenLock())
			{
				// WARNING: YOU CANNOT USE A DEBUGGER IN BETWEEN
				// LOCK..UNLOCK CALLS WHEN USING DIRECTDRAW!

				// Draw a wash offscreen using memset
				char unsigned *pSurface = GetOffscreenBits();
				long Stride = GetOffscreenStride();

				for (int i=0; i<480; ++i)
				{
					memset(pSurface, (Color + i)%256, 640);
					pSurface += Stride;
				}

				OffscreenUnlock();
				// OK TO USE A DEBUGGER NOW

				// Next time through we'll use a different color
				// (Allow this to overflow back to zero)
				++Color;
			}

			SwapBuffer();
		}
	}

	// Undo everything done by BeginFullScreen.
	// Even if BeginFullScreen failed, it may have partially succeeded
	// and may require cleaning up
	EndFullScreen();
}
