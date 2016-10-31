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
#include "procedural.h"
/******************************************************************************/
/*
These are lookup tables for manipulating noise/turbulence.

  Sine and Wood tables - Since we use fixedpoint arithmetic, we found out 
  what is the DOMAIN of our function (which is the RANGE of the turbulence).
  Now, when we know in advance the values we might need to manipulate in 
  realtime, we caluclate the complicated calculation in setup time; and  by 
  rendering time we just index into this table !!!. 
  Sqrt table - we have lookup table for sqrt of the high 13 bits of ~23 bits 
  numbers.
 */ 
 unsigned __int16 sinTable[5000];
 unsigned __int16 woodTable[6000];
 unsigned __int16 sqrtTable[8192];
 /*********************************************************************************/
/* 
   'turbulenceTbl' is filled by the MmxOctave with 'num_ocatves' noise octaves
   and afterwards manipulated by the marble and wood functions.
   It is used also by the shader (modulateColor in render.c)
*/
unsigned __int16    turbulenceTbl[128] , onlyTurb[128];
 /************************************************************************************/
 /* mmx functions called :																					   */
void MMX_Octave(unsigned long u_init, unsigned long v_init, 
				            long du_init, long dv_init,
	                        unsigned long Num_Pix,
                            unsigned _int16* turb_buffer,
							unsigned long num_octaves);
void MMX_Marb(unsigned long u_init, long du_init, 
	                        unsigned long Num_Pix);
void MMX_Wood(unsigned long u_init, unsigned long v_init,
			                long du_init, long dv_init, 
	                        unsigned long Num_Pix);
