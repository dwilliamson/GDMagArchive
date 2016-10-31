// IawShaderMgr.cpp App Wizard Version 2.0 Beta 1
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
// IawShaderMgr.cpp: implementation of the CIawShaderMgr class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

IawShaderMgr::IawShaderMgr(char*	name)
{
  mNumShaders = 0;
  mUsedShaderIds = 0;

  mNumElements = 0;
  mUsedElementIds = 0;

  strcpy(mShaderMgrName,name);

  mpActiveShader = NULL;
  //mpActiveShaderImplementation = NULL;
  //mpActiveShaderComponent = NULL;
  //mCurrectActiveShaderPass = 0;

  mActiveShaderId = 0;
  mpCurrentActiveElement = NULL;

  mpFirstShader = NULL;
  mpFirstElement = NULL;
}

IawShaderMgr::~IawShaderMgr()
{
  //delete all SB's, VS's, etc
  //delete shader hierarchy
}

HRESULT IawShaderMgr::CreateShader(int *pId, char*	name)
{
  if (NULL == mpFirstShader) //then we have no shaders yet
  {
    *pId = ++mNumShaders;
    mUsedShaderIds++;
    mpFirstShader = new IawShader(*pId, name);
  }
  else
  {
    //skip to next to end of list
    IawShader *temp_shader_ptr = mpFirstShader;
    while (temp_shader_ptr->GetNext() != NULL)
      temp_shader_ptr = temp_shader_ptr->GetNext();

    *pId = temp_shader_ptr->GetId();  //one more than the one on the end of the list
    *pId +=1;
    mNumShaders++;
    mUsedShaderIds++;

    IawShader* new_shader = new IawShader(*pId, name);
    if (NULL == new_shader)
      return E_FAIL;

    temp_shader_ptr->AddChild(new_shader);
  }

  return S_OK;
}

HRESULT IawShaderMgr::DeleteShader(int Id)
{
  IawShader* temp_shader_ptr = mpFirstShader;

  if (mpFirstShader->GetId() == Id) //then we have no shaders yet
  {
    mpFirstShader = mpFirstShader->GetNext();
    delete temp_shader_ptr;
  }
  else
  {
    //skip to next to end of list
    temp_shader_ptr = mpFirstShader;
    while ((temp_shader_ptr->GetNext() != NULL) &&
           (temp_shader_ptr->GetNext()->GetId() != Id))
           temp_shader_ptr = temp_shader_ptr->GetNext();

    if (temp_shader_ptr->GetNext() == NULL)
      return E_FAIL;

    IawShader* pTempShader2 = temp_shader_ptr->GetNext();
    temp_shader_ptr->SetNext(temp_shader_ptr->GetNext()->GetNext());
    delete pTempShader2;
  }

  mNumShaders--;

  return S_OK;
}

HRESULT IawShaderMgr::CreateShaderImplementation(int shaderId, int* pId, char* name)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (NULL == temp_shader_ptr)
  {
    OutputDebugString ("Shader not found");
    return E_FAIL;
  }

  //otherwise we found the shader we want
  return temp_shader_ptr->CreateImplementation(pId, name);
}

HRESULT IawShaderMgr::DeleteShaderImplementation(int shaderId, int Id)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (NULL == temp_shader_ptr)
    return E_FAIL; //couldn't find the shader, so couldn't delete the implementation

  return temp_shader_ptr->DeleteImplementation(Id);
}

HRESULT IawShaderMgr::CreateShaderComponent(int shaderId, int implementationId,
                                            int* pId, char* name, DWORD flags)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (temp_shader_ptr == NULL)
  {
    OutputDebugString ("Shader not found");
    return E_FAIL;
  }

  //otherwise we found the shader we want
  return temp_shader_ptr->CreateComponent(implementationId, pId, name, flags);
}

HRESULT IawShaderMgr::CreateNegatingComponent(int shaderId, int implementationId,
                                              int* pId, char* name, DWORD flags)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (NULL == temp_shader_ptr)
  {
    OutputDebugString ("Shader not found");
    return E_FAIL;
  }

  //otherwise we found the shader we want
  return temp_shader_ptr->CreateComponent(implementationId, pId, name, flags | IawShaderComponent::NEGATING);
}


