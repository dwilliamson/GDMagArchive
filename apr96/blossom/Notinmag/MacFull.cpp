/*****************************************************************************
	Full-Screen Access Demo
	Macintosh DisplayManager 2.0 implementation

	This code is Copyright (c) 1996 by Jon Blossom. All Rights Reserved.
 ****************************************************************************/

#ifndef _WINDOWS

//****************************************************************************
// System includes

#include <QDOffscreen.h>
#include <Video.h>
#include <Displays.h>
#include <Palettes.h>
#include <Timer.h>

//****************************************************************************
// Internal functions

void StealMenuBar(void);
void RestoreMenuBar(void);

void DemoMain(void);

long unsigned GetMillisecondTime(void)
{
	// Get a microsecond-accurate time from the Time Manager
	UnsignedWide Microsecs;
	Microseconds(&Microsecs);

	// TODO: Do this right! And don't use doubles!
	double Time;
	Time = (double)Microsecs.hi * (double)0xFFFFFFFF;
	Time = Time + (double)Microsecs.lo;
	Time = Time / 1000.0;
	while (Time > (double)0xFFFFFFFF)
		Time = Time - (double)0xFFFFFFFF;
	return (long unsigned)Time;
}

//****************************************************************************
// Internal globals

static WindowPtr Window =0;
static PaletteHandle hPalette =0;
static GWorldPtr pOffscreenGWorld =0;
static GDHandle DisplayDevice;

static unsigned short csPreviousMode;
static unsigned long csPreviousData;

static char unsigned *pBits =0;
static long Stride =0;

//****************************************************************************
// Application entry point

int main()
{
	// Initialize the Mac
	MaxApplZone();
	MoreMasters();
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs(0L);
	InitCursor();

	// Check for Display Manager using Gestalt
	long GestaltValue = 0;
	long DisplayManagerVersion = 0;
	Gestalt(gestaltDisplayMgrAttr, &GestaltValue);
	if (GestaltValue & (1<<gestaltDisplayMgrPresent))
		Gestalt(gestaltDisplayMgrVers, &DisplayManagerVersion);

	// If Display Manager 2.x is available, proceed
	if (DisplayManagerVersion >= 0x00020000)
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

	// Get the PixMap structure of the offscreen GWorld
	PixMapHandle OffscreenPixMap = GetGWorldPixMap(pOffscreenGWorld);
	if (OffscreenPixMap)
	{
		// Lock the PixMap memory and pull info off the PixMap
		LockPixels(OffscreenPixMap);
		pBits = (char unsigned *)GetPixBaseAddr(OffscreenPixMap);
		Stride = (*OffscreenPixMap)->rowBytes & 0x3FFF;

		ReturnValue = 1;
	}

	return ReturnValue;
}

int OffscreenUnlock(void)
{
	int ReturnValue = 0;

	// Get the PixMap structure of the offscreen GWorld
	PixMapHandle OffscreenPixMap = GetGWorldPixMap(pOffscreenGWorld);
	if (OffscreenPixMap)
	{
		// Unlock the offscreen PixMap and reset the bits and stride
		UnlockPixels(OffscreenPixMap);
		pBits = 0;
		Stride = 0;

		ReturnValue = 1;
	}

	return ReturnValue;
}

//****************************************************************************
// Buffer swapping

void SwapBuffer(void)
{
	// Make sure ctSeed matches so we get a 1:1 blt, even though we
	// have set up everything correctly when creating the GWorld in the
	// first place.
	// Basically, if QuickDraw knows the color tables are the same
	// it will ignore any translation and just copy the bits. This
	// is an extra trick to make sure it's satisfied.
	PixMapHandle OffscreenPixMap = pOffscreenGWorld->portPixMap;
	CTabHandle OffscreenCtab = (*OffscreenPixMap)->pmTable;

	PixMapHandle OnscreenPixMap = ((CWindowPtr)Window)->portPixMap;
	CTabHandle OnscreenCtab = (*OnscreenPixMap)->pmTable;

	(*OffscreenCtab)->ctSeed = (*OnscreenCtab)->ctSeed;

	// Do the blt
	CopyBits(&((GrafPtr)pOffscreenGWorld)->portBits,
		&Window->portBits,
		&Window->portRect, &Window->portRect,
		srcCopy, 0);
}

void SwapRect(int Left, int Top, int Right, int Bottom)
{
	// Make sure ctSeed matches so we get a 1:1 blt, as above
	PixMapHandle OffscreenPixMap = pOffscreenGWorld->portPixMap;
	CTabHandle OffscreenCtab = (*OffscreenPixMap)->pmTable;

	PixMapHandle OnscreenPixMap = ((CWindowPtr)Window)->portPixMap;
	CTabHandle OnscreenCtab = (*OnscreenPixMap)->pmTable;

	(*OffscreenCtab)->ctSeed = (*OnscreenCtab)->ctSeed;

	Rect BltRect = { Top, Left, Bottom, Right };

	// Do the blt
	CopyBits(&((GrafPtr)pOffscreenGWorld)->portBits,
		&Window->portBits,
		&BltRect, &BltRect,
		srcCopy, 0);
}

