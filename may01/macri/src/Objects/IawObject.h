// IawObject.h App Wizard Version 2.0 Beta 1
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
// IawObject.h: declaration of the CIawObject class.
// This class is used to contain an object, it's vertices, matrices, etc.
// The class stores two copies of the vertices, an array of D3DVerts, and a Vertex
// buffer used for rendering. At the time this class was authored, it was unclear
// whether the array was needed in case of a lost vertex buffer (DX7 beta docs unclear)
// so there may be some memory savings to be made there.
//
// The intent is to use this as a base class for deriving other objects, like procedural
// ones, or ones loaded from specific file types
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#if !defined(IawObject_h)
#define IawObject_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULTFVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

/**
 * This class is used to contain an object.
 * This class is used to contain an object, it's vertices, matrices, etc. It
 * stores two copies of the vertices, an array of D3DVerts, and a Vertex buffer
 * used for rendering. At the time this class was authored, it was unclear
 * whether the array was needed in case of a lost vertex buffer (DX7 beta docs
 * unclear) so there may be some memory savings to be made there.
 *
 * The intent is to use this as a base class for deriving other objects, like
 * procedural ones, or ones loaded from specific file types.
 */
class IawObject
{
public:
  /**
   * Constructor.
   * @param flags Flexible Vertex Format flags for the object.
   */
  IawObject(DWORD flags = DEFAULTFVF);

  /**
   * virtual destructor because this is a parent class.
   */
  virtual ~IawObject();

  /**
   * Initialize an object class instance.
   * If the vertices and indices are not null this copies the vertices and
   * indices from the referenced locations.
   */
  HRESULT Init(IawD3dWrapper* pWrapper,
               float* pVerts = NULL,
               WORD* pIndices = NULL,
               DWORD numVerts = 0,
               DWORD numIndices = 0,
               DWORD flags = 0);
/*
               IawVector    pos           = IawVector(0.0f),
               IawVector    scale         = IawVector(1.0f),
               IawVector    rotationAxis  = IawVector(0.0f,1.0f,0.0f),
               float        rotationAngle = 0.0f,
               IawVector    velocity      = IawVector(0.0f),
               IawVector    scaleVelocity = IawVector(0.0f),
               float        rotationalVel = 0.0f,
               DWORD        flags         = 0);
*/

  /**
   * Load the vertices and indices from a .X file, and build a hierarchy of
   * IawObjects from the file info.
   */
  HRESULT Init(IawD3dWrapper* pWrapper, char* strFileName);
/*
               IawVector    pos           = IawVector(0.0f),
               IawVector    scale         = IawVector(1.0f),
               IawVector    rotationAxis  = IawVector(0.0f,1.0f,0.0f),
               float        rotationAngle = 0.0f,
               IawVector    velocity      = IawVector(0.0f),
               IawVector    scaleVelocity = IawVector(0.0f),
               float        rotationalVel = 0.0f,
               DWORD        flags         = 0);
*/

  /**
   * Load the vertices and indices of another object.
   */
  HRESULT Init(IawObject* pObject = NULL);
/*
               IawVector  pos           = IawVector(0.0f),
               IawVector  scale         = IawVector(1.0f),
               IawVector  rotationAxis  = IawVector(0.0f,1.0f,0.0f),
               float      rotationAngle = 0.0f,
               IawVector  velocity      = IawVector(0.0f),
               IawVector  scaleVelocity = IawVector(0.0f),
               float      rotationalVel = 0.0f,
               DWORD      flags         = 0);
*/

  /**
   * Update a frame.
   * This updates all the vectors, and the local to world matrix according 
   * to a timestep, where 1.0 is the number of seconds elapsed since the 
   * last frame.  For example, if a velocity of '6' means that the object
   * should move 6 world units in one second, when the app is running at 
   * 50fps, the timestep value will be 1/50 or 0.02. This is then taken into
   * account when updating the matrices.
   */
  virtual HRESULT Update(float frameTime, bool updateChildren = true);

