// IawShaderMgr.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawShaderMgr_h)
#define IawShaderMgr_h

#include "IawShader.h" // This header defines the shader, shaderimplementation,
                       // ShaderComponent, and ShaderPrimative classes, which
                       // are only ever seen by this class.

// Should the primitives contain a refcount? upped and dec'd by compenent references?

/**
 * This class is used abstract all the shaders from the application.
 *
 * The shader manager is the class the application instantiates. All other
 * classes (shaders, shader implementations, shader components, and shader
 * primitives) are seen only by the ShaderManager.
 *
 * The ShaderManager manages two things, (a) a pool of ShaderPrimitives (basic
 * elements used to affect the API state), and (b) a hierchy of Shaders,
 * ShaderImplementations and ShaderComponents.
 *
 * <h3>The pool of ShaderPrimitives</h3>
 * The ShaderPrimitive is a class that wraps one of four things:
 * <ul>
 *   <li>A Direct3D StateBlock handle</li>
 *   <li>A Direct3D VertexShader handle</li>
 *   <li>A PixelShader handle</li>
 *   <li>...or an IawMatrix</li>
 * </ul>
 * As such, it is the only class containing device dependant data. Calling the
 * IawShaderMgr::TargetChanged() will delete all of these resources from the
 * device. the primatives are then marked as invalid, and the application is
 * responsible for resetting all their components, through the ShaderManager.
 * The primitive also contains some flags that can be used to help the 
 * ShaderManager and scene manager make decisions, for example whether the
 * primitive renders something using alpha (and thus must be rendered last).
 *
 * <h3>Hierchy of Shaders, ShaderImplementations and ShaderComponents.</h3>
 * A Shader is the basic element used to contain one rendering effect. For
 * example "Brick wall with lightmap". Because a rendering effect may be acheive
 * in more than one way (e.g. multi-texture vs multipass) a Shader may contain
 * one or more ShaderImplementations. The Shader contains info about the current
 * active implementations and primitive.
 *
 * A ShaderImplementation contains a linked list of ShaderComponents. Some of
 * these are flagged to mark the end of the current rendering pass and the
 * start of the next.
 *
 * The ShaderComponent is the container class used to combine ShaderPrimitives
 * to acheive affects. The way this is done is through implementing a chain of
 * ShaderComponents. The ShaderComponent is just a linked list node with a
 * descriptive name a pointer to the ShaderPrimitive it uses. It also contains
 * some flags, which among other things can be used to indicate that the current
 * component is the end of the current rendering pass, and the next will be the
 * beginning of a new rendering pass.
 *
 * <strong>The above is best illustrated with a hierarchy example:</strong>
 *
 * - ShaderManager
 *   (Shader linked list)
 *   - Shader 1 - wireframe with no lighting
 *
 *     - Implementation 1 - (works on all HW)
 *       - ShaderComponent 1
 *       - pNext = Null
 *       - pPrimitive = Primative A (enable wireframe,disable lighting)
 *       - ExitComponent?
 *
 *   - Shader 2 - Brick wall with lightmap
 *
 *     - Implementation 1 - (multitex)
 *       - ShaderComponent 1
 *         - pNext = null
 *         - pPrimitive = Primative B (big stateblock setting up multitex)
 *       - ExitComponent?
 *
 *     - Implementation 2 - (multipass)
 *       - ShaderComponent 1
 *         - pNext = ShaderComponent 2
 *         - pPrimitive = Primative C (stateblock setting 1st pass)
 *       - ShaderComponent 1
 *         - pNext = null
 *         - pPrimitive = Primative D (stateblock setting 2st pass)
 *       - ExitComponent?
 *
 *   - Shader 3 - texgen envmap
 *
 *     - Implementation 1 - (works on all HW)
 *       - ShaderComponent 1
 *         - pNext = ShaderComponent 2
 *         - pPrimitive = Primative E (stateblock enabling envmap, turning on texgen)
 *       - ShaderComponent 2
 *         - pNext = null
 *         - pPrimitive = Primative F (sets matrix for texgen)
 *       - ExitComponent?
 *
 * (Primitive linked list)
 * - Primitive A (stateblock)
 * - Primitive B (stateblock)
 * - Primitive C (stateblock)
 * - Primitive D (stateblock)
 * - Primitive E (stateblock)
 * - Primitive F (matrix)
 *
 */
class IawShaderMgr
{
  public:
  /**
   * Constructor.
   * @param name A name for the shader manager.
   */
  IawShaderMgr(char* name = "Unnamed Shader Manager");

  /** Destructor */
  virtual ~IawShaderMgr();

  /**
   * Create a shader.
   * @param pId An integer ID to be assigned to the new shader, and returned.
   * @param name A name for the new shader.
   */
  HRESULT CreateShader(int* pId, char* name = "Unnamed Shader");

  /**
   * Delete a specified shader.
   * @param Id Integer ID for the shader to delete.
   */
  HRESULT DeleteShader(int Id);

