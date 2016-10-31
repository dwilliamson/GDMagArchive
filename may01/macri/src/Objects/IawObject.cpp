// IawObject.cpp App Wizard Version 2.0 Beta 1
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
// IawObject.cpp: implementation of the CIawObject class.
// This class is used to contain an object, it's vertices, matrices, etc...
// The class stores two copies of the vertices, an array of D3DVerts, and a Vertex
// buffer used for rendering. At the time this class was authored, it was unclear
// whether the array was needed in case of a lost vertex buffer (DX7 beta docs unclear)
// so there may be some memory savings to be made there.
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

#include <rmxfguid.h> // Used in loading .X files
#include <rmxftmpl.h> // Used in loading .X files

/* Class constants... */

// Object types...
const int IawObject::IAW_OBJECT_TYPE_BASEOBJECT = 0x00000001;
const int IawObject::IAW_OBJECT_TYPE_BACKGROUND = 0x00000002;
const int IawObject::IAW_OBJECT_TYPE_CONE       = 0x00000004;
const int IawObject::IAW_OBJECT_TYPE_CUBE       = 0x00000008;
const int IawObject::IAW_OBJECT_TYPE_CYLINDER   = 0x00000010;
const int IawObject::IAW_OBJECT_TYPE_PLANE      = 0x00000020;
const int IawObject::IAW_OBJECT_TYPE_SKYPLANE   = 0x00000040;
const int IawObject::IAW_OBJECT_TYPE_SPHERE     = 0x00000080;
const int IawObject::IAW_OBJECT_TYPE_TERRAIN    = 0x00000100;
const int IawObject::IAW_OBJECT_TYPE_STRING     = 0x00000200;
const int IawObject::IAW_OBJECT_TYPE_XOBJECT    = 0x00000400;
const int IawObject::IAW_OBJECT_TYPE_GNOMON     = 0x00000800;
const int IawObject::IAW_OBJECT_TYPE_CAMERA     = 0x00001000;

const int IawObject::IAW_OBJECT_TYPE_OBJMANAGER = 0x10000000;
const int IawObject::IAW_OBJECT_TYPE_TEXTMGR    = (IAW_OBJECT_TYPE_OBJMANAGER | 0x00001000);
// have to convert this to be a derivative of object still.
//const int IawObject::IAW_OBJECT_TYPE_TERRAINMGR = (IAW_OBJECT_TYPE_OBJMANAGER | 0x00002000);

// Object loading directives...
const IawObject::OBJECT_LOAD_FILE = 1;
const IawObject::OBJECT_LOAD_URL  = 2;

// Update flags...
const int IawObject::IAW_UF_POSITION  = 0x001;
const int IawObject::IAW_UF_SCALE     = 0x002;
const int IawObject::IAW_UF_ROTATION  = 0x004;
const int IawObject::IAW_UF_ROTVEL    = 0x010;
const int IawObject::IAW_UF_SCALEVEL  = 0x020;
const int IawObject::IAW_UF_VELOCITY  = 0x040;
const int IawObject::IAW_UF_INVALIDMX = 0x100;

// ----------------------------------------------------------------------------
// Construction/Destruction
// ----------------------------------------------------------------------------
IawObject::IawObject(DWORD flags)
{
  mObjectType = IAW_OBJECT_TYPE_BASEOBJECT;
  sprintf(mStrName, "Unnamed object");

  mpIndexedVertexBuffer = NULL;
  mpIndexBuffer         = NULL;
  mPrimType             = D3DPT_TRIANGLELIST;

  mLocalBaseMx.SetIdentity();
  mLocalToParentMx.SetIdentity();
  mLocalToWorldMx.SetIdentity();

  mPosition      = IawVector(0.0f);
  mScale         = IawVector(1.0f);
  mRotationAxis  = IawVector(0.0f, 1.0f, 0.0f);
  mRotationAngle = 0.0f;

  mRotationalVelocity = 0.0f;
  mScalingVelocity    = IawVector(0.0f);
  mVelocity           = IawVector(0.0f);

  mVisible = true;

  mpNext       = NULL;
  mpFirstChild = NULL;
  mpParent     = NULL;

  mNumVertices = 0;
  mNumIndices  = 0;
  mpVertices   = NULL;
  mpIndices     = NULL;

  mpWrapper   = NULL;
  mpShaderMgr = NULL;
  mShaderId   = 0;

  mFVF = flags;

  SetVertexSize();

  mSharedVb = false;

  mUpdateStatus = 0L;
}


IawObject::~IawObject()
{
  // Get rid of the VB
  if (!mSharedVb)
  {
    if (mpIndexedVertexBuffer)
      SAFE_RELEASE(mpIndexedVertexBuffer);
    if (mpIndexBuffer)
      SAFE_RELEASE(mpIndexBuffer);
  }
  else
  {
    mpIndexedVertexBuffer = NULL;
    mpIndexBuffer = NULL;
  }

  if (mpNext)
    delete mpNext;
  
  if (mpFirstChild)
    delete mpFirstChild;

  DeleteVertMem();
  DeleteIndicesMem();

  mpNext       = NULL;
  mpFirstChild = NULL;
  mpVertices   = NULL;
  mpIndices    = NULL;
  mpWrapper    = NULL;
}

// Access methods set bitflags if/when default values are not used
void IawObject::SetPos(IawVector pos)
{
  mPosition = pos;

  if (pos.mX != 0.0f || pos.mY != 0.0f || pos.mZ != 0.0f)
    mUpdateStatus |= IAW_UF_POSITION;
  else
    mUpdateStatus &= ~IAW_UF_POSITION;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}


void IawObject::SetScale(IawVector scale)
{
  mScale = scale;

  if (scale.mX != 1.0f || scale.mY != 1.0f || scale.mZ != 1.0f)
    mUpdateStatus |= IAW_UF_SCALE;
  else
    mUpdateStatus &= ~IAW_UF_SCALE;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}

void IawObject::SetRotationAxis(IawVector RA)
{
  mRotationAxis = RA;

  if (RA.mX != 0.0f || RA.mY != 1.0f || RA.mZ != 0.0f)
    mUpdateStatus |= IAW_UF_ROTATION;
  else
    mUpdateStatus &= ~IAW_UF_ROTATION;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}

void IawObject::SetRotationAngle(float RA)
{
  mRotationAngle = RA;

  if (RA != 0.0f)
    mUpdateStatus |= IAW_UF_ROTATION;
  else
    mUpdateStatus &= ~IAW_UF_ROTATION;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}

void IawObject::SetRotationalVelocity(float RV)
{
  mRotationalVelocity = RV;

  if (RV != 0.0f)
    mUpdateStatus |= IAW_UF_ROTVEL;
  else
    mUpdateStatus &= ~IAW_UF_ROTVEL;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}

void IawObject::SetScalingVelocity(IawVector SV)
{
  mScalingVelocity    = SV;

  if (SV.mX != 0.0f || SV.mY != 0.0f || SV.mZ != 0.0f)
    mUpdateStatus |= IAW_UF_SCALEVEL;
  else
    mUpdateStatus &= ~IAW_UF_SCALEVEL;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}

