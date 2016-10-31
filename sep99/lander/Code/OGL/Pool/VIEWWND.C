// ViewWnd.c
// This file contains the window procedure and code related to the
// view window. This window does the OpenGL rendering of the terrain.

#include <windows.h>	// Normal Windows stuff
#include <math.h>
#include <gl/gl.h>		// Core OpenGL functions
#include <gl/glu.h>		// OpenGL Utility functions
#include <gl/glaux.h>
#include "externs.h"	// Data shared between files

/////////////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////////////

int		g_MouseHitX, g_MouseHitY;			// Needed for Mouse Interaction
int		g_Dragging = FALSE;
float	g_LastYaw, g_LastPitch;
int		g_DrawingStick = FALSE;
float	g_LastDraw;
HDC		g_hDC;	// Keep the Device Context

///////////////////////////////////////////////////////////////////////////////
// Setup the main view windows Rendering Context
void SetupViewRC(void)
{
	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	// Only draw the outside of CW wound objects
	glCullFace(GL_BACK);	// Cull the back
	glFrontFace(GL_CCW);	// Counter Clock wise wound is front
	glEnable(GL_CULL_FACE); // Enable the culling

	// Do depth testing
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);

	// No lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	// Enable color tracking
//	glEnable(GL_COLOR_MATERIAL);
	
	// Set Material properties to follow glColor values
//	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glPolygonMode(GL_FRONT,GL_FILL);
	glDepthFunc(GL_LESS);		//GL_LEQUAL
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// GL_MODULATE

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

// Select the pixel format for a given device context. This function is identical
// to the above, but also supplies a depth buffer
void SetDCDepthPixelFormat(HDC hDC)
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size of this structure
		1,                                                              // Version of this structure    
		PFD_DRAW_TO_WINDOW |                    // Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |					// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,                       // Double buffered
		PFD_TYPE_RGBA,                          // RGBA Color mode
		24,                                     // Want 24bit color 
		0,0,0,0,0,0,                            // Not used to select mode
		0,0,                                    // Not used to select mode
		0,0,0,0,0,                              // Not used to select mode
		// Try to get away with smaller depth buffer to take advantage
		// of low end PC accelerator cards
		32,                                     // Size of depth buffer
		0,                                      // Not used to select mode
		0,                                      // Not used to select mode
		PFD_MAIN_PLANE,                         // Draw in main plane
		0,                                      // Not used to select mode
		0,0,0 };                                // Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	// Set the pixel format for the device context
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Window procedure, handles all messages for this window
LRESULT CALLBACK WndProcView(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam)
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int tx,ty;
	static HDC	  hDC;	// Keep the Device Context
	static HGLRC  hRC;	// Keep the Rendering Context
	float	magnitude;