  /**
   * Create a shader implementation
   * @param shaderID shader ID.
   * @param pId Shader implementation Id to be returned.
   * @param name A name for the new shader implementation.
   */
  HRESULT CreateShaderImplementation(int shaderId,
                                     int* pId,
                                     char* name = "Unnamed Shader Implementation");

  /** Delete a shader implementation */
  HRESULT DeleteShaderImplementation(int shaderId, int Id);

  /** Create a shader component */
  HRESULT CreateShaderComponent(int shaderId,
                                int ImplementationId,
                                int* pId,
                                char* name = "Unnamed Shader Component",
                                DWORD flags = 0);

  /** Create a negating component */
  HRESULT CreateNegatingComponent(int shaderId,
                                  int ImplementationId,
                                  int* pId,
                                  char* name = "Unnamed Negating Component",
                                  DWORD flags = 0);

  //HRESULT DeleteComponent(int shaderId, int ImplementationId, int Id);

  /** Create a shader element */
  HRESULT CreateElement(int* pId, char* name = "Unnamed Shader Primative");

  /** Delete a shader element */
  HRESULT DeleteElement(int Id);

  /** Set an element in a shader component */
  HRESULT SetComponentElement(int shaderId, int implementationId, int componentId, int elementId);

  /** Set a shader element's state block */
  HRESULT SetElementStateBlock(char* elementName, DWORD stateBlock);

  /* note, we don't delete primitives, they are auto-deleted when no more components reference
  them */

  //@{
  /** Access method */
  int GetShaderId(char* name);
  int GetImplementationId(int shaderId, char* name);
  //  int GetComponentId(int shaderId, int ImplementationId, char* name);
  int GetElementId(char* name);
  char* GetShaderName(int shaderId);

  //  int GetImplementationNumPasses(int shaderId, int ImplementationId);

  int GetActiveShaderId() {return mActiveShaderId;}
  //  int GetActiveImplementationId();
  //  int GetActiveComponentId();
  //  int GetActivePrimitiveId();
  //  int GetCurrentRenderPass();

  //  int GetImplementationNumPasses(int shaderId, int ImplementationId);
  int GetNumPasses(int shaderId);

  // Sets the shader->implementation->component to point to the primitive indicated by 
  //  HRESULT   SetShaderComponent(int shaderId, int ImplementationId, 
  //                    int ComponentId, int NewPrimitiveId,
  //                    DWORD flags);

  /*
  HRESULT SetShaderComponent(int shaderId, int ImplementationId, 
  int ComponentId, DWORD dwCompType,
  DWORD dwComponentValue);

    HRESULT SetShaderComponent(int shaderId, int ImplementationId, 
    int ComponentId,  DWORD dwCompType, 
    IawMatrix dwMatrix);
  */
  //@}

  /** Activate a shader */
  HRESULT ActivateShader(IawD3dWrapper* pWrapper, int shaderId, int pass);

  /** Deactivate a shader */
  HRESULT DeActivateShader(IawD3dWrapper* pWrapper, int shaderId, int pass);

  /** Validating */
  //HRESULT  ValidateShaderImplementation(int shaderId, int ShaderImplementation);
  HRESULT SetShaderActiveImplementation(int shaderId, int ImplementationId);

  /** Cleanup */
  HRESULT  TargetChanging(IawD3dWrapper* pWrapper);

  //for debugging. Don't know if I'll leave it in
  void  PrintShaderHierarchy(); /**< Used in debugging */

  private:
  //Initialization
  // Reset ALL D3D States to defaults
  //  HRESULT  InitDefaultD3DState();

  // Pointer retreival
  IawShader* GetShaderPtr(int shaderId);
  //IawShader *GetImplementationPtr(shaderId, ImplementationId);
  //IawShader *GetComponentPtr(shaderId, ImplementationId, ComponentId);
  IawShaderElement* GetElementPtr(int elementId);

  char mShaderMgrName[255];
  int mNumShaders; //used to track how many shaders exist
  int mUsedShaderIds; //used to track how many have existed, no two have same Id
  int mNumElements; //used to track how many elements exist
  int mUsedElementIds; //used to track how many have existed, no two have same Id

  IawShader* mpActiveShader;
  //  CIAWShaderImplementation* mpActiveImplementation;
  //  CIAWShaderComponent* mpActiveComponent;
  //  CIAWShaderPrimitive* mpActivePrimitive;

  IawShaderElement* mpCurrentActiveElement;

  int mActiveShaderId;
  //  CIAWShaderImplementationId* mpActiveImplementationId;
  //  CIAWShaderComponentId* mpActiveComponentId;
  //  CIAWShaderPrimitiveId* mpActivePrimitiveId;

  //int m_iCurrectActiveShaderPass;

  IawShader* mpFirstShader;
  IawShaderElement* mpFirstElement;
};


#endif // IawShaderMgr_h