//****************************************************************************
// Full Screen initialization

// This structure encapsulates the data sent to the Display Manager list
// enumeration callback function. We fill in the desired values, pass this
// on through the enumeration, and it fills in the csMode and csData info
// we need.
struct DisplayModeRequest
{
	// Returned values
	unsigned short csMode;
	unsigned long csData;

	// Provided values
	long DesiredWidth;
	long DesiredHeight;
	long DesiredDepth;
};

// This function filters through the display modes reported by the Display
// Manager, looking for one that matches the requested resolution. The
// userData pointer will point to a DisplayModeRequest structure.
pascal void DisplayModeCallback(void* userData, DMListIndexType,
	DMDisplayModeListEntryPtr pModeInfo)
{
	DisplayModeRequest *pRequest = (DisplayModeRequest*)userData;

	// Get timing info and make sure this is an OK display mode
	VDTimingInfoRec TimingInfo = *(pModeInfo->displayModeTimingInfo);
	if (TimingInfo.csTimingFlags & 1<<kModeValid)
	{
		// How many modes are being enumerated here?
		unsigned long DepthCount =
			pModeInfo->displayModeDepthBlockInfo->depthBlockCount;

		// Filter through each of the modes provided here
		VDSwitchInfoRec *pSwitchInfo;
		VPBlock *pVPBlockInfo;
		for (short Count = 0; Count < DepthCount; ++Count)
		{
			// This provides the csMode and csData information
			pSwitchInfo =
				pModeInfo->displayModeDepthBlockInfo->
				depthVPBlock[Count].depthSwitchInfo;

			// This tells us the resolution and pixel depth
			pVPBlockInfo =
				pModeInfo->displayModeDepthBlockInfo->
				depthVPBlock[Count].depthVPBlock;

			if (pVPBlockInfo->vpPixelSize == pRequest->DesiredDepth &&
				pVPBlockInfo->vpBounds.right == pRequest->DesiredWidth &&
				pVPBlockInfo->vpBounds.bottom == pRequest->DesiredHeight)
			{
				// Found a mode that matches the request!
				pRequest->csMode = pSwitchInfo->csMode;
				pRequest->csData = pSwitchInfo->csData;
			}
		}
	}
}