void IawObject::SetVelocity(IawVector Vel)
{
  mVelocity       = Vel;

  if (Vel.mX != 0.0f || Vel.mY != 0.0f || Vel.mZ != 0.0f)
    mUpdateStatus |= IAW_UF_VELOCITY;
  else
    mUpdateStatus &= ~IAW_UF_VELOCITY;

  mUpdateStatus |= IAW_UF_INVALIDMX;
}


// Initializers...
HRESULT IawObject::Init(IawD3dWrapper* pWrapper,
                        float* pVerts,
                        WORD* pIndices,
                        DWORD numVerts,
                        DWORD numIndices,
                        DWORD flags)
/*
                        IawVector pos     ,
                        IawVector scale   ,
                        IawVector rotationaxis,
                        float   rotationangle,
                        IawVector vel     ,
                        IawVector scalevel  ,
                        float   rotationalvel,
                        DWORD   dwFlags)
*/
{
/*
  SetRotationAxis(rotationaxis);
  SetRotationAngle(rotationangle);
  SetScale(scale);
  SetPos(pos);
  SetRotationalVelocity(rotationalvel);
  SetScalingVelocity(scalevel);
  SetVelocity(vel);
*/
  // vertex and index counters
  SetNumVerts(numVerts);
  SetNumIndices(numIndices);

  //ptr to the wrapper
  SetWrapper(pWrapper);

  SetVertexSize();
  //mVertexSize = GetVertexSize(dwFlags);

  if (flags)
  {
    SetFVF(flags);
    SetVertexSize();  // Set mVertexSize based on new FVF flags
    //mVertexSize = GetVertexSize(dwFlags);
  }

  //if we are being fed a bunch of verts, then we need to set them up
  if (numVerts)
  {
    if (SetupVertices(pVerts) != S_OK)
    {
      OutputDebugString("unable to set up verts");
      return E_FAIL;
    }

    if (SetupIndices(pIndices) != S_OK)
    {
      OutputDebugString("unable to set up indices");
      return E_FAIL;
    }

    if (SetupIndexedVertexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up VB");
      return E_FAIL;
    }

    if (SetupIndexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up IB");
      return E_FAIL;
    }
  }

  return S_OK;
}


HRESULT IawObject::Init(IawD3dWrapper* pWrapper, char* strFileName)
/*
                        IawVector pos       ,
                        IawVector scale     ,
                        IawVector rotationaxis  ,
                        float   rotationangle ,
                        IawVector vel       ,
                        IawVector scalevel    ,
                        float   rotationalvel ,
                        DWORD   dwFlags     )
*/
{
/*
  SetRotationAxis(rotationaxis);
  SetRotationAngle(rotationangle);
  SetScale(scale);
  SetPos(pos);
  SetRotationalVelocity(rotationalvel);
  SetScalingVelocity(scalevel);
  SetVelocity(vel);
*/

  // vertex and index counters
  SetNumVerts(0);
  SetNumIndices(0);

  //ptr to the wrapper
  SetWrapper(pWrapper);
//  SetVertexSize();			// Set member var mVertexSize based on FVF flags

  //this will load the file for us
  if (strFileName)
    LoadObject(strFileName, NULL);

  if (GetNumVerts())
  {
    if (SetupIndexedVertexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up VB");
      return E_FAIL;
    }

    if (SetupIndexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up VB");
      return E_FAIL;
    }
  }

  return S_OK;
}


HRESULT IawObject::Init(IawObject* pObject)
/*
                        IawVector pos     ,
                        IawVector scale   ,
                        IawVector rotationaxis,
                        float   rotationangle,
                        IawVector vel     ,
                        IawVector scalevel  ,
                        float   rotationalvel,
                        DWORD   dwFlags   )
*/
{
/*
	if (pIAWObject->mFVF != mFVF)
	{
		OutputDebugString("FVF incompatibility with copy object\n");
		return E_FAIL;
	}
*/
/*
  SetRotationAxis(rotationaxis);
  SetRotationAngle(rotationangle);
  SetScale(scale);
  SetPos(pos);
  SetRotationalVelocity(rotationalvel);
  SetScalingVelocity(scalevel);
  SetVelocity(vel);
*/
  if (pObject)
  {
    mpWrapper     = pObject->mpWrapper;

    SetFVF(pObject->GetFVF());
    mVertexSize   = pObject->mVertexSize;

    mNumVertices  = pObject->mNumVertices;
    mNumIndices   = pObject->mNumIndices;

    mpVertices    = new float[mNumVertices * (mVertexSize / 4)];
    memcpy(mpVertices, pObject->mpVertices, mNumVertices * mVertexSize);

    mpIndices     = new WORD[mNumIndices];
    memcpy(mpIndices, pObject->mpIndices, mNumIndices * sizeof(WORD));

    mSharedVb     = true;

    mpIndexedVertexBuffer = pObject->mpIndexedVertexBuffer;
    mpIndexBuffer = pObject->mpIndexBuffer;
  }

  return S_OK;
}


int IawObject::GetTriangleCount()
{
  return (mNumIndices / 3);
}


// Generate vertices
HRESULT IawObject::SetupVertices(float* pVertices)
{
  if (NULL == pVertices)
    return E_FAIL;

  mpVertices = new float[mNumVertices * (mVertexSize / 4)];
  memcpy(mpVertices, pVertices, (mNumVertices * mVertexSize));

  return S_OK;
}

// Set up indexed vertex indices for using DrawIndexedPrimitive(VB/Strided)
HRESULT IawObject::SetupIndices(WORD* pVertexIndices)
{
  mpIndices = new WORD[mNumIndices];
  memcpy(mpIndices,pVertexIndices,mNumIndices * sizeof(WORD));

  return S_OK;
}


// Set up a vertex buffer for indexed vertices
HRESULT IawObject::SetupIndexedVertexBuffer()
{
  HRESULT hr;

  if (!mSharedVb)
  {
    if (mpIndexedVertexBuffer != NULL)
      SAFE_RELEASE(mpIndexedVertexBuffer);

    // Define the vertex buffer descriptor for creating the
    // vertex buffer for DrawIndexedPrimitiveVB

    // Create a vertex buffer using D3D7 interface
    if (mpWrapper)
    {
      if (mpWrapper->mpDevice)
      {
        //DWORD tempflags = 0;
        DWORD tempflags = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY ;

        hr = mpWrapper->mpDevice->CreateVertexBuffer(mVertexSize*mNumVertices, 
          tempflags,
          mFVF,
          D3DPOOL_DEFAULT,
          &mpIndexedVertexBuffer);
        if (FAILED(hr))
          return hr;

        float* pTemp;

        if (FAILED(hr = mpIndexedVertexBuffer->Lock(0,
                                                    mVertexSize*mNumVertices,
                                                    (unsigned char**)&pTemp, 
                                                    NULL)))
          return hr;

        memcpy(pTemp, mpVertices, (mNumVertices * mVertexSize));

        if (FAILED(hr = mpIndexedVertexBuffer->Unlock()))
          return hr;
      }
    }
  }

  return S_OK;
}



