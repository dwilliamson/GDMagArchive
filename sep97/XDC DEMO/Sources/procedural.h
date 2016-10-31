#ifndef  NOISE_H
#define NOISE_H

/******************************************************************************/
#include <math.h>
#include <memory.h>
// init the initial tables
void init_noiseTables(void);

void turbulencePassMMX(unsigned long u_init , unsigned long v_init , 
			           long  du     , long dv,
		   		       long Num_Pix , 
					   unsigned __int16 *screen_buffer);

void  woodPassMMX(unsigned long u_init , unsigned long v_init, 
			           long  du     , long dv,	   
					   long Num_Pix , 
					   unsigned __int16 *screen_buffer);
void  woodPassMmxAndC(unsigned long u_init , unsigned long v_init, 
			           long  du     , long dv,	   
					   long Num_Pix , 
					   unsigned __int16 *screen_buffer);

/******************************************************************************/
#endif