//need to do 
//HRESULT IawShaderMgr::DeleteShaderComponent(int shaderId, int ImplementationId, int Id)

HRESULT IawShaderMgr::CreateElement(int* pId, char* name)
{
  if (NULL == mpFirstElement) //then we have no shaders yet
  {
    *pId = ++mNumElements;
    mUsedElementIds++;
    mpFirstElement = new IawShaderElement(*pId, name);
  }
  else
  {
    //skip to next to end of list
    IawShaderElement *temp_element = mpFirstElement;
    while (temp_element->GetNext() != NULL)
      temp_element = temp_element->GetNext();

    *pId = temp_element->GetId();  //one more than the one on the end of the list
    *pId +=1;
    mNumElements++;
    mUsedElementIds++;

    IawShaderElement* new_element = new IawShaderElement(*pId, name);
    if (NULL == new_element)
      return E_FAIL;

    temp_element->AddChild(new_element);
  }

  return S_OK;
}

// to do
HRESULT IawShaderMgr::DeleteElement(int Id)
{
  IawShaderElement* temp_element = mpFirstElement;

  if (mpFirstElement->GetId() == Id) //then we have no Elements yet
  {
    mpFirstElement = mpFirstElement->GetNext();
    delete temp_element;
  }
  else
  {
    //skip to next to end of list
    IawShaderElement *temp_element = mpFirstElement;
    while ((temp_element->GetNext() != NULL) && (temp_element->GetNext()->GetId() != Id))
      temp_element = temp_element->GetNext();
  
    if (temp_element->GetNext() == NULL)
      return E_FAIL;
  
    IawShaderElement *pTempElement2 = temp_element->GetNext();
    temp_element->SetNext(temp_element->GetNext()->GetNext());
    delete pTempElement2;
  }

  mNumElements--;

  return S_OK;
}

HRESULT IawShaderMgr::SetComponentElement(int shaderId, int implementationId, int componentId, int elementId)
{
  IawShaderElement *temp_element = GetElementPtr(elementId);

  if (temp_element == NULL)
    return E_FAIL;

  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (temp_shader_ptr == NULL)
    return E_FAIL;

  IawShaderImplementation* temp_implementation = temp_shader_ptr->GetImplementationPtr(implementationId);

  if (temp_implementation == NULL)
    return E_FAIL;

  IawShaderComponent* temp_component = temp_implementation->GetComponentPtr(componentId);

  if (temp_component == NULL)
    return E_FAIL;

  temp_component->SetElement(temp_element);

  return S_OK;
}

HRESULT IawShaderMgr::SetElementStateBlock(char* elementName, DWORD stateBlock)
{
  int element_id = GetElementId(elementName);
  if (element_id <= 0)
    return E_FAIL;
  IawShaderElement* temp_element = GetElementPtr(element_id);
  if (temp_element == NULL)
    return E_FAIL;
  temp_element->SetStateBlock(stateBlock, 0);
  temp_element->SetValid(true); //should replace this with a ValidateDevice

  return S_OK;
}


int IawShaderMgr::GetShaderId(char* name)
{
  IawShader* temp_shader_ptr = mpFirstShader;

  while (temp_shader_ptr && strcmp(name, temp_shader_ptr->GetName()) != 0)
    temp_shader_ptr = temp_shader_ptr->GetNext();

  if (temp_shader_ptr)
    return temp_shader_ptr->GetId();
  else
    return -1;
}

char* IawShaderMgr::GetShaderName(int shaderId)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (temp_shader_ptr)
    return temp_shader_ptr->GetName();
  else
    return "error";
}


int IawShaderMgr::GetElementId(char* name)
{
  IawShaderElement* temp_element = mpFirstElement;

  while (temp_element && strcmp(name, temp_element->GetName()) != 0)
    temp_element = temp_element->GetNext();

  if (temp_element)
    return temp_element->GetId();
  else
    return -1;
}