// Set up an index buffer for indexed vertices
HRESULT IawObject::SetupIndexBuffer()
{
  HRESULT hr;

  if (!mSharedVb)
  {
    if (mpIndexBuffer != NULL)
      SAFE_RELEASE(mpIndexBuffer);

    // Define the vertex buffer descriptor for creating the
    // vertex buffer for DrawIndexedPrimitiveVB

    // Create a vertex buffer using D3D7 interface
    if (mpWrapper)
    {
      if (mpWrapper->mpDevice)
      {
        DWORD tempflags = 0;

        hr =  mpWrapper->mpDevice->CreateIndexBuffer(mNumIndices*sizeof(WORD),
          0,
          D3DFMT_INDEX16,
          D3DPOOL_DEFAULT,
          &mpIndexBuffer);

        if (FAILED(hr))
          return hr;

        //-----------------------------------------------------------------------------
        //    D3DVERTEX *pTemp;
        //
        //    if (FAILED(hr = mpIndexedVertexBuffer->Lock(DDLOCK_WAIT, (VOID **)&pTemp, NULL)))
        //      return hr;
        //
        //    memcpy(pTemp,mpVertices,mNumVertices * sizeof(D3DVERTEX));
        //-----------------------------------------------------------------------------
        WORD *pTemp;

        if (FAILED(hr = mpIndexBuffer->Lock(0,
                                            sizeof(WORD)*mNumIndices,
                                            (unsigned char**)&pTemp, 
                                            NULL)))
          return hr;

        memcpy(pTemp, mpIndices, (mNumIndices * sizeof(WORD)));

        if (FAILED(hr = mpIndexBuffer->Unlock()))
          return hr;
      }
    }
  }

  return S_OK;
}


// Update
HRESULT IawObject::Update(float frameTime, bool bUpdateChildren)
{
  //update the object's current rotation angle, scale, and position
  if (mUpdateStatus & IAW_UF_ROTVEL)
    SetRotationAngle(mRotationAngle + mRotationalVelocity * frameTime);

  if (mUpdateStatus & IAW_UF_SCALEVEL)
    SetScale(mScale + mScalingVelocity * frameTime);

  if (mUpdateStatus & IAW_UF_VELOCITY)
    SetPos(mPosition + mVelocity * frameTime);

  // Rebuild the local to parent matrix if it became invalid
  if (mUpdateStatus & IAW_UF_INVALIDMX)
  {
    // Start with the base matrix
    mLocalToParentMx = mLocalBaseMx;

    // Translate the local to parent matrix if the position vector is not
    // the default.
    if (mUpdateStatus & IAW_UF_POSITION)
      mLocalToParentMx.Translate(mPosition.mX, mPosition.mY, mPosition.mZ);

    // Scale the local to parent matrix if the scaleing vector is not the
    // default.
    if (mUpdateStatus & IAW_UF_SCALE)
      mLocalToParentMx.Scale(mScale.mX, mScale.mY, mScale.mZ);

    // Rotate the local to parent matrix if either the rotation axis or the
    // rotation angle is not the default.
    if (mUpdateStatus & IAW_UF_ROTATION)
      mLocalToParentMx.Rotate(mRotationAxis, mRotationAngle);

    // The local to parent matrix is valid as of this point.  It may change
    // by the next frame, at which point we will repeat this "if" statement
    mUpdateStatus &= ~IAW_UF_INVALIDMX;
  }

  if (bUpdateChildren)
  {
    IawObject* temp_object = mpFirstChild;
    while (temp_object != NULL)
    {
      temp_object->Update(frameTime, true);
      temp_object = temp_object->mpNext;
    }
  }

  return S_OK;
}



HRESULT IawObject::Render(IawMatrix& rWorldMat, bool renderChildren)
{
  HRESULT hr = S_OK;

  //Setting the material and matrix here is not the most efficient way to do it (redundant calls
  // while stepping down hierarchy) but gets around some issues with the .x file hierarchy 
  // we build right now. It's at least functional, and will be improved in a later version of 
  // the appwizard

  // Set the world matrix to the local matrix
  SetLocalToWorldMx(rWorldMat, false); //mLocalToWorldMx =  drgMxWorld * mLocalToParentMx ;

  int render_passes = 0;

  int num_shader_passes = 1;
  int current_shader_pass = 1;
  if ((mShaderId != 0) && (mpShaderMgr != NULL))
    num_shader_passes = mpShaderMgr->GetNumPasses(mShaderId);

  do
  {
    //we don't do a apply state block if there is no shader at all at the first level
    if ((mShaderId != 0) && (mpShaderMgr != NULL))
    {
      mpShaderMgr->ActivateShader(mpWrapper, mShaderId, current_shader_pass);
    }

    if (mNumVertices && mpWrapper && mpWrapper->mpDevice && mVisible)
    {
      mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &mLocalToWorldMx.mMx);

      mpWrapper->mpDevice->SetStreamSource(0, mpIndexedVertexBuffer, mVertexSize);

      mpWrapper->mpDevice->SetVertexShader(mFVF);
      mpWrapper->mpDevice->SetIndices( mpIndexBuffer, 0L);

      //placeholder
      int numprims = mNumIndices;
      switch (mPrimType) {
      case D3DPT_TRIANGLELIST:
        numprims/=3;
        break;
      case D3DPT_LINELIST:
        numprims/=2;
        break;
      }

      hr = mpWrapper->mpDevice->DrawIndexedPrimitive(mPrimType,
                                                     0,
                                                     mNumVertices,//mNumIndices,
                                                     0,
                                                     numprims);

      render_passes++;
    }

    //if the object has children, and we want to render them, now is the time
    if (renderChildren)
    {
      IawObject* temp_object = mpFirstChild;
      while (temp_object != NULL)
      {
        hr |= temp_object->Render(mLocalToWorldMx, true);

        // if the child we just rendered had a shader, and the next child does not,
        // then we have to reapply the parent's shader (if the parent had one)

        if ((temp_object->mShaderId != 0) &&
            (temp_object->mpNext) &&
            (temp_object->mpNext->mShaderId == 0) &&
            (mShaderId != 0) && 
            (mpShaderMgr != NULL))
        {
          mpShaderMgr->ActivateShader(mpWrapper, mShaderId, current_shader_pass);
        }

        temp_object = temp_object->mpNext;
      }
    }

    //restore world matrix
    mpWrapper->mpDevice->SetTransform(D3DTS_WORLD, &rWorldMat.mMx);

    current_shader_pass++;
  }
  //while (pTempShader != NULL);
  while (current_shader_pass <= num_shader_passes);

  if ((mShaderId != 0) && (mpShaderMgr != NULL))
  {
    mpShaderMgr->DeActivateShader(mpWrapper, mShaderId, 1);
  }

  return hr;
}

HRESULT IawObject::Render(IawMatrix& rWorldMat, IawObject* pCamera, bool renderChildren)
{
  if (NULL == pCamera)
    return E_FAIL;

  IawObject* temp_object = pCamera;
  IawMatrix mxInvCamLocaltoParent(true);
  IawMatrix mxCamtoRoot(true);

  while ((temp_object->GetParent() != NULL) && (temp_object->GetParent() != this))
  {
    mxInvCamLocaltoParent = temp_object->GetLocalToParentMx();
    mxInvCamLocaltoParent.Invert();
    mxCamtoRoot = mxCamtoRoot * mxInvCamLocaltoParent;
    temp_object = temp_object->GetParent();
  }

  if (temp_object->GetParent() == NULL)
  {
    //this means we didn't get to the root. However, that's not going to cut it. 
    // we need to go from here (the root) down through the tree to find where the camera is.
    return E_FAIL;
  }

  mxInvCamLocaltoParent = temp_object->GetLocalToParentMx();
  mxInvCamLocaltoParent.Invert();

  bool temp = pCamera->GetVisible();

  pCamera->SetVisible(false);

  return Render((rWorldMat * (mxCamtoRoot * mxInvCamLocaltoParent)), renderChildren);

  pCamera->SetVisible(temp);
}



