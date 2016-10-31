// IawTextMgr.cpp App Wizard Version 2.0 Beta 1
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
// IawTextMgr.cpp: implementation of the CIawTextMgr class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// Constructor
IawTextMgr::IawTextMgr(DWORD dwFVF) : IawObject(dwFVF)
{
  SetObjectType(IAW_OBJECT_TYPE_TEXTMGR);
  SetFVF(dwFVF);
  
  SetObjectName("Text Manager");
  
  SetVertexSize();  // Set m_dwVertexSize based on new FVF flags
  
  mLastScreenHRes = 0;
  mLastScreenVRes = 0;
}

// Destructor
IawTextMgr::~IawTextMgr()
{
  //DestroyString();
  // Do I need to destroy all the strings here? Object hierarchy will do it?
}

HRESULT IawTextMgr::Init(IawD3dWrapper* pWrapper, int shaderId)
{
  
  IawObject::Init(pWrapper, (char *)NULL);//IawVector,(NULL); //setup the default matrices and the like
  
  SetWrapper(pWrapper);
  SetShaderID(shaderId, false);
  SetVertexSize();
  
  return S_OK;
}

// Add a string
HRESULT IawTextMgr::AddString(char *pString, int* pStringId, float x, float y, float hsize, float vsize)
{
  //this is for tlverts only!
  if (!(D3DFVF_XYZRHW & GetFVF()))
    return E_FAIL;
  
  IawString* new_string = new IawString(GetFVF());
  
  new_string->Init(GetWrapper(), pString, pStringId,  x, y, hsize, vsize);
  
  new_string->SetShaderID(GetShaderID()); //set it's shader to the managers shader
  
  // Find the object in the list immediately preceeding the last object starting with "this"
  IawObject* temp = GetFirst();
  int id = 0;
  if (temp)
  {
    while (temp->GetNext() != NULL) 
	{
		temp = temp->GetNext();
		id++;
	}
  }
  new_string->SetID( id );
  *pStringId = id;
  
  AddChild(new_string);

  
  return S_OK;
}

HRESULT IawTextMgr::AddString(char* pString,
                              int* pStringId,
                              IawVector pos,
                              IawVector scale,
                              IawVector rotationAxis,
                              float rotationAngle,
                              IawVector velocity,
                              IawVector scaleVelocity,
                              float rotationalVelocity)
{

  //this is not for TL verts
  if (D3DFVF_XYZRHW & GetFVF())
    return E_FAIL;

  IawString *new_string = new IawString(GetFVF());

  new_string->Init(GetWrapper(),
                   pString,
                   pStringId,
                   pos,
                   scale,
                   rotationAxis,
                   rotationAngle,
                   velocity,
                   scaleVelocity,
                   rotationalVelocity);

  new_string->SetShaderID(GetShaderID()); //set it's shader to the managers shader

  // Find the object in the list immediately preceeding the last object starting with "this"
  IawObject* temp = GetFirst();
  int id = 0;
  if (temp)
  {
    while (temp->GetNext() != NULL) 
	{
		temp = temp->GetNext();
		id++;
	}
  }
  new_string->SetID( id );
  *pStringId = id;

  AddChild(new_string);

  return S_OK;
}

//HRESULT GetStringID(char *pString);

HRESULT IawTextMgr::ChangeStringContent(int stringID, char *pString)
{
  IawString *temp = (IawString *)GetFirst();
  
  while ((temp != NULL) && (temp->GetID() != stringID))
    temp = (IawString *)temp->GetNext();
  
  temp->ChangeContent(pString);
  temp->TargetChanged(GetWrapper(),GetShaderMgr(),false);
  
  
  temp->SetShaderID(GetShaderID()); //set it's shader to the managers shader
  
  return S_OK;
  
}

//need a get string function
//HRESULT ChangeStringPos(int stringID, float x, float y);
//HRESULT DeleteString(int stringID);
//HRESULT ToggleStringVisibility(int stringID);

