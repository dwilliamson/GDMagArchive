// IawWindow3d.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawWindow3d_h)
#define IawWindow3d_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IawWindow3d;

/**
 * A list of windows for multi-windowed feature.
 */
struct WndList
{
  HWND hWnd;
  HINSTANCE hInst;
  IawWindow3d* pInst;
  WndList* pNext;
};

/**
 * This class wraps up the details of a Direct3D window in Windows.
 */
class IawWindow3d
{
public:
  /** A static list of windows */
  static WndList* mpWindows;
  
  /** Constructor */
  IawWindow3d();
  
  /** Destructor */
  virtual ~IawWindow3d();
  
  /** Initialize a window for use with Direct3D. */
  bool Init(HINSTANCE hInst, int nCmdShow);
  
  /** Windows message handler */
  LRESULT MessageHandler(UINT Msg, WPARAM wParam, LPARAM lParam);
  
  /** Get the IawD3dWrapper for this window */
  IawD3dWrapper* GetWrapper() { return mpWrapper; }
  
  /** Build an adapter menu */
  bool IawWindow3d::BuildAdapterMenu();
  
  /** Build a device menu */
  bool IawWindow3d::BuildDeviceMenu();
  
  /** Build a mode menu */
  bool IawWindow3d::BuildModeMenu();
  
  /** Set the application */
  void SetApp(AGP_Performance* pApp);
  
  /** Get the application */
  AGP_Performance* GetApp() { return mpApp; }
  
  /** Perform appropriate actions when the render target changes */
  HRESULT TargetChanged();
  
  bool Enable(bool bEnable);
  bool ProcessMessages();
  
private:
  UINT mWindowX, mWindowY;
  UINT mWindowWidth, mWindowHeight;
  WndList mWindow;
  
  IawD3dWrapper* mpWrapper;
  AGP_Performance* mpApp;
  
  bool mIsFullScreen;
  bool mIsActive;
  bool mAnimationIsPaused;
  bool mUseWireFrame;
  bool mIsAlive;
  bool mbShift;
  
  double mCurTime, mLastTime;
  MSG mMsg;
  
  bool SwitchAdapter(int adapter);
  bool SwitchRasterizer(int mode);
  bool SwitchMode(int ID);
};

#endif // !defined(IawWindow3d_h)