  //@{
  /**
   * This actually draws the primitives to the render target.
   *
   * <b>This is used when you are not using the display list manager</b>
   */
  // old function
  virtual HRESULT Render(IawMatrix& rWorldMat, bool renderChildren = true);
  virtual HRESULT Render(IawMatrix& rWorldMat,
                         IawObject* pCamera,
                         bool renderChildren = true);
  //@}
  //@{
  /**
   * This needs to be called anytime there is a change to the D3DDevice.
   * For example, call this when switching to/from full screen.
   */
  virtual HRESULT TargetChanging(IawD3dWrapper* pWrapper,
                                 bool updateChildren = true);

  /**
   * Ensure non-circular hierarchy.
   * Checks to make sure the object you want to make a parent of another, 
   * isn't already one of that objects children. Thou shalt not murder the
   * stack!
   */
  virtual HRESULT TargetChanged(IawD3dWrapper* pWrapper,
                                IawShaderMgr* pShaderMgr,
                                bool updateChildren = true);
  //@}

  /**
   * Add a child to the object hierarchy.
   */
  HRESULT AddChild(IawObject* pChildObject);

  /**
   * Delete a child from the object hierarchy.
   */
  HRESULT RemoveChild(IawObject* pChildObject/*, bool bRemoveChildren = true*/);

  /** Get the triangle count of the object. */
  virtual int GetTriangleCount();

  /**
   * Recursively traverse object hierarchy concatinating matrices pre-render
   * call.
   */
  void SetLocalToWorldMx(IawMatrix& rWorldMat, bool renderChildren = true);

  /**
   * Set object shader.
   * Any child objects with null shaders inherit parent's shader.
   */
  HRESULT SetShaderID(int shader, bool setChildShaderIds =true);

  //@{
  /** Access method */
  void SetPos(IawVector pos);
  void SetScale(IawVector scale);
  void SetRotationAxis(IawVector RA);
  void SetRotationAngle(float RA);
  void SetRotationalVelocity(float RV);
  void SetScalingVelocity(IawVector SV);
  void SetVelocity(IawVector Vel);

  void SetObjectName(char* name)       { sprintf(mStrName,name); }
  
  void SetObjectType(DWORD dwType)       { mObjectType        = dwType;}
  
  void SetVisible(bool bVis)       { mVisible        = bVis; }
  void SetNext(IawObject* pNext)   { mpNext         = pNext;}
  void SetFirst(IawObject* pFrst)    { mpFirstChild     = pFrst;}
  void SetParent(IawObject* pPrnt)   { mpParent       = pPrnt;}
  void SetNumVerts(int NumVerts)     { mNumVertices     = NumVerts;}
  void SetNumIndices(int NumIndices)   { mNumIndices  = NumIndices;}
  void SetWrapper(IawD3dWrapper* pWrapper) { mpWrapper      = pWrapper;}
  void SetShaderMgr(IawShaderMgr* pShaderMgr){ mpShaderMgr = pShaderMgr;}
  void SetShared(bool bShare)      { mSharedVb       = bShare;}
  //  HRESULT SetShader(CDRGShader* pShader, bool SetChildShaders =true);
  void SetFVF(DWORD dwFlags)       { mFVF       = dwFlags;}
  void SetPrimType(D3DPRIMITIVETYPE PT) { mPrimType       = PT;}
  void AllocVertMem(int Size)    { SAFE_DELETE_ARRAY(mpVertices); mpVertices = new float[Size];}
  void AllocIndicesMem(int Size)   { SAFE_DELETE_ARRAY(mpIndices); mpIndices = new  WORD[Size];}
  void DeleteVertMem()         { if(mpVertices) delete [] mpVertices;  }
  void DeleteIndicesMem()        { if(mpIndices)   delete [] mpIndices;  }
  
