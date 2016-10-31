/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	General Cross-Platform library declaration

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#ifndef _XSPLAT_HPP_
#define _XSPLAT_HPP_

#include "Target.hpp"
#include "Buffer.hpp"
#include "Window.hpp"
#include <assert.h>

/***************************************************************************
	System-independent loop and event handling
*/

void XSplatMain(void);
void DispatchEvents(void);

#endif // _XSPLAT_HPP_
