/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Double-Buffered Game Window

	General UI control functions for any platform

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

/****************************************************************************
	Game Window Default Behavior - nothing for now
*/

void CXSplatWindow::Activate(void)
{
	IsActiveFlag = 1;
}

void CXSplatWindow::Deactivate(void)
{
	IsActiveFlag = 0;
}

void CXSplatWindow::KeyDown(char Key, int RepeatCount)
{
}

void CXSplatWindow::MouseDown(int x, int y)
{
	MouseDownFlag = 1;
}

void CXSplatWindow::MouseUp(int x, int y)
{
	MouseDownFlag = 0;
}

void CXSplatWindow::MouseMove(int x, int y)
{
	if (IsActiveFlag && MouseDownFlag &&
		x >= 0 && y >= 0 &&
		x < pOffscreenBuffer->GetWidth() &&
		y < pOffscreenBuffer->GetHeight())
	{
		pOffscreenBuffer->Lock();

		char unsigned *pPixel = pOffscreenBuffer->GetBits() +
			pOffscreenBuffer->GetStride() * y + x;
		*pPixel = ~(*pPixel);

		pOffscreenBuffer->SwapBuffer();
		pOffscreenBuffer->Unlock();
	}
}
