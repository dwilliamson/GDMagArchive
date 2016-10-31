// IawShader.cpp App Wizard Version 2.0 Beta 1
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
// IawShader.cpp: implementation of the CIawShader class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// IawShaderElement -----------------------------------------------------------

// Constants...
const int IawShaderElement::STATE_BLOCK   = 0x00000001;
const int IawShaderElement::VERTEX_SHADER = 0x00000002;
const int IawShaderElement::PIXEL_SHADER  = 0x00000004;
const int IawShaderElement::MATRIX        = 0x00000008;

// Constructor
IawShaderElement::IawShaderElement(int Id, char*	name)
{
  strcpy(mElementName, name);
  mElementId = Id;

  mFlags = 0x00000000;
  mStateBlock = 0xffffffff;

  mValid = false;


  mpNext = NULL;
}

// Destructor
IawShaderElement::~IawShaderElement()
{
  // Don't need to delete state blocks here. Shader manager will do it explicitly
}

// Set name
HRESULT IawShaderElement::SetName(char* name)
{
  strcpy(mElementName, name);
  return S_OK;
}

// Set a state block
HRESULT IawShaderElement::SetStateBlock(DWORD stateBlock, DWORD flags)
{
  mStateBlock = stateBlock;

  if (0xffffffff == stateBlock)
    mValid = false;
  else
    mValid = true; //should really only do this after Validate()

  return S_OK;
}

// Delete a state block
HRESULT IawShaderElement::DeleteStateBlock(IawD3dWrapper* pWrapper)
{
  HRESULT hr = S_OK;
  if ((mStateBlock != 0xffffffff) && (mValid))
    hr = pWrapper->mpDevice->DeleteStateBlock(mStateBlock);
  mValid = false;

  return hr;
}

// Add a child to this element's hierarchy
HRESULT IawShaderElement::AddChild(IawShaderElement* pChildElement)
{
  //need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this, pChildElement) != S_OK)
  {
    OutputDebugString("failed to add as child due to circular link");
    return E_FAIL;
  }

  if (mpNext == NULL)
  {
    mpNext = pChildElement;
  }
  else
  {
    IawShaderElement *pTempElement = mpNext;
    //skip to one before end of list
    while (pTempElement->GetNext() != NULL)
    {
      pTempElement = pTempElement->GetNext();
    }
    //now add child
    pTempElement->SetNext(pChildElement);
  }

  return S_OK;
}

// Safety net... don't add an element as its own child.
HRESULT IawShaderElement::CheckAgainstChildren(IawShaderElement* pPotentialParent,
                                               IawShaderElement* pPotentialChild)
{
  IawShaderElement* temp_element_ptr = pPotentialChild;

  //skip to end of list, comparing against parent Element
  while (temp_element_ptr != NULL)
  {
    if (pPotentialChild == pPotentialParent)
      return E_FAIL;

    temp_element_ptr = temp_element_ptr->GetNext();
  }

  //if we got here, none of the siblings was the potential parent
  if (pPotentialChild->GetNext() == NULL)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent,
    pPotentialChild->GetNext());
}










// IawShaderComponent ---------------------------------------------------------

// Constants...
const int IawShaderComponent::END_OF_PASS = 0x00000100;
const int IawShaderComponent::NEGATING    = 0x00000200;

// Constructor
IawShaderComponent::IawShaderComponent(int Id, char* name, DWORD flags)
{
  strcpy(mComponentName, name);
  mComponentId = Id;

  mFlags = flags;

  mpElement = NULL;

  mpNext = NULL;
}

// Destructor
IawShaderComponent::~IawShaderComponent()
{
  // Need to decrement the refcount on elements
}

// Set name
HRESULT IawShaderComponent::SetName(char* name)
{
  strcpy(mComponentName, name);
  return S_OK;
}

