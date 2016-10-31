/*****************************************************************************
	Full-Screen Access Demo
	Windows DirectDraw(tm) implementation

	This code is Copyright (c) 1996 by Jon Blossom. All Rights Reserved.
 ****************************************************************************/

#ifdef _WINDOWS

//****************************************************************************
// System includes

#include <windows.h>
#include <ddraw.h>

//****************************************************************************
// Internal functions

void DemoMain(void);

long unsigned GetMillisecondTime(void)
{
	return timeGetTime();
}

//****************************************************************************
// Internal globals

static HWND Window =0;
static HINSTANCE ghInstance = 0;

static LPDIRECTDRAW pDirectDraw =0;
static LPDIRECTDRAWSURFACE pPrimarySurface =0;
static LPDIRECTDRAWSURFACE pOffscreenSurface =0;
static LPDIRECTDRAWPALETTE pPalette =0;

static int PageFlip =0;

static char unsigned *pBits =0;
static long Stride =0;

//****************************************************************************
// Application entry point

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR, int)
{
	// Store this away for future use
	ghInstance = hInstance;

	// Transfer control to the demo
	DemoMain();

	return 0;
}

//****************************************************************************
// Offscreen access

char unsigned *GetOffscreenBits(void)
{
	return pBits;
}

long GetOffscreenStride(void)
{
	return Stride;
}

int OffscreenLock(void)
{
	int ReturnValue = 0;

	pBits = 0;
	Stride = 0;

	DDSURFACEDESC SurfaceDesc;
	ZeroMemory(&SurfaceDesc, sizeof(SurfaceDesc));
	SurfaceDesc.dwSize = sizeof(SurfaceDesc);

	// Loop until an error occurs or the lock succeeds
	HRESULT DDReturn = DD_OK;
	while (1)
	{
		// Attempt the lock
		DDReturn = pOffscreenSurface->Lock(0, &SurfaceDesc, 0, 0);

		if (DDReturn == DD_OK)
		{
			// Successful lock - store bits and stride
			pBits = (char unsigned *)SurfaceDesc.lpSurface;
			Stride = SurfaceDesc.lPitch;
			ReturnValue = 1;
			break;
		}
		else if (DDReturn == DDERR_SURFACELOST)
		{
			// Attempt to restore the surfaces
			DDReturn = pPrimarySurface->Restore();
			if (DDReturn == DD_OK)
				DDReturn = pOffscreenSurface->Restore();

			if (DDReturn != DD_OK)
			{
				// Surfaces could not be restored - lock fails
				break;
			}
		}
		else if (DDReturn != DDERR_WASSTILLDRAWING)
		{
			// Some other error happened - fail
			break;
		}
	}

	return ReturnValue;
}

int OffscreenUnlock(void)
{
	int ReturnValue = 0;

	// Loop until an error occurs or the unlock succeeds
	HRESULT DDReturn = DD_OK;
	while (1)
	{
		// Attempt the unlock
		DDReturn = pOffscreenSurface->Unlock(0);

		if (DDReturn == DD_OK)
		{
			// Unlock succeeds
			ReturnValue = 1;
			break;
		}
		else if (DDReturn == DDERR_SURFACELOST)
		{
			// Attempt to restore the surfaces
			DDReturn = pPrimarySurface->Restore();
			if (DDReturn == DD_OK)
				DDReturn = pOffscreenSurface->Restore();
			
			if (DDReturn != DD_OK)
			{
				// Surfaces could not be restored - unlock fails
				break;
			}
		}
		else if (DDReturn != DD_OK)
		{
			// Some other error happened - unlock fails
			break;
		}
	}

	return ReturnValue;
}

//****************************************************************************
// Buffer swapping

void SwapBuffer(void)
{
	if (PageFlip)
	{
		// We can use page flipping to draw the surface
		pPrimarySurface->Flip(0, DDFLIP_WAIT);
	}
	else
	{
		// We have to blt the offscreen image to the screen
		RECT BltRect;
		GetClientRect(Window, &BltRect);
		pPrimarySurface->BltFast(0, 0,
			pOffscreenSurface, &BltRect, DDBLTFAST_WAIT);
	}
}

void SwapRect(int Left, int Top, int Right, int Bottom)
{
	// Blt the requested rectangle to the screen
	RECT BltRect = { Left, Top, Right, Bottom };
	pPrimarySurface->BltFast(Left, Top,
		pOffscreenSurface, &BltRect, DDBLTFAST_WAIT);
}

//****************************************************************************
// Full Screen initialization