/////////////////////////////////////////////////////////////////////////////////////
	
	switch (message)
		{
		// Window creation, setup here
		case WM_CREATE:
			// Save the device context
			hDC = GetDC(hWnd);

			// Set the pixel format
			SetDCDepthPixelFormat(hDC);

			// Create the rendering context and make it current
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			g_hDC = hDC;
			// Do some setup here
			SetupViewRC();

			InitRender();	// Call RenderWorld Routine

			break;

		// Window is being destroyed, cleanup
		case WM_DESTROY:
			// Cleanup...
			// Deselect the current rendering context and delete it
			wglMakeCurrent(hDC,NULL);
			wglDeleteContext(hRC);

			// Destroy the palette if it was created 
			if(hPalette != NULL)
				DeleteObject(hPalette);

			// Release the device context
			ReleaseDC(hWnd,hDC);
			break;

		// Window is resized. Setup the viewing transformation
		case WM_SIZE:
			{
			int nWidth,nHeight;
			double dAspect;

			nWidth = LOWORD(lParam);  // width of client area 
			nHeight = HIWORD(lParam); // height of client area 
	
			if(nHeight == 0)		  // Don't allow divide by zero
				nHeight = 1;

			dAspect = (double)nWidth/(double)nHeight;

			// Make this rendering context current
			wglMakeCurrent(hDC, hRC);

			// Set the viewport to be the entire window
		    glViewport(0, 0, nWidth, nHeight);
	
			// Setup Perspective
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			// Establish viewing volume
			gluPerspective(60.0, dAspect,0.2f, 2000);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			}
			break;

		case WM_PAINT:
			{
			float fRadius = 50.0f;

			RenderWorld();

			// Validate the newly painted client area
			ValidateRect(hWnd,NULL);
			}
			break;


		// Windows is telling the application that it may modify
		// the system palette.  This message in essance asks the 
		// application for a new palette.
		case WM_QUERYNEWPALETTE:
			// If the palette was created.
			if(hPalette)
				{
				int nRet;

				// Selects the palette into the current device context
				SelectPalette(hDC, hPalette, FALSE);

				// Map entries from the currently selected palette to
				// the system palette.  The return value is the number 
				// of palette entries modified.
				nRet = RealizePalette(hDC);

				// Repaint, forces remap of palette in current window
				InvalidateRect(hWnd,NULL,FALSE);

				return nRet;
				}
			break;

	
		// This window may set the palette, even though it is not the 
		// currently active window.
		case WM_PALETTECHANGED:
			// Don't do anything if the palette does not exist, or if
			// this is the window that changed the palette.
			if((hPalette != NULL) && ((HWND)wParam != hWnd))
				{
				// Select the palette into the device context
				SelectPalette(hDC,hPalette,FALSE);

				// Map entries to system palette
				RealizePalette(hDC);
				
				// Remap the current colors to the newly realized palette
				UpdateColors(hDC);
				return 0;
				}
			break;

		// Handle Left Mouse Button Press
		case WM_LBUTTONDOWN:
			g_Dragging = TRUE;
			g_LastYaw = g_POV.rot.y;		// Save the old Yaw
			g_LastPitch = g_POV.rot.x;		// Save the old Pitch
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 

			// Reset Cue Position
			g_CueStick.pos.x = g_CurrentSys->pos.x;
			g_CueStick.pos.z = g_CurrentSys->pos.z;
			SetCapture(hWnd);
			break;

		case WM_MOUSEMOVE:
			tx = LOWORD(lParam);  // horizontal position of cursor 
			ty = HIWORD(lParam);  // vertical position of cursor 
			if (g_Dragging)
			{
				if (tx != g_MouseHitX)
				{
					g_POV.rot.y = g_LastYaw + (float)(tx - g_MouseHitX);
					InvalidateRect(hWnd,NULL,FALSE);
				}
				if (ty != g_MouseHitY)
				{
					g_POV.rot.x = g_LastPitch + (float)(ty - g_MouseHitY);
					if (g_POV.rot.x < 0.0f) g_POV.rot.x = 0.0f;			
					if (g_POV.rot.x > 90.0f) g_POV.rot.x = 90.0f;			
					InvalidateRect(hWnd,NULL,FALSE);
				}

			}
			// Check the Motion of the Cue Stick via right mouse
			if (g_DrawingStick && !g_BallInPlay)
			{
				if (ty != g_MouseHitY)
				{
					g_CueStick.draw = g_LastDraw + ((float)(ty - g_MouseHitY) * 0.1f);
					// Pulling back on the cue stick
					if (g_LastDraw < g_CueStick.draw)
					{
						g_CueStick.old_draw = g_CueStick.draw;
						g_CueStick.drawtime = GetTime();
					}
					// If the stick is at the ball, it is a hit
					else if (g_CueStick.draw < 0.0f)	
					{
						// TODO: Add a Cue Stick hits ball sound here
						g_CueHitBall = TRUE;				// Set when a Cue hits the ball
						magnitude = -CUE_STICK_FORCE * ((g_CueStick.old_draw - g_CueStick.draw) / (GetTime() - g_CueStick.drawtime));
						g_CueForce.x = magnitude * sin(DEGTORAD(g_CueStick.yaw));							
						g_CueForce.z = magnitude * cos(DEGTORAD(g_CueStick.yaw));							
						g_CueStick.draw = 0.2f;			
						g_BallInPlay = TRUE;
						g_DrawingStick = FALSE;
					}
					InvalidateRect(hWnd,NULL,FALSE);
				}

			}
			break;

		// Handle Left Mouse Button Release
		case WM_LBUTTONUP:
			g_Dragging = FALSE;
			ReleaseCapture();
			break;

		// Handle Right Mouse Button Press
		case WM_RBUTTONDOWN:
			g_DrawingStick = TRUE;
			g_LastDraw = g_CueStick.draw;		// Save the old Yaw
			g_CueStick.old_draw = g_CueStick.draw;		// Save the old Yaw
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 
			// Reset Cue Position
			g_CueStick.pos.x = g_CurrentSys->pos.x;
			g_CueStick.pos.z = g_CurrentSys->pos.z;
			SetCapture(hWnd);
			break;

		// Handle Right Mouse Button Release
		case WM_RBUTTONUP:
			g_DrawingStick = FALSE;
			ReleaseCapture();
			break;

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}



