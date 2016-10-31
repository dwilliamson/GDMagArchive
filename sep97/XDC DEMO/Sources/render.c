/**************************************************************************

Mixed Rendering

 **************************************************************************/
/**************************************************************************
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
 **************************************************************************/
#include "render.h"
/************************************************************************/
// uncomment this define to render with only wireframe 
 //#define WIREFRAME
/* 
     Below are different params for the renderer and the shader.
	 These params can be changed by the user.
*/
  int texture_method     = 0;
  int marble_method     = 0;
  
  /* commnet this define to when you want the light direction
      to be (0,0,-1) which  the Z axes.
      You have to coordinate this commenting with the COMMneting of
      TRANSFORM_LIGHT in sphere.c !!!
 */
  #define GENERAL_LIGHT_DIR
  
  /* 
	  If you comment this define we the absolute value of N*L will be
      taken so the light will actually come from both sides of the object.
  */
  #define CHECK_NORMALS_SIGN
  
  D3DVECTOR  lightDir;
  int LIGHT                 = 1;
  float spec_power    = 3.0f;
  int   spec_index      = 1;
  float specs[9] = { 1.0f,3.0f,5.0f,9.0f ,17.0f,33.0f,65.0f,129.0f,257.0f};
  
  int shading_method                =  1;
  int shader_perturb_method    =  0;  
  float inv_fixed_for_shader      = 1.0f / 512;
/********************************************************************************/
/* 
      These tables are from the noise part, the shader uses them.
*/
extern unsigned __int16 onlyTurb[] , turbulenceTbl[] ;
/**************************************************************************/
/*
 *  Z buffer 
 */
void writeZBuffer(unsigned __int16 * screen_buf, unsigned int num_pixels,
						signed __int16 * z_buf,
						signed __int16 startZ, signed __int16 ZBufInc);
/* 
	This function does the Lighting
*/
 void modulateColor(unsigned __int16 * screen_buf, int num_pixels, 
						               float i_init  , float di);
/***************************************************************************/
/* This is our fast ftol which actually rounds to nearset instead
    of the Ansi C truncation requirement . Since we don't change the FP 
	control word it's faster .
The code is  in MMXtexture.asm 
 */
/***************************************************************************/
extern signed long my_ftol(float d);
/*************************************************************************/
  /*  
  TextureMapPolygonInverse - renderer, 
  Description:
    Texture maps the polygon described by Polygon vertices and writes
    the results into the screen DIB.
    This is regular scanline code without perspective correction.
*/
 /*****************************************************************************/
/* 
     params for the Zbuffer. 
*/
int  WITH_ZBUFFER = 1;
signed __int16	*pZBuffer;
int			     ZBufferWidth;
static long      DestStartZ , fDestStartZ,fDestEndZ ,ZDestOffset , fCurrentZ;
static signed __int16  * pZBufferInitial,fZBufferInc;
static long   ZStartGap;
static float zInvPixNum ;

/**********************************************************************************/
  float shft1left16	 = (float)(1<<16);
  float shft1left8    = (float)(1<<8);
  unsigned char zshiftleft      = 6;
  unsigned char xshiftleft      = 16;
/**********************************************************************************/
  long  Tx,Ty,Tz,Lx,Ly,Lz,Rx,Ry,Rz,Bx,By,Bz;
  long  fULx, fULy, fULz, 
	      fURx, fURy, fURz, 
		  fLLx, fLLy, fLLz, 
		  fLRx, fLRy, fLRz;
  long  fULabsleny, fURabsleny, fLLabsleny, fLRabsleny;
  long  num_lines[3];
  long  line,count;
  
  float u0, v0, u2, v2 ,i0,i2;
  long fSrcLineStartX,fSrcLineStartY;
  long dudx,dvdx;
  float didx;
  long num_pixels;
  float inv_pixnum;
  __int16* pixels_buf;
  
  int     jj, min_y_index, max_y_index, miny, maxy;

  long  fDestStartX,            fDestEndX,          fDestY;
  long  fDestDeltaTX,           fDestDeltaLX,       fDestDeltaRX,    fDestDeltaBX;
  long  fDestDeltaTZ,           fDestDeltaLZ,       fDestDeltaRZ,    fDestDeltaBZ;

  long  SrcWidth, SrcHeight;
  long  DestY, DestStartX, DestEndX, DestOffset;
  
   long  fDestStartXArray[3],    fDestEndXArray[3];
  long  fDestStartDeltaX[3],    fDestEndDeltaX[3];
  static long  fDestStartZArray[3],    fDestEndZArray[3];
  static long  fDestStartDeltaZ[3],    fDestEndDeltaZ[3];
  int   orientation;

  float  Tu,Tv,Lu,Lv,Ru,Rv,Bu,Bv;
  float  fDestStartUArray[3],    fDestEndUArray[3];
  float  fDestStartDeltaU[3],    fDestEndDeltaU[3];
  float  fDestStartVArray[3],    fDestEndVArray[3];
  float  fDestStartDeltaV[3],    fDestEndDeltaV[3];
  float  fDestDeltaTU,           fDestDeltaLU,       fDestDeltaRU,    fDestDeltaBU;
  float  fDestDeltaTV,           fDestDeltaLV,       fDestDeltaRV,    fDestDeltaBV;
  float  fDestStartU  , fDestEndU   ,fDestStartV  , fDestEndV;

static float Ti , Li , Ri, Bi , Normals[4][3];
static float  Tix,Tiy,Tiz, Lix,Liy,Liz, Rix,Riy,Riz, Bix,Biy,Biz;
static float  fDestStartIArray[3],    fDestEndIArray[3];
static float  fDestStartDeltaI[3],    fDestEndDeltaI[3];
static float  fDestDeltaTI,fDestDeltaLI,fDestDeltaRI, fDestDeltaBI;
static float  fDestStartI  , fDestEndI ;

/*************************************************************************/
/* To save FP divide we can  keep the 1 / dx in a pre computed table */
#define DIV_TABLE
static float invDeltax[256];
void initRenderTables(void)
{
int dx;
for (dx =1; dx < 256; dx++)
    invDeltax[dx] = 1.0f / dx ;
}
/*************************************************************************/


    long  fSrcTopDelta,           fSrcRightDelta,     fSrcBottomDelta, fSrcLeftDelta;
    long  SignX, SignY, SrcOffsetX, SrcOffsetY;
    long  fSrcEdgeStartDeltaX[3], fSrcEdgeStartDeltaY[3];
    long  fSrcEdgeEndDeltaX[3],   fSrcEdgeEndDeltaY[3];

void TextureMapPolygonInverse( 
						   short    *pRenderBuffer,
						   int       ScreenDibWidth,
						   int       ScreenDibHeight,
						   int       TextureDIBWidth,
						   int       TextureDIBHeight)
						  
