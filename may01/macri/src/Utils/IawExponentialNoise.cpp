// IawExponentialNoise.cpp App Wizard Version 2.0 Beta 1
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
// IawExponentialNoise.cpp: implementation of the CIawExponentialNoise class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// Constructor
IawExponentialNoise::IawExponentialNoise(long lSeed)
{
  mpNoiseGenerator = new IawNoiseGenerator(8/*NUMTERRAINOCTAVES*/, lSeed);
  mpExponentGenerator = new IawNoiseGenerator(4, lSeed+1);
  mpHeightGenerator = new IawNoiseGenerator(1, lSeed+2);
  mExpScale = 0.2f;
  /*
  mExpMin = 1.0f//MIN_EXPONENT;//0.25f;
  mExpMax = 1.0f//MAX_EXPONENT;//2.25f;
  mHeightMin = 1.0f//MIN_HEIGHT;
  mHeightMax = 1.0f//MAX_HEIGHT;
  */
  mExpMin = 0.3f;//MIN_EXPONENT;//0.25f;
  mExpMax = 3.3f;//MAX_EXPONENT;//2.25f;
  mHeightMin = 0.2f;//MIN_HEIGHT;
  mHeightMax = 1.0f;//MAX_HEIGHT;
}

// Destructor
IawExponentialNoise::~IawExponentialNoise()
{
  SAFE_DELETE(mpNoiseGenerator);
  SAFE_DELETE(mpExponentGenerator);
  SAFE_DELETE(mpHeightGenerator);
}

// Generate a value
float IawExponentialNoise::Value(float x, float y, float z)
{
  float height, exp, sgn, height_scale;
  
  height = mpHeightGenerator->Value(x, y, z);
  sgn = height >= (float)0 ? (float)1 : (float)(-1);
  height_scale = (float)(sgn * pow(fabs(height), 0.5f) * 0.5f + 0.5f) * (mHeightMax - mHeightMin) + mHeightMin;
  exp = mpExponentGenerator->Value(x * mExpScale, y, z * mExpScale);
  sgn = exp >= (float)0 ? (float)1 : (float)(-1);
  exp = (float)(sgn * pow(fabs(exp), 0.5f) * 0.5f + 0.5f) * (mExpMax - mExpMin) + mExpMin;
  height = mpNoiseGenerator->Value(x, y, z);
  sgn = height >= (float)0 ? (float)1 : (float)(-1);
  return (sgn * (float)pow(fabs(height),exp)) * height_scale;
}

// Generate an exponential value
float IawExponentialNoise::ExpValue(float x, float y, float z)
{
  float exp, sgn;
  
  exp = mpExponentGenerator->Value(x * mExpScale, y, z * mExpScale);
  sgn = exp >= (float)0 ? (float)1 : (float)(-1);
  exp = (float)(sgn * pow(fabs(exp), 0.5f) * 0.5f + 0.5f) * (mExpMax - mExpMin) + mExpMin;
  return exp;
}

// Generate an amplified value
float IawExponentialNoise::AmpValue(float x, float y, float z)
{
  float amp, sgn;
  
  amp = mpHeightGenerator->Value(x, y, z);
  sgn = amp >= (float)0 ? (float)1 : (float)(-1);
  amp = (float)(sgn * pow(fabs(amp), 0.5f) * 0.5f + 0.5f) * (mHeightMax - mHeightMin) + mHeightMin;
  return amp;
}

// Build a map of values
void IawExponentialNoise::BuildMap(float originX, float originY, float originZ,
                                   float dimX, float dimY, float dimZ,
                                   int mapWidth, int mapHeight, float* pMap)
{
  int i, j;
  float x, y, z, height, sgn, height_scale;
  float fI, fJ;
  float fWidth = (float)mapWidth;
  float fHeight = (float)mapHeight;
  float exp;
  //  char szTemp[20];
  
  for(i=0; i<mapHeight; i++)
  {
    fI = (float)i / (fHeight - 1);
    for(j=0; j<mapWidth; j++)
    {
      fJ = (float)j / (fWidth - 1);
      
      x = originX - dimX / (float)2.0 + dimX * fJ;
      y = originY;
      z = originZ - dimZ / (float)2.0 + dimZ * fI;
      height = mpHeightGenerator->Value(x, y, z);
      sgn = height >= (float)0 ? (float)1 : (float)(-1);
      height_scale = (float)(sgn * pow(fabs(height), 0.5f) * 0.5f + 0.5f) * (mHeightMax - mHeightMin) + mHeightMin;
      exp = mpExponentGenerator->Value(x * mExpScale, y, z * mExpScale);
      sgn = exp >= (float)0 ? (float)1 :(float)(-1);
      exp = (sgn * (float)pow(fabs(exp), 0.5f) * 0.5f + 0.5f) * (mExpMax - mExpMin) + mExpMin;
      height = mpNoiseGenerator->Value(x, y, z);
      sgn = height >= (float)0 ? (float)1 : (float)(-1);
      pMap[ i*mapWidth + j ] = sgn * (float)pow(fabs(height),exp) * height_scale;
    }
  }
}

void IawExponentialNoise::SetSequence(long seq)
{
  mpNoiseGenerator->SetSequence(seq);
  mpExponentGenerator->SetSequence(seq+1);
}