// Add a child to this component's hierarchy
HRESULT IawShaderComponent::AddChild(IawShaderComponent* pChildComponent)
{
  // Need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this, pChildComponent) != S_OK)
  {
    OutputDebugString("failed to add as child due to circular link");
    return E_FAIL;
  }

  if (NULL == mpNext)
  {
    mpNext = pChildComponent;
  }
  else
  {
    IawShaderComponent *temp_component_ptr = mpNext;
    // Skip to one before end of list
    while (temp_component_ptr->GetNext() != NULL)
    {
      temp_component_ptr = temp_component_ptr->GetNext();
    }
    //now add child
    temp_component_ptr->SetNext(pChildComponent);
  }

  return S_OK;
}

// Safety net...
HRESULT IawShaderComponent::CheckAgainstChildren(IawShaderComponent* pPotentialParent,
                                                 IawShaderComponent* pPotentialChild)
{
  IawShaderComponent* temp_component_ptr = pPotentialChild;

  //skip to end of list, comparing against parent Component
  while (temp_component_ptr != NULL)
  {
    if (pPotentialChild == pPotentialParent)
      return E_FAIL;
    temp_component_ptr = temp_component_ptr->GetNext();
  }

  //if we got here, none of the siblings was the potential parent
  if (pPotentialChild->GetNext() == NULL)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent,
    pPotentialChild->GetNext());
}










// IawShaderImplementation ----------------------------------------------------

// Constructor...
IawShaderImplementation::IawShaderImplementation(int Id, char* name)
{
  strcpy(mImplementationName, name);
  mImplementationId = Id;
  mNumIds = 0;

  mNumPasses = 0;
  mNumComponents = 0;

  mpFirstComponent = NULL;
  mpFirstNegatingComponent = NULL;

  mpNext = NULL;
}

// Destructor
IawShaderImplementation::~IawShaderImplementation()
{
  // Need to delete all implementations
}

// Set name
HRESULT IawShaderImplementation::SetName(char* name)
{
  strcpy(mImplementationName, name);
  return S_OK;
}

// Create a component
HRESULT IawShaderImplementation::CreateComponent(int* pId, char* name, DWORD flags)
{
  if (IawShaderComponent::NEGATING & flags)
  {
    if (NULL == mpFirstNegatingComponent) //then we have no shaders yet
    {
      *pId = ++mNumComponents;
      mNumIds++;
      mpFirstNegatingComponent = new IawShaderComponent(*pId, name, flags);
    }
    else
    {
      //skip to next to end of list
      IawShaderComponent* temp_component_ptr = mpFirstNegatingComponent;
      while (temp_component_ptr->GetNext() != NULL)
        temp_component_ptr = temp_component_ptr->GetNext();

      *pId = temp_component_ptr->GetId();  //one more than the one on the end of the list
      *pId +=1;
      mNumComponents++;
      mNumIds++;

      IawShaderComponent *new_component = new IawShaderComponent(*pId, name, flags);
      if (NULL == new_component)
        return E_FAIL;

      temp_component_ptr->AddChild(new_component);
    }

  }
  else
  {
    if (NULL == mpFirstComponent) //then we have no shaders yet
    {
      *pId = ++mNumComponents;
      mNumIds++;
      mpFirstComponent = new IawShaderComponent(*pId, name, flags);
    }
    else
    {
      // Skip to next to end of list
      IawShaderComponent* temp_component_ptr = mpFirstComponent;
      while (temp_component_ptr->GetNext() != NULL)
        temp_component_ptr = temp_component_ptr->GetNext();

      *pId = temp_component_ptr->GetId();  //one more than the one on the end of the list
      *pId +=1;
      mNumComponents++;
      mNumIds++;

      IawShaderComponent* new_component = new IawShaderComponent(*pId, name, flags);
      if (NULL == new_component)
        return E_FAIL;

      temp_component_ptr->AddChild(new_component);
    }
  }

  return S_OK;
}