  void SetVerticesPtr(float* vbptr)  { mpVertices  = vbptr;      }
  void SetIndicesPtr(WORD* iptr)   { mpIndices  = iptr;       }
  void SetIndexArray(WORD* indices)  { memcpy(mpIndices, indices, mNumIndices * sizeof(WORD));}
  
  IawVector  GetPos()          { return mPosition;       }
  IawVector  GetScale()          { return mScale;        }
  IawVector  GetRotationAxis()     { return mRotationAxis;     }
  float    GetRotationAngle()      { return mRotationAngle;    }
  float    GetRotationalVelocity()   { return mRotationalVelocity; }
  IawVector  GetScalingVelocity()    { return mScalingVelocity;    }
  IawVector  GetVelocity()       { return mVelocity;       }
  bool     GetVisible()        { return mVisible;        }
  IawObject* GetNext()         { return mpNext;         }
  IawObject* GetFirst()          { return mpFirstChild;     }
  IawObject* GetParent()         { return mpParent;       }
  int      GetNumVerts()       { return mNumVertices;      }
  int      GetNumIndices()       { return mNumIndices;     }
  IawD3dWrapper* GetWrapper()        { return mpWrapper;        }
  IawShaderMgr* GetShaderMgr()      { return mpShaderMgr;      }
  bool     GetShared()         { return mSharedVb;        }
  int      GetShaderID()       { return mShaderId;       }
  DWORD    GetFVF()          { return mFVF;         }
  float*   GetVerticesPtr()      { return mpVertices;      }
  WORD*    GetIndicesPtr()       { return mpIndices;        }
  DWORD    GetVertexSize()       { return mVertexSize;      }
  char*    GetObjectName()       { return mStrName;       }
  DWORD    GetObjectType()       { return mObjectType;      }
  
  LPDIRECT3DVERTEXBUFFER8 GetVBptr()    { return mpIndexedVertexBuffer;}
  LPDIRECT3DINDEXBUFFER8 GetIBptr()    { return mpIndexBuffer;}
  D3DPRIMITIVETYPE     GetPrimType() { return mPrimType;     }
  
  D3DMATRIX* GetLocalToWorldD3DMx()  { return &mLocalToWorldMx.mMx; }
  IawMatrix& GetLocalToWorldMx()   { return mLocalToWorldMx; }
  IawMatrix& GetLocalToParentMx()    { return mLocalToParentMx; }
  //@}

  //@{
  /** Class constant */
  // Object types...
  static const int IAW_OBJECT_TYPE_BASEOBJECT;
  static const int IAW_OBJECT_TYPE_BACKGROUND;
  static const int IAW_OBJECT_TYPE_CONE;
  static const int IAW_OBJECT_TYPE_CUBE;
  static const int IAW_OBJECT_TYPE_CYLINDER;
  static const int IAW_OBJECT_TYPE_PLANE;
  static const int IAW_OBJECT_TYPE_SKYPLANE;
  static const int IAW_OBJECT_TYPE_SPHERE;
  static const int IAW_OBJECT_TYPE_TERRAIN;
  static const int IAW_OBJECT_TYPE_STRING;
  static const int IAW_OBJECT_TYPE_XOBJECT;
  static const int IAW_OBJECT_TYPE_GNOMON;
  static const int IAW_OBJECT_TYPE_CAMERA;

  static const int IAW_OBJECT_TYPE_OBJMANAGER;
  static const int IAW_OBJECT_TYPE_TEXTMGR;

  // have to convert this to be a derivative of object still.
  //static const int IAW_OBJECT_TYPE_TERRAINMGR;

  // Object loading directives...
  static const int OBJECT_LOAD_FILE;
  static const int OBJECT_LOAD_URL;

  // Update flags...
  static const int IAW_UF_POSITION;
  static const int IAW_UF_SCALE;
  static const int IAW_UF_ROTATION;
  static const int IAW_UF_ROTVEL;
  static const int IAW_UF_SCALEVEL;
  static const int IAW_UF_VELOCITY;
  static const int IAW_UF_INVALIDMX;
  //@}
protected:
  /** Set up a D3D Vertex Buffer using the current device. */
  virtual HRESULT SetupIndexedVertexBuffer();