int BeginFullScreen(int Width, int Height, int Depth)
{
	// Create a Width x Height popup window using the STATIC control
	// class so we don't have to implement a window procedure right now
	Window = CreateWindow("STATIC", "FullScreen", WS_POPUP,
		0, 0, Width, Height,
		0, 0, ghInstance, 0);
	if (!Window)
		goto Failure;
	ShowWindow(Window, SW_SHOWNORMAL);
	UpdateWindow(Window);

	// Connect to DirectDraw, if not already connected
	if (!pDirectDraw)
		DirectDrawCreate(0, &pDirectDraw, 0);
	if (!pDirectDraw)
		goto Failure;

	// Set up DirectDraw for full-screen exclusive mode at the
	// requested resolution
	HRESULT DDReturn;
	DDReturn = pDirectDraw->SetCooperativeLevel(Window,
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (DDReturn != DD_OK)
		goto Failure;

	DDReturn = pDirectDraw->SetDisplayMode(Width, Height, Depth);
	if (DDReturn != DD_OK)
		goto Failure;

	// Create the primary surface
	// Try to get a triple-buffered one we can page flip
	PageFlip = 1;
	DDSURFACEDESC SurfaceDesc;
	ZeroMemory(&SurfaceDesc, sizeof(SurfaceDesc));
	SurfaceDesc.dwSize = sizeof(SurfaceDesc);
	SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	SurfaceDesc.dwBackBufferCount = 2;
	SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
		DDSCAPS_FLIP |
		DDSCAPS_COMPLEX |
		DDSCAPS_VIDEOMEMORY;
	DDReturn = pDirectDraw->CreateSurface(&SurfaceDesc,
		&pPrimarySurface, 0);
	if (DDReturn != DD_OK)
	{
		// We couldn't get a triple buffer, try a double-buffer
		SurfaceDesc.dwBackBufferCount = 1;
		DDReturn = pDirectDraw->CreateSurface(&SurfaceDesc,
			&pPrimarySurface, 0);
		if (DDReturn != DD_OK)
		{
			// We couldn't get a page-flip-able surface at all.
			PageFlip = 0;

			// Just go for a no-frills primary surface
			SurfaceDesc.dwFlags = DDSD_CAPS;
			SurfaceDesc.dwBackBufferCount = 0;
			SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			DDReturn = pDirectDraw->CreateSurface(&SurfaceDesc,
				&pPrimarySurface, 0);
			if (DDReturn != DD_OK)
				goto Failure;
		}
	}

	if (PageFlip)
	{
		// Get the attached page-flip-able surface as the
		// offscreen buffer
		DDSCAPS caps;
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		DDReturn = pPrimarySurface->GetAttachedSurface(&caps,
			&pOffscreenSurface);
	}
	else
	{
		// Create a second surface for the offscreen buffer
		ZeroMemory(&SurfaceDesc, sizeof(SurfaceDesc));
		SurfaceDesc.dwSize = sizeof(SurfaceDesc);
		SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		SurfaceDesc.dwWidth = Width;
		SurfaceDesc.dwHeight = Height;

		DDReturn = pDirectDraw->CreateSurface(&SurfaceDesc,
			&pOffscreenSurface, 0);
	}

	if (DDReturn != DD_OK)
		goto Failure;

	// Set up a palette - a grey wash in 0..255
	PALETTEENTRY PaletteColors[256];
	int i;
	for (i=0; i<256; ++i)
	{
		PaletteColors[i].peRed = i;
		PaletteColors[i].peGreen = i;
		PaletteColors[i].peBlue = i;
		PaletteColors[i].peFlags = PC_RESERVED;
	}

	DDReturn = pDirectDraw->CreatePalette(DDPCAPS_8BIT,
		PaletteColors, &pPalette, 0);
	if (DDReturn != DD_OK)
		goto Failure;

	// Attach the palette to the surface
	DDReturn = pPrimarySurface->SetPalette(pPalette);
	if (DDReturn != DD_OK)
		goto Failure;

	// Success!
	return 1;

Failure:
	return 0;
}

//****************************************************************************
// Full Screen clean-up

void EndFullScreen(void)
{
	// Set us back to the original mode
	if (pDirectDraw)
		pDirectDraw->RestoreDisplayMode();

	if (Window)
	{
		DestroyWindow(Window);
		Window = 0;
	}

	if (pOffscreenSurface && !PageFlip)
	{
		pOffscreenSurface->Release();
		pOffscreenSurface = 0;
	}

	if (pPrimarySurface)
	{
		pPrimarySurface->Release();
		pPrimarySurface = 0;
	}

	if (pPalette)
	{
		pPalette->Release();
		pPalette = 0;
	}

	if (pDirectDraw)
	{
		pDirectDraw->Release();
		pDirectDraw = 0;
	}
}

#endif // _WINDOWS
