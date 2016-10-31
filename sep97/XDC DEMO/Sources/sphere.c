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
/*
  =========================================================
 * 
 * SW thread is procedural textured object.
 * 
 =========================================================
*/
#include <d3d.h>
#include <math.h>
#include <malloc.h>
#include <stdio.h> 
#include "d3dmacs.h"
#include "d3ddemo.h"
#include "main.h"
#include "render.h"
#include "procedural.h"
/*************************************************************************************/
// relative size of the object
#define OBJECT_SIZE_X 64.0f
#define OBJECT_SIZE_Y 64.0f
#define OBJECT_SIZE_Z 64.0f
// fineness of the object's geometry
static int my_rings = 18 , my_sections = 22;
unsigned long  sw_quads_per_frame;
unsigned long  sw_quads_drawn ;
/*************************************************************************************/
/* * A structure which holds the object's data  */
static struct {
    LPD3DVERTEX lpV;                       /* object's vertices     */
    LPD3DTRIANGLE lpTri;                 /*  object's triangles  */
    int num_vertices, num_faces;
    LPD3DVERTEX lpVOutput;  
} objData;
/**********************************************************************************/
static D3DMATRIX world, spin;
// inv spin will be used to transform the light source to object space (each frame) .
static D3DMATRIX  inv_spin;
/**********************************************************************************/
// param for the application -   if to change the noise resulotion for each frame
int change_texture_res  = 0 ;
long NoiseResolution    = 256;
static long NoiseInc       = -16;


// params for the renderer ;
extern int WITH_ZBUFFER;
extern  int texture_method ;
extern  int marble_method;
extern  int LIGHT;
// params of the noise module 
extern void calcMarbleTable(int method);
extern  unsigned long  num_octaves;

// params for the shader
extern int shading_method;
extern  float spec_power , specs[6];
extern int    spec_index;
extern int    shader_perturb_method;
extern float  inv_fixed_for_shader;  
/**********************************************************************************/
DDSURFACEDESC ddsdSp;
DDSURFACEDESC ZddsdSp;
extern signed __int16	*pZBuffer;
extern	int  ZBufferWidth;
/*************************************************************************************/
static  void  objectRender(void);
static  void  transformObject(void);
static  BOOL TickSpotScene();
/**********************************************************************************/
/* For each frame called to render the SW scene ( Torus )
/**********************************************************************************/
BOOL
RenderSWScene(LPDIRECTDRAWSURFACE lpDevelopSurface)
{
 HRESULT result;
    //Lock the buffer because we're getting ready to draw to the Develop buffer .
    ddsdSp.dwSize = sizeof(ddsdSp);
    result = lpDevelopSurface->lpVtbl->Lock(lpDevelopSurface, NULL, &ddsdSp,DDLOCK_WAIT, NULL);
    if (result != DD_OK) {
      lpDevelopSurface->lpVtbl->Unlock(lpDevelopSurface, NULL);
      Msg("ERROR\0", result);
	  return(FALSE);
    }
    // handle the Z buffer        
	ZddsdSp.dwSize = sizeof(ZddsdSp);
	result = lpDDZBufferSystem->lpVtbl->Lock
		          (lpDDZBufferSystem, NULL, &ZddsdSp,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL) ;
	if (result != DD_OK) {
        lpDevelopSurface->lpVtbl->Unlock(lpDevelopSurface, NULL);
		lpDDZBufferSystem->lpVtbl->Unlock(lpDDZBufferSystem, NULL);
	    return(FALSE);
    }
    pZBuffer          =  (signed __int16*)ZddsdSp.lpSurface; // pointer to the Z-buffer
    ZBufferWidth   =   ZddsdSp.dwWidth;  // (ZddsdSp.lPitch >> 1) ;
	transformObject();  // do geomtry 
    objectRender();      // do rendering 
	// We're done writing to the Develop buffer so unlock the surface.
    lpDevelopSurface->lpVtbl->Unlock(lpDevelopSurface, NULL);
    lpDDZBufferSystem->lpVtbl->Unlock(lpDDZBufferSystem,  NULL);
    if (!(TickSpotScene()))
        return FALSE;
return TRUE;
}