  /** Set up an index buffer to index into the vertex buffer for rendering. */
  virtual HRESULT SetupIndexBuffer();

  /**
   * This is a safety check to make sure someone doesn't try to add an object
   * as it's own child (prevent a circular hierarchy).
   */
  HRESULT CheckAgainstChildren(IawObject* pPotentialParent, IawObject* pPotentialChild);

  /** Determine the number of bits necessary for one vertex based on mFVF */
  void SetVertexSize();
  //int GetVertexSize(DWORD dwFVFflags);

  /**
   * Write associated SuperVertx fields to memory at pVertexLoc based on
   * mFVF
   */
  void SetVertex(float*& prVertexLoc, IAW_SUPERVERTEX SV);

  /** Set the world matrix to the local matrix. */
  void SetWorldMxToLocalMx(IawMatrix& drgMxWorld)
  { mLocalToWorldMx =  drgMxWorld*  mLocalToParentMx; }

private:
  DWORD mObjectType;

  char  mStrName[255];
  LPDIRECT3DVERTEXBUFFER8 mpIndexedVertexBuffer;
  LPDIRECT3DINDEXBUFFER8 mpIndexBuffer;

  D3DPRIMITIVETYPE    mPrimType;

  // Beginning offset of any geometry before applying transforms.
  // This is initialized to an identity matrix but might be different when
  // loading a .x file (if the default in the file was other than that)
  IawMatrix mLocalBaseMx;

  // Local transformation matrix, concatenation of the translation, scale, &
  // rotation
  IawMatrix mLocalToParentMx;

  // This gets updated at each frame, as the objects
  // current tX,Rt * the parent's world MX
  IawMatrix mLocalToWorldMx;

  // current position, scale, rotation
  IawVector mPosition;
  IawVector mScale;
  IawVector mRotationAxis;
  float   mRotationAngle;

  // current velocity, rotational velocity, scale velociy
  float   mRotationalVelocity;
  IawVector mScalingVelocity;
  IawVector mVelocity;

  // variable to allow making the object visible without removing from scene
  bool    mVisible;

  // pointer to siblings and children, if object is part of a hierarchy
  IawObject* mpNext;
  IawObject* mpFirstChild;
  IawObject* mpParent;

  // System memory copy of vertex information
  int    mNumVertices;
  int    mNumIndices;
  float* mpVertices;
  WORD*  mpIndices;

  IawD3dWrapper* mpWrapper;
  IawShaderMgr* mpShaderMgr;
  int mShaderId;

  DWORD mFVF;        // Flexible Vertex Format flags
  DWORD mVertexSize; // Size of one vertex (in bits)
  bool mSharedVb;     // Defaults to false; set to true if overloaded
  // Init (3) is used in construction

  DWORD mUpdateStatus; // bitflags that represent matrix validity

  //copy the verts and indices to the member vars for them
  virtual HRESULT SetupVertices(float* pVertices = NULL);
  virtual HRESULT SetupIndices(WORD* pVertexIndices);

  //added to attempt .x file loading
  HRESULT LoadObject(char* szSource, IawObject* pRoot, DWORD dwFlags = OBJECT_LOAD_FILE);

  // these are all used by the LoadObject function
  HRESULT ParseFrame(LPDIRECTXFILEDATA pFileData, IawObject* pParentFrame);
  HRESULT ParseMesh(LPDIRECTXFILEDATA pFileData, IawObject* pParentFrame);
  HRESULT ParseMeshMaterialList(LPDIRECTXFILEDATA pFileData, IawObject* pObject);
  HRESULT ParseMaterial(LPDIRECTXFILEDATA pFileData, IawObject* pObject, DWORD dwMaterial);
};


#endif // if !defined(IawObject_h)
