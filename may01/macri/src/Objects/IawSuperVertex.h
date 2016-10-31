
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
#if !defined(IawSuperVertex_h)
#define IawSuperVertex_h

/**
 * The SUPERVERTEX.
 * This structure contains all the information of all possible permutations of
 * Direct3D vertex structures. It is not intended for direct use other than
 * being a set up tool for more specific (and possible) vertex structures.
 * Clearly a vertex with all these member variables present does not make sense
 * and cannot exist.
 */
typedef struct _IAW_SUPERVERTEX
{
public:
  // Position
  D3DVALUE mX;
  D3DVALUE mY;
  D3DVALUE mZ;

  // Normal
  D3DVALUE mNx;
  D3DVALUE mNy;
  D3DVALUE mNz;

  // Screen Coordinates
  D3DVALUE mRhw;

  // Texture Coordinates
  D3DVALUE mU, mU1, mU2, mU3, mU4, mU5, mU6, mU7;
  D3DVALUE mV, mV1, mV2, mV3, mV4, mV5, mV6, mV7;
  D3DVALUE mS, mS1, mS2, mS3, mS4, mS5, mS6, mS7;
  D3DVALUE mT, mT1, mT2, mT3, mT4, mT5, mT6, mT7;

  // Blending Weight Values
  D3DVALUE mB1, mB2, mB3, mB4, mB5;

  // Diffuse and Specular component of the vertex
  D3DCOLOR mDiffuse;
  D3DCOLOR mSpecular;

#if defined(__cplusplus) && defined(D3D_OVERLOADS)
  _IAW_SUPERVERTEX()
  {
    // Position
    mX = 0.0f;
    mY = 0.0f;
    mZ = 0.0f;

    // RHW
    mRhw = 0.0f;

    // Blending Weight Values
    mB1 = mB2 = mB3 = mB4 = mB5 = 0.0f;

    // Normal
    mNx = 0.0f;
    mNy = 0.0f;
    mNz = 0.0f;

    // Diffuse color
    mDiffuse  = 0L;

    // Specular color
    mSpecular = 0L;

    // Texture Coordinates
    mU = mU1 = mU2 = mU3 = mU4 = mU5 = mU6 = mU7 = 0.0f;
    mV = mV1 = mV2 = mV3 = mV4 = mV5 = mV6 = mV7 = 0.0f;
    mS = mS1 = mS2 = mS3 = mS4 = mS5 = mS6 = mS7 = 0.0f;
    mT = mT1 = mT2 = mT3 = mT4 = mT5 = mT6 = mT7 = 0.0f;
  }

  //_IAW_SUPERVERTEX(const D3DVECTOR& v, const D3DVECTOR& n, float tu, float tv)
  _IAW_SUPERVERTEX(const IawVector& v, const IawVector& n, float tu, float tv)
  {
    // Position
    mX = v.mX;
    mY = v.mY;
    mZ = v.mZ;

    // Normal
    mNx = n.mX;
    mNy = n.mY;
    mNz = n.mZ;

    // RHW
    mRhw = 0.0f;

    // Blending Weight Values
    mB1 = mB2 = mB3 = mB4 = mB5 = 0.0f;

    // Diffuse color
    mDiffuse  = 0L;

    // Specular color
    mSpecular = 0L;

    // Texture coordinates
    mU = tu;
    mV = tv;

    mU1 = mU2 = mU3 = mU4 = mU5 = mU6 = mU7 = 1.0f-tu;//0.0f;
    mV1 = mV2 = mV3 = mV4 = mV5 = mV6 = mV7 = 1.0f-tv;//0.0f;
    mS = mS1 = mS2 = mS3 = mS4 = mS5 = mS6 = mS7 = 0.0f; // set additional texture coordinates to 0.0
    mT = mT1 = mT2 = mT3 = mT4 = mT5 = mT6 = mT7 = 0.0f;
  }
#endif
//  ~IAW_SUPERVERTEX() {}
} IAW_SUPERVERTEX;

#endif //IawSuperVertex_h