{
    // Determine quadrilateral orientation; only 4 possible cases after culling
	//
	//		3				0				1				2
	//	  /	  \           /	  \           /	  \           /	  \ 
	//	 0	   2     	 1	   3     	 2	   0     	 3	   1
	//	  \   /			  \   /           \   /           \   /
	//		1               2               3               0
	//
	//	  case A          case B          case C	      case D


  min_y_index = 0;
  max_y_index = 0;
	
  for(jj=1; jj<4; jj++)
  {
	if(my_Poly.vertex[jj].y <  my_Poly.vertex[min_y_index].y) min_y_index = jj;
    if(my_Poly.vertex[jj].y >  my_Poly.vertex[max_y_index].y) max_y_index = jj;
  }

  miny   = my_Poly.vertex[min_y_index].y;
  maxy  = my_Poly.vertex[max_y_index].y;
 
  if(max_y_index == 3)   // case A
  {
    Lx = my_Poly.vertex[0].x; Ly = my_Poly.vertex[0].y; Lz = my_Poly.vertex[0].z;
    Tx = my_Poly.vertex[3].x; Ty = my_Poly.vertex[3].y; Tz = my_Poly.vertex[3].z;
    Rx = my_Poly.vertex[2].x; Ry = my_Poly.vertex[2].y; Rz = my_Poly.vertex[2].z;
    Bx = my_Poly.vertex[1].x; By = my_Poly.vertex[1].y; Bz = my_Poly.vertex[1].z;
    SrcOffsetX                   = TextureDIBWidth;
    SrcOffsetY                   =  0;
    SignX                        =  1;
    SignY                        =  1;
	Lu = my_Poly.vertex[0].u; Lv = my_Poly.vertex[0].v;
    Tu = my_Poly.vertex[3].u; Tv = my_Poly.vertex[3].v; 
    Ru = my_Poly.vertex[2].u; Rv = my_Poly.vertex[2].v; 
    Bu = my_Poly.vertex[1].u; Bv = my_Poly.vertex[1].v; 

#ifdef GENERAL_LIGHT_DIR
    Lix = my_Poly.vertex[0].nx; Liy = my_Poly.vertex[0].ny;  Liz = my_Poly.vertex[0].nz; 
	Tix = my_Poly.vertex[3].nx; Tiy = my_Poly.vertex[3].ny;  Tiz = my_Poly.vertex[3].nz;
	Rix = my_Poly.vertex[2].nx; Riy = my_Poly.vertex[2].ny; Riz = my_Poly.vertex[2].nz;
	Bix = my_Poly.vertex[1].nx; Biy = my_Poly.vertex[1].ny; Biz = my_Poly.vertex[1].nz;
#else
	Li  = my_Poly.vertex[0].nz; 
	Ti  = my_Poly.vertex[3].nz;
	Ri  = my_Poly.vertex[2].nz;
	Bi  = my_Poly.vertex[1].nz;
#endif
}

  else if(max_y_index == 0)   // case B
  {
    Lx = my_Poly.vertex[1].x; Ly = my_Poly.vertex[1].y; Lz = my_Poly.vertex[1].z;
    Tx = my_Poly.vertex[0].x; Ty = my_Poly.vertex[0].y; Tz = my_Poly.vertex[0].z;
    Rx = my_Poly.vertex[3].x; Ry = my_Poly.vertex[3].y; Rz = my_Poly.vertex[3].z;
    Bx = my_Poly.vertex[2].x; By = my_Poly.vertex[2].y; Bz = my_Poly.vertex[2].z;
    SrcOffsetX                   =  0;
    SrcOffsetY                   =  0;
    SignX                        =  -1;
    SignY                        =  1;
	
	Lu = my_Poly.vertex[1].u; Lv = my_Poly.vertex[1].v; 
    Tu = my_Poly.vertex[0].u; Tv = my_Poly.vertex[0].v; 
    Ru = my_Poly.vertex[3].u; Rv = my_Poly.vertex[3].v; 
    Bu = my_Poly.vertex[2].u; Bv = my_Poly.vertex[2].v; 

#ifdef GENERAL_LIGHT_DIR
    Lix = my_Poly.vertex[1].nx; Liy = my_Poly.vertex[1].ny;  Liz = my_Poly.vertex[1].nz; 
	Tix = my_Poly.vertex[0].nx; Tiy = my_Poly.vertex[0].ny;  Tiz = my_Poly.vertex[0].nz;
	Rix = my_Poly.vertex[3].nx; Riy = my_Poly.vertex[3].ny; Riz = my_Poly.vertex[3].nz;
	Bix = my_Poly.vertex[2].nx; Biy = my_Poly.vertex[2].ny; Biz = my_Poly.vertex[2].nz;
#else
	Li  = my_Poly.vertex[1].nz;
	Ti  = my_Poly.vertex[0].nz;
	Ri  = my_Poly.vertex[3].nz;
	Bi  = my_Poly.vertex[2].nz;
#endif 
 }

  else  if(max_y_index == 1)   // case C
  {
    Lx = my_Poly.vertex[2].x; Ly = my_Poly.vertex[2].y; Lz = my_Poly.vertex[2].z;
    Tx = my_Poly.vertex[1].x; Ty = my_Poly.vertex[1].y; Tz = my_Poly.vertex[1].z;
    Rx = my_Poly.vertex[0].x; Ry = my_Poly.vertex[0].y; Rz = my_Poly.vertex[0].z;
    Bx = my_Poly.vertex[3].x; By = my_Poly.vertex[3].y; Bz = my_Poly.vertex[3].z;
    SrcOffsetX                   =  0;
    SrcOffsetY                   = TextureDIBHeight;
    SignX                        =  -1;
    SignY                        =  -1;
	
	Lu = my_Poly.vertex[2].u; Lv = my_Poly.vertex[2].v; 
    Tu = my_Poly.vertex[1].u; Tv = my_Poly.vertex[1].v; 
    Ru = my_Poly.vertex[0].u; Rv = my_Poly.vertex[0].v; 
    Bu = my_Poly.vertex[3].u; Bv = my_Poly.vertex[3].v; 
	
#ifdef GENERAL_LIGHT_DIR
    Lix = my_Poly.vertex[2].nx; Liy = my_Poly.vertex[2].ny;  Liz = my_Poly.vertex[2].nz; 
	Tix = my_Poly.vertex[1].nx; Tiy = my_Poly.vertex[1].ny;  Tiz = my_Poly.vertex[1].nz;
	Rix = my_Poly.vertex[0].nx; Riy = my_Poly.vertex[0].ny; Riz = my_Poly.vertex[0].nz;
	Bix = my_Poly.vertex[3].nx; Biy = my_Poly.vertex[3].ny; Biz = my_Poly.vertex[3].nz;
#else
	Li  = my_Poly.vertex[2].nz;
	Ti  = my_Poly.vertex[1].nz;
	Ri  = my_Poly.vertex[0].nz;
	Bi  = my_Poly.vertex[3].nz;
#endif
  }
 
  else  if(max_y_index == 2)   // case D
  {
    Lx = my_Poly.vertex[3].x; Ly = my_Poly.vertex[3].y; Lz = my_Poly.vertex[3].z; 
    Tx = my_Poly.vertex[2].x; Ty = my_Poly.vertex[2].y; Tz = my_Poly.vertex[2].z; 
    Rx = my_Poly.vertex[1].x; Ry = my_Poly.vertex[1].y; Rz = my_Poly.vertex[1].z;
    Bx = my_Poly.vertex[0].x; By = my_Poly.vertex[0].y; Bz = my_Poly.vertex[0].z;
    SrcOffsetX                   = TextureDIBWidth;
    SrcOffsetY                   = TextureDIBHeight;
    SignX                        = 1;
    SignY                        = -1;
  
  Lu =  my_Poly.vertex[3].u;  Lv =  my_Poly.vertex[3].v; 
  Tu =  my_Poly.vertex[2].u;  Tv =  my_Poly.vertex[2].v;  
  Ru =  my_Poly.vertex[1].u;  Rv =  my_Poly.vertex[1].v; 
  Bu =  my_Poly.vertex[0].u;  Bv =  my_Poly.vertex[0].v; 
 
#ifdef GENERAL_LIGHT_DIR
    Lix = my_Poly.vertex[3].nx; Liy = my_Poly.vertex[3].ny;  Liz = my_Poly.vertex[3].nz; 
	Tix = my_Poly.vertex[2].nx; Tiy = my_Poly.vertex[2].ny;  Tiz = my_Poly.vertex[2].nz;
	Rix = my_Poly.vertex[1].nx; Riy = my_Poly.vertex[1].ny; Riz = my_Poly.vertex[1].nz;
	Bix = my_Poly.vertex[0].nx; Biy = my_Poly.vertex[0].ny; Biz = my_Poly.vertex[0].nz;
#else
 Li  = my_Poly.vertex[3].nz;
 Ti  = my_Poly.vertex[2].nz; 
 Ri  = my_Poly.vertex[1].nz;
 Bi  = my_Poly.vertex[0].nz;
#endif
  }

  else                        // error condition
  {
    return;
  }
if(LIGHT) {

#ifdef GENERAL_LIGHT_DIR
Li = Lix * lightDir.x   + Liy * lightDir.y  +  Liz * lightDir.z ;
Ti = Tix * lightDir.x   + Tiy *lightDir.y   +  Tiz * lightDir.z ;
Ri = Rix * lightDir.x  +  Riy *lightDir.y  +  Riz * lightDir.z ;
Bi = Bix * lightDir.x  +  Biy *lightDir.y  +  Biz * lightDir.z ;
#endif 

#ifdef CHECK_NORMALS_SIGN
 Li  = (float)pow(Li,spec_power);
 Ti  = (float)pow(Ti,spec_power);
 Ri  = (float)pow(Ri,spec_power);
 Bi  = (float)pow(Bi,spec_power);
#else
 Li  = fabs(pow(Li,spec_power));
 Ti  = fabs(pow(Ti,spec_power));
 Ri  = fabs(pow(Ri,spec_power));
 Bi  = fabs(pow(Bi,spec_power));
#endif 
}
  fULx = Tx - Lx; fULy = Ty - Ly; fULz = Tz - Lz;
  fURx = Rx - Tx; fURy = Ry - Ty; fURz = Rz - Tz;
  fLLx = Bx - Lx; fLLy = By - Ly; fLLz = Bz - Lz;
  fLRx = Rx - Bx; fLRy = Ry - By; fLRz = Rz - Bz;

  SrcWidth        = TextureDIBWidth;
  SrcHeight       = TextureDIBHeight;
  
  Lu  *=   SrcWidth;
  Lv  *=  SrcHeight;
  
  Tu  *=   SrcWidth;
  Tv  *=  SrcHeight;
  
  Ru  *=   SrcWidth;
  Rv  *=  SrcHeight;
  
  Bu  *=   SrcWidth;
  Bv  *=  SrcHeight;

 

  fULabsleny = (fULy>0 ? fULy : -fULy);
  fURabsleny = (fURy>0 ? fURy : -fURy);
  fLLabsleny = (fLLy>0 ? fLLy : -fLLy);
  fLRabsleny = (fLRy>0 ? fLRy : -fLRy);

  fDestDeltaTX = fULabsleny==0 ? (fULx<<xshiftleft) : (fULx<<xshiftleft)/(fULabsleny);
  fDestDeltaLX = fLLabsleny==0 ? (fLLx<<xshiftleft) : (fLLx<<xshiftleft)/(fLLabsleny);
  fDestDeltaRX = fURabsleny==0 ? (fURx<<xshiftleft) : (fURx<<xshiftleft)/(fURabsleny);
  fDestDeltaBX = fLRabsleny==0 ? (fLRx<<xshiftleft) : (fLRx<<xshiftleft)/(fLRabsleny);

  fDestDeltaTZ = fULabsleny==0 ? (fULz<<zshiftleft) : (fULz<<zshiftleft)/(fULabsleny);
  fDestDeltaLZ = fLLabsleny==0 ? (fLLz<<zshiftleft) : (fLLz<<zshiftleft)/(fLLabsleny);
  fDestDeltaRZ = fURabsleny==0 ? (fURz<<zshiftleft) : (fURz<<zshiftleft)/(fURabsleny);
  fDestDeltaBZ = fLRabsleny==0 ? (fLRz<<zshiftleft) : (fLRz<<zshiftleft)/(fLRabsleny);

  
  fDestDeltaTU  = fULabsleny==0 ?   (Lu - Tu ) : (Lu - Tu ) / (fULabsleny);
  fDestDeltaLU  = fLLabsleny==0 ?  (Bu - Lu)   : (Bu - Lu) / (fLLabsleny);
  fDestDeltaRU = fURabsleny==0 ?  (Ru - Tu)  :  (Ru - Tu) / (fURabsleny);
  fDestDeltaBU = fLRabsleny==0 ?  (Bu - Ru)  : (Bu - Ru) / (fLRabsleny);
  
  fDestDeltaTV  = fULabsleny==0 ?   (Lv - Tv ) : (Lv - Tv ) / (fULabsleny);
  fDestDeltaLV  = fLLabsleny==0 ?  (Bv - Lv)   : (Bv - Lv) / (fLLabsleny);
  fDestDeltaRV = fURabsleny==0 ?  (Rv - Tv)  :  (Rv - Tv) / (fURabsleny);
  fDestDeltaBV = fLRabsleny==0 ?  (Bv - Rv)  : (Bv - Rv) / (fLRabsleny);
  
  fDestDeltaTI  = fULabsleny==0 ?   (Li - Ti ) : (Li - Ti ) / (fULabsleny);
  fDestDeltaLI  = fLLabsleny==0 ?  (Bi - Li)   : (Bi - Li) / (fLLabsleny);
  fDestDeltaRI = fURabsleny==0 ?  (Ri - Ti)  :  (Ri - Ti) / (fURabsleny);
  fDestDeltaBI = fLRabsleny==0 ?  (Bi - Ri)  : (Bi - Ri) / (fLRabsleny);
  


  /*
  fSrcWidth       = SrcWidth<<16;
  fSrcHeight      = SrcHeight<<16;
  fSrcTopDelta    = fULy == 0 ? -(fSrcWidth  - (1<<16)) : (fSrcWidth  - (1<<16)) / fULy;
  fSrcRightDelta  = fURy == 0 ?  (fSrcHeight - (1<<16)) : (fSrcHeight - (1<<16)) / fURy;
  fSrcBottomDelta = fLRy == 0 ?  (fSrcWidth  - (1<<16)) : (fSrcWidth  - (1<<16)) / fLRy;
  fSrcLeftDelta   = fLLy == 0 ? -(fSrcHeight - (1<<16)) : (fSrcHeight - (1<<16)) / fLLy;
*/
  num_lines[0]    = 0;
  num_lines[1]    = 0;
  num_lines[2]    = 0;

  if((Ly >= Ry) && (Ry >= By))
  {
	//Top
	orientation             = 0;
    
	fDestStartXArray[0]     = Tx<<xshiftleft;
    fDestEndXArray[0]       = Tx<<xshiftleft;
    fDestStartDeltaX[0]     = -fDestDeltaTX;
    fDestEndDeltaX[0]       = fDestDeltaRX; 	  
    fDestStartZArray[0]     = Tz<<zshiftleft;
	fDestEndZArray[0]       = Tz<<zshiftleft;
	fDestStartDeltaZ[0]     = -fDestDeltaTZ;
	fDestEndDeltaZ[0]       = fDestDeltaRZ;

    fSrcEdgeStartDeltaX[0]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[0]  = 0;
    fSrcEdgeEndDeltaX[0]    = 0;
    fSrcEdgeEndDeltaY[0]    = -fSrcRightDelta;
    num_lines[0]            = Ty-Ly;
   	
	fDestStartUArray[0]     = Tu;
    fDestEndUArray[0]      = Tu;
    fDestStartDeltaU[0]     = fDestDeltaTU;
    fDestEndDeltaU[0]      = fDestDeltaRU; 
	
	fDestStartVArray[0]     = Tv;
    fDestEndVArray[0]      = Tv;
    fDestStartDeltaV[0]     = fDestDeltaTV;
    fDestEndDeltaV[0]      = fDestDeltaRV; 
    
    fDestStartIArray[0]     = Ti;
    fDestEndIArray[0]      = Ti;
    fDestStartDeltaI[0]     = fDestDeltaTI;
    fDestEndDeltaI[0]      = fDestDeltaRI; 
	
	//Middle
	fDestStartXArray[1]     = Lx<<xshiftleft;
    fDestEndXArray[1]       = fDestEndXArray[0] + fDestEndDeltaX[0]*num_lines[0];
    fDestStartDeltaX[1]     = fDestDeltaLX;
    fDestEndDeltaX[1]       = fDestDeltaRX;
    fDestStartZArray[1]     = Lz<<zshiftleft;
	fDestEndZArray[1]       = fDestEndZArray[0] + fDestEndDeltaZ[0]*num_lines[0];
	fDestStartDeltaZ[1]     = fDestDeltaLZ;
	fDestEndDeltaZ[1]       = fDestDeltaRZ;
    
	fSrcEdgeStartDeltaX[1]  = 0;
    fSrcEdgeStartDeltaY[1]  = -fSrcLeftDelta;
    fSrcEdgeEndDeltaX[1]    = 0;
    fSrcEdgeEndDeltaY[1]    = -fSrcRightDelta;
    num_lines[1]            = Ly-Ry;

    fDestStartUArray[1]     = Lu;
    fDestEndUArray[1]      = fDestEndUArray[0] + fDestEndDeltaU[0]*num_lines[0];
    fDestStartDeltaU[1]     = fDestDeltaLU;
    fDestEndDeltaU[1]      =  fDestDeltaRU;
	
	fDestStartVArray[1]     = Lv;
    fDestEndVArray[1]      = fDestEndVArray[0] + fDestEndDeltaV[0]*num_lines[0];
    fDestStartDeltaV[1]     = fDestDeltaLV;
    fDestEndDeltaV[1]      =  fDestDeltaRV;
    
	fDestStartIArray[1]     = Li;
    fDestEndIArray[1]      = fDestEndIArray[0] + fDestEndDeltaI[0]*num_lines[0];
    fDestStartDeltaI[1]     = fDestDeltaLI;
    fDestEndDeltaI[1]      =  fDestDeltaRI;
	
	//Bottom
	fDestStartXArray[2]     = fDestStartXArray[1] + fDestStartDeltaX[1]*num_lines[1];
    fDestEndXArray[2]       = Rx<<xshiftleft;
    fDestStartDeltaX[2]     = fDestDeltaLX;
    fDestEndDeltaX[2]       = -fDestDeltaBX;
    fDestStartZArray[2]     = fDestStartZArray[1] + fDestStartDeltaZ[1]*num_lines[1];
	fDestEndZArray[2]       = Rz<<zshiftleft;
	fDestStartDeltaZ[2]     = fDestDeltaLZ;
	fDestEndDeltaZ[2]       = -fDestDeltaBZ;

	fSrcEdgeStartDeltaX[2]  = 0;
    fSrcEdgeStartDeltaY[2]  = -fSrcLeftDelta;
    fSrcEdgeEndDeltaX[2]    = -fSrcBottomDelta;
    fSrcEdgeEndDeltaY[2]    = 0;
    num_lines[2]                 = Ry-By;
	
    fDestStartUArray[2]    = fDestStartUArray[1] + fDestStartDeltaU[1]*num_lines[1];
    fDestEndUArray[2]      = Ru;
    fDestStartDeltaU[2]     = fDestDeltaLU;
    fDestEndDeltaU[2]      = fDestDeltaBU;
	
	fDestStartVArray[2]     =  fDestStartVArray[1] + fDestStartDeltaV[1]*num_lines[1];
    fDestEndVArray[2]      = Rv;
    fDestStartDeltaV[2]     = fDestDeltaLV;
    fDestEndDeltaV[2]      =  fDestDeltaBV;
	
	fDestStartIArray[2]     =  fDestStartIArray[1] + fDestStartDeltaI[1]*num_lines[1];
    fDestEndIArray[2]      = Ri;
    fDestStartDeltaI[2]     = fDestDeltaLI;
    fDestEndDeltaI[2]      =  fDestDeltaBI;
  }
  
  else if((Ly >= By) && (By >= Ry))	      
  { 
	//Top
	orientation             = 1;
    fDestStartXArray[0]     = Tx<<xshiftleft;
    fDestEndXArray[0]       = Tx<<xshiftleft;
    fDestStartDeltaX[0]     = -fDestDeltaTX;
	fDestEndDeltaX[0]       = fDestDeltaRX;
    fDestStartZArray[0]     = Tz<<zshiftleft;
	fDestEndZArray[0]       = Tz<<zshiftleft;
	fDestStartDeltaZ[0]     = -fDestDeltaTZ;
	fDestEndDeltaZ[0]       = fDestDeltaRZ;

	fSrcEdgeStartDeltaX[0]  = -fSrcTopDelta;
	fSrcEdgeStartDeltaY[0]  = 0;
	fSrcEdgeEndDeltaX[0]    = 0;
	fSrcEdgeEndDeltaY[0]    = -fSrcRightDelta;
	num_lines[0]            = Ty-Ly;
	
	fDestStartUArray[0]     = Tu;
    fDestEndUArray[0]      = Tu;
    fDestStartDeltaU[0]     = fDestDeltaTU;
    fDestEndDeltaU[0]      =  fDestDeltaRU; 
	
	fDestStartVArray[0]     = Tv;
    fDestEndVArray[0]      = Tv;
    fDestStartDeltaV[0]     =  fDestDeltaTV ;
    fDestEndDeltaV[0]      =  fDestDeltaRV ; 
	
	fDestStartIArray[0]     = Ti;
    fDestEndIArray[0]      = Ti;
    fDestStartDeltaI[0]     =  fDestDeltaTI ;
    fDestEndDeltaI[0]      =  fDestDeltaRI ; 
	
	//Middle
	fDestStartXArray[1]     = Lx<<xshiftleft;
	fDestEndXArray[1]       = fDestEndXArray[0] + fDestEndDeltaX[0]*num_lines[0];
	fDestStartDeltaX[1]     = fDestDeltaLX;
    fDestEndDeltaX[1]       = fDestDeltaRX;
    fDestStartZArray[1]     = Lz<<zshiftleft;
	fDestEndZArray[1]       = fDestEndZArray[0] + fDestEndDeltaZ[0]*num_lines[0];
	fDestStartDeltaZ[1]     = fDestDeltaLZ;
	fDestEndDeltaZ[1]       = fDestDeltaRZ;

    fSrcEdgeStartDeltaX[1]  = 0;
    fSrcEdgeStartDeltaY[1]  = -fSrcLeftDelta;
    fSrcEdgeEndDeltaX[1]    = 0;
    fSrcEdgeEndDeltaY[1]    = -fSrcRightDelta;
    num_lines[1]            = Ly-By;

	fDestStartUArray[1]     = Lu;
	fDestEndUArray[1]       = fDestEndUArray[0] + fDestEndDeltaU[0]*num_lines[0];
	fDestStartDeltaU[1]     = fDestDeltaLU;
    fDestEndDeltaU[1]       = fDestDeltaRU;
    
	fDestStartVArray[1]     = Lv;
	fDestEndVArray[1]       = fDestEndVArray[0] + fDestEndDeltaV[0]*num_lines[0];
	fDestStartDeltaV[1]     = fDestDeltaLV;
	fDestEndDeltaV[1]       = fDestDeltaRV;

	fDestStartIArray[1]     = Li;
	fDestEndIArray[1]       = fDestEndIArray[0] + fDestEndDeltaI[0]*num_lines[0];
	fDestStartDeltaI[1]     = fDestDeltaLI;
	fDestEndDeltaI[1]       = fDestDeltaRI;
    
	//Bottom
	fDestStartXArray[2]     = Bx<<xshiftleft;
    fDestEndXArray[2]       = fDestEndXArray[1] + fDestEndDeltaX[1]*num_lines[1];
    fDestStartDeltaX[2]     = fDestDeltaBX;
    fDestEndDeltaX[2]       = fDestDeltaRX;
    fDestStartZArray[2]     = Bz<<zshiftleft;
	fDestEndZArray[2]       = fDestEndZArray[1] + fDestEndDeltaZ[1]*num_lines[1];
	fDestStartDeltaZ[2]     = fDestDeltaBZ;
	fDestEndDeltaZ[2]       = fDestDeltaRZ;

    fSrcEdgeStartDeltaX[2]  = fSrcBottomDelta;
    fSrcEdgeStartDeltaY[2]  = 0;
    fSrcEdgeEndDeltaX[2]    = 0;
    fSrcEdgeEndDeltaY[2]    = -fSrcRightDelta;
    num_lines[2]            = By-Ry;
 
  	fDestStartUArray[2]     = Bu;
    fDestEndUArray[2]       = fDestEndUArray[1] + fDestEndDeltaU[1]*num_lines[1];
    fDestStartDeltaU[2]     = -fDestDeltaBU;
    fDestEndDeltaU[2]       = fDestDeltaRU;
    
	fDestStartVArray[2]     = Bv;
	fDestEndVArray[2]       = fDestEndVArray[1] + fDestEndDeltaV[1]*num_lines[1];
	fDestStartDeltaV[2]     = -fDestDeltaBV;
	fDestEndDeltaV[2]       = fDestDeltaRV;
	
	fDestStartIArray[2]     = Bi;
	fDestEndIArray[2]       = fDestEndIArray[1] + fDestEndDeltaI[1]*num_lines[1];
	fDestStartDeltaI[2]     = -fDestDeltaBI;
	fDestEndDeltaI[2]       = fDestDeltaRI;
 }
  
  else if((Ry >= By) && (By >= Ly))
  {
	Ty = Ty + 0;

	//Top
	orientation             = 2;
    fDestStartXArray[0]     = Tx<<xshiftleft;
    fDestEndXArray[0]       = Tx<<xshiftleft;
    fDestStartDeltaX[0]     = -fDestDeltaTX;
    fDestEndDeltaX[0]       = fDestDeltaRX;
    fDestStartZArray[0]     = Tz<<zshiftleft;
	fDestEndZArray[0]       = Tz<<zshiftleft;
	fDestStartDeltaZ[0]     = -fDestDeltaTZ;
	fDestEndDeltaZ[0]       = fDestDeltaRZ;

    fSrcEdgeStartDeltaX[0]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[0]  = 0;
    fSrcEdgeEndDeltaX[0]    = 0;
    fSrcEdgeEndDeltaY[0]    = -fSrcRightDelta;
    num_lines[0]            = Ty-Ry;
    
	fDestStartUArray[0]     = Tu;
    fDestEndUArray[0]       = Tu;
    fDestStartDeltaU[0]     = fDestDeltaTU;
    fDestEndDeltaU[0]       = fDestDeltaRU;
    
	fDestStartVArray[0]     = Tv;
	fDestEndVArray[0]       = Tv;
	fDestStartDeltaV[0]     =  fDestDeltaTV;
	fDestEndDeltaV[0]       = fDestDeltaRV;
	
	fDestStartIArray[0]     = Ti;
	fDestEndIArray[0]       = Ti;
	fDestStartDeltaI[0]     =  fDestDeltaTI;
	fDestEndDeltaI[0]       = fDestDeltaRI;

    //Middle
	fDestStartXArray[1]     = fDestStartXArray[0] + fDestStartDeltaX[0]*num_lines[0];
    fDestEndXArray[1]       = Rx<<xshiftleft;
    fDestStartDeltaX[1]     = -fDestDeltaTX;
    fDestEndDeltaX[1]       = -fDestDeltaBX;
    fDestStartZArray[1]     = fDestStartZArray[0] + fDestStartDeltaZ[0]*num_lines[0];
	fDestEndZArray[1]       = Rz<<zshiftleft;
	fDestStartDeltaZ[1]     = -fDestDeltaTZ;
	fDestEndDeltaZ[1]       = -fDestDeltaBZ;
    
	fSrcEdgeStartDeltaX[1]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[1]  = 0;
    fSrcEdgeEndDeltaX[1]    = -fSrcBottomDelta;
    fSrcEdgeEndDeltaY[1]    = 0;
    num_lines[1]            = Ry-By;
	
	fDestStartUArray[1]     = fDestStartUArray[0] + fDestStartDeltaU[0]*num_lines[0];
    fDestEndUArray[1]       = Ru;
    fDestStartDeltaU[1]     = fDestDeltaTU;
    fDestEndDeltaU[1]       = fDestDeltaBU;
    
	fDestStartVArray[1]     = fDestStartVArray[0] + fDestStartDeltaV[0]*num_lines[0];
	fDestEndVArray[1]       = Rv;
	fDestStartDeltaV[1]     = fDestDeltaTV;
	fDestEndDeltaV[1]       = fDestDeltaBV;

	fDestStartIArray[1]     = fDestStartIArray[0] + fDestStartDeltaI[0]*num_lines[0];
	fDestEndIArray[1]       = Ri;
	fDestStartDeltaI[1]     = fDestDeltaTI;
	fDestEndDeltaI[1]       = fDestDeltaBI;

    //Bottom
	fDestStartXArray[2]     = fDestStartXArray[1] + fDestStartDeltaX[1]*num_lines[1];
    fDestEndXArray[2]       = (Bx+0)<<16;
    fDestStartDeltaX[2]     = -fDestDeltaTX;
    fDestEndDeltaX[2]       = -fDestDeltaLX;
    fDestStartZArray[2]     = fDestStartZArray[1] + fDestStartDeltaZ[1]*num_lines[1];
	fDestEndZArray[2]       = Bz<<zshiftleft;
	fDestStartDeltaZ[2]     = -fDestDeltaTZ;
	fDestEndDeltaZ[2]       = -fDestDeltaLZ;
    
	fSrcEdgeStartDeltaX[2]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[2]  = 0;
    fSrcEdgeEndDeltaX[2]    = 0;
    fSrcEdgeEndDeltaY[2]    = -fSrcLeftDelta;
    num_lines[2]            = By-Ly;
	
	fDestStartUArray[2]     = fDestStartUArray[1] + fDestStartDeltaU[1]*num_lines[1];
    fDestEndUArray[2]       = Bu;
    fDestStartDeltaU[2]     = fDestDeltaTU;
    fDestEndDeltaU[2]       = -fDestDeltaLU;
    
	fDestStartVArray[2]     = fDestStartVArray[1] + fDestStartDeltaV[1]*num_lines[1];
	fDestEndVArray[2]       = Bv;
	fDestStartDeltaV[2]     = fDestDeltaTV;
	fDestEndDeltaV[2]       = -fDestDeltaLV;
	
	fDestStartIArray[2]     = fDestStartIArray[1] + fDestStartDeltaI[1]*num_lines[1];
	fDestEndIArray[2]       = Bi;
	fDestStartDeltaI[2]     = fDestDeltaTI;
	fDestEndDeltaI[2]       = -fDestDeltaLI;


  }
  
  else if((Ry >= Ly) && (Ly >= By))
  {
	//Top
	orientation             = 3;
    fDestStartXArray[0]     = Tx<<xshiftleft;
    fDestEndXArray[0]       = Tx<<xshiftleft;
    fDestStartDeltaX[0]     = -fDestDeltaTX;
    fDestEndDeltaX[0]       = fDestDeltaRX;
    fDestStartZArray[0]     = Tz<<zshiftleft;
	fDestEndZArray[0]       = Tz<<zshiftleft;
	fDestStartDeltaZ[0]     = -fDestDeltaTZ;
	fDestEndDeltaZ[0]       = fDestDeltaRZ;
    
	fSrcEdgeStartDeltaX[0]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[0]  = 0;
    fSrcEdgeEndDeltaX[0]    = 0;
    fSrcEdgeEndDeltaY[0]    = -fSrcRightDelta;
    num_lines[0]            = Ty-Ry;

	fDestStartUArray[0]     = Tu;
    fDestEndUArray[0]       = Tu;
    fDestStartDeltaU[0]     = fDestDeltaTU;
    fDestEndDeltaU[0]       = fDestDeltaRU;
    
	fDestStartVArray[0]     = Tv;
	fDestEndVArray[0]       = Tv;
	fDestStartDeltaV[0]     = fDestDeltaTV;
	fDestEndDeltaV[0]       = fDestDeltaRV;
    
	fDestStartIArray[0]     = Ti;
	fDestEndIArray[0]       = Ti;
	fDestStartDeltaI[0]     = fDestDeltaTI;
	fDestEndDeltaI[0]       = fDestDeltaRI;

    //Middle
	fDestStartXArray[1]     = fDestStartXArray[0] + fDestStartDeltaX[0]*num_lines[0];
    fDestEndXArray[1]       = Rx<<xshiftleft;
    fDestStartDeltaX[1]     = -fDestDeltaTX;
    fDestEndDeltaX[1]       = -fDestDeltaBX;
    fDestStartZArray[1]     = fDestStartZArray[0] + fDestStartDeltaZ[0]*num_lines[0];
	fDestEndZArray[1]       = Rz<<zshiftleft;
	fDestStartDeltaZ[1]     = -fDestDeltaTZ;
	fDestEndDeltaZ[1]       = -fDestDeltaBZ;

    fSrcEdgeStartDeltaX[1]  = -fSrcTopDelta;
    fSrcEdgeStartDeltaY[1]  = 0;
    fSrcEdgeEndDeltaX[1]    = -fSrcBottomDelta;
    fSrcEdgeEndDeltaY[1]    = 0;
    num_lines[1]            = Ry-Ly;
	
	fDestStartUArray[1]     = fDestStartUArray[0] + fDestStartDeltaU[0]*num_lines[0];
    fDestEndUArray[1]       = Ru;
    fDestStartDeltaU[1]     = fDestDeltaTU;
    fDestEndDeltaU[1]       = fDestDeltaBU;
    
	fDestStartVArray[1]     = fDestStartVArray[0] + fDestStartDeltaV[0]*num_lines[0];
	fDestEndVArray[1]       = Rv;
	fDestStartDeltaV[1]     = fDestDeltaTV;
	fDestEndDeltaV[1]       = fDestDeltaBV;
	
	fDestStartIArray[1]     = fDestStartIArray[0] + fDestStartDeltaI[0]*num_lines[0];
	fDestEndIArray[1]       = Ri;
	fDestStartDeltaI[1]     = fDestDeltaTI;
	fDestEndDeltaI[1]       = fDestDeltaBI;

    //Bottom
	fDestStartXArray[2]     = Lx<<xshiftleft;
    fDestEndXArray[2]       = fDestEndXArray[1] + fDestEndDeltaX[1]*num_lines[1];
    fDestStartDeltaX[2]     = fDestDeltaLX;
    fDestEndDeltaX[2]       = -fDestDeltaBX;
    fDestStartZArray[2]     = Lz<<zshiftleft;
	fDestEndZArray[2]       = fDestEndZArray[1] + fDestEndDeltaZ[1]*num_lines[1];
	fDestStartDeltaZ[2]     = fDestDeltaLZ;
	fDestEndDeltaZ[2]       = -fDestDeltaBZ;

    fSrcEdgeStartDeltaX[2]  = 0;
    fSrcEdgeStartDeltaY[2]  = -fSrcLeftDelta;
    fSrcEdgeEndDeltaX[2]    = -fSrcBottomDelta;
    fSrcEdgeEndDeltaY[2]    = 0;
    num_lines[2]            = Ly-By;
	
	fDestStartUArray[2]     = Lu;
    fDestEndUArray[2]       = fDestEndUArray[1] + fDestEndDeltaU[1]*num_lines[1];
    fDestStartDeltaU[2]     = fDestDeltaLU;
    fDestEndDeltaU[2]       = fDestDeltaBU;
    
	fDestStartVArray[2]     = Lv;
	fDestEndVArray[2]       = fDestEndVArray[1] + fDestEndDeltaV[1]*num_lines[1];
	fDestStartDeltaV[2]     = fDestDeltaLV;
	fDestEndDeltaV[2]       = fDestDeltaBV;
    
	fDestStartIArray[2]     = Li;
	fDestEndIArray[2]       = fDestEndIArray[1] + fDestEndDeltaI[1]*num_lines[1];
	fDestStartDeltaI[2]     = fDestDeltaLI;
	fDestEndDeltaI[2]       = fDestDeltaBI;

  }
  
  else  // error 
  {
    orientation             = 4;
    return;
  }

  
  fDestY = Ty;//<<16;

  for(count=0;count<3; count++)
  {
	fDestStartX = fDestStartXArray[count];
    fDestEndX   = fDestEndXArray[count];
  
if(WITH_ZBUFFER) {
	fDestStartZ = fDestStartZArray[count];
    fDestEndZ   = fDestEndZArray[count];
 }
	fDestStartU   = fDestStartUArray[count];
    fDestEndU    = fDestEndUArray[count];
	fDestStartV   = fDestStartVArray[count];
    fDestEndV    = fDestEndVArray[count];
	
	fDestStartI   = fDestStartIArray[count];
    fDestEndI    = fDestEndIArray[count];
	
	for(line = 0; line <= num_lines[count]; line++)
	{
	  if(line>0)
	  {
	    fDestStartX     += fDestStartDeltaX[count];
	    fDestEndX       += fDestEndDeltaX[count];
if(WITH_ZBUFFER) {
		fDestStartZ     += fDestStartDeltaZ[count];
	    fDestEndZ       += fDestEndDeltaZ[count];
		}
		fDestY          -= 1; // (1<<16);
	   
		fDestStartU     += fDestStartDeltaU[count];
	    fDestEndU       += fDestEndDeltaU[count];
		fDestStartV     += fDestStartDeltaV[count];
	    fDestEndV       += fDestEndDeltaV[count];
	    
		fDestStartI     += fDestStartDeltaI[count];
	    fDestEndI       += fDestEndDeltaI[count];
     }
      //fCurrentZ        = ((65536 << 16) / (fDestStartZ)) << 16;
      
	  DestY        = fDestY;// >>16;
	  if((DestY < 0) || (DestY > (ScreenDibHeight-1)))
	  continue;
	
	  DestStartX    = fDestStartX>>xshiftleft;
	  DestEndX     = fDestEndX>>xshiftleft;

	  
#ifdef DIV_TABLE	  
	  num_pixels = DestEndX - DestStartX;
     // for correct cliping of Z values 
	 zInvPixNum = invDeltax[num_pixels];
#else
	 	zInvPixNum = 1.0f / (DestEndX - DestStartX);
#endif
	 
	 ZStartGap     = 0;
	  
	   if ( DestStartX < 0)	{
		  ZStartGap = -DestStartX ;
	      DestStartX = 0;  
	   }

	   if ( DestEndX >=  ScreenDibWidth) {   
	       DestEndX  = (ScreenDibWidth -1);  
	   }
	    
	  num_pixels = DestEndX - DestStartX;
	  
	  DestOffset   = (DestY * (ScreenDibWidth)) + DestStartX;
	  pixels_buf = pRenderBuffer + DestOffset;


	 //num_pixels     = ((fDestEndX>>16) - (fDestStartX>>16));
  	 //num_pixels     = num_pixels>0 ? num_pixels+1 : (-num_pixels)+1;
  	 num_pixels     = num_pixels>0 ? num_pixels+1 : 0;
	  
	  if (num_pixels > 0)
       {
			u0 = fDestStartU;     v0  = fDestStartV;  
			u2 = fDestEndU;      v2  =  fDestEndV;  
			i0  = fDestStartI;       i2  = fDestEndI;

#ifdef DIV_TABLE	  
            inv_pixnum = invDeltax[num_pixels];
#else
			 inv_pixnum  = 1.0f / num_pixels;
#endif


#ifdef MY_FTOL
		fSrcLineStartX = my_ftol(u0*shft1left16);
		fSrcLineStartY = my_ftol(v0*shft1left16);
		
		dudx  = my_ftol(((u2 - u0)* inv_pixnum)*shft1left16);
	    dvdx  = my_ftol(((v2 - v0) * inv_pixnum)*shft1left16);

#else
		fSrcLineStartX = ((long)(u0*shft1left16));
		fSrcLineStartY = ((long)(v0*shft1left16));	

	    dudx  = (long)(((u2 - u0)* inv_pixnum)*shft1left16);
		dvdx  =  (long)(((v2 - v0) * inv_pixnum)*shft1left16);

#endif		        
					
		 didx    =  (i2 - i0) * inv_pixnum;
         
if(WITH_ZBUFFER) {
      ZDestOffset   = (DestY * (ZBufferWidth)) + DestStartX;
	  if(ZDestOffset != DestOffset) { Msg("Z-offset != Pix-offset\0", 0); return; } 
	  pZBufferInitial = pZBuffer + ZDestOffset;	
      fZBufferInc   = (signed __int16)((fDestEndZ - fDestStartZ) * zInvPixNum);
	  DestStartZ = fDestStartZ;
	 if(ZStartGap )
	   DestStartZ += (ZStartGap * fZBufferInc);

}
#ifdef WIREFRAME
	         if(DestStartZ > 0) pixels_buf[0] = (unsigned  __int16)0xffff; 
			 else					   pixels_buf[0] = (unsigned  __int16)0xf33f; 
			 if ((DestStartZ + fZBufferInc * num_pixels) > 0) 
				    pixels_buf[num_pixels-1] = (unsigned  __int16)0xffff;
			else  pixels_buf[num_pixels-1] = (unsigned  __int16)0xf33f;
#else 
/* 
if(num_pixels == 1)    {
	  *(pixels_buf) = pixels_buf[-1];
	   continue;
  }
 */
  if (texture_method == MARBLE)     
		 turbulencePassMMX
		 (fSrcLineStartX, fSrcLineStartY, dudx, dvdx 
										,num_pixels,pixels_buf);
  else  // texture_method == WOOD 
	  //woodPassMmxAndC
	  //turbulencePass1
	  woodPassMMX(fSrcLineStartX, fSrcLineStartY, dudx, dvdx, 
					  num_pixels,  pixels_buf); 
 if (LIGHT)
     modulateColor(pixels_buf, (__int16)num_pixels, i0  , didx);
 if(WITH_ZBUFFER)
      writeZBuffer(pixels_buf,(__int16)num_pixels,
					     pZBufferInitial,
						 (signed __int16)DestStartZ,fZBufferInc);
else if(!LIGHT)
      memcpy(pixels_buf, turbulenceTbl , sizeof(__int16) * num_pixels);
#endif
            } // for num_pixels
		  } // check to see if scan line on screen for left/right edges
		} // check to see if scan line on screen for top/down edges
    }   
