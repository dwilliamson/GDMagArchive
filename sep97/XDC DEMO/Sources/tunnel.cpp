/**************************************************************************

Mixed Rendering

 **************************************************************************/
/***************************************************************
*
*       This program has been developed by Intel Corporation.  
*		You have Intel's permission to incorporate this code 
*       into your product, royalty free.  Intel has various 
*	    intellectual property rights which it may assert under
*       certain circumstances, such as if another manufacturer's
*       processor mis-identifies itself as being "GenuineIntel"
*		when the CPUID instruction is executed.
*
*       Intel specifically disclaims all warranties, express or
*       implied, and all liability, including consequential and
*		other indirect damages, for the use of this code, 
*		including liability for infringement of any proprietary
*		rights, and including the warranties of merchantability
*		and fitness for a particular purpose.  Intel does not 
*		assume any responsibility for any errors which may 
*		appear in this code nor any responsibility to update it.
*
*  * Other brands and names are the property of their respective
*    owners.
*
*  Copyright (c) 1995, Intel Corporation.  All rights reserved.
***************************************************************/
/*==========================================================================
 *
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: tunnel.c
 *
 ***************************************************************************/

#include <math.h>
#include <malloc.h>

#define D3D_OVERLOADS

#include <d3d.h>
#include "d3ddemo.h"
#include "main.h" 
#include "ddutil.h"

/*
 * Globals to keep track of execute buffer
 */
static D3DEXECUTEDATA d3dExData;
static LPDIRECT3DEXECUTEBUFFER lpD3DExBuf;
static D3DEXECUTEBUFFERDESC debDesc;

 
//  This define tells us if for each frame the Spot will be transformed  
//   to camera coordinates and then to screen coordinates,
//   or will be setup once (!!!) at creation time to screen coordinated.
// #define SCREEN_COORDINATES

#define SW_SPOT

#ifdef SW_SPOT
    // execute buffer for the 
	static D3DEXECUTEDATA			    d3dExDataSpot;
    static LPDIRECT3DEXECUTEBUFFER	   lpD3DExBufSpot;
	
	// we have four vertices and two triangles for the spot
	// in th HW scene
    #define NUM_VERTICES_SPOT  4
	#define NUM_TRIANGLES_SPOT 2
    D3DTLVERTEX  src_v[NUM_VERTICES_SPOT];
	D3DTRIANGLE  src_tri[NUM_TRIANGLES_SPOT];
    
	// for each frame calcs new location for the spot
	void moveSWPolygon();
    // init the vertices
	void initSwSpot(); 
    
    // onCruve is the currnet location of the SPot in camera space 
	D3DVECTOR onCurve , viewVec , upVec ; 
	
	// create execute buffer for the spot and fill it
	BOOL
    InitViewSW(LPDIRECTDRAW lpDD, LPDIRECT3D lpD3D, 
			   LPDIRECT3DDEVICE lpDev,LPDIRECT3DVIEWPORT lpView, 
			   int NumTextures, LPD3DTEXTUREHANDLE TextureHandle);
#include "d3dmain.h"

#endif

/*
 * More globals
 */
LPDIRECT3DMATERIAL lpbmat;
LPDIRECT3DMATERIAL lpmat;   /* Material object */

/*
 * Global projection and view matrices
 */
D3DMATRIXHANDLE hProj;

D3DMATRIX proj, view;

static D3DMATRIX
IdentityMatrix()
{
    D3DMATRIX ret;
    for (int i = 0; i < 4; i += 1)
        for (int j = 0; j < 4; j += 1)
            ret(i, j) = (i==j) ? 1.0f : 0.0f;
    return ret;
}

static D3DMATRIX
ProjectionMatrix()
{
    D3DMATRIX ret = IdentityMatrix();
    ret(0, 0) = 2.0f;
    ret(1, 1) = 2.0f;
    ret(3, 3) = 0.0f;
    ret(3, 2) = -1.0f;
    ret(2, 3) = 1.0f;
    return ret;
}

D3DMATRIXHANDLE hView;

D3DMATRIX
ViewMatrix()
{
    D3DMATRIX ret = IdentityMatrix();
    ret(3, 2) = 10.0f;
    return ret;
}

D3DMATRIXHANDLE hWorld;

#define PI 3.14159265359f

/*
 * These defines describe the section of the tube in the execute buffer at
 * one time. (Note, tube and tunnel are used interchangeably).
 */
#define SEGMENTS 20   /* Number of segments in memory at one time.  Each
                       * segment is made up oftriangles spanning between
                       * two rings.
                       */ 
#define SIDES 8       /* Number of sides on each ring. */
#define TEX_RINGS 5   /* Number of rings to stretch the texture over. */
#define NUM_V (SIDES*(SEGMENTS+1)) // Number of vertices in memory at once
#define NUM_TRI (SIDES*SEGMENTS*2) // Number of triangles in memory
#define TUBE_R 1.0f      /* Radius of the tube. */
#define SPLINE_POINTS 50 /* Number of spline points to initially
                          * calculate.  The section in memory represents
                          * only a fraction of this.
                          */
/*
 * Movement and track scalars given in terms of position along the spline
 * curve.
 */
#define SEGMENT_LENGTH 0.05 /* Length of each segment along curve. */
#define SPEED 0.01   /*0.02    /* Amount to increment camera position along
                             * curve for each frame.
                             */