/**********************************************************************************/
// if you want to render sphere uncomment this define and call generateSphere 
// instead in initScene .
// #define RENDER_SPHERE
/**********************************************************************************/
VERTEX   *myVertexList ;
void objectRender(void)   
{
 static int ring , section ,i, j , m , loc_sections , triangle;
 static VERTEX *vrtxPtr;

#ifdef RENDER_SPHERE
m = 1;
#else
m = 0;
#endif
loc_sections = (my_sections +1);
for(i=0; i< objData.num_vertices; i++) {
     myVertexList[i].x =  my_ftol(objData.lpV[i].x);
 	 myVertexList[i].y =  my_ftol(objData.lpV[i].y);	
	 myVertexList[i].z =  my_ftol(objData.lpV[i].z);
 }	
 
 for(ring  = 0; ring <  my_rings ; ring++)  { 
	for (section = 0; section < my_sections  ; section++) {    
		  for(j=0; j < 4; j++) {
          switch(j) {
              case 0 :
					vrtxPtr  = &(myVertexList[m + section]); break;
			  case 1 : 
					vrtxPtr  = &(myVertexList[m +loc_sections + section]); break;
			  case 2 : 
			  		vrtxPtr  = &(myVertexList[m + loc_sections+(section + 1) ]); break;
                case 3 : 
		  		    vrtxPtr  = &(myVertexList[m + (section + 1)]); break;
		  } 
	        memcpy(&(my_Poly.vertex[j]),vrtxPtr,sizeof(VERTEX));
         }
        TextureMapPolygonInverse(ddsdSp.lpSurface, 
								 (ddsdSp.lPitch >> 1), 
								  ddsdSp.dwHeight, 
								  NoiseResolution, NoiseResolution);
         }
   m  += loc_sections;  
 }

// Comment out next 2 lines to stop "frequency pulsating"
if (change_texture_res ) {
    NoiseResolution += NoiseInc;
	
	if(texture_method  == MARBLE) {
	    if ((NoiseResolution < 120) || (NoiseResolution > 912))
	    NoiseInc *= -1; 
	  }
    else { // texture_method  == WOOD
          if ((NoiseResolution < 280) || (NoiseResolution > 400))
          //if ((NoiseResolution < 120) || (NoiseResolution > 912))
	        NoiseInc *= -1; 
	  } 
}


}
/**********************************************************************************/
/* 
    We can transform the Light direction instead of the Normals of the Object !!!
	(This is done in D3D also).
    If you commnet TRANSFORM_LIGHT the object's Normals will be 
	transformed for each frame.
    'orig_light' is the light direction. 
	You can change it arbitrarily (don't worry, it will be normalized ! ).
*/
#define TRANSFORM_LIGHT
D3DVECTOR orig_light = { 0.0f,0.0f,-1.0f };
extern D3DVERTEX lightDir;
/************************************************************************************/
// preform  the transformation for each frame.
  void transformObject() {  
#ifdef TRANSFORM_LIGHT
    transformCopyD3DVERTEX(&inv_spin ,&orig_light,&orig_light);
#else	 
	transformD3DNormals(&spin, objData.num_vertices, objData.lpVOutput);
#endif
	memcpy(&lightDir,&orig_light,sizeof(D3DVECTOR));
   	 
    memcpy(objData.lpV,objData.lpVOutput,sizeof(D3DVERTEX)*objData.num_vertices);
    transformD3DVERTEX(&world, objData.num_vertices, objData.lpV);
  }
/*****************************************************************************/

    D3DMATRIX identity = {
    D3DVAL(1.0), D3DVAL(0.0), D3DVAL(0.0), D3DVAL(0.0),
    D3DVAL(0.0), D3DVAL(1.0), D3DVAL(0.0), D3DVAL(0.0),
    D3DVAL(0.0), D3DVAL(0.0), D3DVAL(1.0), D3DVAL(0.0),
    D3DVAL(0.0), D3DVAL(0.0), D3DVAL(0.0), D3DVAL(1.0)
};

BOOL
TickSpotScene()
{ 
  MultiplyD3DMATRIX(&world, &spin, &world);
   return TRUE;
}