/********************************************************************************/
/*
   We do a quite cheap (but it's time consuming) Phong like shading.
   The cosine of the angle between [0,0,1] which is the viewer, 
   and the object's Normal (actually only it's z component) is interpulated
   for each pixel.
   When we have this cosine we manipulate the texture color, and add some 
   ambient light.
*/   
void MMX_INCZbuffer(signed __int16 startZ, signed __int16 ZBufInc,
	                    int alignPixNum,
						signed __int16 *zLine,       unsigned  __int16 *turbulenceTbl,
						signed __int16 *align_zbuf,unsigned  __int16 *screen_buf);

void MMX_Zbuffer(int startZ, int ZBufInc,
	                    int alignPixNum,
						__int16 *zLine, __int16 *turbulenceTbl,
						__int16 *align_zbuf, __int16 *screen_buf);
/********************************************************************************/
#define MMX_Z
void writeZBuffer(unsigned __int16 * screen_buf, 
				          unsigned int num_pixels ,
			              signed __int16 * z_buf,
						  signed __int16 startZ, 
						  signed __int16 ZBufInc)
{
static unsigned __int32  alignPixNum ,index ;
static signed __int16 currentZ , zLine[128] , align_zbuf[128];

alignPixNum = (num_pixels + 3) & 0xfffffffC;

currentZ = startZ;

#ifdef MMX_Z
   memcpy(align_zbuf,z_buf,sizeof( __int16) * num_pixels);
    
	MMX_INCZbuffer(startZ, ZBufInc,
	                    (int)alignPixNum,
						zLine, turbulenceTbl,
						align_zbuf, screen_buf);
   memcpy(z_buf,align_zbuf             , sizeof( __int16)    * num_pixels);
   memcpy(screen_buf,turbulenceTbl, sizeof( __int16) * num_pixels);
#else
 currentZ = startZ;
 for(index = 0 ; index < num_pixels; index++) {
          if (currentZ < z_buf[index] )
			{
				  z_buf[index] = currentZ;
				  *(screen_buf) = turbulenceTbl[index];
			}
			screen_buf++;
			currentZ += ZBufInc;
     }
#endif
}
/*
*/
/********************************************************************************/
void modulateColor(unsigned __int16 * screen_buf, int num_pixels, float i_init  , float di)
{
static signed long red, blue , green ;
static unsigned __int16 color,index,*tableOfPerturbPtr;
static float i_color,frac,newI; 

 if (shader_perturb_method)
      tableOfPerturbPtr = &(onlyTurb[0]);
else 
      tableOfPerturbPtr = &(turbulenceTbl[0]);

for(index = 0 ; index < num_pixels; index++) {
	
	color    = turbulenceTbl[index];
	
	red      = (color >> 11) & 0x1f;       // unpack the color components for manipulation
	green   = (color >> 5 ) & 0x3F;
	blue     = (color &  0x1F);
       
  if (shading_method == 0) {
		// another pertubration of the normal by the color itself .... (better !!!)
	    // or by the turbulence .
        frac         =  tableOfPerturbPtr[index] * inv_fixed_for_shader;
        frac        -=  (float)floor(frac); 
	    newI       =  i_init  * frac;
		i_color   = 0.85f  * newI + 0.35f;     // 0.85 is the diffuse and 'specular' coeff 
															   // 0.35 is the ambient 	
		#ifdef CHECK_NORMALS_SIGN
		if(newI < 0.0001f) newI = 0.00001f;
#endif
	  }
     else
	    i_color  = 0.85f * i_init  + 0.35f; 

#ifdef MY_FTOL
	 red    = my_ftol(red     *  i_color);     // ftol to rgb first stage
     green  = my_ftol(green *  i_color);
     blue   = my_ftol(blue    *  i_color); 
#else 
	 red     = red     *  i_color; 
     green   = green *  i_color;
     blue    = blue    *  i_color; 
#endif
    // saturation 
	 if(red >    0x1f) red    = 0x1f;        // saturate for 5:6:5 format
    if(green > 0x3f) green   = 0x3f; 
    if(blue   > 0x1f) blue   = 0x1f;

if( WITH_ZBUFFER)
	  turbulenceTbl[index] = (unsigned short)( (red << 11) | (green << 5 ) | blue ); 
else
*(screen_buf++) = (unsigned short)( (red << 11) | (green << 5 ) | blue );  // write the rgb  in 5:6:5 format
     i_init += di;
	 }
}