HRESULT IawObject::TargetChanging(IawD3dWrapper* pWrapper,
                                  bool updateChildren)
{
  mpWrapper = pWrapper;
  //mpShaderMgr = pShaderMgr;

  SAFE_RELEASE(mpIndexedVertexBuffer);
  SAFE_RELEASE(mpIndexBuffer);

  //recursively refresh children
  if (updateChildren)
  {
    IawObject* temp_object = mpFirstChild;
    while (temp_object != NULL)
    {
      temp_object->TargetChanging(pWrapper, updateChildren);
      temp_object = temp_object->mpNext;
    }
  }

  return S_OK;
}


// (Re)set device dependent info for each object in hierarchy
HRESULT IawObject::TargetChanged(IawD3dWrapper* pWrapper,
                                 IawShaderMgr* pShaderMgr,
                                 bool updateChildren)
{
  mpWrapper = pWrapper;
  mpShaderMgr = pShaderMgr;

  // setup VB's
  if (mNumVertices)
  {
    //if the object is dependant on the size of the window (e.g. TLVerts) then we should call setup verts again
    if (mFVF & D3DFVF_XYZRHW)
    {
      if (SetupVertices(GetVerticesPtr()) != S_OK)
      {
        OutputDebugString("unable to set up verts");
        return E_FAIL;
      }
    }

    if (SetupIndexedVertexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up VB");
      return E_FAIL;
    }
    if (SetupIndexBuffer() != S_OK)
    {
      OutputDebugString("unable to set up VB");
      return E_FAIL;
    }
  }

  //recursively refresh children
  if (updateChildren)
  {
    IawObject* temp_object = mpFirstChild;
    while (temp_object != NULL)
    {
      temp_object->TargetChanged(pWrapper, pShaderMgr, updateChildren);
      temp_object = temp_object->mpNext;
    }
  }

  return S_OK;
}


// Add object to scene hierarchy
HRESULT IawObject::AddChild(IawObject* pChildObject)
{
  //need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this, pChildObject) != S_OK)
  {
    OutputDebugString("failed to add as child. Likely due to circular link");
    return E_FAIL;
  }

  if (NULL == mpFirstChild)
  {
    mpFirstChild = pChildObject;
  }
  else
  {
    IawObject* temp_object = mpFirstChild;
    //skip to one before end of list
    while (temp_object->mpNext != NULL)
    {
      temp_object = temp_object->mpNext;
    }
    //now add child
    temp_object->mpNext = pChildObject;
  }

  //now point the child's parent back here
  pChildObject->SetParent(this);

  return S_OK;
}


HRESULT IawObject::CheckAgainstChildren(IawObject* pPotentialParent,
                                        IawObject* pPotentialChild)
{
  IawObject* temp_object = pPotentialChild;

  //skip to end of list, comparing against parent object
  while (temp_object != NULL)
  {
    if (pPotentialChild == pPotentialParent)
      return E_FAIL;

    temp_object = temp_object->mpNext;
  }

  //if we got here, none of the siblings was the potential parent
  if (NULL == pPotentialChild->mpFirstChild)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent,
                                pPotentialChild->mpFirstChild);
}

HRESULT IawObject::RemoveChild(IawObject* pChildObject /*, bool bRemoveChildren */)
{
  if (pChildObject && mpFirstChild)
  {
    IawObject* pTempObj = mpFirstChild;

    if (mpFirstChild == pChildObject)
    {
      mpFirstChild = mpFirstChild->GetNext();
      return S_OK;
    }
    else
    {
      if (mpFirstChild)
        mpFirstChild->RemoveChild(pChildObject);
      if (mpNext)
        mpNext->RemoveChild(pChildObject);
    }
  }
  return E_FAIL;
}


HRESULT IawObject::SetShaderID(int shaderId, bool setChildShaderIds)
{
  if (setChildShaderIds)
  {
    //old
/*
    if (mpFirstChild)
    mpFirstChild->SetShaderID(shaderId);

      if (mpNext)
      mpNext->SetShaderID(shaderId);
*/
    // new
    IawObject* temp = mpFirstChild;
    while (temp)
    {
      temp->SetShaderID(shaderId);
      temp = temp->GetNext();
    }
  }

  mShaderId = shaderId;

  return S_OK;
}


void IawObject::SetLocalToWorldMx(IawMatrix& rWorldMat, bool renderChildren)
{
  if (mFVF & D3DFVF_XYZ)
    mLocalToWorldMx = rWorldMat * mLocalToParentMx;

  //if an object has children, concatonate Matrices
  if (renderChildren)
  {
    IawObject* temp_object = mpFirstChild;
    while (temp_object != NULL)
    {
      temp_object->SetLocalToWorldMx(mLocalToWorldMx, true);
      temp_object = temp_object->GetNext();
    }
  }
}


