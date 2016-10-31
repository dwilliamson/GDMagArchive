/**************************************************************************
   RENDER.H - Rasterization and Geometry functions
**************************************************************************/
#ifndef DEMORENDER_H
#define DEMORENDER_H

#include <d3d.h>
#include <stdio.h>
#include <math.h>
#include "d3dmain.h"
#include "procedural.h"

#ifdef __cplusplus
extern "C" {
#endif 
/******************************************************************************/
#include  <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
/******************************************************************************/
 #define MY_FTOL
 #define MARBLE  0
 #define WOOD    1
/******************************************************************************/
// geometry functions
void ConcatenateXRotation(LPD3DMATRIX lpM, float Degrees );
void ConcatenateYRotation(LPD3DMATRIX lpM, float Degrees );
void ConcatenateZRotation(LPD3DMATRIX lpM, float Degrees );
void RotateD3DVERTEX(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV);
void ScaleD3DVERTEX(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV);
void transformD3DNormals(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV);
void transformD3DVERTEX(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV);
void transformCopyD3DVERTEX(LPD3DMATRIX lpMat, D3DVECTOR *lpVSrc, D3DVECTOR *lpVDst);

// renderer
void TextureMapPolygonInverse( 
						   short    *pRenderBuffer,
						   int       ScreenDibWidth,
						   int       ScreenDibHeight,
						   int       TextureDIBWidth,
						   int       TextureDIBHeight);
						  
signed long my_ftol(float d);

// user interface 
BOOL KeyboardHandler(UINT message, WPARAM wParam, LPARAM lParam);
BOOL MouseHandler(UINT message, WPARAM wParam, LPARAM lParam);

extern void readtime();

#define M_PI            3.14159265358979323846


#ifdef __cplusplus
#define VERTEX  vertex
#define POLYGON polygon
#else
#define VERTEX  struct vertex
#define POLYGON struct polygon
#endif 


struct vertex
{
    LONG  x;
    LONG  y;
    LONG  z;
    D3DVALUE     nx;    /* Normal */
    D3DVALUE     ny;
    D3DVALUE     nz;
    D3DVALUE     u;     /* Texture coordinates */
    D3DVALUE     v;
};


struct polygon
{ 
  //unsigned  short V[4];
  VERTEX   vertex[4];
};
POLYGON my_Poly;


#ifdef __cplusplus
}
#endif 

#endif
