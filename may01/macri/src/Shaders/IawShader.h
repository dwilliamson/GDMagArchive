// IawShader.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawShader_h)
#define IawShader_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
//This class is used encapsulate a component of a shader pass. 
class IawShaderComponent {

//public methods
public:

	IawShaderComponent(char*	strname = "Unnamed");
	virtual		~IawShaderComponent();

	HRESULT		ActivateShaderComponent();

	HRESULT		SetStateBlock(DWORD	dwSB);
	HRESULT		SetVertexShader(DWORD	dwVS);
	HRESULT		SetPixelShader(DWORD	dwPS);
	HRESULT		SetMatrix(IawMatrix mx, DWORD mxtype);
	
	HRESULT		SetName(char *strname);

	inline DWORD		GetComponentType () {return m_dwComponentType};
	inline DWORD		GetStateBlock () {return m_dwStateBlock};
	inline DWORD		GetVertexShader () {return m_dwVertexShader};
	inline DWORD		GetPixelShader () {return m_dwPixelShader};
	inline IawMatrix	GetMatrix() {return m_mxMat};
	inline DWORD		GetMatrixType() {return m_dwMatType};

	HRESULT AddChild(IawShaderComponent *pComponent);
	inline IawShaderComponent* GetChild() {return m_pNext;}

	void TargetChanging();

	void CheckProperties(CD3DWrapper *pD3DWrapper, bool &b_alpha);

	HRESULT	Activate();
	

//private methods & member vars
private:

	char		m_strShaderComponentName[255];
	DWORD	m_dwShaderComponentType;

	DWORD	m_dwStateBlock;
	DWORD	m_dwVertexShader;
	DWORD	m_dwPixelShader;
	IawMatrix	m_mxMat;
	DWORD	m_dwMatType;

	bool	m_bValidOnCurrentDevice;

//	IawShaderComponent		*m_pNext;
	
	HRESULT	CheckAgainstChildren(IawShaderComponent	*pPotentialParent, IawShaderComponent *pPotentialChild);
};

  */

/**
 * This class describes a shader element.
 */
class IawShaderElement
{
public:
  /**
   * Constructor.
   * @param Id The Id for this element.
   * @param name A name for this element.
   */
  IawShaderElement(int Id, char* name = "Unnamed Element");

  /** Destructor */
  virtual ~IawShaderElement();

  /** Set the element name */
  HRESULT SetName(char* name);

  //@{
  /** Access method */
  IawShaderElement* GetNext() {return mpNext;}
  void SetNext(IawShaderElement* pTemp) {mpNext = pTemp;}
  int GetId() {return mElementId;}
  char* GetName() {return mElementName;}
  void SetValid(bool valid) {mValid = valid;}
  bool GetValid() {return mValid;}
  HRESULT SetStateBlock(DWORD stateBlock, DWORD flags);
  DWORD GetStateBlock() {return mStateBlock;}
  //@}

  /**
   * Delete the state block for this element.
   * @param pWrapper The wrapper for the device this element as a state block on
   */
  HRESULT DeleteStateBlock(IawD3dWrapper* pWrapper);

  /**
   * Add a child to this element's hierarchy.
   */
  HRESULT AddChild(IawShaderElement* pChildElement);

  /** Flag this element as a state block */
  static const int STATE_BLOCK;

  /** Flag this element as a vertex shader */
  static const int VERTEX_SHADER;

  /** Flag this element as a pixel shader */
  static const int PIXEL_SHADER;

  /** Flag this element as a matrix */
  static const int MATRIX;

private:
  char mElementName[255];
  int  mElementId;

  DWORD mStateBlock;
  DWORD mFlags;
  bool  mValid;

  IawShaderElement* mpNext;

  HRESULT CheckAgainstChildren(IawShaderElement* pPotentialParent,
                               IawShaderElement* pPotentialChild);
};


/**
 * This class encapsulates a component of a shader pass.
 */
class IawShaderComponent
{
public:
  /**
   * Constructor.
   * @param Id The Id for this component.
   * @param name A name for this component.
   * @param flags Any flags (END_OF_PASS, NEGATING,etc...).
   */
  IawShaderComponent(int Id, char* name = "Unnamed Component", DWORD flags = 0);

  /** Destructor */
  virtual ~IawShaderComponent();

  /** Set the component name */
  HRESULT SetName(char* name);

  //@{
  /** Access method */
  IawShaderComponent* GetNext() {return mpNext;}
  void SetNext(IawShaderComponent* pTemp) {mpNext = pTemp;}
  int GetId() {return mComponentId;}
  char* GetName() {return mComponentName;}
  DWORD GetFlags() {return mFlags;}

  IawShaderElement* GetElement() {return mpElement;}
  void SetElement(IawShaderElement* pElement) {mpElement = pElement;}
  //@}

  /** Add a child to this component hierarchy */
  HRESULT AddChild(IawShaderComponent* pChildComponent);

  //void TargetChanging();

  //void CheckProperties(CD3DWrapper *pD3DWrapper, bool &b_alpha);
  //bool Validate();
  //bool Activate();

  /** Flags the end of a shader for a render pass */
  static const int END_OF_PASS;

  /** Component negating */
  static const int NEGATING;

private:
  char mComponentName[255];
  int  mComponentId;
  //	int			mNumIds;

  IawShaderElement* mpElement;
  DWORD mFlags;