// Write floating point values to memory describing vertex information.
//
// NOTE: Careful with this one.  The pointer passed in moves within the memory
//       block pointed to, so your calling function does not have to concern
//       itself with such minor details.
void IawObject::SetVertex(float*& prVertexLoc, IAW_SUPERVERTEX SV)
{
  // Position
  if (mFVF & D3DFVF_XYZ)
  {
    *prVertexLoc++ = SV.mX;
    *prVertexLoc++ = SV.mY;
    *prVertexLoc++ = SV.mZ;
  }
  else if (mFVF & D3DFVF_XYZRHW)  // RHW (transformed only)
  {
    *prVertexLoc++ = SV.mX;
    *prVertexLoc++ = SV.mY;
    *prVertexLoc++ = SV.mZ;
    *prVertexLoc++ = SV.mRhw;
  }

  // Vertex blending weight values (up to 5 floats)
  if ((mFVF & D3DFVF_XYZB1) == D3DFVF_XYZB1)
    *prVertexLoc++ = SV.mB1;
  else if ((mFVF & D3DFVF_XYZB2) == D3DFVF_XYZB2)
  {
    *prVertexLoc++ = SV.mB1;
    *prVertexLoc++ = SV.mB2;
  }
  else if ((mFVF & D3DFVF_XYZB3) == D3DFVF_XYZB3)
  {
    *prVertexLoc++ = SV.mB1;
    *prVertexLoc++ = SV.mB2;
    *prVertexLoc++ = SV.mB3;
  }
  else if ((mFVF & D3DFVF_XYZB4) == D3DFVF_XYZB4)
  {
    *prVertexLoc++ = SV.mB1;
    *prVertexLoc++ = SV.mB2;
    *prVertexLoc++ = SV.mB3;
    *prVertexLoc++ = SV.mB4;
  }
  else if ((mFVF & D3DFVF_XYZB5) == D3DFVF_XYZB5)
  {
    *prVertexLoc++ = SV.mB1;
    *prVertexLoc++ = SV.mB2;
    *prVertexLoc++ = SV.mB3;
    *prVertexLoc++ = SV.mB4;
    *prVertexLoc++ = SV.mB5;
  }

  // Normal
  if (mFVF & D3DFVF_NORMAL)
  {
    *prVertexLoc++ = SV.mNx;
    *prVertexLoc++ = SV.mNy;
    *prVertexLoc++ = SV.mNz;
  }

  // Reserved
  //  if (mFVF & D3DFVF_RESERVED1)
  //    *prVertexLoc++ = 0x0;

  // Diffuse color
  if (mFVF & D3DFVF_DIFFUSE)
    *prVertexLoc++ = DWORDtoFLOAT(SV.mDiffuse);

  // Specular color
  if (mFVF & D3DFVF_SPECULAR)
    *prVertexLoc++ = DWORDtoFLOAT(SV.mSpecular);

  // Texture coordinate sets
  int iNumTextureCoordSets = 0;

  if (mFVF & D3DFVF_TEX1)
    iNumTextureCoordSets = 1;
  else if (mFVF & D3DFVF_TEX2)
    iNumTextureCoordSets = 2;
  else if (mFVF & D3DFVF_TEX3)
    iNumTextureCoordSets = 3;
  else if (mFVF & D3DFVF_TEX4)
    iNumTextureCoordSets = 4;
  else if (mFVF & D3DFVF_TEX5)
    iNumTextureCoordSets = 5;
  else if (mFVF & D3DFVF_TEX6)
    iNumTextureCoordSets = 6;
  else if (mFVF & D3DFVF_TEX7)
    iNumTextureCoordSets = 7;
  else if (mFVF & D3DFVF_TEX8)
    iNumTextureCoordSets = 8;

  for (int i=0; i<iNumTextureCoordSets; i++)
  {
    if (mFVF & D3DFVF_TEXCOORDSIZE1(i)) // One floating point value
    {
      switch (i)
      {
      case 0:
        *prVertexLoc++ = SV.mU;
        break;
      case 1:
        *prVertexLoc++ = SV.mU1;
        break;
      case 2:
        *prVertexLoc++ = SV.mU2;
        break;
      case 3:
        *prVertexLoc++ = SV.mU3;
        break;
      case 4:
        *prVertexLoc++ = SV.mU4;
        break;
      case 5:
        *prVertexLoc++ = SV.mU5;
        break;
      case 6:
        *prVertexLoc++ = SV.mU6;
        break;
      case 7:
        *prVertexLoc++ = SV.mU7;
        break;
      }
    }
    else if (mFVF & D3DFVF_TEXCOORDSIZE3(i)) // Three floating point values
    {
      switch (i)
      {
      case 0:
        *prVertexLoc++ = SV.mU;
        *prVertexLoc++ = SV.mV;
        *prVertexLoc++ = SV.mS;
        break;
      case 1:
        *prVertexLoc++ = SV.mU1;
        *prVertexLoc++ = SV.mV1;
        *prVertexLoc++ = SV.mS1;
        break;
      case 2:
        *prVertexLoc++ = SV.mU2;
        *prVertexLoc++ = SV.mV2;
        *prVertexLoc++ = SV.mS2;
        break;
      case 3:
        *prVertexLoc++ = SV.mU3;
        *prVertexLoc++ = SV.mV3;
        *prVertexLoc++ = SV.mS3;
        break;
      case 4:
        *prVertexLoc++ = SV.mU4;
        *prVertexLoc++ = SV.mV4;
        *prVertexLoc++ = SV.mS4;
        break;
      case 5:
        *prVertexLoc++ = SV.mU5;
        *prVertexLoc++ = SV.mV5;
        *prVertexLoc++ = SV.mS5;
        break;
      case 6:
        *prVertexLoc++ = SV.mU6;
        *prVertexLoc++ = SV.mV6;
        *prVertexLoc++ = SV.mS6;
        break;
      case 7:
        *prVertexLoc++ = SV.mU7;
        *prVertexLoc++ = SV.mV7;
        *prVertexLoc++ = SV.mS7;
        break;
      }
    }
    else if (mFVF & D3DFVF_TEXCOORDSIZE4(i)) // Four floating point values
    {
      switch (i)
      {
      case 0:
        *prVertexLoc++ = SV.mU;
        *prVertexLoc++ = SV.mV;
        *prVertexLoc++ = SV.mS;
        *prVertexLoc++ = SV.mT;
        break;
      case 1:
        *prVertexLoc++ = SV.mU1;
        *prVertexLoc++ = SV.mV1;
        *prVertexLoc++ = SV.mS1;
        *prVertexLoc++ = SV.mT1;
        break;
      case 2:
        *prVertexLoc++ = SV.mU2;
        *prVertexLoc++ = SV.mV2;
        *prVertexLoc++ = SV.mS2;
        *prVertexLoc++ = SV.mT2;
        break;
      case 3:
        *prVertexLoc++ = SV.mU3;
        *prVertexLoc++ = SV.mV3;
        *prVertexLoc++ = SV.mS3;
        *prVertexLoc++ = SV.mT3;
        break;
      case 4:
        *prVertexLoc++ = SV.mU4;
        *prVertexLoc++ = SV.mV4;
        *prVertexLoc++ = SV.mS4;
        *prVertexLoc++ = SV.mT4;
        break;
      case 5:
        *prVertexLoc++ = SV.mU5;
        *prVertexLoc++ = SV.mV5;
        *prVertexLoc++ = SV.mS5;
        *prVertexLoc++ = SV.mT5;
        break;
      case 6:
        *prVertexLoc++ = SV.mU6;
        *prVertexLoc++ = SV.mV6;
        *prVertexLoc++ = SV.mS6;
        *prVertexLoc++ = SV.mT6;
        break;
      case 7:
        *prVertexLoc++ = SV.mU7;
        *prVertexLoc++ = SV.mV7;
        *prVertexLoc++ = SV.mS7;
        *prVertexLoc++ = SV.mT7;
        break;
      }
    }
    else                    // Two floating point values
    {
      switch (i)
      {
      case 0:
        *prVertexLoc++ = SV.mU;
        *prVertexLoc++ = SV.mV;
        break;
      case 1:
        *prVertexLoc++ = SV.mU1;
        *prVertexLoc++ = SV.mV1;
        break;
      case 2:
        *prVertexLoc++ = SV.mU2;
        *prVertexLoc++ = SV.mV2;
        break;
      case 3:
        *prVertexLoc++ = SV.mU3;
        *prVertexLoc++ = SV.mV3;
        break;
      case 4:
        *prVertexLoc++ = SV.mU4;
        *prVertexLoc++ = SV.mV4;
        break;
      case 5:
        *prVertexLoc++ = SV.mU5;
        *prVertexLoc++ = SV.mV5;
        break;
      case 6:
        *prVertexLoc++ = SV.mU6;
        *prVertexLoc++ = SV.mV6;
        break;
      case 7:
        *prVertexLoc++ = SV.mU7;
        *prVertexLoc++ = SV.mV7;
        break;
      }
    }
  }
}