#define DEPTH 0.8           /* How close the camera can get to the end of
                             * track before new segments are added.
                             */
#define PATH_LENGTH (SPLINE_POINTS - 1) /*Total length of the tunnel.*/

/*
 * A global structure holding the tube data.
 */
static struct _tube {
    LPD3DVERTEX lpV;                /* Points to the vertices. */
    LPD3DTRIANGLE lpTri;          /* Points to the triangles which make up the segments.                 */
    int TriOffset;                            /* Offset into the execute buffer were the     triangle list is found. */
    LPD3DVECTOR lpPoints;        /* Points to the points defining the spline curve. */
	D3DMATERIALHANDLE hMat; /* Handle for the material on the tube. */
    D3DTEXTUREHANDLE hTex;  /* Handle for the texture on the material.*/
    D3DLIGHT light;                      /* Structure defining the light. */
    LPDIRECT3DLIGHT lpD3DLight;  /* Object pointer for the light. */
    D3DVECTOR cameraP, cameraD, cameraN; /* Vectors defining the camera position, direction and up. */
    float cameraPos;										      /* Camera position along the spline curve. */
    D3DVECTOR endP, endD, endN;    /* Vectors defining the position, direction and up 
						                                     * at the foremost end of  the section in memory. */
    float endPos;								   /* Position along the spline curve of the end. */
    int currentRing, currentSegment;   /* Numbers of the ring and tube at  the back end of the section. */
                                      
} tube;

extern LPD3DMATRIX D3DMATRIXInvert(LPD3DMATRIX d, LPD3DMATRIX a);
extern LPD3DMATRIX D3DMATRIXSetRotation(LPD3DMATRIX lpM, LPD3DVECTOR lpD,
                                        LPD3DVECTOR lpU);

/*
 * Calculates a point along a B-Spline curve defined by four points. p
 * n output, contain the point. t     Position
 * along the curve between p2 and p3.  This position is a float between 0
 * and 1. p1, p2, p3, p4    Points defining spline curve. p, at parameter
 * t along the spline curve
 */
D3DVECTOR 
spline(double t, const LPD3DVECTOR p[4])
{
    double t2 = t * t;
    double t3 = t2 * t;
    D3DVECTOR ret(0.0f);
    float m[4];

    m[0] = (float) (0.5f*((-1.0 * t3) + (2.0 * t2) + (-1.0 * t)));
    m[1] = (float) (0.5f*((3.0 * t3) + (-5.0 * t2) + (0.0 * t) + 2.0));
    m[2] = (float) (0.5f*((-3.0 * t3) + (4.0 * t2) + (1.0 * t)));
    m[3] = (float) (0.5f*((1.0 * t3) + (-1.0 * t2) + (0.0 * t)));

    for (int i = 0; i < 4; i += 1) {
        ret += *p[i]*m[i];
    }
    return ret;
}

/*
 * Creates a matrix which is equivalent to having the camera at a
 * specified position. This matrix can be used to convert vertices to
 * camera coordinates. lpP    Position of the camera. lpD    Direction of
 * view. lpN    Up vector. lpM    Matrix to update.
 */
void 
PositionCamera(LPD3DVECTOR lpP, LPD3DVECTOR lpD, LPD3DVECTOR lpN, 
               LPD3DMATRIX lpM)
{
    D3DMATRIX tmp;

    /*
     * Set the rotation part of the matrix and invert it. Vertices must be
     * inverse rotated to achieve the same result of a corresponding 
     * camera rotation.
     */
    tmp._14 = tmp._24 = tmp._34 = tmp._41 = tmp._42 = tmp._43 = (float)0.0;
    tmp._44 = (float)1.0;
    D3DMATRIXSetRotation(&tmp, lpD, lpN);
    D3DMATRIXInvert(lpM, &tmp);
    /*
     * Multiply the rotation matrix by a translation transform.  The
     * translation matrix must be applied first (left of rotation).
     */
    lpM->_41=-(lpM->_11 * lpP->x + lpM->_21 * lpP->y + lpM->_31 * lpP->z);
    lpM->_42=-(lpM->_12 * lpP->x + lpM->_22 * lpP->y + lpM->_32 * lpP->z);
    lpM->_43=-(lpM->_13 * lpP->x + lpM->_23 * lpP->y + lpM->_33 * lpP->z);
}

/*
 * Updates the given position, direction and normal vectors to a given
 * position on the spline curve.  The given up vector is used to determine
 * the new up vector.
 */

void 
MoveToPosition(float position, LPD3DVECTOR lpP, LPD3DVECTOR lpD, 
               LPD3DVECTOR lpN)
{
    LPD3DVECTOR lpSplinePoint[4];
    D3DVECTOR pp, x;
    int i, j;
    float t;

    /*
     * Find the four points along the curve which are around the position.
     */
    i = 0;
    t = position;
    while (t > 1.0) {
        i++;
        if (i == SPLINE_POINTS)
            i = 0;
        t -= 1.0f;
    }
    for (j = 0; j < 4; j++) {
        lpSplinePoint[j] = &tube.lpPoints[i];
        i++;
        if (i == SPLINE_POINTS)
            i = 0;
    }
    /*
     * Get the point at the given position and one just before it.
     */
    *lpP = spline(t, lpSplinePoint);
    pp = spline(t - (float)0.01, lpSplinePoint);
    /*
     * Calculate the direction.
     */
    *lpD = Normalize(*lpP - pp);
    /*
     * Find the new normal.  This method will work provided the change in
     * the normal is not very large.
     */
    *lpN = Normalize(*lpN);
    x = CrossProduct(*lpN, *lpD);
    *lpN = Normalize(CrossProduct(*lpD, x));
}


