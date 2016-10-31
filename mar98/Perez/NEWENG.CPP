/*
source for testing application to use the classes

neweng.cpp


NOTES:

*if you notice any simlarities to this code and to that of FlyBy, a demo whose 
 source was released to the public, that's because this was my first real Win32
 project and I needed some help initially to get going.  Really only the file
 class and some of the windows init stuff is from flyby, the rest is my own work
 (except for the dib code of course)

*you'll probably notice a large level of basically unnecessary abstraction, well
 that's because this is a 4000+ project i'm working on, I cut almost all of it
 out to make this demo, it's in there to help me along.  (for anyone concerned
 about speed hits of all this abstraction, all profiles i've done have my non-
 raster code taking up like 1% of my processing time, it's not worth the time
 to optimize)

*there's some jerkiness in the controls, you'll probably notice, as in the 
 collision detection.  I don't know how to fix this problem, i think it's that
 whole messages-handled-in-a-different-thread action.  that's what i get for
 not using DirectInput, thanks Big-M.

*if you also notice any simliarities to the Abrash DDJ code 'clip.c' well you 
 get the picture pages award for the day;  I'm not too astute a windows 
 programmer and just used what abrash did, mostly because it worked beautifully.
 "One good thief is worth more than 10 good programmers"

*you may wonder why i use min(xscale,yscale) instead of max... this it what
 causes that weird fov>120 action when you distort the window... I did this 
 just so the overhead view wouldn't crash, with max, the unclipped overhead 
 polys would be drawn outside the DIB, which is A Bad Thing.

*what?  this code is slow and unoptimized?  in fact, seeing the code makes you
 gag?  well TOUGH! :)  it's just there to show you what's happening

*texture mapping would be nice, the added code complexity wouldn't be. 

*/

#include <crtdbg.h>
#include "neweng.h"
#include "Zinit.h"
#include "Zrender.h"
#include "glide.h"
#include "newengg.h"
#include "newengh.h"
#include "zportal.h"

LPNewEng _app;
GFloat t_adj;
extern float sinlook[32767],coslook[32767];
extern void BuildLookupTables();
LPZPortal _portal;
plane_t plane;
poly_t poly;
ZPfrus_t frus;
int result;
int done;
float delta;
HANDLE hAccelTable;

//the following is for the dib, straight from clip.c :)
#define INITIAL_DIB_WIDTH  	320		// initial dimensions of DIB
#define INITIAL_DIB_HEIGHT	240		//  into which we'll draw
#define MAX_SCREEN_HEIGHT   2048
BITMAPINFO *pbmiDIB;		// pointer to the BITMAPINFO
char *pDIB, *pDIBBase;		// pointers to DIB section we'll draw into
HBITMAP hDIBSection;        // handle of DIB section
HINSTANCE hInst;            // current instance
char szAppName[] = "Portal Demo";  // The name of this application
char szTitle[]   = "Portal Demo"; // The title bar text
HPALETTE hpalold, hpalDIB;
HWND hwndOutput;
int DIBWidth, DIBHeight;
int DIBPitch;
double  roll, pitch, yaw;
double  currentspeed;
double  fieldofview, xcenter, ycenter;
double  xscreenscale, yscreenscale, maxscale;


/*/////////////////////////////////////////////////////////////////////
WinMain

main windows function
/////////////////////////////////////////////////////////////////////*/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	_app = new NewEng(hInstance,hPrevInstance,lpCmdLine,nCmdShow); // Init app class
	_cam = new ZCamera(2.0,800);
	_portal = new ZPortal();
	_cam->Place(5.0f,0.0f,0.0f);
	_cam->InitRot(0,0,0);
	_portal->LoadB2("3rooms.b2");
	_portal->flag=0;
	ZInit::OpenGEngine(G_HRESOLUTION, G_WRESOLUTION);
	_app->Run();
	ZInit::CloseGEngine();
	delete _cam;
	delete _app;
	return NULL; // return to windows shell
}

/*/////////////////////////////////////////////////////////////////////
InitApplication

typical ugly windows code
/////////////////////////////////////////////////////////////////////*/