int IawShaderMgr::GetImplementationId(int shaderId, char* name)
{
  IawShader* temp_shader_ptr = GetShaderPtr(shaderId);

  if (temp_shader_ptr)
    return temp_shader_ptr->GetImplementationId(name);
  else
    return -1;
}

// need to do
// int		IawShaderMgr::GetComponentId(int shaderId, char*	name)
/*
int		IawShaderMgr::GetElementId(char*	name)
{
IawShaderElement *pTempElement = mpFirstElement;

  while (pTempElement && strcmp(name, pTempElement->GetName()) != 0)
		pTempElement = pTempElement->GetNext();
    
      if (pTempElement)
      return pTempElement->GetId();
      else
      return -1;
      }
*/



//private
IawShader* IawShaderMgr::GetShaderPtr(int shaderId)
{
  IawShader* temp_shader_ptr = mpFirstShader;

  while (temp_shader_ptr && (temp_shader_ptr->GetId() != shaderId))
    temp_shader_ptr = temp_shader_ptr->GetNext();

  return temp_shader_ptr; //will return NULL if the search finds nothing
}

IawShaderElement* IawShaderMgr::GetElementPtr(int elementId)
{
  IawShaderElement* pTempElement = mpFirstElement;

  while (pTempElement && (pTempElement->GetId() != elementId))
    pTempElement = pTempElement->GetNext();

  return pTempElement; //will return NULL if the search finds nothing
}

//for now, assumes use of first implementation. Need to do the ValidateImplementation stuff
int IawShaderMgr::GetNumPasses(int shaderId)
{
  IawShader* pShader = GetShaderPtr(shaderId);
  if (pShader == NULL)
    return 0;

  IawShaderImplementation* pImp = pShader->GetActiveImplementation();
  if (pImp == NULL)
    return 0;

  IawShaderComponent* pComp = pImp->GetFirstComponent();
  if (pImp == NULL)
    return 0;

  int numpasses = 1;
  while (pComp != NULL)
  {
    if (pComp->GetFlags() & IawShaderComponent::END_OF_PASS)
      numpasses++;
    pComp = pComp->GetNext();
  }

  return numpasses;
}

HRESULT IawShaderMgr::ActivateShader(IawD3dWrapper* pWrapper, int shaderId, int pass)
{
  IawShader* pShader = GetShaderPtr(shaderId);
  if (pShader == NULL)
    return E_FAIL;

  IawShaderImplementation* pImp = pShader->GetActiveImplementation();
  if (pImp == NULL)
    return E_FAIL;

  IawShaderComponent* pComp = pImp->GetFirstComponent();
  if (pComp == NULL)
    return E_FAIL;

  int currpass = 1;
  while ((currpass != pass) && (pComp != NULL))
  {
    if (pComp->GetFlags() & IawShaderComponent::END_OF_PASS)
      currpass++;
    pComp = pComp->GetNext();
  }

  //we should now have pComp pointing to the first component of the next pass
  while ((currpass <= pass) && (pComp != NULL))
  {
    if (pComp->GetElement() != mpCurrentActiveElement)
    {
      //ActivateComponent
      /*
      char str[255];
      sprintf(str, "Activating Element: ");
      OutputDebugString(str);
      OutputDebugString(pComp->GetName());
      OutputDebugString("\n");
      */
      pWrapper->mpDevice->ApplyStateBlock(pComp->GetElement()->GetStateBlock());
      if (pComp->GetFlags() & IawShaderComponent::END_OF_PASS)
        currpass++;
    
      mpCurrentActiveElement = pComp->GetElement();
    }
    pComp = pComp->GetNext();
  }

  return S_OK;
}

