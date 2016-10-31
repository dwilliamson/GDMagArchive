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
#include <wing.h>

#else // Default to Macintosh

// Metrowerks doesn't let you define symbols outside of files
#define _MACINTOSH

/***************************************************************************
	Macintosh Inclusions
*/

#include <QDOffscreen.h>
#include <Palettes.h>

#endif
