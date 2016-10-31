/*
source for render class

myrender.cpp
*/

//uncomment the following line to turn off rendering
#define DONTRENDERD3D


#include "zrender.h"
#include "NewEngg.h"
#include "neweng.h"
//#include "glide.h"
#include "z3d.h"
#include <crtdbg.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>


LPDIRECTDRAW m_pDD;
LPDIRECTDRAWSURFACE m_pPrimary;
LPDIRECTDRAWSURFACE m_pBackBuffer;
LPDIRECTDRAWSURFACE m_pZBuffer;
DDSURFACEDESC ddsd;
LPDIRECT3D2 m_pD3D;
LPDIRECT3DDEVICE2 m_pDevice;
LPDIRECT3DVIEWPORT2 m_pViewport;
int hardware=0;


//************************************
// Constructor
//------------------------------------
// Set up variables and scratch arrays
//************************************

ZRender::ZRender(GU32 height, GU32 width) {

	// Setup hardware
	InitHardware();
}

//************************************
// Destructor
//------------------------------------
// Do nothing
//************************************
#define RELEASE(x) if (x) { x->Release(); x = NULL; }
ZRender::~ZRender() {
#ifndef DONTRENDERD3D	
    RELEASE(m_pDevice);
    RELEASE(m_pViewport);
    RELEASE(m_pBackBuffer);
    RELEASE(m_pPrimary);
    RELEASE(m_pZBuffer);
    RELEASE(m_pD3D);
    RELEASE(m_pDD);
#endif
}


void ZRender::ClearPage(void)
{
#ifndef DONTRENDERD3D	
	// Clear the back buffer
	DDBLTFX bltfx;
	ZeroMemory(&bltfx, sizeof(bltfx)); // Sets dwFillColor to 0 as well
	bltfx.dwSize = sizeof(bltfx);
	m_pBackBuffer->Blt(NULL,NULL,NULL,DDBLT_WAIT|DDBLT_COLORFILL,&bltfx);
	// Clear the z-buffer
	D3DRECT rect;
	rect.x1 = 0;
	rect.y1 = 0;
	rect.x2 = SCREEN_WIDTH;
	rect.y2 = SCREEN_HEIGHT;
	m_pViewport->Clear(1, &rect, D3DCLEAR_ZBUFFER);
	//begin our scene
	m_pDevice->BeginScene();
#endif
}

void ZRender::ShowPage(void)
{
#ifndef DONTRENDERD3D	
	m_pDevice->EndScene();
	// Flip the back buffer to the primary surface
	m_pPrimary->Flip(NULL,DDFLIP_WAIT);
#endif
}

//************************************
// InitHardware
//------------------------------------
// Setup glide
//************************************



int ZRender::InitHardware(void) {
#ifndef DONTRENDERD3D	
// Create main DirectDraw object
  if (DirectDrawCreate(NULL, &m_pDD, NULL) != DD_OK)
   return FALSE;
	if(m_pDD->SetCooperativeLevel(_app->hMainWindow, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX) != DD_OK)
		 return FALSE;
	if (m_pDD->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BITS))
	 return FALSE;
	// Get the primary display surface
	//TRACE("DrawPrim Demo: Creating the primary surface\n");
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	if (hardware)
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
	if (m_pDD->CreateSurface(&ddsd, &m_pPrimary, NULL) != DD_OK)
		return FALSE;
	// Create a back buffer and attach it to the primary display surface to make a flippable surface
	//TRACE("DrawPrim Demo: Creating the back buffer\n");
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;
	if (hardware)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_SYSTEMMEMORY;
	if (m_pDD->CreateSurface(&ddsd, &m_pBackBuffer, NULL) != DD_OK)
		return FALSE;
	if (m_pPrimary->AddAttachedSurface(m_pBackBuffer) != DD_OK)
		return FALSE;
	if (m_pDD->QueryInterface(IID_IDirect3D2, (LPVOID *)&m_pD3D) != S_OK)
		return FALSE;
	// Find a device we can use
	D3DFINDDEVICESEARCH search;
	D3DFINDDEVICERESULT result;
	ZeroMemory(&search, sizeof(search));
	search.dwSize = sizeof(search);
	search.dwFlags = D3DFDS_HARDWARE;
	search.bHardware = hardware;
	ZeroMemory(&result, sizeof(result));
	result.dwSize = sizeof(result);
	if (m_pD3D->FindDevice(&search, &result) != D3D_OK)
		return FALSE;
	// Create the D3D device
	if (m_pD3D->CreateDevice(result.guid, m_pBackBuffer, &m_pDevice) != D3D_OK)
		return FALSE;
	// Create a viewport
	D3DVIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(viewport));
	viewport.dwSize = sizeof(viewport);
	viewport.dwWidth = SCREEN_WIDTH;
	viewport.dwHeight = SCREEN_HEIGHT;
	viewport.dvScaleX = SCREEN_WIDTH / 2.0f;
	viewport.dvScaleY = SCREEN_HEIGHT / 2.0f;
	viewport.dvMaxX = D3DVAL(1.0);
	viewport.dvMaxY = D3DVAL(1.0);
	if (m_pD3D->CreateViewport(&m_pViewport, NULL) != D3D_OK)
		return FALSE;
	if (m_pDevice->AddViewport(m_pViewport) != D3D_OK)
		return FALSE;
	if (m_pViewport->SetViewport(&viewport) != D3D_OK)
		return FALSE;
	if (m_pDevice->SetCurrentViewport(m_pViewport) != D3D_OK)
		return FALSE;
	//set up the zbuffer
	DWORD depths = hardware ? result.ddHwDesc.dwDeviceZBufferBitDepth :
	result.ddSwDesc.dwDeviceZBufferBitDepth;
	DWORD bitdepth;
	if (depths & DDBD_16) bitdepth=16;
	else if (depths & DDBD_24) bitdepth=24; 
	else if (depths & DDBD_32) bitdepth=32; 
	else if (depths & DDBD_8) bitdepth=8; 
	else return FALSE;
