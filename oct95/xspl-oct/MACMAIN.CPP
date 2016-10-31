/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Platform Setup

	Macintosh Version

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"

#ifdef _MACINTOSH

/****************************************************************************
	main

	This just initializes the Toolbox and jumpst into the XSplatMain
	function, but it could also perform system checks.
*/

main()
{
	// Set up for memory allocation
	MaxApplZone();
	MoreMasters();

	// Set up the toolbox
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitCursor();

	// Set up a menu bar with nothing on it but the Apple menu
	Handle menuBar = GetNewMBar(128);
	if (menuBar)
	{
		SetMenuBar(menuBar);
		AddResMenu(GetMHandle(128), 'DRVR');
		DrawMenuBar();
		DisposHandle(menuBar);
	}
	
	// Play the game!
	XSplatMain();

	return 0;
}

#endif // _MACINTOSH