/*
 * Evaluate the Spline at the given position, 
 */

void 
SplineAtPosition(float position, LPD3DVECTOR lpP)
{
    LPD3DVECTOR lpSplinePoint[4];
    int i, j;
    float t;

    /*
     * Find the four points along the curve which are around the position.
     */
    i = 0;
    t = position;
    while (t > 1.0) {
        i++;
        if (i == SPLINE_POINTS)
            i = 0;
        t -= 1.0f;
    }
    for (j = 0; j < 4; j++) {
        lpSplinePoint[j] = &tube.lpPoints[i];
        i++;
        if (i == SPLINE_POINTS)
            i = 0;
    }
    /*
     * Get the point at the given position 
     */
    *lpP = spline(t, lpSplinePoint);
}

/*
 * Generates a ring of vertices in a plane defined by n and the cross
 * product of n and p.  On exit, joint contains the vertices.  Join must
 * be pre-allocated. Normals are generated pointing in.  Texture
 * coordinates are generated along tu axis and are given along tv.
 */
static void 
MakeRing(const D3DVECTOR& p, 
         const D3DVECTOR& d, 
         const D3DVECTOR& n, float tv,
         LPD3DVERTEX joint)
{
    D3DVECTOR nxd = CrossProduct(n, d);
    for (int spoke = 0; spoke < SIDES; spoke++) {
        float theta = (float)(2.0 * PI) * spoke / SIDES;
        /*
         * v, u defines a unit vector in the plane
         * defined by vectors nxd and n.
         */
        float v = (float)sin(theta);
        float u = (float)cos(theta);
        /*
         * x, y, z define a unit vector in standard coordiante space
         */
        D3DVECTOR pt = u*nxd + v*n;
        /*
         * Position, normals and texture coordiantes.
         */
        joint[spoke] = D3DVERTEX(p + pt * TUBE_R, -pt,
                                 1.0f - theta / (float) (2.0f * PI), tv);
	}
}


/*
 * Defines the triangles which form a segment between ring1 and ring2 and
 * stores them at lpTri.  lpTri must be pre-allocated.
 */
void 
MakeSegment(int ring1, int ring2, LPD3DTRIANGLE lpTri)
{
    int side, triangle = 0;

    for (side = 0; side < SIDES; side++) {
        /*
         * Each side consists of two triangles.
         */
        lpTri[triangle].v1 = ring1 * SIDES + side;
        lpTri[triangle].v2 = ring2 * SIDES + side;
        lpTri[triangle].v3 = ring2 * SIDES + ((side + 1) % SIDES);
        
        /*
         * for wireframe only need first two edges.
         * Start a two triangle flat fan for each tunnel face.
         */
                
        lpTri[triangle].wFlags = D3DTRIFLAG_STARTFLAT(1);
        lpTri[triangle].wFlags |= D3DTRIFLAG_EDGEENABLE1 |
                                  D3DTRIFLAG_EDGEENABLE2;
        
        triangle++;
        lpTri[triangle].v2 = ring2 * SIDES + ((side + 1) % SIDES);
        lpTri[triangle].v3 = ring1 * SIDES + ((side + 1) % SIDES);
        lpTri[triangle].v1 = ring1 * SIDES + side;
        
        /*
         * Dont need any edges for wireframe.
         */
        lpTri[triangle].wFlags = D3DTRIFLAG_EVEN;
        
        triangle++;
    }
}


/*
 * Creates a new segment of the tunnel at the current end position.
 * Creates a new ring and segment.
 */
void 
UpdateTubeInMemory(void)
{
    static int texRing = 0; /* Static counter defining the position of
                             * this ring on the texture.
                             */
    int endRing; /* The ring at the end of the tube in memory. */
    int RingOffset, SegmentOffset; /* Offsets into the vertex and triangle 
                                    * lists for the new data.
                                    */
    /*
     * Replace the back ring with a new ring at the front of the tube
     * in memory.
     */
    memmove(&tube.lpV[SIDES], &tube.lpV[0], sizeof(tube.lpV[0]) * (NUM_V - SIDES));
    MakeRing(tube.endP, tube.endD, tube.endN, texRing/(float)TEX_RINGS,
             &tube.lpV[0]);
    /*
     * Replace the back segment with a new segment at the front of the
     * tube in memory. Update the current end position of the tube in
     * memory.
     */
    endRing = (tube.currentRing + SEGMENTS) % (SEGMENTS + 1);
    MoveToPosition(tube.endPos, &tube.endP, &tube.endD, &tube.endN);
    /*
     * Update the execute buffer with the new vertices and triangles.
     */
    RingOffset = sizeof(D3DVERTEX) * tube.currentRing * SIDES;
    SegmentOffset = sizeof(D3DTRIANGLE) * tube.currentSegment * SIDES * 2;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    if (lpD3DExBuf->Lock(&debDesc) != D3D_OK)
        return;
    memcpy((char *) debDesc.lpData,
           &tube.lpV[0], sizeof(D3DVERTEX) * NUM_V);
    lpD3DExBuf->Unlock();
    /*
     * Update the position of the back of the tube in memory and texture
     * counter.
     */
    tube.currentRing = (tube.currentRing + 1) % (SEGMENTS + 1);
    tube.currentSegment = (tube.currentSegment + 1) % SEGMENTS;
    texRing = (texRing + 1) % TEX_RINGS;
}