// Create a z-buffer and attach it to the backbuffer
//TRACE("DrawPrim Demo: Creating z-buffer\n");
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH;
	ddsd.dwWidth = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;
	ddsd.dwZBufferBitDepth = bitdepth;
	if (hardware)
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
	if (m_pDD->CreateSurface(&ddsd, &m_pZBuffer, NULL) != DD_OK)
		return FALSE;
	if (m_pBackBuffer->AddAttachedSurface(m_pZBuffer) != DD_OK)
		return FALSE;

	m_pDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
	m_pDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);

#endif
	return TRUE;
}


void ZRender::DrawTriangle(LPvert3d_t v1,LPvert3d_t c1,LPvert3d_t v2,LPvert3d_t c2,LPvert3d_t v3,LPvert3d_t c3)
{
	// Render a triangle
	D3DTLVERTEX v[3];
	v[0] = D3DTLVERTEX(D3DVECTOR(v1->v[0], v1->v[1],v1->v[2]),0,D3DRGB(c1->v[0]/255, c1->v[1]/255,c1->v[2]/255),D3DRGB(0,0,0),0,0);
	v[1] = D3DTLVERTEX(D3DVECTOR(v3->v[0], v3->v[1],v3->v[2]),0,D3DRGB(c3->v[0]/255, c3->v[1]/255,c3->v[2]/255),D3DRGB(0,0,0),0,0);
	v[2] = D3DTLVERTEX(D3DVECTOR(v2->v[0], v2->v[1],v2->v[2]),0,D3DRGB(c2->v[0]/255, c2->v[1]/255,c2->v[2]/255),D3DRGB(0,0,0),0,0);
#ifndef DONTRENDERD3D	
	m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
#endif

}


void ZRender::DrawTest()
{
	// Render a triangle
	D3DTLVERTEX v[3];
	v[0] = D3DTLVERTEX(D3DVECTOR(160, 50,0),1,D3DRGB(1,0,0),D3DRGB(0,0,0),0,0);
	v[1] = D3DTLVERTEX(D3DVECTOR(240,200,0),1,D3DRGB(0,1,0),D3DRGB(0,0,0),0,0);
	v[2] = D3DTLVERTEX(D3DVECTOR( 80,200,0),1,D3DRGB(0,0,1),D3DRGB(0,0,0),0,0);
#ifndef DONTRENDERD3D	
	m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
#endif
}

//for snapping reasons, these are in here.

void ProjectVert(vert3d_t *vert){
	float cinv;
	cinv=1/vert->v[2];
	vert->v[0]=(320.0f+320.f*vert->v[0]*cinv);
	vert->v[1]=(240.0f-320.f*vert->v[1]*cinv);
	vert->v[2]=(vert->v[2]-1)/(10-1);
}

void ProjectVerts(vert3d_t *verts,int nverts){
	float cinv;
	vert3d_t *vert=verts;
	for(int i=0;i<nverts;i++){
		cinv=1/verts->v[2];
		vert->v[0]=(320.0f+320.f*vert->v[0]*cinv);
		vert->v[1]=(240.0f-320.f*vert->v[1]*cinv);
		vert->v[2]=(vert->v[2]-1)/(10-1);
		vert++;
	}
}