HRESULT IawShaderMgr::DeActivateShader(IawD3dWrapper* pWrapper, int shaderId, int pass)
{
  IawShader* pShader = GetShaderPtr(shaderId);
  if (pShader == NULL)
    return E_FAIL;

  IawShaderImplementation *pImp = pShader->GetActiveImplementation();
  if (pImp == NULL)
    return E_FAIL;

  IawShaderComponent *pComp = pImp->GetFirstNegatingComponent();
  if (pComp == NULL)
    return E_FAIL;

  int currpass = 1;
  while ((currpass != pass) && (pComp != NULL))
  {
    if (pComp->GetFlags() & IawShaderComponent::END_OF_PASS)
      currpass++;
    pComp = pComp->GetNext();
  }

  //we should now have pComp pointing to the first component of the next pass
  while ((currpass <= pass) && (pComp != NULL))
  {
    if (pComp->GetElement() != mpCurrentActiveElement)
    {
      //ActivateComponent
      /*
      char str[255];
      sprintf(str, "Activating Element: ");
      OutputDebugString(str);
      OutputDebugString(pComp->GetName());
      OutputDebugString("\n");
      */
      pWrapper->mpDevice->ApplyStateBlock(pComp->GetElement()->GetStateBlock());
      if (pComp->GetFlags() & IawShaderComponent::END_OF_PASS)
        currpass++;
    
      mpCurrentActiveElement = pComp->GetElement();
    }
    pComp = pComp->GetNext();
  }

  return S_OK;
}


HRESULT IawShaderMgr::SetShaderActiveImplementation(int shaderId, int implementationId)
{
  IawShader* pShader = GetShaderPtr(shaderId);
  if (pShader == NULL)
    return E_FAIL;

  IawShaderImplementation* pTempImplementation = pShader->GetImplementationPtr(implementationId);
  if (pTempImplementation == NULL)
    return E_FAIL;

  pShader->SetActiveImplementation(pTempImplementation);

  return S_OK;

}




HRESULT IawShaderMgr::TargetChanging(IawD3dWrapper* pWrapper)
{
  if (pWrapper && pWrapper->mpDevice)
  {
    for (int i=0; i<8; i++)
      pWrapper->mpDevice->SetTexture(i, NULL );
  }

  //delete and invalidate all elements
  IawShaderElement* pTempElement = mpFirstElement;

  while (pTempElement)
  {
    pTempElement->DeleteStateBlock(pWrapper); //will delete SB and invalidate element
  
    pTempElement = pTempElement->GetNext();
  }

  mpCurrentActiveElement = NULL;

  return S_OK; //will return NULL if the search finds 
}


void IawShaderMgr::PrintShaderHierarchy()
{
  OutputDebugString("---------------------------------\n");
  OutputDebugString(mShaderMgrName);
  OutputDebugString("\n");
  
  
  IawShader	*temp_shader_ptr = mpFirstShader;
  if (temp_shader_ptr == NULL)
  {
    OutputDebugString ("No Shaders Exist\n");
    return;
  }
  
  while (temp_shader_ptr != NULL)
  {
    OutputDebugString("      ");
    OutputDebugString(temp_shader_ptr->GetName());
    OutputDebugString("\n");
    
    //////////////////////////
    //print implementation info
    IawShaderImplementation	*pTempImplementation = temp_shader_ptr->GetFirstImplementation();
    
    if (pTempImplementation == NULL)
    {
      OutputDebugString("      ");
      OutputDebugString("      ");
      OutputDebugString ("No Implementations exist for this shader\n");
    }
    
    while (pTempImplementation != NULL)
    {
      OutputDebugString("      ");
      OutputDebugString("      ");
      OutputDebugString(pTempImplementation->GetName());
      OutputDebugString("\n");
      
      //////////////////////////
      //print component info
      IawShaderComponent	*pTempComponent = pTempImplementation->GetFirstComponent();
      
      if (pTempComponent == NULL)
      {
        OutputDebugString("      ");
        OutputDebugString("      ");
        OutputDebugString("      ");
        OutputDebugString ("No Components exist for this Implementation\n");
      }
      
      while (pTempComponent != NULL)
      {
        OutputDebugString("      ");
        OutputDebugString("      ");
        OutputDebugString("      ");
        OutputDebugString(pTempComponent->GetName());
        OutputDebugString("      ");
        IawShaderElement *pTempElement = pTempComponent->GetElement();
        if (pTempElement == NULL)
          OutputDebugString("No assigned Element");
        else
        {
          OutputDebugString("Uses Element:");
          OutputDebugString(pTempElement->GetName());
        }
        OutputDebugString("\n");
        
        pTempComponent = pTempComponent->GetNext();
        
      }
      // end print component info
      //////////////////////////
      
      
      pTempImplementation = pTempImplementation->GetNext();
      
    }
    //////////////////////////
    
    temp_shader_ptr = temp_shader_ptr->GetNext();
  }
  
  OutputDebugString("\n---------------------------------\n");
  
  return;
  
}




