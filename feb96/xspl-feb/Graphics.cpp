/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Graphics support functions

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

#include "XSplat.hpp"
#include "Breakout.hpp"
#include <string.h>

//---------------------------------------------------------------------------
// Rectangles
// NOTE: NONE of these performs any clipping! If you pass bad values,
//       you will crash!

void FillRectangle(COffscreenBuffer *pLockedBuffer,
	int XLeft, int YTop, int Width, int Height, char unsigned Color)
{
	if (pLockedBuffer)
	{
		// Get the pointer to the origin and the line offset
		char unsigned *pBits = pLockedBuffer->GetBits();
		long Stride = pLockedBuffer->GetStride();

		if (pBits && Stride != 0)
		{
			// Advance the pointer to the requested XLeft, YTop
			pBits += Stride * YTop + XLeft;

			// Fill the number of requested lines
			for (int y=0; y<Height; ++y)
			{
				memset((void*)pBits, Color, Width);
				pBits += Stride;
			}
		}
	}
}
