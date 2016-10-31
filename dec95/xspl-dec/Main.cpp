/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Main Loop

	Initialization and message processing for any platform

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"
#include <string.h>

// This declares all the important stuff for the Breakout demo
#include "Breakout.hpp"

// GameRunningFlag is reset by the event handling system when the user
// quits the game.
int GameRunningFlag = 1;

void XSplatMain(void)
{
	// TODO: Check system requirements!
	// Assumes monitor is 8-bit palettized

	// Create the user interface and initialize the game
	CBreakoutWindow GameWindow;

	// Loop until termination
	while (GameRunningFlag)
	{
		// Process system events
		DispatchEvents();

		// Make sure a dispatched event didn't end the game
		if (GameRunningFlag)
		{
			// Let the game do its stuff
			GameWindow.Idle();
		}
	}

	// Destruction of GameWindow will clean up the game data, if any
}