/*
 * Move the camera through the tunnel.  Create new segments of the tunnel
 * when the camera gets close to the end of the section in memory.
 */
void 
MoveCamera(LPDIRECT3DDEVICE lpDev, LPDIRECT3DVIEWPORT lpView)
{
    /*
     * Update the position on curve and camera vectors.
     */
    tube.cameraPos += (float)SPEED;
    if (tube.cameraPos > PATH_LENGTH)
        tube.cameraPos -= PATH_LENGTH;
    MoveToPosition(tube.cameraPos, &tube.cameraP, &tube.cameraD,
                   &tube.cameraN);
    /*
     * If the camera is close to the end, add a new segment.
     */
    if (tube.endPos - tube.cameraPos < DEPTH) {
        tube.endPos = tube.endPos + (float)SEGMENT_LENGTH;
        if (tube.endPos > PATH_LENGTH)
            tube.endPos -= PATH_LENGTH;
        UpdateTubeInMemory();
    }
}


// update the location of the spot in camera space
// need to be executed only if are NOT in the SCREEN_COORDINATES mode
// or we use the BLT_COMPOSITE method

void updateSWPolygon()
{
#ifndef SCREEN_COORDINATES
	if( compositeMethod  ==  TEXTUREMAP_COMPOSITE)  {
		moveSWPolygon();
        memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
        debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
	if (lpD3DExBufSpot->Lock(&debDesc) != D3D_OK)
        return;
    memcpy((char *) debDesc.lpData, &src_v[0], sizeof(D3DTLVERTEX) * NUM_VERTICES_SPOT);
    lpD3DExBufSpot->Unlock();
    }
#endif
}

/*
 * Modify the buffer between rendering frames
 */
static void 
TickScene(LPDIRECT3DDEVICE lpDev, LPDIRECT3DVIEWPORT lpView)
{
    MoveCamera(lpDev, lpView);
    updateSWPolygon();
}

/*
 * Each frame, renders the scene and calls TickScene to modify the object
 * for the next frame.
 */
BOOL
RenderScene(LPDIRECT3DDEVICE lpDev, LPDIRECT3DVIEWPORT lpView,
            LPD3DRECT lpExtent)
{
    HRESULT ddrval;
	BOOL loadResult;
/*
 * Move the camera by updating the view matrix and move the light.
 */
    PositionCamera(&tube.cameraP, &tube.cameraD, &tube.cameraN, &view);
    ddrval = lpDev->SetMatrix(hView, &view);
    if (ddrval != D3D_OK)
        return FALSE;
    tube.light.dvPosition = tube.cameraP;
    ddrval = tube.lpD3DLight->SetLight(&tube.light);
    if (ddrval != D3D_OK)
        return FALSE;
    
    if( compositeMethod  ==  TEXTUREMAP_COMPOSITE) 
	{

	// Here is one of the synchronization calls.
    // The timeout is 1 second so at least it would move if deadlock were possible
	WaitForSingleObject(eventHandle, 1000);
    // OK, now reset the event and go onto the composite
    ResetEvent(eventHandle);
	
	// now we add the quads drawn by the SW thread to the statistics
	sw_quads_drawn += sw_quads_per_frame ;
	
	// Switch the active and the develope buffers and continue
	lpDDTempSurfaceSystem     = lpDDActiveSurfaceSystem;
    lpDDActiveSurfaceSystem   = lpDDDevelopSurfaceSystem;
    lpDDDevelopSurfaceSystem  = lpDDTempSurfaceSystem;

    // Resume the SW Thread - that is, the main thread has consumed the
    // output of the SW rendering.  
	// We Use double buffering so letting the  SW to go ahead to the next frame early
	SetEvent(compositeHandle);
    // Load texture from the SW thread to be used in the HW scene in the next frame.
   loadResult =  LoadTextureFromSW();
   if(! loadResult)  
		  return FALSE; 
	/* * Execute the instruction buffer and update the view  */
    ddrval = lpDev->BeginScene();
    if (ddrval != D3D_OK)
        return FALSE;
    ddrval = lpDev->Execute(lpD3DExBuf, lpView, D3DEXECUTE_CLIPPED);
             if (ddrval != D3D_OK)    return FALSE;
     ddrval = lpDev->Execute(lpD3DExBufSpot, lpView, D3DEXECUTE_UNCLIPPED);
			 if (ddrval != D3D_OK)   { 
				 char * error_str = D3DAppErrorToString(ddrval);
                 Msg(error_str,0);
                 return FALSE;
			 }
    ddrval = lpDev->EndScene();
			 if (ddrval != D3D_OK)    return FALSE;
    ddrval = lpD3DExBuf->GetExecuteData(&d3dExData);
			 if (ddrval != D3D_OK)    return FALSE;
    ddrval = lpD3DExBufSpot->GetExecuteData(&d3dExDataSpot);
			if (ddrval != D3D_OK)    return FALSE;
	*lpExtent = d3dExData.dsStatus.drExtent;
	}
	else   //   compositeMethod  == BLT_COMPOSITE) 
	{
    /* * Execute the instruction buffer and update the view  */
    ddrval = lpDev->BeginScene();
    if (ddrval != D3D_OK)
        return FALSE;
    ddrval = lpDev->Execute(lpD3DExBuf, lpView, D3DEXECUTE_CLIPPED);
             if (ddrval != D3D_OK)    return FALSE;
    ddrval = lpDev->EndScene();
			 if (ddrval != D3D_OK)    return FALSE;
    ddrval = lpD3DExBuf->GetExecuteData(&d3dExData);
			 if (ddrval != D3D_OK)    return FALSE;
	*lpExtent = d3dExData.dsStatus.drExtent;
    // Here is one of the synchronization calls.
    // The timeout is 1 second so at least it would move if deadlock were possible
	WaitForSingleObject(eventHandle, 1000);
    // OK, now reset the event and go onto the composite
    ResetEvent(eventHandle);
	
	// now we add the quads drawn by the SW thread to the statistics
	sw_quads_drawn += sw_quads_per_frame ;
   }
	/*
     * Modify for the next time around
     */
    TickScene(lpDev, lpView);
    return TRUE;
}

