// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : copybuf.cpp
// Description : Copies a buffer into a window - for debug
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------



#include "copybuf.hpp"
#include <memory.h>
#include <windows.h>



	typedef struct
	{
		RGBQUAD palette[255];
	} pal;

	typedef struct
	{
		BITMAPINFOHEADER bmiheader;
		pal pale;
	} tbitmapinf;

static HWND thehwnd;


//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void  Hset_copy2videohwnd(int h)
{
	thehwnd = (HWND)h;
}



//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void  copy2video(unsigned short *output, Hrect &src, Hrect &dst, int bpp)
{
BITMAPINFOHEADER ihead;
tbitmapinf tbitinfo;
HDC handle;

 ihead.biSize          = sizeof(BITMAPINFOHEADER);
 ihead.biWidth         = src.right - src.left;
 ihead.biHeight        = - (src.bottom - src.top);
 ihead.biPlanes        = 1;
 ihead.biBitCount      = bpp;
 ihead.biCompression   = BI_RGB;
 ihead.biSizeImage     = 0;
 ihead.biXPelsPerMeter = 1;
 ihead.biYPelsPerMeter = 1;
 ihead.biClrUsed       = 0;
 ihead.biClrImportant  = 0;
 tbitinfo.bmiheader	= ihead;
 

 handle=GetDC(thehwnd);
 

 StretchDIBits( 
	 handle, 
	 dst.left, 
	 dst.top, 
	 (dst.right - dst.left), 
	 (dst.bottom - dst.top), 
	 src.left, 
	 src.top, 
	 (src.right - src.left), 
	 (src.bottom - src.top), 
	 (CONST void *)output, 
	 (CONST BITMAPINFO *)(&tbitinfo), 
	 DIB_RGB_COLORS, 
	 SRCCOPY );

 ReleaseDC(thehwnd,handle);
}
