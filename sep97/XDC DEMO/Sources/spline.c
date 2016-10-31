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
#include <stdio.h>

/* Coefficients of basis matrix. */
#define CR00     -0.5f
#define CR01      1.5f
#define CR02     -1.5f
#define CR03      0.5f
#define CR10      1.0f
#define CR11     -2.5f
#define CR12      2.0f
#define CR13     -0.5f
#define CR20     -0.5f
#define CR21      0.0f
#define CR22      0.5f
#define CR23      0.0f
#define CR30      0.0f
#define CR31      1.0f
#define CR32      0.0f
#define CR33      0.0f

float clamp(float cl, float a, float b);

float
my_spline(float t, int nknots, float *knot)
{
    int span;
    int nspans = nknots - 3;
    float x,c0, c1, c2, c3;	/* coefficients of the cubic.*/

    if (nspans < 1) {  /* illegal */
        fprintf(stderr, "Spline has too few knots.\n");
        return 0.0f;
    }

    /* Find the appropriate 4-point span of the spline. */
    x  = clamp(t, (float)0.0, (float)1.0);
	x *= (float)nspans;
    span = (int) x;
    if (span >= nknots - 3)
        span = nknots - 3;
    x -= span;
    knot += span;

    /* Evaluate the span cubic at x using Horner's rule. */
    c3 = CR00*knot[0] + CR01*knot[1]
       + CR02*knot[2] + CR03*knot[3];
    c2 = CR10*knot[0] + CR11*knot[1]
       + CR12*knot[2] + CR13*knot[3];
    c1 = CR20*knot[0] + CR21*knot[1]
       + CR22*knot[2] + CR23*knot[3];
    c0 = CR30*knot[0] + CR31*knot[1]
       + CR32*knot[2] + CR33*knot[3];

    return ((c3*x + c2)*x + c1)*x + c0;
}



float
my_spl(float t, int nknots, float *knot)
{
  int span;
  int nspans = nknots - 3;
  float x,spl_res;
  double t2, t3;
  float m1, m2, m3, m4;

    /* Find the appropriate 4-point span of the spline. */
    x  = clamp(t, (float)0.0, (float)1.0);
	x *= (float)nspans;
    span = (int) x;
    if (span >= nknots - 3)
        span = nknots - 3;
    x -= span;
    knot += span;


    
    t2 = (double)(t * t);
    t3 = t2 * (double)t;

    m1 = (float)((-1.0 * t3) + (2.0 * t2) + (-1.0 * (double)t));
    m2 = (float)((3.0 * t3) + (-5.0 * t2) + (0.0 * (double)t) + 2.0);
    m3 = (float)((-3.0 * t3) + (4.0 * t2) + (1.0 * (double)t));
    m4 = (float)((1.0 * t3) + (-1.0 * t2) + (0.0 * (double)t));

    m1 /= (float)2.0;
    m2 /= (float)2.0;
    m3 /= (float)2.0;
    m4 /= (float)2.0;

    spl_res = knot[0] * m1 + knot[1] * m2 + knot[2] * m3 + knot[4] * m4;
    return spl_res;
}


float redKnots[13]={ 0.25f , 0.25f ,
							  0.10f , 0.10f  , 0.10f,
							  0.25f , 0.25f  ,
							  0.05f , 0.05f  ,
							  0.03f , 0.03f  ,
							  0.25f , 0.03f };
  
float greenKnots[13]={ 0.25f , 0.25f ,
								  0.10f , 0.10f  , 0.10f,
							      0.25f , 0.25f  ,
							      0.05f , 0.05f  ,
							      0.03f , 0.03f  ,
							      0.25f , 0.03f };
float blueKnots[13]={ 
							  0.35f , 0.35f ,
							  0.30f , 0.30f  , 0.30f,
							  0.35f , 0.35f  ,
							  0.26f , 0.26f  ,
							  0.20f , 0.20f  ,
							  0.35f , 0.20f };


float clamp(float cl, float a, float b)
{
float clampedVal ;
clampedVal = (cl < a ? a : (cl > b ? b : cl));
return clampedVal;
}
 
extern float marb_red,marb_green, marb_blue;
void marble_color(double m)
{
  float  res;
  res               =   clamp((((float)m + 1.0f) * 0.5f), 0.0f , 1.0f);
  marb_red      =   my_spline(res,13 , redKnots);
  marb_green   =   my_spline(res,13 , greenKnots);
  marb_blue     =   my_spline(res,13 , blueKnots);
 }     