int BeginFullScreen(int Width, int Height, int Depth)
{
	// Hide the menu bar
	StealMenuBar();

	// Create a window of the requested size
	Rect WindowRect = { 0, 0, Height, Width };
	Window = NewCWindow(0, &WindowRect, "\pFullScreen",
		TRUE, plainDBox,
		WindowPtr(-1), FALSE, 0);
	if (!Window)
		goto Failure;

	// Set up a palette with a gray wash
	hPalette = NewPalette(256, 0, pmExplicit | pmAnimated, 0);
	if (!hPalette)
		goto Failure;

	RGBColor Color;
	int i;
	for (i=0; i<256; ++i)
	{
		Color.red= i << 8;
		Color.green = i << 8;
		Color.blue = i << 8;

		SetEntryColor(hPalette, i, &Color);
	}

	// Force 0 and 255 to White and Black
	SetEntryUsage(hPalette, 0, pmExplicit | pmAnimated | pmWhite, 0);
	SetEntryUsage(hPalette, 255, pmExplicit | pmAnimated | pmBlack, 0);

	SetPalette(Window, hPalette, TRUE);

	// Store information about the current display settings
	DisplayDevice = GetMainDevice();
	if (!DisplayDevice)
		goto Failure;

	csPreviousMode = -1;
	csPreviousData = -1;

	VDSwitchInfoRec DisplayInfo;
	OSErr MacError = DMGetDisplayMode(DisplayDevice, &DisplayInfo);
	if (MacError != noErr)
		goto Failure;

	csPreviousMode = DisplayInfo.csMode;
	csPreviousData = DisplayInfo.csData;

	// Get the display ID for the main display
	DisplayIDType DisplayID;
	DMGetDisplayIDByGDevice(DisplayDevice, &DisplayID, FALSE);

	// Use it to get a list of available modes from the Display Manager
	DMListIndexType DisplayModeCount = 0;
	DMListType DisplayModeList;
	MacError = DMNewDisplayModeList(DisplayID, 0, 0,
		&DisplayModeCount, &DisplayModeList);
	if (MacError != noErr)
		goto Failure;

	// Create a callback function pointer to filter available modes
	DMDisplayModeListIteratorUPP uppDisplayModeCallback =
		NewDMDisplayModeListIteratorProc(DisplayModeCallback);
	if (!uppDisplayModeCallback)
	{
		// Aborting - let go of the mode list
		DMDisposeList(DisplayModeList);
		goto Failure;
	}

	// Go through the list, comparing each available mode with
	// this mode request
	DisplayModeRequest ModeRequest;
	ModeRequest.csMode = -1;
	ModeRequest.csData = -1;
	ModeRequest.DesiredWidth = Width;
	ModeRequest.DesiredHeight = Height;
	ModeRequest.DesiredDepth = Depth;

	for (short Count = 0; Count < DisplayModeCount; ++Count)
	{
		DMGetIndexedDisplayModeFromList(DisplayModeList, Count,
			0, uppDisplayModeCallback, (void*)&ModeRequest);
	}

	// Done with the list
	DMDisposeList(DisplayModeList);

	// Done with the callback
	DisposeRoutineDescriptor(uppDisplayModeCallback);

	// If we found a mode fitting the request, switch to it!
	if (ModeRequest.csMode == -1 || ModeRequest.csData == -1)
		goto Failure;

	DisplayInfo.csMode = ModeRequest.csMode;
	DisplayInfo.csData = ModeRequest.csData;

	unsigned long Mode = DisplayInfo.csMode;
	MacError = DMSetDisplayMode(DisplayDevice,
		DisplayInfo.csData, &Mode,
		(unsigned long)&DisplayInfo,
		0);
	if (MacError != noErr)
		goto Failure;

	// Create a matching GWorld using current device and window
	CGrafPtr CurrentPort = (CWindowPtr)Window;
	
	PixMapHandle CurrentPixMap = CurrentPort->portPixMap;
	CTabHandle ColorTable = (*CurrentPixMap)->pmTable;

	NewGWorld(&pOffscreenGWorld, (short)Depth,
		&CurrentPort->portRect, ColorTable,
		DisplayDevice, noNewDevice);
	if (!pOffscreenGWorld)
		goto Failure;

	// Success!
	return 1;

Failure:
	RestoreMenuBar();
	return 0;
}

//****************************************************************************
// Full Screen clean-up

void EndFullScreen(void)
{
	if (Window)
	{
		DisposeWindow(Window);
		Window = 0;
	}
	
	if (pOffscreenGWorld)
	{
		DisposeGWorld(pOffscreenGWorld);
		pOffscreenGWorld = 0;
	}

	if (csPreviousMode != -1 && csPreviousData != -1)
	{
		// Set the display back to its starting place
		VDSwitchInfoRec OriginalInfo;
		OriginalInfo.csMode = csPreviousMode;
		OriginalInfo.csData = csPreviousData;

		unsigned long Mode = csPreviousMode;
		DMSetDisplayMode(DisplayDevice,
			csPreviousData, &Mode,
			(unsigned long)&OriginalInfo,
			0);
	}

	RestoreMenuBar();
}

//****************************************************************************
// Menu bar manipulation functions

static short _OldMBarHeight = 0;
static RgnHandle _OldDesktopRgn = 0;

void StealMenuBar(void)
{
	RgnHandle DesktopRgn;

	if (_OldMBarHeight == 0)
	{
		// Get and copy the current gray region
		DesktopRgn = LMGetGrayRgn();
		_OldDesktopRgn = NewRgn();
		CopyRgn(DesktopRgn, _OldDesktopRgn);
		
		// Fudge the menu bar height
		_OldMBarHeight = GetMBarHeight();
		LMSetMBarHeight(0);

		// Turn the gray into the old gray region plus the menu bar region
		Rect MenuRect;
		MenuRect.left = 0;
		MenuRect.top = 0;
		MenuRect.right = qd.screenBits.bounds.right;
		MenuRect.bottom = _OldMBarHeight;
		RgnHandle MenuRgn = NewRgn();
		RectRgn(MenuRgn, &MenuRect);

		UnionRgn(_OldDesktopRgn, MenuRgn, DesktopRgn);
		DisposeRgn(MenuRgn);
	}
}

void RestoreMenuBar(void)
{
	if (_OldMBarHeight && _OldDesktopRgn)
	{
		// Restore the menu bar height
		LMSetMBarHeight(_OldMBarHeight);
		_OldMBarHeight = 0;

		// Restore the old desktop region
		CopyRgn(_OldDesktopRgn, LMGetGrayRgn());
		DisposeRgn(_OldDesktopRgn);
		_OldDesktopRgn = 0;
		
		// Redraw the menu bar
		HiliteMenu(0);
		DrawMenuBar();
	}
}

#endif // not _WINDOWS
