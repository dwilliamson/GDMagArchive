/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Target platform management

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

/***************************************************************************
	Windows Inclusions
*/

#if defined(_WINDOWS)

#include <windows.h>


// By default, the Windows version will use CreateDIBSection instead
// of WinG. If you want it to use WinG instead, set USE_WinG to 1
// and be sure to link with wing32.lib
#define USE_WinG 0

#if USE_WinG
#include <wing.h>
#endif

#else // Default to Macintosh

// Metrowerks doesn't let you define symbols outside of files
#define _MACINTOSH

/***************************************************************************
	Macintosh Inclusions
*/

#include <QDOffscreen.h>
#include <Palettes.h>

#endif