void
OverrideDefaults(Defaults* defaults)
{
    defaults->rs.bZBufferOn = FALSE;
    defaults->rs.bPerspCorrect = TRUE;
    defaults->bClearsOn = FALSE;
    lstrcpy(defaults->Name, "Tunnel D3D Example");
}

BOOL
InitScene(void)
{
    float position;             /* Curve position counter. */
    int i;                      /* counter */

    /*
     * Reserved memory for vertices, triangles and spline points.
     */
    tube.lpV = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * NUM_V);
    tube.lpTri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * NUM_TRI);
    tube.lpPoints = (LPD3DVECTOR) malloc(sizeof(D3DVECTOR)*SPLINE_POINTS);
    /*
     * Generate spline points
     */
    for (i = 0; i < SPLINE_POINTS; i++) {
        tube.lpPoints[i].x = (float)(cos(i * 4.0) * 20.0);
        tube.lpPoints[i].y = (float)(sin(i * 4.0) * 20.0);
        tube.lpPoints[i].z = i * 20.0f;
    }
    /*
     * Create the initial tube section in memory.
     */
    tube.endN.x = (float)0.0;
    tube.endN.y = (float)1.0;
    tube.endN.z = (float)0.0;
    position = (float)0.0;
    for (i = 0; i < SEGMENTS + 1; i++) {
        MoveToPosition(position, &tube.endP, &tube.endD, &tube.endN);
        position += (float)SEGMENT_LENGTH;
        MakeRing(tube.endP, tube.endD, tube.endN, 
                 (float)(i % TEX_RINGS) / TEX_RINGS,
                 &tube.lpV[(SEGMENTS - i) * SIDES]);
    }
    for (i = 0; i < SEGMENTS; i++)
        MakeSegment(i + 1, i, &tube.lpTri[i * SIDES * 2]);
    /*
     * Move the camera to the begining and set some globals
     */
    tube.cameraN.x = (float)0.0;
    tube.cameraN.y = (float)1.0;
    tube.cameraN.z = (float)0.0;
    MoveToPosition((float)0.0, &tube.cameraP, &tube.cameraD, &tube.cameraN);
	tube.currentRing = 0;
    tube.currentSegment = 0;
    tube.cameraPos = (float)0.0;
    tube.endPos = position;
    
    #if defined(SW_SPOT)
	initSwSpot();
    upVec.x = (float)0.0;
    upVec.y = (float)1.0;
    upVec.z = (float)0.0;
    #ifndef SCREEN_COORDINATES
     moveSWPolygon();
    #endif
#endif
	return TRUE;
}

void
ReleaseScene(void)
{
    if (tube.lpPoints)
        free(tube.lpPoints);
    if (tube.lpTri)
        free(tube.lpTri);
    if (tube.lpV)
        free(tube.lpV);
}
 
void
ReleaseView(LPDIRECT3DVIEWPORT lpView)
{
    if (lpView)
        lpView->DeleteLight(tube.lpD3DLight);
    RELEASE(lpD3DExBuf);
    RELEASE(tube.lpD3DLight);
    RELEASE(lpmat);
    RELEASE(lpbmat);
#if defined(SW_SPOT)
    RELEASE(lpD3DExBufSpot);
#endif  
}


/*
 * Builds the scene and initializes the execute buffer for rendering.
 * Returns 0 on failure.
 */