BOOL InitApplication(HINSTANCE hInstance)
{
        WNDCLASS  wc;

        // Fill in window class structure with parameters that
        // describe the main window.
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = (WNDPROC)_app->MainWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon (hInstance, szAppName);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = szAppName;
        wc.lpszClassName = szAppName;

        // Register the window class and return success/failure code.
        return (RegisterClass(&wc));
}

/*/////////////////////////////////////////////////////////////////////
InitInstance

considerably uglier windows code
/////////////////////////////////////////////////////////////////////*/

BOOL InitInstance(
        HINSTANCE       hInstance,
        int             nCmdShow)
{
        HWND            hwnd; // Main window handle.
		HDC				hdc;
		INT				i, j, k;
		LOGPALETTE *	plogpal;
		PUSHORT 		pusTemp;
		HPALETTE		hpal;
		RECT			rctmp;
		int				screenx, screeny;

        // Save the instance handle in static variable, which will be
        // used in many subsequent calls from this application to
        // Windows
        hInst = hInstance; // Store inst handle in our global variable

        // Create a main window for this application instance
		DIBWidth = INITIAL_DIB_WIDTH;
		DIBHeight = INITIAL_DIB_HEIGHT;
	   	rctmp.left = 0;
		rctmp.top = 0;
		rctmp.right = DIBWidth;
		rctmp.bottom = DIBHeight;
   
	  	AdjustWindowRect(&rctmp, WS_OVERLAPPEDWINDOW, 1);

		screenx = GetSystemMetrics(SM_CXSCREEN);
		screeny = GetSystemMetrics(SM_CYSCREEN);

        hwnd = CreateWindow(
                szAppName,           // See RegisterClass() call.
                szTitle,             // Text for window title bar.
                WS_OVERLAPPEDWINDOW,// Window style.
                screenx - (rctmp.right - rctmp.left),
                screeny - (rctmp.bottom - rctmp.top)-40,
				rctmp.right - rctmp.left,
				rctmp.bottom - rctmp.top,
                NULL,
                NULL,
                hInstance,
                NULL
        );

        // If window could not be created, return "failure"
        if (!hwnd) {
        	return (FALSE);
        }

		// Make the window visible and draw it
        ShowWindow(hwnd, nCmdShow); // Show the window
        UpdateWindow(hwnd);         // Sends WM_PAINT message

        hdc = GetDC(hwnd);

        // For 256-color mode, set up the palette for maximum speed
        // in copying to the screen. If not a 256-color mode, the
        // adapter isn't palettized, so we'll just use the default
        // palette. However, we will run a lot slower in this case
        // due to translation while copying
        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) {
            // This is a 256-color palettized mode.
    		// Set up and realize our palette and the identity color
    		// table for the DIB (identity means that DIB color
            // indices and screen palette indices match exactly, so
            // GDI doesn't have to do any translation when copying
            // from DIB to screen. This helps performance a lot)
		    plogpal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) +
	    	        256 * sizeof(PALETTEENTRY));

    		if (plogpal == NULL) {
		    	return(FALSE);
    		}

	    	// Take up all physical palette entries, to flush out
	    	// anything that's currently in the palette
	    	plogpal->palVersion = 0x300;
		    plogpal->palNumEntries = 236;
	    	for (i=0; i<236; i++) {
		    	plogpal->palPalEntry[i].peRed = i;
			    plogpal->palPalEntry[i].peGreen = 0;
    			plogpal->palPalEntry[i].peBlue = 0;
	    		plogpal->palPalEntry[i].peFlags =
		    	        PC_RESERVED | PC_NOCOLLAPSE;
    		}

	    	hpal = CreatePalette(plogpal);

    		if (hpal == 0) {
		    	return(FALSE);
    		}

		    hpalold = SelectPalette(hdc, hpal, FALSE);

    		if (hpalold == 0) {
		    	return(FALSE);
    		}

    		if (RealizePalette(hdc) != plogpal->palNumEntries) {
			    return(FALSE);
		    }

    		if (SelectPalette(hdc, hpalold, FALSE) == 0) {
		    	return(FALSE);
    		}

	    	if (!DeleteObject(hpal)) {
			    return(FALSE);
    		}

		    // Now set up the 6value-6value-6value RGB palette,
		    // followed by 20 gray levels, that we want to work with
		    // into the physical palette
	    	for (i=0; i<6; i++) {
			    for (j=0; j<6; j++) {
	    			for (k=0; k<6; k++) {
			    		plogpal->palPalEntry[i*36+j*6+k].peRed =
                                i*255/6;
					    plogpal->palPalEntry[i*36+j*6+k].peGreen =
					            j*255/6;
    					plogpal->palPalEntry[i*36+j*6+k].peBlue =
	   				            k*255/6;
	    				plogpal->palPalEntry[i*36+j*6+k].peFlags =
					        PC_RESERVED | PC_NOCOLLAPSE;
		    		}
    			}
	    	}

		    for (i=0; i<20; i++) {
    			plogpal->palPalEntry[i+216].peRed = i*255/20;
	    		plogpal->palPalEntry[i+216].peGreen = i*255/20;
			    plogpal->palPalEntry[i+216].peBlue = i*255/20;
    			plogpal->palPalEntry[i+216].peFlags =
	    		        PC_RESERVED | PC_NOCOLLAPSE;
		    }

    		hpal = CreatePalette(plogpal);

	    	if (hpal == 0) {
			    return(FALSE);
    		}

	    	if (SelectPalette(hdc, hpal, FALSE) == 0) {
			    return(FALSE);
    		}

	    	if (RealizePalette(hdc) != plogpal->palNumEntries) {
			    return(FALSE);
    		}

		    // Get back the 256 colors now in the physical palette,
		    // which are the 236 we just selected, plus the 20 static
		    // colors
    		if (GetSystemPaletteEntries(hdc, 0, 256,
    		        plogpal->palPalEntry) != 256) {
	    		return(FALSE);
		    }

    		for (i=10; i<246; i++) {
		    	plogpal->palPalEntry[i].peFlags =
			            PC_RESERVED | PC_NOCOLLAPSE;
    		}

	    	// Now create a logical palette that exactly matches the
		    // physical palette, and realize it. This is the palette
    		// into which the DIB pixel values will be indices
    		plogpal->palNumEntries = 256;

	    	hpalDIB = CreatePalette(plogpal);

    		if (hpalDIB == 0)
		    	return(FALSE);

	    	if (SelectPalette(hdc, hpalDIB, FALSE) == 0)
			    return(FALSE);

	    	DeleteObject(hpal);

    		if (RealizePalette(hdc) != plogpal->palNumEntries)
		    	return(FALSE);

	    	if (SelectPalette(hdc, hpalold, FALSE) == FALSE)
			    return(FALSE);

       		free(plogpal);
        }

		// Finally, set up the DIB section
		pbmiDIB = (BITMAPINFO *) malloc(sizeof(BITMAPINFO) - 4 + 256*sizeof(USHORT));

		if (pbmiDIB == NULL)
			return(FALSE);


		pbmiDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pbmiDIB->bmiHeader.biWidth = DIBWidth;
		pbmiDIB->bmiHeader.biHeight = DIBHeight;
		pbmiDIB->bmiHeader.biPlanes = 1;
		pbmiDIB->bmiHeader.biBitCount = 8;
		pbmiDIB->bmiHeader.biCompression = BI_RGB;
		pbmiDIB->bmiHeader.biSizeImage = 0;
		pbmiDIB->bmiHeader.biXPelsPerMeter = 0;
		pbmiDIB->bmiHeader.biYPelsPerMeter = 0;
		pbmiDIB->bmiHeader.biClrUsed = 256;
		pbmiDIB->bmiHeader.biClrImportant = 256;

		pusTemp = (PUSHORT) pbmiDIB->bmiColors;

		for (i=0; i<256; i++) {
			*pusTemp++ = i;
		}

        hDIBSection = CreateDIBSection (hdc, pbmiDIB, DIB_PAL_COLORS,
                          (void **)&pDIBBase, NULL, 0);

        if (!hDIBSection) {
            free(pbmiDIB);
            return(FALSE);
        }

        if (pbmiDIB->bmiHeader.biHeight > 0)
        {
            pDIB = (pDIBBase + (DIBHeight - 1) * DIBWidth);
            DIBPitch = -DIBWidth;   // bottom-up
        }
        else
        {
            pDIB = pDIBBase;
            DIBPitch = DIBWidth;    // top-down
        }

		// Clear the DIB
		memset(pDIBBase, 0, DIBWidth*DIBHeight);

		ReleaseDC(hwnd, hdc);

		hwndOutput = hwnd;

        // Set the initial location, direction, and speed
        roll = 0.0;
        pitch = 0.0;
        yaw = 0.0;
        fieldofview = 2.0;
        xscreenscale = DIBWidth / fieldofview;
        yscreenscale = DIBHeight / fieldofview;
        maxscale = min(xscreenscale, yscreenscale);
        xcenter = (DIBWidth / 2.0) - 0.5;
        ycenter = (DIBHeight / 2.0) + 0.5;

        return (TRUE);              // We succeeded...
}

