// AGP_Performance.h App Wizard Version 2.0 Beta 1
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

#if !defined(AGP_Performance_h)
#define AGP_Performance_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This is the main application class.
 */

//
// Some defines we'll need
//
#define PI 3.14159265358979323846f  // I know, we won't get this much precision from a float :-)

//define options for the scene
#define CAMERA_FOV		(PI * 0.5f)
#define CAMERA_NEARCLIP	0.2f
#define CAMERA_FARCLIP	10.0f
#define CAMERA_POS		IawVector(0, 0, -3 )
#define CAMERA_TARGET	IawVector(0, -0.5, 0)


//
// Number of spheres to display -- this has to be a perfect square (1, 4, 9, 16, ...)
// or the program will crash :-)
//
#define NUM_SPHERES 1

//
// Took this from the DX7 include file because it's not included in DX8
//
typedef struct _D3DVERTEX {
    union {
		D3DVALUE     x;             /* Homogeneous coordinates */
		D3DVALUE     dvX;
    };
    union {
		D3DVALUE     y;
		D3DVALUE     dvY;
    };
    union {
		D3DVALUE     z;
		D3DVALUE     dvZ;
    };
    union {
		D3DVALUE     nx;            /* Normal */
		D3DVALUE     dvNX;
    };
    union {
		D3DVALUE     ny;
		D3DVALUE     dvNY;
    };
    union {
		D3DVALUE     nz;
		D3DVALUE     dvNZ;
    };
    union {
		D3DVALUE     tu;            /* Texture coordinates */
		D3DVALUE     dvTU;
    };
    union {
		D3DVALUE     tv;
		D3DVALUE     dvTV;
    };
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    _D3DVERTEX() { }
    _D3DVERTEX(const D3DVECTOR& v, const D3DVECTOR& n, float _tu, float _tv)
	{ x = v.x; y = v.y; z = v.z;
	nx = n.x; ny = n.y; nz = n.z;
	tu = _tu; tv = _tv;
	}
#endif
#endif /* DIRECT3D_VERSION >= 0x0500 */
} D3DVERTEX, *LPD3DVERTEX;


class AGP_Performance
{
public:
  AGP_Performance();
  virtual ~AGP_Performance();

  virtual HRESULT InitWorld(IawD3dWrapper* pWrapper);
  virtual HRESULT DestroyWorld();
  virtual HRESULT UpdateWorld();
  virtual HRESULT RenderWorld();
  virtual HRESULT TargetChanging();
  virtual HRESULT TargetChanged();
  virtual HRESULT SetupShaders();
  

  void KeyDown(WPARAM wParam, bool bShift);
  void KeyUp(WPARAM wParam);

  void MouseLeftUp(WPARAM wParam, LPARAM lParam);
  void MouseLeftDown(WPARAM wParam, LPARAM lParam);
  void MouseRightUp(WPARAM wParam, LPARAM lParam);
  void MouseRightDown(WPARAM wParam, LPARAM lParam);
  void MouseMove(WPARAM wParam, LPARAM lParam);

  //
  // Stuff for the write combining test
  //
  int mFloatsToTouch;
  int mVertexSkip;
  float mTheta;


private:
  IawD3dWrapper* mpWrapper;

  IawMatrix mWorldMat;
  IawMatrix mProjectionMat;
  IawMatrix mViewMat;

  //objects in the world
  IawObject* mpRootObj;
  IawSphere* mpSphere[NUM_SPHERES];
  float mRadius;

  //object for drawing text in our scene;
  IawTexture* mpFontTexture;
  IawTextMgr *mpTextMgr;
  float mTimeSinceLastFpsUpdate;
  int mNumFrames;

  int mStringId[4];


  ////////////////////////////
  //this is the container that willhold all our shaders
  IawShaderMgr* mpShaderManager;
  ////////////////////////////

  IawTexture* mpTexture;

  int mMousePrevX;
  int mMousePrevY;
};

#endif // AGP_Performance_h

