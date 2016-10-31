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
#include <memory.h>
#include <math.h>
#include <d3d.h>
#include "d3dmain.h"
#include "render.h"
/***************************************************************************************/
// 1/2 size of the SW thread window (256*256)
static long    halfSwWinSize= 256/2;
/****************************************************************************************/
void 
transformD3DNormals(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV)
{
    static int i;
    static float x,y, z;
    static float aMat[4][4];
    memcpy(aMat, lpMRot, sizeof(D3DMATRIX));
	for (i = 0; i < count; i++) {
        x = lpV[i].nx;  y = lpV[i].ny; z = lpV[i].nz; 
		lpV[i].nx =  x * aMat[0][0] + y * aMat[1][0] +  z * aMat[2][0];
		lpV[i].ny =  x * aMat[0][1] + y * aMat[1][1] +  z * aMat[2][1];
        lpV[i].nz =  x * aMat[0][2] + y * aMat[1][2] +  z * aMat[2][2];
       }
}
/****************************************************************************************/
void 
transformD3DVERTEX(LPD3DMATRIX lpMRot, int count, LPD3DVERTEX lpV)
{
    int i;
    float x,y, z;
    float aMat[4][4];
    
	memcpy(aMat, lpMRot, sizeof(D3DMATRIX));
     for (i = 0; i < count; i++) {
        x = lpV[i].x;  y = lpV[i].y; z = lpV[i].z; 
        lpV[i].x =  x * aMat[0][0] + y * aMat[1][0] +  z * aMat[2][0] +  halfSwWinSize;
		lpV[i].y =  x * aMat[0][1] + y * aMat[1][1] +  z * aMat[2][1] +  halfSwWinSize;
        lpV[i].z =  x * aMat[0][2] + y * aMat[1][2] +  z * aMat[2][2];
       }
}
/****************************************************************************************/
void
ConcatenateXRotation(LPD3DMATRIX lpM, float Degrees )
{
  float Temp01, Temp11, Temp21, Temp31;
  float Temp02, Temp12, Temp22, Temp32;
  float aElements[4][4];

  float Radians = (float)((Degrees/360) * M_PI * 2.0);

  float Sin = (float)sin(Radians), Cos = (float)cos(Radians);

  memcpy(aElements, lpM, sizeof(D3DMATRIX));
  Temp01 = aElements[0][1] * Cos + aElements[0][2] * Sin;
  Temp11 = aElements[1][1] * Cos + aElements[1][2] * Sin;
  Temp21 = aElements[2][1] * Cos + aElements[2][2] * Sin;
  Temp31 = aElements[3][1] * Cos + aElements[3][2] * Sin;

  Temp02 = aElements[0][1] * -Sin + aElements[0][2] * Cos;
  Temp12 = aElements[1][1] * -Sin + aElements[1][2] * Cos;
  Temp22 = aElements[2][1] * -Sin + aElements[2][2] * Cos;
  Temp32 = aElements[3][1] * -Sin + aElements[3][2] * Cos;

  lpM->_12 = Temp01;
  lpM->_22 = Temp11;
  lpM->_32 = Temp21;
  lpM->_42 = Temp31;
  lpM->_13 = Temp02;
  lpM->_23 = Temp12;
  lpM->_33 = Temp22;
  lpM->_43 = Temp32;
}
/************************************************************************/
void
ConcatenateYRotation(LPD3DMATRIX lpM, float Degrees )
{
  float Temp00, Temp10, Temp20, Temp30;
  float Temp02, Temp12, Temp22, Temp32;
  float aElements[4][4];

  float Radians = (float)((Degrees/360) * M_PI * 2);

  float Sin = (float)sin(Radians), Cos = (float)cos(Radians);

  memcpy(aElements, lpM, sizeof(D3DMATRIX));
  Temp00 = aElements[0][0] * Cos + aElements[0][2] * -Sin;
  Temp10 = aElements[1][0] * Cos + aElements[1][2] * -Sin;
  Temp20 = aElements[2][0] * Cos + aElements[2][2] * -Sin;
  Temp30 = aElements[3][0] * Cos + aElements[3][2] * -Sin;

  Temp02 = aElements[0][0] * Sin + aElements[0][2] * Cos;
  Temp12 = aElements[1][0] * Sin + aElements[1][2] * Cos;
  Temp22 = aElements[2][0] * Sin + aElements[2][2] * Cos;
  Temp32 = aElements[3][0] * Sin + aElements[3][2] * Cos;

  lpM->_11 = Temp00;
  lpM->_21 = Temp10;
  lpM->_31 = Temp20;
  lpM->_41 = Temp30;
  lpM->_13 = Temp02;
  lpM->_23 = Temp12;
  lpM->_33 = Temp22;
  lpM->_43 = Temp32;
}
/************************************************************************/
void
ConcatenateZRotation(LPD3DMATRIX lpM, float Degrees )
{
  float Temp00, Temp10, Temp20, Temp30;
  float Temp01, Temp11, Temp21, Temp31;
  float aElements[4][4];

  float Radians = (float)((Degrees/360) * M_PI * 2);

  float Sin = (float)sin(Radians), Cos = (float)cos(Radians);

  memcpy(aElements, lpM, sizeof(D3DMATRIX));
  Temp00 = aElements[0][0] * Cos + aElements[0][1] * Sin;
  Temp10 = aElements[1][0] * Cos + aElements[1][1] * Sin;
  Temp20 = aElements[2][0] * Cos + aElements[2][1] * Sin;
  Temp30 = aElements[3][0] * Cos + aElements[3][1] * Sin;

  Temp01 = aElements[0][0] * -Sin + aElements[0][1] * Cos;
  Temp11 = aElements[1][0] * -Sin + aElements[1][1] * Cos;
  Temp21 = aElements[2][0] * -Sin + aElements[2][1] * Cos;
  Temp31 = aElements[3][0] * -Sin + aElements[3][1] * Cos;

  lpM->_11 = Temp00;
  lpM->_21 = Temp10;
  lpM->_31 = Temp20;
  lpM->_41 = Temp30;
  lpM->_12 = Temp01;
  lpM->_22 = Temp11;
  lpM->_32 = Temp21;
  lpM->_42 = Temp31;
}


void 
transformCopyD3DVERTEX(LPD3DMATRIX lpMat, D3DVECTOR *lpVSrc, D3DVECTOR *lpVDst)
{
  static   float x,y, z;
  static   float aMat[4][4];
  memcpy(aMat, lpMat, sizeof(D3DMATRIX));
  x = lpVSrc[0].x;  y = lpVSrc[0].y; z = lpVSrc[0].z; 
  
  lpVDst[0].x =  x * aMat[0][0] + y * aMat[1][0] +  z * aMat[2][0];
  lpVDst[0].y =  x * aMat[0][1] + y * aMat[1][1] +  z * aMat[2][1] ;
  lpVDst[0].z =  x * aMat[0][2] + y * aMat[1][2] +  z * aMat[2][2];
 
}