/*/////////////////////////////////////////////////////////////////////
neweng constructor

in a perfect world, this should have all the ugly windows code in it.
/////////////////////////////////////////////////////////////////////*/

NewEng::NewEng(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {



	WNDCLASS wc;
	BOOL bResult;

    if (!hPrevInstance) {       // Other instances of app running?
        if (!InitApplication(hInstance)) { // Initialize shared things
        }
    }

    // Perform initializations that apply to a specific instance
    if (!InitInstance(hInstance, nCmdShow)) {
    }
	// Alias pointer to class
    hAccelTable = LoadAccelerators (hInstance, szAppName);

}

/*/////////////////////////////////////////////////////////////////////
neweng destructor

called right before we close
/////////////////////////////////////////////////////////////////////*/

NewEng::~NewEng() {

	DestroyWindow(hMainWindow);

}

/*/////////////////////////////////////////////////////////////////////
neweng::run

where the main loop really is, does all the neat stuff.
/////////////////////////////////////////////////////////////////////*/

void NewEng::Run() {

	GBool bQuit = GFalse; // Windows messages
	MSG msg;
	HPALETTE        holdpal;
    HDC             hdcScreen, hdcDIBSection;
    HBITMAP         holdbitmap;
	//set up our frustum before we go crazy.
    _cam->GenFrus();
	//hey! miscellaneous init code!
    done=0;
	delta=0.00;
	_portal->currcell=0;
	while(!done) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			//don't ask why this is here.
			_cam->xang=0;
			_cam->yang=0;
			_cam->zang=0;
			do {
	    		if (msg.message == WM_QUIT) {
					//yes, a goto.  i hope my programming teacher
					//doesn't read this.
					goto Done;
				}
				//*gasp!* ugly windows code!
	      		if (!TranslateAccelerator (msg.hwnd, hAccelTable,
	      		        &msg)) {
            		TranslateMessage(&msg);// xlates virt keycodes
       	        	DispatchMessage(&msg); // Dispatches msg to window
	       	   	}
			} while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
		}
		//woohoo!  now comes the fun stuff!!

		//first, move the camera according to what the user is telling us.
		_cam->loc.v[0]+=_cam->axis[2].v[0]*delta;
		_cam->loc.v[1]+=_cam->axis[2].v[1]*delta;
		_cam->loc.v[2]+=_cam->axis[2].v[2]*delta;
		//find which cell the camera is in, perform collision detection if needed
		_portal->FindCameraCell();
		//generate the new camera axis
		_cam->RotateAxis();
		//and the frustum to go with it.
		_cam->GenNewFrus();
		//clear the frame
	    memset(pDIBBase, 0, DIBWidth*DIBHeight);    
		//*little* hack, deal with it.
		frus.p[0]=_cam->frusw[0];
		frus.p[1]=_cam->frusw[1];
		frus.p[2]=_cam->frusw[2];
		frus.p[3]=_cam->frusw[3];
		frus.nfrus=4;
		//start drawing the scene with the cell we're in and the camera frustum
		_portal->DrawCell(&_portal->c[_portal->currcell],&frus);
		// We've drawn the frame; copy it to the screen
		// MARCH OF THE UGLY WINDOWS CODE!
		hdcScreen = GetDC(hwndOutput);
		holdpal = SelectPalette(hdcScreen, hpalDIB, FALSE);
		RealizePalette(hdcScreen);

	    hdcDIBSection = CreateCompatibleDC(hdcScreen);
	    holdbitmap = SelectObject(hdcDIBSection, hDIBSection);

		BitBlt(hdcScreen, 0, 0, DIBWidth, DIBHeight, hdcDIBSection,
           0, 0, SRCCOPY);

		SelectPalette(hdcScreen, holdpal, FALSE);
		ReleaseDC(hwndOutput, hdcScreen);
		SelectObject(hdcDIBSection, holdbitmap);
		DeleteDC(hdcDIBSection);
	}
	//easy out, without this, the program doesn' exit correctly, the goto
	//was needed (sorry) :)