//should have an overrided addchild to reject the addition of non strings

HRESULT IawTextMgr::Render(IawMatrix& rWorldMat, bool renderChildren)
{
  IawObject* temp = GetFirst();
  
  while (temp)
  {
    temp->Render(rWorldMat, false);
    temp = temp->GetNext();
  }
  
  return S_OK;
  
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
//   Begin implementation of string class
//
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
IawString::IawString(DWORD dwFVF) : IawObject(dwFVF)
{
  SetObjectType(IAW_OBJECT_TYPE_STRING);

  mpString = NULL;
  mStringId = 0;
  mStringLen = 0;

  SetObjectName("String");

  //these are used if it's in screenspace
  mXpos = 0;
  mYpos = 0;
  mHsize = 0;
  mVsize = 0;
}

IawString::~IawString()
{
  SAFE_DELETE(mpString);
}

HRESULT IawString::Init(IawD3dWrapper *pWrapper,
                        char* pString,
                        int* pStringId,
                        IawVector pos,
                        IawVector scale,
                        IawVector rotationaxis,
                        float rotationangle,
                        IawVector vel ,
                        IawVector scalevel,
                        float   rotationalvel)
{
  SetWrapper(pWrapper);
//  IawObject::Init(NULL,pos,scale,rotationaxis,rotationangle,vel,scalevel,rotationalvel,0);
  SetupStringContents(pString, pos.mX, pos.mY, pos.mZ, scale.mX, scale.mY);
  return S_OK;
}

HRESULT IawString::Init(IawD3dWrapper *pWrapper,
                        char *pString,
                        int* stringID,
                        float x,
                        float y,
                        float hsize,
                        float vsize)
{
  SetWrapper(pWrapper);

  IawObject::Init(pWrapper, (float*)NULL);

  SetupStringContents(pString, x, y, 0.05f, hsize, vsize);

  return S_OK;
}

HRESULT IawString::ChangeContent(char* pString)
{
  SetupStringContents(pString, mXpos, mYpos, 0.05f, mHsize, mVsize);
  return S_OK;
}

void IawString::SetupStringContents(char *pString, float x, float y, float z, float hsize, float vsize)
{
  mStringLen = lstrlen(pString);

  SAFE_DELETE(mpString);

  mpString = new char[mStringLen+1];
  strcpy(mpString, pString);

  mXpos   = x;
  mYpos   = y;
  mHsize  = hsize;
  mVsize  = vsize;

  //mLastScreenHRes = GetWrapper()->m_dwRenderWidth;
  //mLastScreenVRes = GetWrapper()->m_dwRenderHeight;

  SetNumVerts(mStringLen * 4);
  SetNumIndices(mStringLen * 6);

  AllocVertMem(GetNumVerts() * (GetVertexSize() / 4));
  AllocIndicesMem(GetNumIndices());

  SetupVertices(GetVerticesPtr());
  SetupIndices();

/*
  char str[64];
  OutputDebugString("\n");

  sprintf(str, "stringlen: %d \n", mStringLen);
  OutputDebugString(str);
  sprintf(str, "numverts: %d \n", GetNumVerts());
  OutputDebugString(str);
  sprintf(str, "numindices: %d \n", GetNumIndices());
  OutputDebugString(str);
  OutputDebugString("\n");
*/
}

HRESULT IawString::SetupVertices(float *pVertices)
{
  float x_start;
  float y_start;
  float char_width;
  float char_height;

  if (D3DFVF_XYZRHW & GetFVF())
  {
    x_start   = mXpos * (float)GetWrapper()->GetWidth();
    y_start   = mYpos * (float)GetWrapper()->GetHeight();;
    char_width  = mHsize * (float)GetWrapper()->GetWidth();
    char_height = mVsize * (float)GetWrapper()->GetHeight();
  }
  else
  {

    x_start   = mXpos;
    y_start   = mYpos;
    char_width  = mHsize;
    char_height = mVsize;
  }

  float char_offset = 1.0f/IawTexture::FONT_TEXT_CHARS_PER_ROW; //the fraction between 0&1 per char

  float u_start   = 0.0f;
  float v_start   = 0.0f;

  //AllocVertMem(GetVertexSize() * GetNumVerts());
  //AllocIndicesMem(GetNumIndices());

  IAW_SUPERVERTEX curr_char[4];   // Each character in string contains 4 vertices

  for (int i = 0; i < mStringLen; i++) 
  {
    // Set (x,y) coordinates
    //NEED TO CHECK SCREEN EXTENTS!!! Assumes D3D will clip!

    // Top left
    curr_char[0].mX = (float)x_start + (i * char_width);
    curr_char[0].mY = (float)y_start;

    // Top right
    curr_char[1].mX = (float)x_start + (i * char_width) + char_width;
    curr_char[1].mY = (float)y_start;

    // Bottom left
    curr_char[2].mX = (float)x_start + (i * char_width);
    curr_char[2].mY = (float)y_start + char_height;

    // Bottom right
    curr_char[3].mX = (float)x_start + (i * char_width) + char_width;
    curr_char[3].mY = (float)y_start + char_height;


    for (int j = 0; j < 4; j++)
    {
      //note, the Z value may need to change if you operate out of this range
      curr_char[j].mZ     = 0.01f;
      curr_char[j].mRhw   = 1.0;
      curr_char[j].mSpecular  = 0;
    }

    curr_char[0].mDiffuse = IawTexture::TOP_COLOR;
    curr_char[1].mDiffuse = IawTexture::TOP_COLOR;
    curr_char[2].mDiffuse = IawTexture::BOTTOM_COLOR;
    curr_char[3].mDiffuse = IawTexture::BOTTOM_COLOR;

    //set the UV's into the font texture.
    u_start = (float)((int)mpString[i] % IawTexture::FONT_TEXT_CHARS_PER_SIDE) * char_offset;
    v_start = (float)((int)mpString[i] / IawTexture::FONT_TEXT_CHARS_PER_SIDE) * char_offset;

    // Top left
    curr_char[0].mU = u_start;
    curr_char[0].mV = 1.0f - (v_start);

    // Top right
    curr_char[1].mU = u_start + char_offset;
    curr_char[1].mV = 1.0f - (v_start);

    // Bottom left
    curr_char[2].mU = u_start;
    curr_char[2].mV = 1.0f - (v_start + char_offset);

    // Bottom right
    curr_char[3].mU = u_start + char_offset;
    curr_char[3].mV = 1.0f - (v_start + char_offset);


    for (j = 0; j < 4; j++)
      SetVertex(pVertices, curr_char[j]);
  }

  return S_OK;
}

// This is needed for the buggy MSVC compiler.  With optimizations on, the compiler
// spits out some bogus code -- Intel compiler doesn't have this problem
HRESULT IawString::SetupIndices()
{
  WORD *indices_array = new WORD[GetNumIndices()];
  int j;

  for (int i=0;i<GetNumIndices()/6;i++)
  {
    j = i*6;

    indices_array[j+0] = i*4+0;
    indices_array[j+1] = i*4+1; 
    indices_array[j+2] = i*4+2;
    indices_array[j+3] = i*4+1;
    indices_array[j+4] = i*4+3;
    indices_array[j+5] = i*4+2;
  }

  SetIndexArray(indices_array);

  delete [] indices_array;

  return S_OK;
}


/*
void IawTextMgr::ModifyStringContent(int stringID, char *pString)
{
	IawTextMgr *pTemp = this;
	if (pTemp->mStringId != stringID) pTemp = (IawTextMgr*)pTemp->GetFirst();
	while (pTemp != NULL && pTemp->mStringId != stringID)
		pTemp = (IawTextMgr*)pTemp->GetNext();

	if (pTemp)
	{
		pTemp->DestroyString();
		pTemp->CreateString(string, pTemp->mXpos, pTemp->mYpos, pTemp->mHsize, pTemp->mVsize, pTemp->m_dwColor);
	}
}

void IawTextMgr::DestroyString()
{
	if (mpString)
		delete [] mpString;

	mpString			= NULL;
	mStringLen		= 0;

	SetNumVerts(0);
	DeleteVertMem();

	mLastScreenHRes	= 0;
	mLastScreenVRes	= 0;
}

void IawTextMgr::ToggleStringOnOff()
{
	SetVisible(!GetVisible());
}
*/








/*

// -----------------------------------------------------------------------
// HRESULT IAWObject::Render(IawMatrix& iawMxMatrix, bool bRenderChilren = true)
//
//  This draws the object to the screen using DrawIndexedPrimitiveVB. 
// -----------------------------------------------------------------------
HRESULT IawTextMgr::Render(IawMatrix& iawMxWorld, bool bRenderChildren)
{

	HRESULT hr = S_OK;

	//Setting the material and matrix here is not the most efficient way to do it (redundant calls
	// while stepping down hierarchy) but gets around some issues with the .x file hierarchy 
	// we build right now. It's at least functional, and will be improved in a later version of 
	// the appwizard

	// Set the world matrix to the local matrix
	SetLocalToWorldMx(iawMxWorld, false);	//mLocalToWorldMx =  iawMxWorld * mLocalToParentMx ;

	int	renderpasses = 0;

	CDRGShader	*pTempShader = m_pShader;

	do
	{		
		//we don't do a apply state block if there is no shader at all at the first level
		if (m_pShader != NULL) 
		{
			m_pWrapper->m_pDevice->ApplyStateBlock(pTempShader->m_dwStateBlock);
			//You can use these for debugging			
			//OutputDebugString(pTempShader->m_strShaderName);
			//OutputDebugString(" - shader activated\n");
		}

		if (m_iNumVertices && m_pWrapper && m_pWrapper->m_pDevice && m_bVisible)
		{
			m_pWrapper->m_pDevice->SetTransform(D3DTS_WORLD, &mLocalToWorldMx.m_Mx);

			m_pWrapper->m_pDevice->SetStreamSource(0, m_pIndexedVertexBuffer, m_dwVertexSize);
			
			m_pWrapper->m_pDevice->SetVertexShader(m_dwFVF);
			m_pWrapper->m_pDevice->SetIndices( m_pIndexBuffer, 0L);
			

			//placeholder
			int numprims = m_iNumIndices;
			switch (mPrimType) {
			case D3DPT_TRIANGLELIST:
				numprims/=3;
				break;
			case D3DPT_LINELIST:
				numprims/=2;
				break;
			}

			hr = m_pWrapper->m_pDevice->DrawIndexedPrimitive(mPrimType,
															  0,
															  m_iNumVertices,//m_iNumIndices,
															  0,
															  numprims);			 

			renderpasses++;
		}

		//if the object has children, and we want to render them, now is the time
		if (bRenderChildren)
		{
			IawObject	*pTempObject = m_pFirstChild;
			while (pTempObject != NULL)
			{
				hr |= pTempObject->Render(mLocalToWorldMx, true);

				// if the child we just rendered had a shader, and the next child does not,
				// then we have to reapply the parent's shader (if the parent had one)
				if ((pTempObject->m_pShader) && 
					(pTempObject->m_pNext) &&
					(pTempObject->m_pNext->m_pShader == NULL) && 
					(m_pShader != NULL))
				{
					m_pWrapper->m_pDevice->ApplyStateBlock(m_pShader->m_dwStateBlock);
					//can be used for debugging			
					//OutputDebugString(pTempShader->m_strShaderName);
					//OutputDebugString(" - shader re-activated\n");
				}

				pTempObject = pTempObject->m_pNext;
			}
		}

		//restore world matrix
		m_pWrapper->m_pDevice->SetTransform(D3DTS_WORLD, &iawwMxWorld.m_Mx);

		//again, the if is there to ignore this the 1st time through if there is no shader
		if (m_pShader)
			pTempShader=pTempShader->m_pNext;
	}
	while (pTempShader != NULL);

	//more stuff usefull for debugging
	//char	str[255];
	//sprintf(str,"Object rendered in %d passes\n",renderpasses);
	//OutputDebugString(str);

	return hr;
}










*/
/*



HRESULT IawTextMgr::Render(IawMatrix& iawMxWorld, bool bRenderChildren)
{
	HRESULT hr = S_OK;

	//if the user resized the window or went to a different fullscreen res since last time...
	if ((mLastScreenHRes != GetWrapper()->GetWidth()) ||
		(mLastScreenVRes != GetWrapper()->GetHeight()))
		ResizeString();

	//Setting the material and matrix here is not the most efficient way to do it (redundant calls
	// while stepping down hierarchy) but gets around some issues with the .x file hierarchy 
	// we build right now. It's at least functional, and will be improved in a later version of 
	// the appwizard

	// Set the world matrix to the local matrix
	if (GetFVF() & D3DFVF_XYZ)
		SetLocalToWorldMx(iawMxWorld * GetLocalToParentMx(), false) ;

	int	renderpasses = 0;

	CDRGShader	*pTempShader = GetShader();

	do
	{		
		//we don't do a apply state block if there is no shader at all at the first level
		if (pTempShader != NULL) 
			GetWrapper()->m_pDevice->ApplyStateBlock(pTempShader->m_dwStateBlock);

		if (GetNumVerts() && GetWrapper() && GetWrapper()->m_pDevice && GetVisible())
		{
			if (GetFVF() & D3DFVF_XYZ)
				GetWrapper()->m_pDevice->SetTransform(D3DTS_WORLD, GetLocalToWorldD3DMx());

			renderpasses++;
		}

		//if the object has children, and we want to render them, now is the time
		if (bRenderChildren)
		{
			IawObject	*pTempObject = GetFirst();
			while (pTempObject != NULL)
			{
				hr |= pTempObject->Render(GetLocalToWorldMx(), true);

				// if the child we just rendered had a shader, and the next child does not,
				// then we have to reapply the parent's shader (if the parent had one)
				if ((pTempObject->GetShader()) && 
					(pTempObject->GetNext()) &&
					(pTempObject->GetNext()->GetShader() == NULL) && 
					(GetShader() != NULL))
				{
					GetWrapper()->m_pDevice->ApplyStateBlock(GetShader()->m_dwStateBlock);
				}

				pTempObject = pTempObject->GetNext();
			}
		}

		//restore world matrix
		if (GetFVF() & D3DFVF_XYZ)
			GetWrapper()->m_pDevice->SetTransform(D3DTS_WORLD, &iawMxWorld.m_Mx);

		//again, the if is there to ignore this the 1st time through if there is no shader
		if (GetShader())
			pTempShader = pTempShader->m_pNext;
	}
	while (pTempShader != NULL);

	return hr;
}

  */

/*
void IawTextMgr::ResizeString()
{
	float Hdifference = (float)GetWrapper()->GetWidth()  / (float)mLastScreenHRes;
	float Vdifference = (float)GetWrapper()->GetHeight() / (float)mLastScreenVRes;

	float *pVerts = GetVerticesPtr();

	for (int i = 0; i < (mStringLen * 6); i++)
	{
		//offset verts by change in resolution X current position
		*pVerts++ = Hdifference * (*pVerts);
		*pVerts++ = Vdifference * (*pVerts);

		pVerts+=6;
	}

	mLastScreenHRes = GetWrapper()->GetWidth();
	mLastScreenVRes = GetWrapper()->GetHeight();
}
*/