//-----------------------------------------------------------------------------
// The following function and the ones after it are for loading .x files
//  note that we currently don't use the materials from the X file. This will
//  follow in a future version of the appwizard
//-----------------------------------------------------------------------------
HRESULT IawObject::LoadObject(char* szSource, IawObject* pRoot, DWORD dwFlags)
{
  HRESULT hr = S_OK;

  LPDIRECTXFILE           pDXFile;
  LPDIRECTXFILEENUMOBJECT pEnumObj;
  LPDIRECTXFILEDATA       pFileData;
  const GUID*             pGUID;

  pDXFile = NULL;

  // Create the file object, and register the D3DRM templates for .X files
  //KP remmed out to get DX8 stuff working
  /*
  if (FAILED(DirectXFileCreate(&pDXFile)))
  return E_FAIL;
    */
  //KP added this too....
  if (pDXFile == NULL)
    return E_FAIL;

  if (FAILED(pDXFile->RegisterTemplates((VOID*)D3DRM_XTEMPLATES,
    D3DRM_XTEMPLATE_BYTES)))
  {
    pDXFile->Release();
    return E_FAIL;
  }

  //
  // Open the source file or url
  //
  if (dwFlags & OBJECT_LOAD_FILE)
  {
    // Create an enumerator object, to enumerate through the .X file objects.
    // This will open the file in the current directory.
    hr = pDXFile->CreateEnumObject(szSource, DXFILELOAD_FROMFILE, &pEnumObj);
  }
  else if (dwFlags & OBJECT_LOAD_URL)
  {
    // Create an enumerator object from an URL
    hr = pDXFile->CreateEnumObject(szSource, DXFILELOAD_FROMURL, &pEnumObj);
  }
  else
    hr = E_FAIL;
  if (FAILED(hr))
  {
    pDXFile->Release();
    return hr;
  }

  // Cycle through each object. Parse meshes and frames as appropriate
  while (SUCCEEDED(hr = pEnumObj->GetNextDataObject(&pFileData)))
  {
    pFileData->GetType(&pGUID);

    if (*pGUID == TID_D3DRMFrame)
      ParseFrame(pFileData, pRoot);

    if (*pGUID == TID_D3DRMMesh)
      ParseMesh(pFileData, pRoot);

    pFileData->Release();
  }

  // Success will result in hr == DXFILEERR_NOMOREOBJECTS
  if (DXFILEERR_NOMOREOBJECTS == hr) 
    hr = S_OK;

  pEnumObj->Release();
  pDXFile->Release();

  return hr;
}

HRESULT IawObject::ParseFrame(LPDIRECTXFILEDATA pFileData, IawObject* pParentFrame)
{
  DWORD dwNameLen=80;
  TCHAR strName[80];
  if (FAILED(pFileData->GetName(strName, &dwNameLen)))
    return E_FAIL;

  IawObject* pFrame;

  if (pParentFrame == NULL)
    pFrame = this;
  else
    pFrame = new IawObject();

  // Enumerate child objects.
  LPDIRECTXFILEOBJECT pChildObj;
  while (SUCCEEDED(pFileData->GetNextObject(&pChildObj)))
  {
    LPDIRECTXFILEDATA pChildData;
    if (SUCCEEDED(pChildObj->QueryInterface(IID_IDirectXFileData,
      (VOID**)&pChildData)))
    {
      const GUID* pGUID;
      pChildData->GetType(&pGUID);

      if (TID_D3DRMFrame == *pGUID)
        ParseFrame(pChildData, pFrame);

      if (TID_D3DRMMesh == *pGUID)
        ParseMesh(pChildData, pFrame);

      if (TID_D3DRMFrameTransformMatrix == *pGUID)
      {
        DWORD dwSize;
        VOID* pData;
        if (FAILED(pChildData->GetData(NULL, &dwSize, &pData)))
        {
          delete pFrame;
          return E_FAIL;
        }

        if (dwSize == sizeof(D3DMATRIX))
        {
          // Convert from a left- to a right-handed cordinate system
          D3DMATRIX* pmatFrame = (D3DMATRIX*)pData;
          pmatFrame->_13 *= -1.0f;
          pmatFrame->_31 *= -1.0f;
          pmatFrame->_23 *= -1.0f;
          pmatFrame->_32 *= -1.0f;
          pmatFrame->_43 *= -1.0f;
          pFrame->mLocalBaseMx = *pmatFrame;
        }
      }

      pChildData->Release();
    }

    pChildObj->Release();
  }

  if (pFrame != this)
  {
    pParentFrame->AddChild(pFrame);
    //  pFrame->m_pMaterial = pParentFrame->m_pMaterial;
  }

  return S_OK;
}


//-----------------------------------------------------------------------------
// Name: GetFace
// Desc: Get the nth face
//-----------------------------------------------------------------------------
DWORD* GetFace(DWORD* pFaceData, DWORD dwFace)
{
    for (DWORD i=0; i<dwFace; i++)
        pFaceData += (*pFaceData) + 1;

    return pFaceData;
}


//-----------------------------------------------------------------------------
// Name: gGetNumpIndices
// Desc: (global) Get number of indices from face data
//-----------------------------------------------------------------------------
DWORD gGetNumpIndices(DWORD* pFaceData, DWORD dwNumFaces)
{
    DWORD dwNumpIndices = 0;
    while (dwNumFaces-- > 0)
    {
        dwNumpIndices += (*pFaceData-2)*3;
        pFaceData += *pFaceData + 1;
    }

    return dwNumpIndices;
}