Done:
	frus.nfrus=4;//stop complaining about no ; before }, you stupid compiler.
}

/*/////////////////////////////////////////////////////////////////////
mainwndproc

standard windows message handler
/////////////////////////////////////////////////////////////////////*/

LRESULT CALLBACK NewEng::MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;
	UINT fwSizeType;
	int oldDIBWidth, oldDIBHeight;
    HBITMAP holdDIBSection;
    HDC hdc;

	switch(message) {

		case WM_ACTIVATEAPP:
			break;
		case WM_KEYDOWN:
			switch (wParam) {

				case 'Q':
					done=1;
					break;
				case VK_LEFT:
					_cam->yang=32767-100;
					break;
				case VK_RIGHT:
					_cam->yang=100;
					break;
				case VK_UP:
					delta=0.08;
					_cam->yang=0;
					break;
				case VK_DOWN:
					delta=-0.08;
					_cam->yang=0;
					break;
				case ' ':
					delta=0;
					_cam->yang=0;
					break;
				case 'Z':
					_portal->flag=1;
					break;
				case 'X':
					_portal->flag=0;
					break;
			}
			break;
	case WM_SIZE:	// window size changed
		fwSizeType = wParam;
		if (fwSizeType != SIZE_MINIMIZED) {
            // Skip when this is called before the first DIB
            // section is created
            if (hDIBSection == 0)
                break;

			oldDIBWidth = DIBWidth;
			oldDIBHeight = DIBHeight;

			DIBWidth = LOWORD(lParam);
			DIBWidth = (DIBWidth + 3) & ~3;
			DIBHeight = HIWORD(lParam);

            if ((DIBHeight < 10) || (DIBWidth < 10))
            {
            // Keep the DIB section big enough so we don't start
            // drawing outside the DIB (the window can get smaller,
            // but we don't really care, and GDI will clip the
            // blts for us)
    			DIBWidth = oldDIBWidth;
    			DIBHeight = oldDIBHeight;
            }

			// Resize the DIB section to the new size
            holdDIBSection = hDIBSection;
			pbmiDIB->bmiHeader.biWidth = DIBWidth;
			pbmiDIB->bmiHeader.biHeight = DIBHeight;

            hdc = GetDC(hWnd);
            hDIBSection = CreateDIBSection (hdc, pbmiDIB,
                    DIB_PAL_COLORS, (void **)&pDIBBase, NULL, 0);

            if (hDIBSection) {
                // Success
                DeleteObject(holdDIBSection);

                if (pbmiDIB->bmiHeader.biHeight > 0)
                {
                    pDIB = (pDIBBase + (DIBHeight - 1) * DIBWidth);
                    DIBPitch = -DIBWidth;   // bottom-up
                }
                else
                {
                    pDIB = pDIBBase;
                    DIBPitch = DIBWidth;    // top-down
                }

                xscreenscale = DIBWidth / fieldofview;
                yscreenscale = DIBHeight / fieldofview;
                maxscale = min(xscreenscale, yscreenscale);
                xcenter = (DIBWidth / 2.0) - 0.5;
                ycenter = (DIBHeight / 2.0) + 0.5;
			    _cam->GenFrus();
            } else {
                // Failed, just use old size
    			pbmiDIB->bmiHeader.biWidth = oldDIBWidth;
	    		pbmiDIB->bmiHeader.biHeight = oldDIBHeight;
    			DIBWidth = oldDIBWidth;
    			DIBHeight = oldDIBHeight;
            }

			// Clear the DIB
			memset(pDIBBase, 0, DIBWidth*DIBHeight);
		}
		break;
	case WM_DESTROY:  // message: window being destroyed
		free(pbmiDIB);
        DeleteObject(hDIBSection);
		DeleteObject(hpalold);
                        
        PostQuitMessage(0);
        break;

	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

