/*
header for render file

Zrender.h
*/

#pragma once
#ifndef ZD3D_H
#define D3D_OVERLOADS
#include "Ztypes.h"
//#include "glide.h"
#include "z3d.h"
#include <ddraw.h>
#include <d3d.h>
#define G_HRESOLUTION 480
#define G_WRESOLUTION 640


#pragma once

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_BITS 16

typedef class ZRender {

public:

	ZRender (GU32 height, GU32 width);
	~ZRender ();

	void ClearPage(void);

	void ShowPage(void);

	void DrawTriangle(LPvert3d_t v1,LPvert3d_t c1,LPvert3d_t v2,LPvert3d_t c2,LPvert3d_t v3,LPvert3d_t c3);
	void DrawTest();

	int InitHardware(void);
	
	GU32 id; // Shared processing
	int error;
	
} ZRender , *LPZRender ;
#define ZD3D_H
#endif