/******************************************************************************/
// Global values for the diff textures.
#define M_PI            3.14159265358979323846
static double myPi         = 3.14159265;
static double my1_5Pi    = 1.570796326;
static double my2_3Pi    = 2.0797343366;
static float inv_256     = (float)(1.0 / 256.0) ; 
static float inv_512    = (float)(1.0 / 512.0) ;
static float smoothstep(float a, float b, float x);
/***************************************************************/
unsigned long  num_octaves = 3;
/***************************************************************/
/* Init the lookup tabels -
/**********************************************************************/
void init_noiseTables(void)
{
unsigned long red ,green,blue;
int i;
float my_shftleft16 = (float)(1<< 8);
double sin_val, val;
float r,comp_r, wood_red,wood_green,wood_blue;


for(i = 0 ; i < 8192; i++)
     sqrtTable[i] = (unsigned __int16)floor(sqrt((i << 10)  + (1<<5)));    

for(i = 0 ; i < 6000; i++) {
	r = 4.0f * (i + 735);
	r *= inv_512;
	r -= (float)floor(r); 	
	r =  smoothstep((float)0,(float)0.83,r) - smoothstep((float)0.83,(float)1.0,r);

	comp_r	= 1-r;

	wood_red       =  r  *  0.3f   + comp_r *  0.05f;
	wood_green 	 =  r  *  0.12f  + comp_r *  0.01f;
	wood_blue     =  r *  0.03f   + comp_r  * 0.005f;

	red    =  ((long)(wood_red * 255)) & 0xF8;
	green = ((long)(wood_green * 255)) & 0xFC; 
	blue   =  ((long)(wood_blue * 255)) & 0xF8;
 
	woodTable[i] = (unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
}

for(i=0; i < 5000; i++) { 
     /*val				  = ((double)i + 1500.0) * inv_256;
     sin_val			=  sin(val * M_PI) ;
	 sin_val            = (sin_val  + 1.0) * 0.5 ;
     sin_val			= (3.0 * pow(sin_val,2.0)) -  (2.0  * pow(sin_val,3.0));
	 sinTable[i]	   = (unsigned  __int16)(sin_val * 255 * my_shftleft16); */
	    val		 = ((double)i + 1500.0) * inv_256;
		sin_val   = (sin(val * M_PI) + 1.0) * 0.5;
		red     = ((long) ((0.33 + 0.66 * sin_val) * 256)) & 0xF8;
		green = ((long) ((0.27 + 0.72 * sin_val) * 256)) & 0xFC; 
		blue   = ((long) ((0.60 + 0.39 * sin_val) * 256)) & 0xF8;
		sinTable[i]  = (unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
	 } 
 }


/***************************************************************/
float marb_red,marb_green, marb_blue;
// for one of the marbels. 
extern void marble_color(double m); 
/***************************************************************/
/* 
In this function we can switch to each of the marble methods
we have; all based on the same input.
The methods are :
	0. just blue sine wave
	1. some sort of spline (taken over sine output) between diff blues
	2. mixing the sine(x*Pi) output with natural coeffs for marble 
	3. mixing the sine(x*Pi) *cos(x * Pi/2) output with natural coeffs for marble 
	4. pertubation using sin(sin(x)) and more.
*/
/***************************************************************/
void calcMarbleTable(int method)
{
int i;
static double sin_val, val;
static float rd, grn, blu;
static float chaos, brownLayer, greenLayer;
static float perturb, brownPerturb, greenPerturb, grnPerturb;
static float tt;
static unsigned long red ,green,blue;

#define myMAX(x,y) ((x) > (y))? (x) : (y) 
  
memset(sinTable,0,sizeof(__int16) * 5000);
if(method == 0) {     
     for(i=0; i < 5000; i++) { 
     val				  = ((double)i + 1500.0) * inv_256;
      sin_val	    = (sin(val * M_PI) + 1.0) * 0.5;
	  sinTable[i]  = (unsigned __int16)((((long)(sin_val * 255)) & 0xF8) >> 3);
     }
  } else if (method == 1) {
		    for(i=0; i < 5000; i++) { 
			val	      = ((double)i + 1500.0) * inv_256;
			sin_val = sin(val * M_PI);
			// val_val = val - (int)val;
			marble_color(sin_val);
		//	marbSpline(sin_val,&marb_red, &marb_green , &marb_blue);
			red      = (long)((0.8 * marb_red          + 0.2)   * 255) & 0xF8  ;
	        green  = (long)((0.8 * marb_green       + 0.2)   * 255) & 0xFC  ;
			blue    = (long)((0.7 * marb_blue  + 0.3)   * 255) & 0xF8  ;
            sinTable[i]  = (unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
          }
} else if(method == 2) {
	  for(i=0; i < 5000; i++) { 
		val		 = ((double)i + 1500.0) * inv_256;
		sin_val   = (sin(val * myPi) + 1.0) * 0.5;
		red     = ((long) ((0.33 + 0.66 * sin_val) * 256)) & 0xF8;
		green = ((long) ((0.27 + 0.72 * sin_val) * 256)) & 0xFC; 
		blue   = ((long) ((0.60 + 0.39 * sin_val) * 256)) & 0xF8;
		sinTable[i]  =(unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
      }
  }  else if (method == 3) {     
     for(i=0; i < 5000; i++) { 
      val		      = ((double)i + 1500.0) * inv_256;
      sin_val	    = ((sin(val * M_PI) * cos(val * my1_5Pi) ) + 1.0) * 0.5;
	  sin_val       = (3.0 * pow(sin_val,2.0)) -  (2.0  * pow(sin_val,3.0));
	  red            = ((long) ((0.33 + 0.66 * sin_val) * 256)) & 0xF8;
	  green         = ((long) ((0.27 + 0.72 * sin_val) * 256)) & 0xFC; 
	  blue           = ((long) ((0.60 + 0.39 * sin_val) * 256)) & 0xF8;
	  sinTable[i]    = (unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
     }
  } else  if(method == 4)  {
    for(i=0; i < 5000; i++) { 
			chaos	= ((float)i + 1500.0f) * inv_256;
			tt          = (float)sin(sin(chaos * M_PI));

	greenLayer = brownLayer = (float)fabs(tt);

	perturb = (float)sin(chaos * my2_3Pi);
	perturb = (float)fabs(perturb);

	brownPerturb = .6f*perturb + 0.3f;
	greenPerturb = .2f*perturb + 0.8f;
	grnPerturb = .15f*perturb + 0.85f;
	grn = 0.5f * (float)pow(fabs(brownLayer), 0.3f);
	brownLayer = (float)pow(0.5 * (brownLayer+1.0), 0.6f) * brownPerturb;
	greenLayer = (float)pow(0.5 * (greenLayer+1.0), 0.6f) * greenPerturb;
 
	rd = (0.5f*brownLayer + 0.35f*greenLayer)*2.0f*grn;
	blu = (0.25f*brownLayer + 0.35f*greenLayer)*2.0f*grn;
	grn *= myMAX(brownLayer, greenLayer) * grnPerturb;
    
	green = ((long) (grn * 256)) & 0xFC;// i need 6 bits.
    red    = ((long) ( rd  * 256)) & 0xF8; 
    blue   = ((long) ( blu * 256)) & 0xF8;
    sinTable[i]  = (unsigned __int16)((red << 8) | (green << 3 ) | (blue >> 3));
   }
  }
}

/******************************************************************************/
/******************************************************************************/
/*
turbulencePassMMX - 
- To save branches in the mmx code we align up the pixels number in the 
   scanline to muliply of 4.
-  MMX_Octave is called to calc 'num_octaves' noise octaves which are 
    accumulated in turbulenceTbl.
-  MMX_Marb is called to calc indexes into our marble table and extract the
   colors from sinTable (are written to turbulenceTbl)
- We copy only Num_Pix pixels to the screen memory.
*/
/******************************************************************************/
extern int LIGHT;
extern int shader_perturb_method;
void turbulencePassMMX(unsigned long u_init , unsigned long v_init, 
			                          long  du					, long dv,
									  long Num_Pix, 
					                  unsigned __int16 *screen_buffer)
{
static unsigned __int16   alignPixNum;
/* since the mmx noise function avarages values for each two texels 
    we won't get anything for a single pixel scanline, so we
	just copy the previous value from the screen buffer 
*/
if(Num_Pix == 1)    {
	  *(screen_buffer) = screen_buffer[-1];
	   turbulenceTbl[0] =  screen_buffer[-1];
	   return;
  } 

alignPixNum = (Num_Pix + 3) & 0xfffffffC;

memset(turbulenceTbl,0,sizeof(__int16) * alignPixNum);

MMX_Octave(u_init,     v_init, 
	                  du,         dv,
					  (alignPixNum >> 2), turbulenceTbl
					  ,num_octaves);

turbulenceTbl[0] = turbulenceTbl[1];

if(LIGHT && shader_perturb_method) 
     memcpy(onlyTurb,turbulenceTbl,sizeof(__int16)*Num_Pix);

MMX_Marb(u_init,du,alignPixNum);

} 
/******************************************************************************/
/* 
 woodPassMMX - similar to the marble function, calcs turbulence
 and then call to MMX_wood.
*/
/******************************************************************************/

#include <stdio.h>
void woodPassMMX(unsigned long u_init,  unsigned long v_init, 
			         long  du,     long dv,
	                 long Num_Pix, 
					 unsigned __int16 *screen_buffer)
{
static unsigned __int16   alignPixNum;

if(Num_Pix == 1)    {
	  *(screen_buffer) = screen_buffer[-1];
	   return;
  }

alignPixNum = (Num_Pix + 3) & 0xfffffffC;

memset(turbulenceTbl,0,sizeof(__int16) * alignPixNum);

MMX_Octave(u_init,     v_init, 
	                  du,         dv,
					  (alignPixNum >> 2), turbulenceTbl,
					  num_octaves);

alignPixNum =  (Num_Pix + 1) & 0xfffffffE;

turbulenceTbl[0] = turbulenceTbl[1];

MMX_Wood(u_init, v_init, 
		            du, dv, 
		            alignPixNum); 

}

/******************************************************************************/

/*
turbulencePass - scalar (marble texture) implementation
*/
/******************************************************************************/
void turbulencePass(unsigned long u_init , unsigned long v_init, 
			                     signed long  du			         , signed long dv,
							     long Num_Pix, 
					             unsigned __int16 *screen_buffer)
{
static unsigned __int16   alignPixNum , index , u_16bit  , marb_index;

if(Num_Pix == 1)    {
	  *(screen_buffer) = screen_buffer[-1];
	   return;
  }
	
alignPixNum = (Num_Pix + 3) & 0xfffffffC;

memset(turbulenceTbl,0,sizeof(__int16) * alignPixNum);

MMX_Octave(u_init,     v_init, 
	                  du,         dv,
					  (alignPixNum >> 2), turbulenceTbl
					  ,num_octaves);

turbulenceTbl[0] = turbulenceTbl[1];

if(LIGHT && shader_perturb_method) 
     memcpy(onlyTurb,turbulenceTbl,sizeof(__int16)*Num_Pix);

      for(index = 0; index < Num_Pix; index++) {
         u_16bit					   = (unsigned __int16)(u_init >> 14); 
         marb_index			      =  (u_16bit    + (10 * turbulenceTbl[index]));     // marble(x) = sine(x + turbulence (x))
	     marb_index              -=  1500;
		 turbulenceTbl[index]    =  sinTable[marb_index];                      // get the marble from the table
	     u_init                          += du;
	}
}

void turbulencePass1(unsigned long u_init , unsigned long v_init, 
			                     signed long  du			         , signed long dv,
							     long Num_Pix, 
					             unsigned __int16 *screen_buffer)
{
static unsigned __int16   alignPixNum , index , u_16bit  , marb_index;

if(Num_Pix == 1)    {
	  *(screen_buffer) = screen_buffer[-1];
	   return;
  }
	
alignPixNum = (Num_Pix + 3) & 0xfffffffC;

memset(turbulenceTbl,0,sizeof(__int16) * alignPixNum);

MMX_Octave(u_init,     v_init, 
	                  du,         dv,
					  (alignPixNum >> 2), turbulenceTbl
					  ,num_octaves);

turbulenceTbl[0] = turbulenceTbl[1];

if(LIGHT && shader_perturb_method) 
     memcpy(onlyTurb,turbulenceTbl,sizeof(__int16)*Num_Pix);

u_init >>= 14;
du      >>=    14;

  
  for(index = 0; index < Num_Pix; index++) {
         u_16bit					   = (unsigned __int16)u_init; 
         marb_index			      =  (u_16bit    + (10 * turbulenceTbl[index]));     // marble(x) = sine(x + turbulence (x))
	     marb_index              -=  1500;
		 if(marb_index >=  4999) marb_index = 4999;
		 turbulenceTbl[index]    =  sinTable[marb_index];                      // get the marble from the table
	     u_init                          += du;
	}
}

/************************************************************************************/
/* 
    Here is the wood implemented by C code,noise is still calculated by mmx.
    To use it, just replace the call in render.c for woodPathMMX
	to be woodPassMmxAndC.
*/
/************************************************************************************/
static unsigned __int16 sqrtRes;
void  sqrtApprox(unsigned long n);
/************************************************************************************/
void woodPass(unsigned long u_init,  unsigned long v_init, 
									   signed long  du,     signed long dv,
									   long Num_Pix, 
									   unsigned __int16 *screen_buffer)
{
static int index;
static unsigned long uu,vv,res;
static unsigned __int16 alignPixNum, u_16bit, v_16bit;

if(Num_Pix == 1)    {
	  *(screen_buffer) = screen_buffer[-1];
	   return;
  }

alignPixNum = (Num_Pix + 3) & 0xfffffffC;

memset(turbulenceTbl,0,sizeof(__int16) * alignPixNum);

MMX_Octave(u_init,     v_init, 
	                  du,         dv,
					  (alignPixNum >> 2), turbulenceTbl,
					  num_octaves);

turbulenceTbl[0] = turbulenceTbl[1];

for(index = 0; index < Num_Pix; index++)
  {

 u_16bit = (unsigned __int16)(u_init >> 14); 
 v_16bit = (unsigned __int16)(v_init >> 14); 
 
 uu		    = u_16bit * u_16bit;
 vv		    = v_16bit * v_16bit;
 res		= uu + vv;                   // r^2 = u^2 + v^2

 res	    = (res >> 10);
 if(res > 8191) res = 8191;
 sqrtRes      = sqrtTable[res];    // get the sqrt of r^2 from the table
 
 sqrtRes     *= 10;
 sqrtRes     += 15 * turbulenceTbl[index] ;   // weight the Radius by 10 and add turbulence
																	 //  wood = 10 * r + 15 * turbulence 
 
 sqrtRes    >>= 2;     
 sqrtRes      = sqrtRes - 735;
 if(sqrtRes >= 6000) sqrtRes = 5999;
 turbulenceTbl[index] =  woodTable[sqrtRes];     // get the wood from the table
 
 u_init += du;  
 v_init += dv;
}

}


/************************************************************************************/
static float smoothstep(float a, float b, float x)
{
    if (x < a)
        return (float)0;
    if (x >= b)
        return (float)1;
    x = (x - a)/(b - a); /* normalize to [0:1] */
    return (x*x * (3 - 2*x));
}
/************************************************************************************/
static void  sqrtApprox(unsigned long n)
{
static unsigned long Si;
Si = (n >> 1);
do {
	  sqrtRes = (unsigned __int16)Si;
	  Si  = (Si + (n/Si) ) >> 1; 
	} while (Si < sqrtRes);
}
/************************************************************************************/

/*
// sqrtRes   = (unsigned __int16)floor(sqrt(res));
// sqrtApprox(res);
res		     = res >> 10;
sqrtRes      = sqrtTable[res];  
sqrtRes     *= 10;
sqrtRes    += 15 * turbulenceTbl[index] ;
sqrtRes  >>= 2;
sqrtRes      = sqrtRes - 735;
if(sqrtRes >= 6000) sqrtRes = 5999;
turbulenceTbl[index] =  woodTable[sqrtRes];
// *(screen_buffer++) =   woodTable[sqrtRes];
*/