BOOL
InitView(LPDIRECTDRAW lpDD, LPDIRECT3D lpD3D, LPDIRECT3DDEVICE lpDev, 
           LPDIRECT3DVIEWPORT lpView, int NumTextures, 
           LPD3DTEXTUREHANDLE TextureHandle)
{
    /* Variables for exectue buffer generation */
    LPVOID lpBufStart, lpInsStart, lpPointer;
    LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf;
    DWORD size;

    /* Background material variables */
    D3DMATERIAL bmat;
    D3DMATERIALHANDLE hbmat;
    D3DMATERIAL mat;

    /*
     * Set background to black material
     */
    if (lpD3D->CreateMaterial(&lpbmat, NULL) != D3D_OK)
        return FALSE;
    memset(&bmat, 0, sizeof(D3DMATERIAL));
    bmat.dwSize = sizeof(D3DMATERIAL);
    bmat.dwRampSize = 1;
    lpbmat->SetMaterial(&bmat);
    lpbmat->GetHandle(lpDev, &hbmat);
    lpView->SetBackground(hbmat);
    /*
     * Set the view, projection and world matrices in an execute buffer
     */
    MAKE_MATRIX(lpDev, hView, view);
    MAKE_MATRIX(lpDev, hProj, ProjectionMatrix());
    MAKE_MATRIX(lpDev, hWorld, IdentityMatrix());
    /*
     * Create an execute buffer
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE) * 4;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    if (lpDev->CreateExecuteBuffer(&debDesc, &lpD3DExCmdBuf, NULL) != D3D_OK)
        return FALSE;
    if (lpD3DExCmdBuf->Lock(&debDesc) != D3D_OK)
        return FALSE;
    lpBufStart = debDesc.lpData;
    memset(lpBufStart, 0, size);
    lpPointer = lpBufStart;
    /*
     * Fill the execute buffer with instructions
     */
    lpInsStart = lpPointer;
    OP_STATE_TRANSFORM(3, lpPointer);
        STATE_DATA(D3DTRANSFORMSTATE_WORLD, hWorld, lpPointer);
        STATE_DATA(D3DTRANSFORMSTATE_VIEW, hView, lpPointer);
        STATE_DATA(D3DTRANSFORMSTATE_PROJECTION, hProj, lpPointer);
    OP_STATE_LIGHT(1, lpPointer);
        STATE_DATA(D3DLIGHTSTATE_AMBIENT, RGBA_MAKE(40, 40, 40, 40), lpPointer);
    OP_EXIT(lpPointer);
    /*
     * Setup the execute data describing the buffer
     */
    lpD3DExCmdBuf->Unlock();
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char *)lpPointer - (char*)lpInsStart);
    lpD3DExCmdBuf->SetExecuteData(&d3dExData);
    lpDev->BeginScene();
    lpDev->Execute(lpD3DExCmdBuf, lpView, D3DEXECUTE_UNCLIPPED);
    lpDev->EndScene();
    /*
     * We are done with the command buffer.
     */
    lpD3DExCmdBuf->Release();
    /*
     * Setup materials and lights
     */
    tube.hTex = TextureHandle[1];
    if (lpD3D->CreateMaterial(&lpmat, NULL) != D3D_OK)
        return FALSE;
    memset(&mat, 0, sizeof(D3DMATERIAL));
    mat.dwSize = sizeof(D3DMATERIAL);
    mat.diffuse.r = (D3DVALUE)1.0;
    mat.diffuse.g = (D3DVALUE)1.0;
    mat.diffuse.b = (D3DVALUE)1.0;
    mat.diffuse.a = (D3DVALUE)1.0;
    mat.ambient.r = (D3DVALUE)1.0;
    mat.ambient.g = (D3DVALUE)1.0;
    mat.ambient.b = (D3DVALUE)1.0;
    mat.specular.r = (D3DVALUE)1.0;
    mat.specular.g = (D3DVALUE)1.0;
    mat.specular.b = (D3DVALUE)1.0;
    mat.power = (float)20.0;
    mat.dwRampSize = 16;
    mat.hTexture = tube.hTex;
    lpmat->SetMaterial(&mat);
    lpmat->GetHandle(lpDev, &tube.hMat);

    memset(&tube.light, 0, sizeof(D3DLIGHT));
    tube.light.dwSize = sizeof(D3DLIGHT);
    tube.light.dltType = D3DLIGHT_POINT;
    tube.light.dvPosition = tube.cameraP;
    tube.light.dcvColor.r = D3DVAL(0.9);
    tube.light.dcvColor.g = D3DVAL(0.9);
    tube.light.dcvColor.b = D3DVAL(0.9);
    tube.light.dvAttenuation0 = (float)0.0;
    tube.light.dvAttenuation1 = (float)0.0;
    tube.light.dvAttenuation2 = (float)0.05;
    if (lpD3D->CreateLight(&tube.lpD3DLight, NULL)!=D3D_OK)
        return FALSE;
    if (tube.lpD3DLight->SetLight(&tube.light)
        !=D3D_OK)
        return FALSE;
    if (lpView->AddLight(tube.lpD3DLight) != D3D_OK)
        return FALSE;

    /*
     * Create an execute buffer
     */
    size = sizeof(D3DVERTEX) * NUM_V;
    size += sizeof(D3DPROCESSVERTICES);
    size += sizeof(D3DINSTRUCTION) * 6;
    size += sizeof(D3DSTATE) * 4;
    size += sizeof(D3DTRIANGLE) * NUM_TRI;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    if (lpDev->CreateExecuteBuffer(&debDesc, &lpD3DExBuf, NULL) != D3D_OK)
        return FALSE;
    /*
     * lock it so it can be filled
     */
    if (lpD3DExBuf->Lock(&debDesc) != D3D_OK)
        return FALSE;
    lpBufStart = debDesc.lpData;
    memset(lpBufStart, 0, size);
    lpPointer = lpBufStart;
    VERTEX_DATA(tube.lpV, NUM_V, lpPointer);
    /*
     * Save the location of the first instruction and add instructions to
     * execute buffer.
     */
    lpInsStart = lpPointer;
    OP_STATE_LIGHT(1, lpPointer);
        STATE_DATA(D3DLIGHTSTATE_MATERIAL, tube.hMat, lpPointer);
    OP_PROCESS_VERTICES(1, lpPointer);
        PROCESSVERTICES_DATA(D3DPROCESSVERTICES_TRANSFORMLIGHT, 0, NUM_V, lpPointer);
    OP_STATE_RENDER(3, lpPointer);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, tube.hTex, lpPointer);
        STATE_DATA(D3DRENDERSTATE_WRAPU, TRUE, lpPointer);
        STATE_DATA(D3DRENDERSTATE_WRAPV, TRUE, lpPointer);
    /*
     * Make sure that the triangle data (not OP) will be QWORD aligned
     */
    if (QWORD_ALIGNED(lpPointer)) {
        OP_NOP(lpPointer);
    }
    OP_TRIANGLE_LIST(NUM_TRI, lpPointer);
        tube.TriOffset = (char *)lpPointer - (char *)lpBufStart;
        TRIANGLE_LIST_DATA(tube.lpTri, NUM_TRI, lpPointer);
    OP_EXIT(lpPointer);
    /*
     * Setup the execute data describing the buffer
     */
    lpD3DExBuf->Unlock();
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwVertexCount = NUM_V;
    d3dExData.dwInstructionOffset = (ULONG) ((char *)lpInsStart - (char *)lpBufStart);
    d3dExData.dwInstructionLength = (ULONG) ((char *)lpPointer - (char *)lpInsStart);
    lpD3DExBuf->SetExecuteData(&d3dExData);

    
