// IawNoiseGenerator.h App Wizard Version 2.0 Beta 1
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
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#if !defined(IawNoiseGenerator_h)
#define IawNoiseGenerator_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This class is a psuedo-random number gereator.
 * A seeded value will generate the same results.  Useful for terrain height
 * map generation.
 */
class IawNoiseGenerator
{
public:
  /** Constructor */
  IawNoiseGenerator(int numOctaves = 8, long seq = 174829);

  /** Destructor */
  virtual ~IawNoiseGenerator();

  /** Build a map of values */
  virtual void BuildMap(float originX, float originY, float originZ,
                        float dimX, float dimY, float dimZ,
                        int mapWidth, int mapHeight, float* pMap);

  void SetSequence(long seq);

  float Value(float x, float y, float z);

protected:
  long mSeq;
  long mPrimes[16][3];
  int  mNumOctaves;

private:
  inline float Noise(int octave, long x, long y);
  inline float Interpolate(float a, float b, float frac);
  inline float InterpolatedNoise(int octave, float x, float y);
};

#endif // IawNoiseGenerator_h