// Access...
IawShaderComponent* IawShaderImplementation::GetComponentPtr(int componentId)
{
  IawShaderComponent* temp_component_ptr = mpFirstComponent;

  while (temp_component_ptr && (temp_component_ptr->GetId() != componentId))
    temp_component_ptr = temp_component_ptr->GetNext();

  if (NULL == temp_component_ptr) //if we skipped through without finding it
  {
    temp_component_ptr = mpFirstNegatingComponent;

    while (temp_component_ptr && (temp_component_ptr->GetId() != componentId))
      temp_component_ptr = temp_component_ptr->GetNext();
  }
  return temp_component_ptr; //will return NULL if the search finds nothing in either linked list
}

// Add a child to this implementation's hierarchy
HRESULT IawShaderImplementation::AddChild(IawShaderImplementation* pChildImplementation)
{
  // Need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this, pChildImplementation) != S_OK)
  {
    OutputDebugString("failed to add as child due to circular link");
    return E_FAIL;
  }

  if (NULL == mpNext)
  {
    mpNext = pChildImplementation;
  }
  else
  {
    IawShaderImplementation* temp_implementation_ptr = mpNext;
    //skip to one before end of list
    while (temp_implementation_ptr->GetNext() != NULL)
    {
      temp_implementation_ptr = temp_implementation_ptr->GetNext();
    }
    //now add child
    temp_implementation_ptr->SetNext(pChildImplementation);
  }

  return S_OK;
};

// Safety net...
HRESULT	IawShaderImplementation::CheckAgainstChildren(IawShaderImplementation* pPotentialParent,
                                                      IawShaderImplementation* pPotentialChild)
{
  IawShaderImplementation *temp_implementation_ptr = pPotentialChild;

  //skip to end of list, comparing against parent Implementation
  while (temp_implementation_ptr != NULL)
  {
    if (pPotentialChild == pPotentialParent)
      return E_FAIL;

    temp_implementation_ptr = temp_implementation_ptr->GetNext();
  }

  //if we got here, none of the siblings was the potential parent
  if (pPotentialChild->GetNext() == NULL)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent, pPotentialChild->GetNext());
}










// IawShader ------------------------------------------------------------------

// Constructor...
IawShader::IawShader(int Id, char* name)
{
  strcpy(mShaderName, name);
  mShaderId = Id;
  mNumImplementations = 0;
  mNumIds = 0; //total implementation Ids handed out

  mpFirstImplementation = NULL;
  mpActiveImplementation = NULL;

  mpNext = NULL;
}

// Destructor
IawShader::~IawShader()
{
  // Need to delete all implementations

  if (mpFirstImplementation != NULL)
  {
    IawShaderImplementation *pTempImp1 = mpFirstImplementation;
    IawShaderImplementation *pTempImp2 = mpFirstImplementation->GetNext();
  
    while (pTempImp2 != NULL)
    {
      delete pTempImp1;
      pTempImp1 = pTempImp2;
      pTempImp2 = pTempImp2->GetNext(); 
    }
    delete pTempImp1;
  }
  }

  // Set name
  HRESULT IawShader::SetName(char* name)
  {
  strcpy(mShaderName, name);
  return S_OK;
}

// Add a child to this shader's hierarchy
HRESULT IawShader::AddChild(IawShader* pChildShader)
{
  //need to make sure no one tried something nasty like a circular loop!
  if (CheckAgainstChildren(this, pChildShader) != S_OK)
  {
    OutputDebugString("failed to add as child due to circular link");
    return E_FAIL;
  }

  if (NULL == mpNext)
  {
    mpNext = pChildShader;
  }
  else
  {
    IawShader* temp_shader_ptr = mpNext;
    //skip to one before end of list
    while (temp_shader_ptr->GetNext() != NULL)
    {
      temp_shader_ptr = temp_shader_ptr->GetNext();
    }
    //now add child
    temp_shader_ptr->SetNext(pChildShader);
  }

  return S_OK;
}

