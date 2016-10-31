// IawNoiseGenerator.cpp App Wizard Version 2.0 Beta 1
// ----------------------------------------------------------------------
// 
// Copyright © 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// ----------------------------------------------------------------------
//
// PURPOSE:
//    
// IawNoiseGenerator.cpp: implementation of the CIawNoiseGenerator class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// Constructor
IawNoiseGenerator::IawNoiseGenerator(int numoctaves , long seq)
{
  mNumOctaves = numoctaves;
  mSeq = seq;
  mPrimes[ 0][0] = 15731; mPrimes[ 0][1] = 789221; mPrimes[ 0][2] = 1376312589; 
  mPrimes[ 1][0] = 15859; mPrimes[ 1][1] = 789491; mPrimes[ 1][2] = 1376312627; 
  mPrimes[ 2][0] = 16007; mPrimes[ 2][1] = 789683; mPrimes[ 2][2] = 1376312953; 
  mPrimes[ 3][0] = 16183; mPrimes[ 3][1] = 789959; mPrimes[ 3][2] = 1376313223; 
  mPrimes[ 4][0] = 16349; mPrimes[ 4][1] = 790169; mPrimes[ 4][2] = 1376313623; 
  mPrimes[ 5][0] = 16493; mPrimes[ 5][1] = 790369; mPrimes[ 5][2] = 1376313977; 
  mPrimes[ 6][0] = 16661; mPrimes[ 6][1] = 790567; mPrimes[ 6][2] = 1376314361; 
  mPrimes[ 7][0] = 16843; mPrimes[ 7][1] = 790753; mPrimes[ 7][2] = 1376314739; 
  mPrimes[ 8][0] = 16993; mPrimes[ 8][1] = 791191; mPrimes[ 8][2] = 1376315111; 
  mPrimes[ 9][0] = 17159; mPrimes[ 9][1] = 791387; mPrimes[ 9][2] = 1376315459; 
  mPrimes[10][0] = 17327; mPrimes[10][1] = 791629; mPrimes[10][2] = 1376315827; 
  mPrimes[11][0] = 17467; mPrimes[11][1] = 791897; mPrimes[11][2] = 1376316257; 
  mPrimes[12][0] = 17599; mPrimes[12][1] = 792067; mPrimes[12][2] = 1376316581; 
  mPrimes[13][0] = 17783; mPrimes[13][1] = 792263; mPrimes[13][2] = 1376316791; 
  mPrimes[14][0] = 17929; mPrimes[14][1] = 792479; mPrimes[14][2] = 1376317333; 
  mPrimes[15][0] = 18077; mPrimes[15][1] = 792643; mPrimes[15][2] = 1376317711; 
}

// Destructor
IawNoiseGenerator::~IawNoiseGenerator()
{
}

// Set the seed
void IawNoiseGenerator::SetSequence(long seq)
{
  mSeq = seq;
}

// Create some noise
float IawNoiseGenerator::Noise(int octave, long x, long y)
{
  long n = x + (y + mSeq) * 59 + mSeq;
  n = (n<<13) ^ n;
  return (float)(1.0 - ((n * (n * n * mPrimes[octave][0] + mPrimes[octave][1]) + mPrimes[octave][2]) & 0x7fffffff) / 1073741824.0);    
}

// Cosine interpolation
float IawNoiseGenerator::Interpolate(float a, float b, float frac)
{
  //  float angle = frac * 3.1415926535897932;
  
  //  frac = (1.0 - cos(angle)) * 0.5;
  //  frac *= frac;
  
  return a + (b - a) * frac;
}

float IawNoiseGenerator::InterpolatedNoise(int octave, float x, float y )
{
  long lX, lY;  
  float fx, fy;
  
  if (x >= 0.0)
    lX = (long)x;
  else
    lX = (long)x - 1;
  if (y >= 0.0)
    lY = (long)y;
  else
    lY = (long)y - 1;
  
  fx = x - (float)lX;
  fy = y - (float)lY;
  
  float v1, v2, v3, v4, i1, i2;
  
  v1 = Noise(octave, lX, lY);
  v2 = Noise(octave, lX+1, lY);
  v3 = Noise(octave, lX, lY + 1);
  v4 = Noise(octave, lX+1, lY+1);
  
  i1 = Interpolate(v1, v2, fx);
  i2 = Interpolate(v3, v4, fx);
  
  return Interpolate(i1, i2, fy);
}

float IawNoiseGenerator::Value(float x, float y, float z)
{
  float total = 0.0;
  float total_amp = 0.0;
  float freq, ampl ;
  
  int n=mNumOctaves; //8 by default
  
  for (int i=0; i<n+1; i++)
  {
    //    if ((i == 0) || (i == 1) || (i == 2) || (i == 3))
    //    {
    freq = (float)(1 << i);
    ampl = (float)1.0 / freq;
    total_amp += ampl;
    total += InterpolatedNoise(i, x * freq, z * freq) * ampl;
    //    }
  }
  return total/total_amp;
  
  
}


void IawNoiseGenerator::BuildMap(float originX, float originY, float originZ,
                                 float dimX, float dimY, float dimZ,
                                 int mapWidth, int mapHeight, float *pMap)
{
  int i, j;
  float x, y, z, height;
  float fI, fJ;
  float fWidth = (float)mapWidth;
  float fHeight = (float)mapHeight;
  
  for (i=0; i<mapHeight; i++)
  {
    fI = (float)i / (fHeight - 1);
    for (j=0; j<mapWidth; j++)
    {
      fJ = (float)j / (fWidth - 1);
      
      x = originX - dimX / (float)2.0 + dimX * fJ;
      y = originY;
      z = originZ - dimZ / (float)2.0 + dimZ * fI;
      height = Value(x, y, z);
      pMap[ i*mapWidth + j ] = height * dimY;
    }
  }
}


