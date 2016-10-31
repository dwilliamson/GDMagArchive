// IawExponentialNoise.h App Wizard Version 2.0 Beta 1
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

#if !defined(ExponentialNoise_h)
#define ExponentialNoise_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This class generates noise using an exponential formula.
 * This is especially useful for making procedural textures smooth, or
 * generating height maps for procedural terrain.
 */
class IawExponentialNoise
{
public:
  /** Constructor */
  IawExponentialNoise(long lSeed);

  /** Destructor */
  virtual ~IawExponentialNoise();

  virtual float Value(float x, float y, float z);
  virtual float ExpValue(float x, float y, float z);
  virtual float AmpValue(float x, float y, float z);
  virtual void BuildMap(float originX, float originY, float originZ,
                        float dimX, float dimY, float dimZ,
                        int mapWidth, int mapHeight, float* pMap);
  void SetSequence(long seq);

private:
  IawNoiseGenerator *mpNoiseGenerator;
  IawNoiseGenerator *mpExponentGenerator;
  IawNoiseGenerator *mpHeightGenerator;
  float mExpScale, mExpMin, mExpMax;
  float mHeightMin, mHeightMax;
};

#endif // ExponentialNoise_h

