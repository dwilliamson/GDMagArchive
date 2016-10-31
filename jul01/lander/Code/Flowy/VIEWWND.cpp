// ViewWnd.cpp
// Created:
//		JL 8/5/200		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2000 Darwin 3D, LLC, All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

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
int		g_Emitting = FALSE;
int		g_Picked;
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

int CheckPickElement(tVector pickpnt)
{
	for (int loop = 0; loop < g_ElementCnt; loop++)
	{
		if (pickpnt.VectorDistance(&g_Element[loop].pos) < g_Element[loop].size * 5)
		{
			return loop;
		}
	}
	return -1;
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
//			gluPerspective(60.0, dAspect,0.2f, 2000);
			gluOrtho2D(0.0f,(GLfloat)nWidth,0.0f,(GLfloat)nHeight);	// USE WINDOW SETTINGS

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

		// Handle Right Mouse Button Press
		case WM_RBUTTONDOWN:
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 
			g_Dragging = TRUE;
			g_Picked = CheckPickElement(tVector((float)g_MouseHitX,(float)g_ScreenHeight - g_MouseHitY,0.0f));
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
			// If I am dragging a picked element
			if (g_Dragging && g_Picked > -1)
			{
				g_Element[g_Picked].pos.x = tx;
				g_Element[g_Picked].pos.y = g_ScreenHeight - ty;
				InvalidateRect(hWnd,NULL,FALSE);
			}
			if (g_Emitting)	// Am I emitting particles
			{
				AddParticle((float)tx, (float)(g_ScreenHeight - ty));
			}
			break;

		// Handle Right Mouse Button Release
		case WM_RBUTTONUP:
			g_Dragging = FALSE;
			if (g_Picked == -1)
			{
				g_Element[g_ElementCnt].pos.x = g_MouseHitX;
				g_Element[g_ElementCnt].pos.y = g_ScreenHeight - g_MouseHitY;
				g_Element[g_ElementCnt].pos.z = 0.0f;
                                g_Element[g_ElementCnt].emitParticles = false;
				if (g_ElementCnt > 0)
				{
					g_Element[g_ElementCnt].type = g_Element[g_ElementCnt - 1].type;
					g_Element[g_ElementCnt].size = g_Element[g_ElementCnt - 1].size;
					g_Element[g_ElementCnt].strength = g_Element[g_ElementCnt - 1].strength;
				}
				else
				{
					g_Element[g_ElementCnt].type = 0;
					g_Element[g_ElementCnt].size = 2.0f;
					g_Element[g_ElementCnt].strength = 0.0f;
				}
				g_Picked = g_ElementCnt;
				g_ElementCnt++;
			}
			ReleaseCapture();
			InvalidateRect(hWnd,NULL,FALSE);
			break;

		// Handle Left Mouse Button Press
		case WM_LBUTTONDOWN:
			g_Emitting = TRUE;
			tx = LOWORD(lParam);  // horizontal position of cursor 
			ty = HIWORD(lParam);  // vertical position of cursor 
			AddParticle((float)tx, (float)(g_ScreenHeight - ty));
			SetCapture(hWnd);
			break;

		// Handle Left Mouse Button Release
		case WM_LBUTTONUP:
			g_MouseHitX = LOWORD(lParam);  // horizontal position of cursor 
			g_MouseHitY = HIWORD(lParam);  // vertical position of cursor 
			g_Emitting = FALSE;
			ReleaseCapture();
			InvalidateRect(hWnd,NULL,FALSE);
			break;

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}

void AddElement()
{
	if (g_ElementCnt > 0)
	{
		g_Element[g_ElementCnt].pos = g_Element[g_ElementCnt - 1].pos;
		g_Element[g_ElementCnt].type = g_Element[g_ElementCnt - 1].type;
		g_Element[g_ElementCnt].size = g_Element[g_ElementCnt - 1].size;
		g_Element[g_ElementCnt].strength = g_Element[g_ElementCnt - 1].strength;
	}
	else
	{
		g_Element[g_ElementCnt].pos.x = (float)(g_ScreenWidth/2);
		g_Element[g_ElementCnt].pos.y = (float)(g_ScreenHeight/2);
		g_Element[g_ElementCnt].pos.z = 0.0f;
		g_Element[g_ElementCnt].type = 0;
		g_Element[g_ElementCnt].size = 2.0f;
		g_Element[g_ElementCnt].strength = 0.0f;
	}
	g_Picked = g_ElementCnt;
	g_ElementCnt++;
}

// Delete a flow element from the system
void DeleteElement()
{
	if (g_Picked < g_ElementCnt)
	{
		if (g_Picked < g_ElementCnt - 1)
		{
			memcpy(&g_Element[g_Picked],&g_Element[g_Picked+1],sizeof(t_Element) * (g_ElementCnt - g_Picked));
		}
		g_ElementCnt--;
	}
}