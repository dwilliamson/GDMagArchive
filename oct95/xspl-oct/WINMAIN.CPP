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

	XSplatMain();

	return 0;
}

#endif // _WINDOWS