  IawShaderComponent* mpNext;

  //bool mValidOnCurrentDevice;


  HRESULT CheckAgainstChildren(IawShaderComponent* pPotentialParent,
                               IawShaderComponent* pPotentialChild);
};


/**
 * A shader implementation.
 * An implementation may be made up of one or more render passes.
 *
 * Each implementation may also have a string of "exit components"
 *
 * Each render pass is comprised of one or more components, the final one of
 * which is flagged as 'END_OF_PASS'.
 */
class IawShaderImplementation
{
public:
  /**
   * Constructor.
   * @param Id An Id for the implementation.
   * @param name A name for the implementation.
   */
  IawShaderImplementation(int Id, char* name = "Unnamed");

  /** Destructor */
  virtual ~IawShaderImplementation();

  /** Set the implementation name */
  HRESULT SetName(char* name);

  /**
   * Create a component for this implementation
   * @param pId The Id number that will be assigned to the component.
   * @param name The component name.
   * @param flags Any additional flags.
   */
  HRESULT CreateComponent(int *pId, char* name = "Unnamed Shader Component", DWORD flags = 0);

  //@{
  /** Access method */
  IawShaderImplementation* GetNext() {return mpNext;}
  IawShaderComponent* GetFirstComponent() {return mpFirstComponent;}
  IawShaderComponent* GetFirstNegatingComponent() {return mpFirstNegatingComponent;}
  void SetNext(IawShaderImplementation* pTemp) {mpNext = pTemp;}
  int GetId() {return mImplementationId;}
  char* GetName() {return mImplementationName;}

  IawShaderComponent* GetComponentPtr(int componentId);
  //@}

  /** Add a child to this implementation hierarchy */
  HRESULT AddChild(IawShaderImplementation* pChildImplementation);

  //	void TargetChanging();

  //	void CheckProperties(CD3DWrapper *pD3DWrapper, bool &b_alpha);
  // bool Validate();
  //bool Activate();
  //	inline int GetNumPasses() {return mNumPasses};

private:
  char mImplementationName[255];
  int mImplementationId;
  int mNumIds; //total number handed out

  int mNumPasses;
  int mNumComponents;

  IawShaderComponent* mpFirstComponent;
  IawShaderComponent* mpFirstNegatingComponent;
  //IawShaderComponent		 *mpFirstStateResetComponent;

  IawShaderImplementation* mpNext;

  //bool mValidOnCurrentDevice;

  HRESULT CheckAgainstChildren(IawShaderImplementation* pPotentialParent,
                               IawShaderImplementation* pPotentialChild);
};


/**
 * This class encapsulates a shader.
 *
 * Each shader can have more than one implementation, and each implementation
 * may be made up of one or more render passes.
 *
 * Each implementation may also have a string of "exit components"
 *
 * Each render pass is comprised of one or more components, the final one of
 * which is flagged as 'END_OF_PASS'.
 *
 * @see IawShaderImplementation
 * @see IawShaderComponent
 * @see IawShaderElement
 */
class IawShader
{
public:
  /**
   * Constructor.
   * @param Id The shader id number.
   * @param name A name for the shader.
   */
  IawShader(int Id, char* name = "Unnamed");

  /** Destructor */
  virtual ~IawShader();

  /** Set the shader name */
  HRESULT SetName(char* name);

  /** Create an implementation for this shader */
  HRESULT CreateImplementation(int* pId, char* name = "Unnamed Shader Implementation");

  /**
   * Delete a specified implementation of this shader.
   * @param Id The Id number of the implementation to delete.
   */
  HRESULT DeleteImplementation(int Id);

  /**
   * Create a component for this shader.
   * @param implementationId The Id of the implementation this component will belong to.
   * @param pId The Id number that will be assigned to the component.
   * @param name A name for the component.
   * @param flags Any additional flags for the component.
   */
  HRESULT CreateComponent(int implementationId, int *pId, char* name = "Unnamed Shader Component", DWORD flags = 0);

  //@{
  /** Access method */
  IawShader* GetNext() {return mpNext;}
  IawShaderImplementation* GetFirstImplementation() {return mpFirstImplementation;}
  IawShaderImplementation* GetActiveImplementation() {return mpActiveImplementation;}
  void SetActiveImplementation(IawShaderImplementation* pTemp) {mpActiveImplementation = pTemp;}
  void SetNext(IawShader* pTemp) {mpNext = pTemp;}
  int GetId() {return mShaderId;}
  char* GetName() {return mShaderName;}

  int GetImplementationId(char* name);
  IawShaderImplementation* GetImplementationPtr(int implementationId);
  //@}

  /** Add a child to this shader hierarchy */
  HRESULT AddChild(IawShader* pChildShader);

  // void CheckProperties(CD3DWrapper *pD3DWrapper, bool &b_alpha);
  // bool Validate();
  // bool Activate();
  //  inline int GetNumPasses() {return miNumPasses;}

private:
  char mShaderName[255];
  int  mShaderId;
  int  mNumImplementations;
  int  mNumIds;

  IawShaderImplementation* mpFirstImplementation;
  IawShaderImplementation* mpActiveImplementation;

  IawShader* mpNext;

  HRESULT CheckAgainstChildren(IawShader* pPotentialParent,
                               IawShader* pPotentialChild);
};

#endif // IawShader_h