// Safety net...
HRESULT IawShader::CheckAgainstChildren(IawShader* pPotentialParent,
                                        IawShader* pPotentialChild)
{
  IawShader *temp_shader_ptr = pPotentialChild;

  //skip to end of list, comparing against parent Shader
  while (temp_shader_ptr != NULL)
  {
    if (pPotentialChild == pPotentialParent)
      return E_FAIL;

    temp_shader_ptr = temp_shader_ptr->GetNext();
  }

  //if we got here, none of the siblings was the potential parent
  if (pPotentialChild->GetNext() == NULL)
    return S_OK;
  else
    return CheckAgainstChildren(pPotentialParent, pPotentialChild->GetNext());
}

// Create an implementation
HRESULT IawShader::CreateImplementation(int* pId, char* name)
{
  if (NULL == mpFirstImplementation) //then we have no shaders yet
  {
    *pId = ++mNumImplementations;
    mNumIds++;
    mpFirstImplementation = new IawShaderImplementation(*pId, name);
    mpActiveImplementation = mpFirstImplementation;
  }
  else
  {
    //skip to next to end of list
    IawShaderImplementation *temp_implementation_ptr = mpFirstImplementation;
    while (temp_implementation_ptr->GetNext() != NULL)
      temp_implementation_ptr = temp_implementation_ptr->GetNext();

    *pId = temp_implementation_ptr->GetId();  //one more than the one on the end of the list
    *pId +=1;
    mNumImplementations++;
    mNumIds++;

    IawShaderImplementation *pNewImplementation = new IawShaderImplementation(*pId, name);
    if (pNewImplementation == NULL)
      return E_FAIL;

    temp_implementation_ptr->AddChild(pNewImplementation);

  }

  return S_OK;
}

// Delete an implementation
HRESULT IawShader::DeleteImplementation(int Id)
{
  IawShaderImplementation* temp_implementation_ptr = mpFirstImplementation;

  if (mpFirstImplementation->GetId() == Id) //then we have no Implementations yet
  {
    mpFirstImplementation = mpFirstImplementation->GetNext();
    delete temp_implementation_ptr;
  }
  else
  {
    // Skip to next to end of list
    IawShaderImplementation* temp_implementation_ptr = mpFirstImplementation;
    while ((temp_implementation_ptr->GetNext() != NULL) &&
      (temp_implementation_ptr->GetNext()->GetId() != Id))
      temp_implementation_ptr = temp_implementation_ptr->GetNext();

    if (temp_implementation_ptr->GetNext() == NULL)
      return E_FAIL;

    IawShaderImplementation* pTempImplementation2 = temp_implementation_ptr->GetNext();
    temp_implementation_ptr->SetNext(temp_implementation_ptr->GetNext()->GetNext());
    delete pTempImplementation2;
  }

  mNumImplementations--;

  return S_OK;
}

// Access...
int IawShader::GetImplementationId(char* name)
{
  IawShaderImplementation* temp_implementation_ptr = mpFirstImplementation;

  while (temp_implementation_ptr && strcmp(name, temp_implementation_ptr->GetName()) != 0)
    temp_implementation_ptr = temp_implementation_ptr->GetNext();

  if (temp_implementation_ptr)
    return temp_implementation_ptr->GetId();
  else
    return -1;
}

IawShaderImplementation* IawShader::GetImplementationPtr(int ImplementationId)
{
  IawShaderImplementation *temp_implementation_ptr = mpFirstImplementation;

  while (temp_implementation_ptr && (temp_implementation_ptr->GetId() != ImplementationId))
    temp_implementation_ptr = temp_implementation_ptr->GetNext();

  return temp_implementation_ptr; //will return NULL if the search finds nothing
}

// Create a component
HRESULT IawShader::CreateComponent(int ImplementationId, int* pId, char* name, DWORD flags)
{
  IawShaderImplementation *temp_implementation_ptr = GetImplementationPtr(ImplementationId);

  if (NULL == temp_implementation_ptr)
  {
    OutputDebugString ("Implementation not found");
    return E_FAIL;
  }

  //otherwise we found the Implementation we want
  return temp_implementation_ptr->CreateComponent(pId, name, flags);
}

