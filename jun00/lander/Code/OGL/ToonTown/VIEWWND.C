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
int		g_ScreenHeight,g_ScreenWidth;
int		g_Dragging = FALSE;
int		g_ForceDrag = FALSE;
float	g_LastYaw, g_LastPitch;
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

	glPointSize(8.0f);
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
	tVector localX,localY;
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

			g_ScreenWidth  = nWidth;
			g_ScreenHeight = nHeight;
			
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
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 
			g_ForceDrag = TRUE;			
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glPushMatrix();

					// Set root skeleton's orientation and position
					glTranslatef(-g_POV.trans.x, -g_POV.trans.y, -g_POV.trans.z);

					glRotatef(g_POV.rot.z, 0.0f, 0.0f, 1.0f);
					glRotatef(g_POV.rot.x, 1.0f, 0.0f, 0.0f); 
					glRotatef(g_POV.rot.y, 0.0f, 1.0f, 0.0f);
		
					GetNearestPoint(g_MouseHitX, g_ScreenHeight - g_MouseHitY);

				glPopMatrix();

			SetCapture(hWnd);
			break;

		case WM_MOUSEMOVE:
			tx = LOWORD(lParam);  // horizontal position of cursor 
			ty = HIWORD(lParam);  // vertical position of cursor 
			if (tx > 32767)
			{
				tx = 0;
			}
			if (ty > 32767)
			{
				ty = 0;
			}
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
			if (g_ForceDrag)
			{
				// NEED TO GET THE VECTORS FOR THE LOCAL X AND Y AXES
				localY.x = g_ViewMatrix.m[1];
				localY.y = g_ViewMatrix.m[5];
				localY.z = g_ViewMatrix.m[9];

				localX.x = g_ViewMatrix.m[0];
				localX.y = g_ViewMatrix.m[4];
				localX.z = g_ViewMatrix.m[8];

				SetMouseForce(tx - g_MouseHitX,ty - g_MouseHitY,&localX,&localY);
				g_MouseForceActive = TRUE;
			}
			break;

		// Handle Left Mouse Button Release
		case WM_LBUTTONUP:
			g_ForceDrag = FALSE;			
			g_MouseForceActive = FALSE;
			ReleaseCapture();
			break;

		// Handle Right Mouse Button Press
		case WM_RBUTTONDOWN:
			g_Dragging = TRUE;
			g_LastYaw = g_POV.rot.y;		// Save the old Yaw
			g_LastPitch = g_POV.rot.x;		// Save the old Pitch
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 

			SetCapture(hWnd);
			break;

		// Handle Right Mouse Button Release
		case WM_RBUTTONUP:
			g_Dragging = FALSE;
			ReleaseCapture();
			break;

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}