void
InitSpin(void)
{
    spin = identity;
    ConcatenateYRotation(&spin, D3DVAL(5.0));
    ConcatenateXRotation(&spin, D3DVAL(3.5));
    ConcatenateZRotation(&spin, D3DVAL(2.0));
	
	D3DVECTORNormalise(&orig_light); 
    D3DMATRIXInvert(&inv_spin,&spin);

}
/*****************************************************************************/
extern void initRenderTables(void);
BOOL
InitSwScene(void)
{
    //int ring , section ,i, j , m , loc_sections;
    int i;
	world = identity;
    world._11  *= D3DVAL(OBJECT_SIZE_X);
    world._22  *= D3DVAL(OBJECT_SIZE_Y); 
    world._33  *= D3DVAL(OBJECT_SIZE_Z); 
	InitSpin();
    init_noiseTables();
    initRenderTables();	
	
	/* * Generate the sphere. */
#ifdef RENDER_SPHERE
	if (!(GenerateSphereClose(1.0f, 
											my_rings, my_sections, 
											1.02f,1.23f,1.12f, 
											&objData.lpV,				  &objData.lpTri,
											&objData.num_vertices, &objData.num_faces)))
#else	      
  	if (!(GenerateTorus(0.6f, 0.25f, 
		                         my_rings , my_sections, 
								 1.0f, 1.0f,1.0f, 
								 &objData.lpV, &objData.lpTri,
								&objData.num_vertices, &objData.num_faces)))
#endif
   	    return FALSE;
/*
  if(!(GenerateEllipsoid(1.1f,1.5f,1.9f, 
		                         my_rings ,my_sections, 
								 &objData.lpV, &objData.lpTri,
                                  &objData.num_vertices, &objData.num_faces)))
*/

  // initialize counters
  sw_quads_per_frame = my_sections * my_rings ;
  sw_quads_drawn        = 0 ;

 objData.lpVOutput = 
	     (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * objData.num_vertices);
  memcpy(objData.lpVOutput,objData.lpV,sizeof(D3DVERTEX) * objData.num_vertices);  
  if (!(SetKeyboardCallback(KeyboardHandler)))
        return FALSE;
  
  myVertexList = (VERTEX *) malloc (sizeof(VERTEX)  * objData.num_vertices);
  for(i=0; i< objData.num_vertices; i++) {
      memcpy(&(myVertexList[i]),&(objData.lpV[i]),sizeof(D3DVERTEX));
  }
 /* 
 myPolyList = (POLYGON *) malloc (sizeof(POLYGON)  *  my_rings * my_sections);
 m = 0;
 loc_sections = (my_sections +1);
 for(ring  = 0; ring <  my_rings ; ring++)  
	for (section = 0; section < my_sections  ; section++) {    
		  for(j=0; j < 4; j++) 
             switch(j) {
                case 0 : myPolyList[i].V[0] = m + section;                    break;
			    case 1 : myPolyList[i].V[1] = m + loc_sections + section;     break;
			    case 2 : myPolyList[i].V[2] = m + loc_sections + section + 1; break;
                case 3 : myPolyList[i].V[3] = m + section + 1 ;				  break;
		    }
		 
       m  += loc_sections;  
	} */
 return TRUE;
 
}
/************************************************************************************/
/************************************************************************************/
#include "d3dres.h"
BOOL CDECL
KeyboardHandler(UINT message, WPARAM wParam, LPARAM lParam) {
    D3DMATRIX m = identity;

	int userKey = LOWORD(wParam);
    
	if (userKey == MENU_SW_SCALE_UP) {
            m._11 = D3DVAL(1.5);
            m._22 = D3DVAL(1.5);
            m._33 = D3DVAL(1.5);
            MultiplyD3DMATRIX(&world,&world,  &m);
			return TRUE;
        } else  if (userKey == MENU_SW_SCALE_DOWN){
            m._11 = D3DVAL(0.9);
            m._22 = D3DVAL(0.9);
            m._33 = D3DVAL(0.9);
            MultiplyD3DMATRIX(&world,&world,  &m);
			return TRUE;
        }   else if (userKey == MENU_SW_TOGGLE_LIGHT) {
			LIGHT  =  (LIGHT + 1) % 2; 
		    return TRUE;
         }  else if (userKey == MENU_SW_COS_POWER) {
			spec_index   =  (spec_index+ 1) % 9;
			spec_power = specs[	spec_index ];
		    return TRUE;
         } else if (userKey == MENU_SW_LIGHT_MODE) {
			shading_method =  (shading_method + 1) % 2;
			return TRUE;
        }  else if (userKey == MENU_SW_TOGGLE_LIGHT_EFFECT) {
			shader_perturb_method =  (shader_perturb_method + 1) % 2;
		    if(shader_perturb_method) inv_fixed_for_shader = 1.0f / 64;
		    else					  inv_fixed_for_shader = 1.0f / 512;
			return TRUE;
		} else if (userKey == MENU_SW_TOGGLE_RESOLUTION) {
			change_texture_res =  (change_texture_res + 1) % 2;
		    return TRUE;
        }  else if (userKey == MENU_SW_OCTAVE_NUM) {
			if(texture_method == MARBLE) {
						num_octaves =  (num_octaves + 1) % 7;
		    	        if(num_octaves == 0) num_octaves = 2;
			   }
			else  {  
				
				        num_octaves =  (num_octaves + 1) % 6;
			            if(num_octaves == 0) num_octaves = 3;
				/*
						num_octaves =  (num_octaves + 1) % 7;
		    	        if(num_octaves == 0) num_octaves = 1;
				*/		
			}
			return TRUE;
		} else if (userKey == MENU_SW_MARBLE_METHOD) {
			marble_method =  (marble_method + 1) % 5;
		   calcMarbleTable(marble_method);
			return TRUE;
		} else if (userKey == MENU_SW_TOGGLE_TEXTURE) {
			texture_method =  (texture_method + 1) % 2;
			if(texture_method == WOOD) {
					if(LIGHT) LIGHT = 0;
					num_octaves = 3;
                    NoiseResolution = 350;
				  }
		   return TRUE;
		} else 	if(userKey == MENU_SW_TOGGLE_ZBUF) {
            WITH_ZBUFFER = (WITH_ZBUFFER + 1) %2; 
			return TRUE;
		} 
    return FALSE;

}