HRESULT IawObject::ParseMesh(LPDIRECTXFILEDATA pFileData, IawObject* pParentFrame)
{
  DWORD dwNameLen=80;
  TCHAR strName[80];
  IawObject* pMesh;

  if (FAILED(pFileData->GetName(strName, &dwNameLen)))
    return E_FAIL;

  // Read the Mesh data from the file
  LONG  pData;
  DWORD dwSize;
  if (FAILED(pFileData->GetData(NULL, &dwSize, (VOID**)&pData)))
    return E_FAIL;

  DWORD      dwNumVertices =    *((DWORD*)pData); pData += 4;
  D3DVECTOR* pVertices     = ((D3DVECTOR*)pData); pData += 12*dwNumVertices;
  DWORD      dwNumFaces    =    *((DWORD*)pData); pData += 4;
  DWORD*     pFaceData     =      (DWORD*)pData;

  // Create the Vertex Data
  //
  //  D3DVERTEX *pVerts     = new D3DVERTEX[dwNumVertices];
  int   ipVertsSize    = dwNumVertices * (mVertexSize / 4);
  float* pVerts      = new float[ipVertsSize];

  if (NULL == pVertices)
    return E_FAIL;

  ZeroMemory(pVerts, (dwNumVertices * mVertexSize)); //dwNumVertices * sizeof(D3DVERTEX));
  //  for (DWORD i=0; i<ipVertsSize /*dwNumVertices*/; i++)
  //  
  int i = 0;
  DWORD j = 0;
  while ((i < ipVertsSize) && (j < dwNumVertices))
  {
    pVerts[i++] = pVertices[j].x;
    pVerts[i++] = pVertices[j].y;
    pVerts[i++] = pVertices[j].z;
    i+=5;             // Move forward in float array to the start of the next vertex
    j++;
  }
  // Count the number of indices (converting n-sided faces to triangles)
  DWORD dwNumpIndices = gGetNumpIndices(pFaceData, dwNumFaces);
  // Allocate memory for the indices
  WORD *pIndices = new WORD[dwNumpIndices];
  if (NULL == pIndices)
    return E_FAIL;

  //
  // Assign the indices (build a triangle fan for high-order polygons)
  //
  DWORD *pFaceData2 = pFaceData;
  WORD *pIndices2 = pIndices;
  for (j=0; j<dwNumFaces; j++)
  {
    DWORD dwNumVerticesPerFace = *pFaceData2++;

    for (DWORD i=2; i<dwNumVerticesPerFace; i++)
    {
      *pIndices2++ = (WORD)pFaceData2[0];
      *pIndices2++ = (WORD)pFaceData2[i-1];
      *pIndices2++ = (WORD)pFaceData2[i];
    }

    pFaceData2 += dwNumVerticesPerFace;
  }


  BOOL bHasNormals = FALSE;
  BOOL bHasMaterials = FALSE;

  // Enumerate child objects.
  LPDIRECTXFILEOBJECT pChildObj;
  while (SUCCEEDED(pFileData->GetNextObject(&pChildObj)))
  {
    LPDIRECTXFILEDATA pChildData;

    if (SUCCEEDED(pChildObj->QueryInterface(IID_IDirectXFileData,
      (VOID**)&pChildData)))
    {
      const GUID* pGUID;
      LONG        pData;
      DWORD       dwSize;

      pChildData->GetType(&pGUID);
      if (FAILED(pChildData->GetData(NULL, &dwSize, (VOID**)&pData)))
      {
        return NULL;
      }

      if (TID_D3DRMMeshMaterialList == *pGUID)
      {
        DWORD  dwNumMaterials = *((DWORD*)pData);   pData += 4;
        DWORD  dwNumMatFaces  = *((DWORD*)pData);   pData += 4;
        DWORD* pMatFace       =   (DWORD*)pData;

        if (dwNumMaterials == 1 || dwNumMatFaces != dwNumFaces)
        {
          pMesh = new IawObject();

          pMesh->Init(mpWrapper, pVerts, pIndices, dwNumVertices, dwNumpIndices, 0);
/*
            IawVector(0), IawVector(1), IawVector(0,1,0), 0, 
            IawVector(0), IawVector(0), 0 );
*/
          // Only one material add all faces at once
          if (pParentFrame == NULL)
          {
            AddChild(pMesh);
            //  pMesh->m_pMaterial = m_pMaterial;
          }
          else
          {
            pParentFrame->AddChild(pMesh);
            //  pMesh->m_pMaterial = pParentFrame->m_pMaterial;
          }
        }
        else
        {
          DWORD dwIndices = 0, dwFaceIdx = 0;
          // Multiple materials, add in sorted order
          for (DWORD mat=0; mat<dwNumMaterials; mat++)
          {
            for (DWORD face=0; face<dwNumMatFaces; face++)
            {
              if (pMatFace[face] != mat)
              {
                if (dwIndices)
                {
                  pMesh = new IawObject();

                  pMesh->Init(mpWrapper,
                              pVerts,
                              &pIndices[dwFaceIdx],
                              dwNumVertices,
                              dwIndices,
                              0);
/*
                    IawVector(0), IawVector(1), IawVector(0,1,0), 0, 
                    IawVector(0),  IawVector(0), 0);
*/
                  // Only one material add all faces at once
                  if (pParentFrame == NULL)
                  {
                    AddChild(pMesh);
                    //    pMesh->m_pMaterial = m_pMaterial;
                  }
                  else
                  {
                    pParentFrame->AddChild(pMesh);
                    //  pMesh->m_pMaterial = pParentFrame->m_pMaterial;
                  }
                }
                dwIndices = 0;
                dwFaceIdx = face;
              }
              else
                dwIndices++;
            }
            if (dwIndices)
            {
              pMesh = new IawObject();

              pMesh->Init(mpWrapper,
                          pVerts,
                          &pIndices[dwFaceIdx],
                          dwNumVertices,
                          dwIndices,
                          0);
/*
                IawVector(0), IawVector(1), IawVector(0,1,0), 0, IawVector(0), 
                IawVector(0), 0);
*/
              // Only one material add all faces at once
              if (pParentFrame == NULL)
              {
                AddChild(pMesh);
                //  pMesh->m_pMaterial = m_pMaterial;
              }
              else
              {
                pParentFrame->AddChild(pMesh);
                //  pMesh->m_pMaterial = pParentFrame->m_pMaterial;
              }

            }
          }
        }

        //ParseMeshMaterialList(pChildData, pMesh);
        bHasMaterials = TRUE;
      }

      if (TID_D3DRMMeshNormals == *pGUID)
      {
        DWORD      dwNumNormals = *((DWORD*)pData);
        D3DVECTOR* pNormals     = (D3DVECTOR*)(pData+4);

        if (dwNumNormals == dwNumVertices)
        {
          i = 3;
          j = 0;
          //for (DWORD i=0; i<ipVertsSize /*dwNumVertices*/; i++)
          //
          while ((i < ipVertsSize) && (j < dwNumVertices))
          {
            pMesh->mpVertices[i++] = pNormals[j].x;
            pMesh->mpVertices[i++] = pNormals[j].y;
            pMesh->mpVertices[i++] = pNormals[j].z;
            i+=5;
            j++;
          }
          bHasNormals = TRUE;
        }
      }

      if (TID_D3DRMMeshTextureCoords == *pGUID)
      {
        // Copy the texture coords into the mesh's vertices
        DWORD  dwNumTexCoords = *((DWORD*)pData);
        FLOAT* pTexCoords     = (FLOAT*)(pData+4);

        if (dwNumTexCoords == dwNumVertices)
        {
          i = 6;
          j = 0;
          //          for (DWORD i=0; i<dwNumVertices; i++)
          //
          while ((i < ipVertsSize) && (j < dwNumVertices))
          {
            pMesh->mpVertices[i++] = pTexCoords[j*2];
            pMesh->mpVertices[i++] = pTexCoords[j*2+1];
            i+=6;
            j++;
          }
        }
      }

      pChildData->Release();
    }

    pChildObj->Release();
  }

  if (!bHasMaterials)
  {
    pMesh = new IawObject();

    pMesh->Init(mpWrapper,
                pVerts,
                pIndices,
                dwNumVertices,
                dwNumpIndices,
                0);
/*
      IawVector(0), IawVector(1), IawVector(0,1,0), 0, 
      IawVector(0), IawVector(0), 0);
*/
    // Only one material add all faces at once
    if (pParentFrame == NULL)
    {
      AddChild(pMesh);
      //  pMesh->m_pMaterial = m_pMaterial;
    }
    else
    {
      pParentFrame->AddChild(pMesh);
      //  pMesh->m_pMaterial = pParentFrame->m_pMaterial;
    }

  }

  // if (!bHasNormals)
  //   pMesh->ComputeNormals();

  // pParentFrame->AddChild(pMesh);
  return S_OK;
}

