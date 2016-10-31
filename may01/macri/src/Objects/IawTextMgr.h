// IawTextMgr.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawTextMgr_h)
#define IawTextMgr_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This class manages graphical text.
 */
class IawTextMgr : public IawObject
{
public:
  /** Constructor */
  IawTextMgr(DWORD dwFVF = D3DFVF_TLVERTEX);

  /** Destructor */
  virtual ~IawTextMgr();

  /** Initialize text */
  HRESULT Init(IawD3dWrapper *pWrapper, /*CDRGShader *pShader,*/ int ShaderID);

  /**
   * Add a string in screen space.
   * This should fail if the vertex type is other than TLVert.
   */
  HRESULT AddString(char* pString,
                    int*  pStringID,
                    float x,
                    float y,
                    float hsize,
                    float vsize);

  /**
   * Add a string in world space.
   * This should fail if vertex type is TLVert.
   */
  HRESULT AddString(char* pString,
                    int* pStringID,
                    IawVector pos       = IawVector(0.0f),
                    IawVector scale     = IawVector(1.0f),
                    IawVector rotationaxis  = IawVector(0.0f,1.0f,0.0f),
                    float   rotationangle = 0.0f,
                    IawVector vel       = IawVector(0.0f),
                    IawVector scalevel    = IawVector(0.0f),
                    float   rotationalvel = 0.0f);

  //HRESULT GetStringID(char *string);
  HRESULT ChangeStringContent(int stringID, char* pString);
  //HRESULT ChangeStringPos(int stringID, float x, float y);
  //HRESULT DeleteString(int stringID);
  //HRESULT ToggleStringVisibility(int stringID);
  
    HRESULT Render(IawMatrix& rWorldMat, bool renderChildren = true);

  //ivoid SetFontTexture(IawTexture *pFontTexture) { m_pFontTexture = pFontTexture;}

private:
  DWORD mLastScreenHRes;
  DWORD mLastScreenVRes;

  //IawTexture* mpFontTexture;
}; //end of definition of StringManager class


/**
 * A string object for D3D text.
 */
class IawString : public IawObject
{
public:
  /** Constructor */
  IawString(DWORD dwFVF = D3DFVF_TLVERTEX);

  /** Destructor */
  virtual ~IawString();

  /** Initialize a string */
  HRESULT Init(IawD3dWrapper* pWrapper,
               char* pString,
               int* pStringId,
               IawVector pos = IawVector(0.0f),
               IawVector scale = IawVector(1.0f),
               IawVector rotationaxis  = IawVector(0.0f,1.0f,0.0f),
               float rotationangle = 0.0f,
               IawVector vel = IawVector(0.0f),
               IawVector scalevel = IawVector(0.0f),
               float   rotationalvel = 0.0f);

    HRESULT Init(IawD3dWrapper* pD3DWrapper,
                 char* pString,
                 int* pStringId,
                 float x,// = 10.0f,
                 float y,// = 10.0f, 
                 float hsize = 0.05f,
                 float vsize = 0.05f);

    HRESULT ChangeContent(char* pString);

    int  GetID() {return mStringId; }
	void SetID( int id ) { mStringId = id; }

    //HRESULT SetStringPosition(int iStringNum, float x,float y);
    //HRESULT SetStringColor(int iStringNum, DWORD color);
    //HRESULT SetStringSize(int iStringNum, int hsize, int vsize);
private:
  char* mpString;
  int   mStringId;
  int   mStringLen;

  //these are used if it's in screenspace
  float mXpos;      // A fraction between 0.0 and 1.0 where (0,0) is top left and
  float mYpos;      // (1,1) is bottom right
  float mHsize;
 float mVsize;

  void SetupStringContents(char* pString, float x, float y, float z, float hsize, float vsize);

  HRESULT SetupVertices(float* pVertices);
  HRESULT SetupIndices();

  //IawString* mpNext;
};

#endif // IawTextMgr_h

