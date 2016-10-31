#ifndef __MAIN_H__
#define __MAIN_H__

#define SYNCH_WITH_SW

#ifdef __cplusplus
extern "C" {
#endif

extern D3DAppInfo d3dappi;

// for synchronization between HW and SW threads
extern HANDLE eventHandle;
extern HANDLE compositeHandle;

// We have two composite methods : 
#define BLT_COMPOSITE        0
#define TEXTUREMAP_COMPOSITE 1

// current composite method
 extern int compositeMethod  ;
// for TEXTUREMAP_COMPOSITE method 
	
    // buffers in System memory 
	extern	  LPDIRECT3DTEXTURE	       lpActiveTexture;
	extern    LPDIRECTDRAWSURFACE  lpDDActiveSurfaceSystem;
	extern    LPDIRECTDRAWSURFACE  lpDDDevelopSurfaceSystem;
	extern    LPDIRECTDRAWSURFACE  lpDDTempSurfaceSystem ;

	// Texture - Mapping staff in Video memory
	extern    LPDIRECTDRAWSURFACE   lpDDTextureSurfaceSPOT;
	extern    LPDIRECT3DTEXTURE         lpSPOTTexture ;
	extern    D3DTEXTUREHANDLE         SPOTTextureHandle;
 
// for BLT_COMPOSITE method
 extern    LPDIRECTDRAWSURFACE  lpDDSurfaceSystem;

// Z buffer for the SW thread
 extern    LPDIRECTDRAWSURFACE     lpDDZBufferSystem;

 BOOL InitSwScene(void);
 BOOL RenderSWScene(LPDIRECTDRAWSURFACE lpDevelopSurface);

 // statistics params of the SW thread 
 extern  unsigned long  sw_quads_per_frame;
 extern  unsigned long  sw_quads_drawn ;

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__