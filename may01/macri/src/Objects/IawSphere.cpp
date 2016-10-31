// IawSphere.cpp App Wizard Version 2.0 Beta 1
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
// IawSphere.cpp: definition of the Sphere Object, derived from the IAWObject base class.
// This class is used to contain a Sphere object, it's vertices, matrices, etc.
// The class stores two copies of the vertices, an array of D3DVerts, and a Vertex
// buffer used for rendering. At the time this class was authored, it was unclear
// whether the array was needed in case of a lost vertex buffer (DX7 beta docs unclear)
// so there may be some memory savings to be made there.
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// Constructor
IawSphere::IawSphere(DWORD flags) : IawObject(flags)
{
  SetObjectType(IAW_OBJECT_TYPE_SPHERE);
}

// Destructor
IawSphere::~IawSphere()
{
  // void
}

// Initialize
HRESULT IawSphere::Init(IawD3dWrapper* pWrapper,
                        float radius,
                        int div)
/*
                        IawVector pos,
                        IawVector scale,
                        IawVector rotationAxis,
                        float rotationAngle,
                        IawVector velocity,
                        IawVector scaleVelocity,
                        float rotationalVel ,
                        DWORD flags)
*/
{
  // vertex and index counters
  mNumDiv = div;
  mRadius = radius;
/*
  SetRotationAxis(rotationAxis);
  SetRotationAngle(rotationAngle);
  SetScale(scale);
  SetPos(pos);
  SetRotationalVelocity(rotationalVel);
  SetScalingVelocity(scaleVelocity);
  SetVelocity(velocity);
*/
  SetWrapper(pWrapper);

  SetNumVerts((mNumDiv * 2) + ((mNumDiv + 1) * (mNumDiv - 1)) + (mNumDiv + 1));
  SetNumIndices(6 * (mNumDiv + (mNumDiv * (mNumDiv-2))));

  AllocVertMem(GetNumVerts() * (GetVertexSize() / 4));
  AllocIndicesMem(GetNumIndices());

  //if we are being fed a bunch of verts, then we need to set them up
  if (SetupVertices(GetVerticesPtr()) != S_OK)
  {
    OutputDebugString("unable to set up verts");
    return E_FAIL;
  }

  if (SetupIndices() != S_OK)
  {
    OutputDebugString("unable to set up indices");
    return E_FAIL;
  }

  if (SetupIndexedVertexBuffer() != S_OK)
  {
    OutputDebugString("unable to set up VB");
    return E_FAIL;
  }

  return S_OK;
}


// Generate vertices that describe sphere geometry
HRESULT IawSphere::SetupVertices(float* pVertices)
{
  if (NULL == pVertices)
    return E_FAIL;

  float pi = 3.14159265358979323846f;

  // theta is the current latitudinal angle
  float theta = pi / 2.0f;

  // phi is the current longitudinal angle
  float phi = 0.0f;

  // delta values for the angles depending on the number of divisions
  float d_theta = pi / mNumDiv;
  float d_phi = 2.0f * pi / mNumDiv;

  IAW_SUPERVERTEX SV;
  float x, y, z;

  // Initialize the vertices at the top of the sphere
  SV.mX  = SV.mZ = SV.mNx = SV.mNz = 0.0f;
  SV.mY  = mRadius;
  SV.mNy = 1.0f;
  SV.mV  = 0.0f;

  for (int i=0; i<mNumDiv; i++)
  {
    SV.mU  = (float)i / mNumDiv;
    SetVertex(pVertices, SV);
  }

  // Intialize the vertices that make up the middle section
  for (i=0; i<mNumDiv; i++)
  {
    theta += d_theta;  // Longitude angle
    phi = 0.0f;     // Latitude angle

    // Iterates mNumDiv+1 times because we need an extra vertex at
    // each latitude to map a texture completely around the sphere
    for (int j=0; j<mNumDiv+1; j++)
    {
      x = (float)cos(theta) * (float)cos(phi);
      y = (float)sin(theta);
      z = (float)cos(theta) * (float)sin(phi);
      SV = IAW_SUPERVERTEX(mRadius * IawVector(x, y, z), IawVector(x, y, z),
                           (float)j/mNumDiv, (float)i/mNumDiv);
      phi += d_phi;

      SetVertex(pVertices, SV);
    }
  }

  // Initialize the bottom section vertices
  SV.mX = SV.mZ = SV.mNx = SV.mNz = 0.0f;
  SV.mY = -mRadius;
  SV.mNy = -1.0f;
  SV.mV = 1.0f;

  for (i = 0; i<mNumDiv; i++)
  {
    SV.mU = (float)i / mNumDiv;
    SetVertex(pVertices, SV);
  }

  return S_OK;
}

// Set up indexed vertex indices for using DrawIndexedPrimitive(VB/Strided)
HRESULT IawSphere::SetupIndices()
{
  int cur_lat, cur_long;
  int curVert = 0;
  WORD* indices_array = new WORD[GetNumIndices()];

  // Top section
  for (cur_lat=0; cur_lat<mNumDiv; cur_lat++)
  {
    indices_array[curVert++] = cur_lat;

    // Skip past the vertices at the top of the sphere (+mNumDiv)
    // and also skip to the next vertice (+1)
    indices_array[curVert++] = mNumDiv + (cur_lat + 1);

    // Skip past the vertices at the top of the sphere (+mNumDiv)
    indices_array[curVert++] = mNumDiv + cur_lat;  
  }

  // Middle section
  for (cur_lat=1; cur_lat<mNumDiv-1; cur_lat++)
  {
    for (cur_long=0; cur_long<mNumDiv; cur_long++)
    {
      // There are mNumDiv vertices at each latitude so we need to 
      // multiply by that amount to skip to a specific latitude
      // *----* <- Each quadrilateral space is composed of 2 triangles
      // | \ 2|    Triangle 1 is initialized after Triangle 2
      // |1  \|    
      // *----*    

      // first triangle of the quad area
      indices_array[curVert++] = (cur_lat * mNumDiv) + (cur_lat-1) + cur_long;
      indices_array[curVert++] = ((cur_lat + 1) * mNumDiv) + cur_lat + cur_long + 1;
      indices_array[curVert++] = ((cur_lat + 1) * mNumDiv) + cur_lat + cur_long;

      // second triangle of the quad area
      indices_array[curVert++] = (cur_lat * mNumDiv) + (cur_lat-1) + cur_long;
      indices_array[curVert++] = (cur_lat * mNumDiv) + (cur_lat-1) + cur_long + 1;
      indices_array[curVert++] = ((cur_lat + 1) * mNumDiv) + cur_lat + cur_long + 1;
    }
  }

  // Bottom section
  for (cur_lat = 0; cur_lat<mNumDiv; cur_lat++)
  {
    // Go to the second to last latitude (last is the bottom-most point)
    indices_array[curVert++] = ((mNumDiv * mNumDiv) - 2) + cur_lat;
    indices_array[curVert++] = ((mNumDiv * mNumDiv)-2) + (cur_lat + 1);  
    indices_array[curVert++] = ((mNumDiv * (mNumDiv + 1)) - 1) + cur_lat;
  }

  SetIndexArray(indices_array);

  delete [] indices_array;

  return S_OK;
}