#if  defined(SW_SPOT)
  if(! InitViewSW( lpDD, lpD3D, lpDev, lpView, NumTextures, TextureHandle))
		 return FALSE;
#endif	
return TRUE;
}

/******************************************************************************************************/
/* Here we initial the execute buffer for the software object															*/
/******************************************************************************************************/
BOOL
InitViewSW(LPDIRECTDRAW lpDD, LPDIRECT3D lpD3D, LPDIRECT3DDEVICE lpDev,
	   LPDIRECT3DVIEWPORT lpView, int NumTextures,
	   LPD3DTEXTUREHANDLE TextureHandle)
{
    LPVOID lpBufStart, lpInsStart, lpPointer;
    D3DEXECUTEBUFFERDESC debDesc;
    size_t size;
   
	/*
     * Create an execute buffer
     */
    size = sizeof(D3DVERTEX) * NUM_VERTICES_SPOT;
    size += sizeof(D3DINSTRUCTION) * 6;
    size += sizeof(D3DSTATE) * 2;
    size += sizeof(D3DPROCESSVERTICES);
    size += sizeof(D3DTRIANGLE) * NUM_TRIANGLES_SPOT;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    if (lpDev->CreateExecuteBuffer(&debDesc, &lpD3DExBufSpot, NULL) != D3D_OK) 
	   return FALSE;
    if (lpD3DExBufSpot->Lock( &debDesc) != D3D_OK) 
		return FALSE;
    lpBufStart = debDesc.lpData;
    memset(lpBufStart, 0, size);
    lpPointer = lpBufStart;

    /*
     * Copy vertices to execute buffer
     */
    VERTEX_DATA(&src_v[0], NUM_VERTICES_SPOT, lpPointer);
    /*
     * Setup instructions in execute buffer
     */
	
	lpInsStart = lpPointer;
    OP_PROCESS_VERTICES(1, lpPointer);
#ifdef  SCREEN_COORDINATES
// The Spot need not to be transformed
	PROCESSVERTICES_DATA(D3DPROCESSVERTICES_COPY  |
		     D3DPROCESSVERTICES_UPDATEEXTENTS, 0, NUM_VERTICES_SPOT, lpPointer);
#else
// we need only to transform the Spot (not to light it)
     PROCESSVERTICES_DATA(D3DPROCESSVERTICES_TRANSFORM, 0, NUM_VERTICES_SPOT, lpPointer);
#endif
	 OP_STATE_RENDER(2, lpPointer);
	  // the textured pointed by this hande will be loaded
	  // from the SW thread each frame
	  STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, SPOTTextureHandle, lpPointer);
	  // we use the color key flag to get transparent texture (the spot)
	  //STATE_DATA(D3DRENDERSTATE_BLENDENABLE, FALSE, lpPointer);
	  //STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE, lpPointer);
	  STATE_DATA(D3DRENDERSTATE_COLORKEYENABLE, TRUE, lpPointer);
    
	/*
     * Make sure that the triangle data (not OP) will be QWORD aligned
     */
    if (QWORD_ALIGNED(lpPointer)) {
	OP_NOP(lpPointer);
    }
   /* 
   OP_TRIANGLE_LIST(NUM_TRIANGLES_SPOT, lpPointer);
	//	TRIANGLE_LIST_DATA(src_tri, NUM_TRIANGLES_SPOT, lpPointer);
    lpPointer = ((char*)lpPointer) + (sizeof(D3DTRIANGLE) * NUM_TRIANGLES_SPOT);
    OP_EXIT(lpPointer);
   */
	OP_TRIANGLE_LIST(NUM_TRIANGLES_SPOT, lpPointer);
        ((LPD3DTRIANGLE)lpPointer)->v1 = 0;
        ((LPD3DTRIANGLE)lpPointer)->v2 = 1;
        ((LPD3DTRIANGLE)lpPointer)->v3 = 2;
        ((LPD3DTRIANGLE)lpPointer)->wFlags = D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE3; 
		lpPointer = ((char*)lpPointer) + sizeof(D3DTRIANGLE);
	    ((LPD3DTRIANGLE)lpPointer)->v1 = 3;
        ((LPD3DTRIANGLE)lpPointer)->v2 = 2;
        ((LPD3DTRIANGLE)lpPointer)->v3 = 1;
        ((LPD3DTRIANGLE)lpPointer)->wFlags = D3DTRIFLAG_EDGEENABLE1 | D3DTRIFLAG_EDGEENABLE3;
        lpPointer = ((char*)lpPointer) + sizeof(D3DTRIANGLE);
    OP_EXIT(lpPointer);
	
	/*
     * Setup the execute data
     */
    lpD3DExBufSpot->Unlock();
    memset(&d3dExDataSpot, 0, sizeof(D3DEXECUTEDATA));
    d3dExDataSpot.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExDataSpot.dwVertexCount = NUM_VERTICES_SPOT;
    d3dExDataSpot.dwInstructionOffset = (ULONG) ((char *)lpInsStart - (char *)lpBufStart);
    d3dExDataSpot.dwInstructionLength = (ULONG) ((char *)lpPointer - (char *)lpInsStart);
    lpD3DExBufSpot->SetExecuteData( &d3dExDataSpot);

    return TRUE;
}