HRESULT IawObject::ParseMeshMaterialList(LPDIRECTXFILEDATA pFileData, IawObject* pObject)
{
/*
    LPDIRECTXFILEOBJECT        pChildObj;
    LPDIRECTXFILEDATA          pChildData;
    LPDIRECTXFILEDATAREFERENCE pChildDataRef;
    DWORD                      dwMaterial = 0;

    while (SUCCEEDED(pFileData->GetNextObject(&pChildObj)))
    {
        if (SUCCEEDED(pChildObj->QueryInterface(IID_IDirectXFileData,
                                                    (VOID**)&pChildData)))
        {
            const GUID* pguid;
            pChildData->GetType(&pguid);

            if (TID_D3DRMMaterial == *pguid)
            {
                ParseMaterial(pChildData, pObject, dwMaterial++);
            }

            pChildData->Release();
        }

        if (SUCCEEDED(pChildObj->QueryInterface(IID_IDirectXFileDataReference,
                                                    (VOID**)&pChildDataRef)))
        {
            if (SUCCEEDED(pChildDataRef->Resolve(&pChildData)))
            {
                const GUID* pguid;
                pChildData->GetType(&pguid);

                if (TID_D3DRMMaterial == *pguid)
                {
                    ParseMaterial(pChildData, pObject, dwMaterial++);
                }

                pChildData->Release();
            }
            pChildDataRef->Release();
        }

        pChildObj->Release();
    }
*/
  return S_OK;
}

HRESULT IawObject::ParseMaterial(LPDIRECTXFILEDATA pFileData, IawObject* pObject, DWORD dwMaterial)
{
    // Read data from the file
/*
    LONG  pData;
    DWORD dwSize;
    TCHAR strTexture[128];

    if (FAILED(pFileData->GetData(NULL, &dwSize, (VOID**)&pData)))
        return NULL;

    // Set the material properties for the mesh
    D3DMATERIAL7 mtrl;
    ZeroMemory(&mtrl, sizeof(mtrl));
    memcpy(&mtrl.diffuse,  (VOID*)(pData+0),  sizeof(FLOAT)*4);
    memcpy(&mtrl.ambient,  (VOID*)(pData+0),  sizeof(FLOAT)*4);
    memcpy(&mtrl.power,    (VOID*)(pData+16), sizeof(FLOAT)*1);
    memcpy(&mtrl.specular, (VOID*)(pData+20), sizeof(FLOAT)*3);
    memcpy(&mtrl.emissive, (VOID*)(pData+32), sizeof(FLOAT)*3);
    strTexture[0] = 0;

    LPDIRECTXFILEOBJECT pChildObj;
    if (SUCCEEDED(pFileData->GetNextObject(&pChildObj)))
    {
        LPDIRECTXFILEDATA pChildData;

        if (SUCCEEDED(pChildObj->QueryInterface(IID_IDirectXFileData,
                                                    (VOID**)&pChildData)))
        {
            const GUID* pguid;
            pChildData->GetType(&pguid);

            if (TID_D3DRMTextureFilename == *pguid)
            {
                TCHAR** string;

                if (FAILED(pChildData->GetData(NULL, &dwSize, (VOID**)&string)))
                    return NULL;

                D3DTextr_CreateTextureFromFile(*string);
                lstrcpyn(strTexture, *string, 128);
            }

            pChildData->Release();
        }

        pChildObj->Release();
    }

    pObject->SetMaterialData(dwMaterial, &mtrl, strTexture);
*/
  return S_OK;
}

//returns the size of a struct in bytes
void IawObject::SetVertexSize()
{
  int i;

  DWORD dwFVFflags = mFVF;

  i=0;

  // Position
  if (dwFVFflags & D3DFVF_XYZ)
    i+= 3*sizeof(D3DVALUE);
  else if (dwFVFflags & D3DFVF_XYZRHW)  // RHW (transformed only)
    i+= 4*sizeof(D3DVALUE);


  // Vertex blending weight values (up to 5 floats)
  if ((dwFVFflags & D3DFVF_XYZB1) ==  D3DFVF_XYZB1)
    i+= sizeof(D3DVALUE); 
  if ((dwFVFflags & D3DFVF_XYZB2) == D3DFVF_XYZB2)
    i+= 2*sizeof(D3DVALUE); 
  if ((dwFVFflags & D3DFVF_XYZB3) == D3DFVF_XYZB3)
    i+= 3*sizeof(D3DVALUE); 
  if ((dwFVFflags & D3DFVF_XYZB4) == D3DFVF_XYZB4)
    i+= 4*sizeof(D3DVALUE); 
  if ((dwFVFflags & D3DFVF_XYZB5) == D3DFVF_XYZB5)
    i+= 5*sizeof(D3DVALUE); 

  // Normal
  if (dwFVFflags & D3DFVF_NORMAL)
    i+= 3*sizeof(D3DVALUE); 

  // Diffuse color
  if (dwFVFflags & D3DFVF_DIFFUSE)
    i+= sizeof(D3DVALUE); 

  // Specular color
  if (dwFVFflags & D3DFVF_SPECULAR)
    i+= sizeof(D3DVALUE); 

  // Texture coordinate sets
  int iNumTextureCoordSets = 0;

  if (dwFVFflags & D3DFVF_TEX1)
    iNumTextureCoordSets = 1;
  else if (dwFVFflags & D3DFVF_TEX2)
    iNumTextureCoordSets = 2;
  else if (dwFVFflags & D3DFVF_TEX3)
    iNumTextureCoordSets = 3;
  else if (dwFVFflags & D3DFVF_TEX4)
    iNumTextureCoordSets = 4;
  else if (dwFVFflags & D3DFVF_TEX5)
    iNumTextureCoordSets = 5;
  else if (dwFVFflags & D3DFVF_TEX6)
    iNumTextureCoordSets = 6;
  else if (dwFVFflags & D3DFVF_TEX7)
    iNumTextureCoordSets = 7;
  else if (dwFVFflags & D3DFVF_TEX8)
    iNumTextureCoordSets = 8;

  for (int j=0; j<iNumTextureCoordSets; j++)
  {
    if (dwFVFflags & D3DFVF_TEXCOORDSIZE1(j)) // One floating point value
    {
      switch (j) {
      case 0:
        i+= sizeof(D3DVALUE);
        break;
      case 1:
        i+= sizeof(D3DVALUE);
        break;
      case 2:
        i+= sizeof(D3DVALUE);
        break;
      case 3:
        i+= sizeof(D3DVALUE);
        break;
      case 4:
        i+= sizeof(D3DVALUE);
        break;
      case 5:
        i+= sizeof(D3DVALUE);
        break;
      case 6:
        i+= sizeof(D3DVALUE);
        break;
      case 7:
        i+= sizeof(D3DVALUE);
        break;
      }
    }
    else if (dwFVFflags & D3DFVF_TEXCOORDSIZE3(j)) // Three floating point values
    {
      switch (j) {
      case 0:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 1:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 2:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 3:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 4:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 5:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 6:
        i+= 3*sizeof(D3DVALUE);
        break;
      case 7:
        i+= 3*sizeof(D3DVALUE);
        break;
      }
    }
    else if (dwFVFflags & D3DFVF_TEXCOORDSIZE4(j)) // Four floating point values
    {
      switch (j) {
      case 0:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 1:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 2:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 3:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 4:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 5:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 6:
        i+= 4*sizeof(D3DVALUE);
        break;
      case 7:
        i+= 4*sizeof(D3DVALUE);
        break;
      }
    }
    else                    // Two floating point values
    {
      switch (j) {
      case 0:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 1:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 2:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 3:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 4:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 5:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 6:
        i+= 2*sizeof(D3DVALUE);
        break;
      case 7:
        i+= 2*sizeof(D3DVALUE);
        break;
      } //end switch
    } //end else
  } //end for

  mVertexSize = i;

  //test only, assumes d3dvertex
  //mVertexSize =  8*sizeof(D3DVALUE);

}


