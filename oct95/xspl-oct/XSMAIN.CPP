/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Main Loop

	Initialization and message processing for any platform

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"
#include <string.h>

// GameRunningFlag is reset by the event handling system when the user
// quits the game.
int GameRunningFlag = 1;

void XSplatMain(void)
{
	// TODO: Check system requirements!
	// Assumes monitor is 8-bit palettized

	// Create a 320x240 window with no specific colors
	CXSplatWindow *pWindow =
		new CXSplatWindow("Test Window", 320, 240);

	if (pWindow)
	{
		// Fill the buffer with the palette in horizontal lines
		COffscreenBuffer *pBuffer = pWindow->GetOffscreenBuffer();
		if (pBuffer)
		{
			pBuffer->Lock();
			char unsigned *pBits = pBuffer->GetBits();
			for (int y=0; y<pBuffer->GetHeight(); ++y)
			{
				memset((void *)pBits, y % 256, pBuffer->GetWidth());
				pBits += pBuffer->GetStride();
			}
			pBuffer->Unlock();
		}

		// Loop until termination
		while (GameRunningFlag)
		{
			DispatchEvents();
		}

		delete pWindow;
	}
}