void  moveSWPolygon()
{
   static float x_corner[4] = { -0.25f ,  0.25f ,   -0.25f  , 0.25f  } ;
   static float y_corner[4] = {  0.25f ,  0.25f ,   -0.25f  ,-0.25f  } ;
   static float swPolyPosition , u, v;
   static D3DVECTOR nxd , p , pt ;
   swPolyPosition  = tube.cameraPos  + 6.0f * (float)SPEED ; 
   SplineAtPosition( swPolyPosition, &onCurve);

   // While the camera is at Spline( t[i] ) the spot is LOCATED at Spline( t[i+6] ) . 
   // Now we have two choises for the Spot plane (one at t[i+6] and one at t[i]),
   // in the first the spot plane will not be parallel to the one spanned by the camera
   // Up and 'x' vectors and in the second it will be .
   
   //  parallel
   nxd  =CrossProduct (tube.cameraN ,  tube.cameraD ); 
   Normalize(nxd);

  for (int i = 0; i < 4; i++) {
        /*
         * v, u defines a unit vector in the plane
         * defined by vectors nxd and n.
         */
        u = x_corner[i];
        v = y_corner[i];
        /*  * x, y, z define a (~unit) vector in standard coordiante space */
        pt = u*nxd + v*tube.cameraN;
		p  = onCurve + pt;
		/*
         * Position, normals and texture coordiantes.
         */
       src_v[i].sx =  p.x ;
	   src_v[i].sy =  p.y ; 
       src_v[i].sz =  p.z ;
   }
}

void initSwSpot()
{
   // V 0 
    src_v[0].rhw = D3DVAL(1.0);
    src_v[0].color = RGBA_MAKE(255, 255, 255, 255);
    src_v[0].specular = RGB_MAKE(255,255, 255);
    src_v[0].tu = D3DVAL(0.25);
    src_v[0].tv = D3DVAL(0.25);
    // V 1
    src_v[1].rhw = D3DVAL(1.0);
    src_v[1].color = RGBA_MAKE(255, 255, 255, 255);
    src_v[1].specular = RGB_MAKE(255,255, 255);
    src_v[1].tu = D3DVAL(0.75);
    src_v[1].tv = D3DVAL(0.25);
	// V 2 
    src_v[2].rhw = D3DVAL(1.0);
    src_v[2].color = RGBA_MAKE(255, 255, 255, 255);
    src_v[2].specular = RGB_MAKE(255,255, 255);
    src_v[2].tu = D3DVAL(0.25);
    src_v[2].tv = D3DVAL(0.75);
    // V3
    src_v[3].rhw = D3DVAL(1.0);
    src_v[3].color = RGBA_MAKE(255, 255, 255, 255);
    src_v[3].specular = RGB_MAKE(255,255, 255);
    src_v[3].tu = D3DVAL(0.75);
    src_v[3].tv = D3DVAL(0.75);

#ifdef SCREEN_COORDINATES
   // V 0 
    src_v[0].sx = D3DVAL(200.0);
    src_v[0].sy = D3DVAL(180.0);
    src_v[0].sz = D3DVAL(1.0);
    // V 1
    src_v[1].sx = D3DVAL(455.0);
    src_v[1].sy = D3DVAL(180.0);
    src_v[1].sz = D3DVAL(1.0);
	// V 2 
    src_v[2].sx = D3DVAL(200.0);
    src_v[2].sy = D3DVAL(435.0);
    src_v[2].sz = D3DVAL(1.0);
    // V3
	src_v[3].sx = D3DVAL(455.0);
    src_v[3].sy = D3DVAL(435.0);
    src_v[3].sz = D3DVAL(1.0);
#endif	
	memset(&src_tri[0], 0, sizeof(D3DTRIANGLE) * NUM_TRIANGLES_SPOT);
    /* Triangle 0 */ 
    src_tri[0].v1 = 0;
    src_tri[0].v2 = 1;
	src_tri[0].v3 = 2;
    src_tri[0].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
    
	// Triangle 1 
	src_tri[1].v1 = 1;
    src_tri[1].v2 = 2;
	src_tri[1].v3 = 3;
    src_tri[1].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
}