/*
//creating
HRESULT CreateShaderImplementation(int shaderId, 
                                   int &Id, char*	name = "Unnamed Shader Implementation");
HRESULT		CreateShaderComponent(int shaderId, int ImplementationId, 
                                int &Id, char* name = "Unnamed Shader Component");

//getting
int			GetShaderId(char*	name);
int			GetImplementationId(int shaderId, char*	name);
int			GetComponentId(int shaderId, int ImplementationId, char* name);

int			GetImplementationNumPasses(int shaderId, int ImplementationId);

//setting
HRESULT		SetShaderComponent(int shaderId, int ImplementationId, 
                             int ComponentId, DWORD dwCompType,
                             DWORD dwComponentValue);

HRESULT		SetShaderComponent(int shaderId, int ImplementationId, 
                             int ComponentId,  DWORD dwCompType, 
                             IawMatrix dwMatrix);

//Activating
HRESULT		ActivateShaderPass(int shaderId, int pass);


HRESULT		DeleteAllStateBlocks();















void IawShaderMgr::SetStateBlock(DWORD sb)
{
  m_dwStateBlock = sb;
  //return S_OK;
}

HRESULT	IawShaderMgr::AddChild(IawShaderMgr	*pChildShader)
{
  
  //need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this,pChildShader) != S_OK)
  {
    OutputDebugString("failed to add shader as child. Likely due to circular link");
    return E_FAIL;
  }
  
  if (m_pNext == NULL)
  {
    m_pNext = pChildShader;
  }
  else
  {
    IawShaderMgr	*temp_shader_ptr = m_pNext;
    //skip to one before end of list
    while (temp_shader_ptr->m_pNext != NULL)
    {			
      temp_shader_ptr = temp_shader_ptr->m_pNext;
    }
    //now add shader to end of linked list
    temp_shader_ptr->m_pNext = pChildShader;
  }
  return S_OK;
  
}

// -----------------------------------------------------------------------
// HRESULT IawObject::CheckAgainstChildren(IawObject	*pPotentialParent, 
//											IawObject *pPotentialChild)
//
//  Checks to make sure the object you want to make a parent of another, 
// isn't already one of that objects children. Thou shalt not murder the stack!
// -----------------------------------------------------------------------
HRESULT	IawShaderMgr::CheckAgainstChildren(IawShaderMgr	*pPotentialParent, IawShaderMgr *pPotentialChild)
{
  IawShaderMgr	*temp_shader_ptr = pPotentialChild;
  
  //skip to end of list, comparing against parent Shader
  while (temp_shader_ptr != NULL)
  {
    if(pPotentialChild == pPotentialParent)
      return E_FAIL;
    
    temp_shader_ptr = temp_shader_ptr->m_pNext;
  }
  
  //if we got here, none of the children was the potential parent, no circular list
  if (pPotentialChild->m_pNext == NULL)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent, pPotentialChild->m_pNext);
}


void IawShaderMgr::CheckProperties(CD3DWrapper *pD3DWrapper, bool &b_alpha)
{
  DWORD dwAlpha;
  
  pD3DWrapper->m_pDevice->ApplyStateBlock(m_dwStateBlock);
  pD3DWrapper->m_pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
  
  
  b_alpha = (dwAlpha == 0) ? false : true;
}

*/
