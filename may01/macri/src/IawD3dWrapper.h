// IawD3DWrapper.h App Wizard Version 2.0 Beta 1
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
// IawD3DWrapper.h: Declaration of the IawD3dWrapper class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#if !defined(IawD3dWrapper_h)
#define IawD3dWrapper_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Data structure to store an entry in a list of display modes
 */
struct IawDisplayMode
{
  UINT mModeNum;
  //char      strDesc[MAX_DEVICE_IDENTIFIER_STRING];
  D3DDISPLAYMODE mD3dDisplayMode;
  IawDisplayMode *mpNext;
};

/**
 * Data structure to store the adapters found in a list
 */
struct IawDisplayAdapter
{
  UINT mAdapterNum;
  char mStrDesc[MAX_DEVICE_IDENTIFIER_STRING];
  D3DADAPTER_IDENTIFIER8 mD3dAdapterIdentifier8;
  IawDisplayMode* mpFirstDisplayMode;
  IawDisplayAdapter* mpNext;
};

/**
 * A Direct3D wrapper class.
 */
class IawD3dWrapper
{
public:
  IDirect3DDevice8* mpDevice;
  IDirect3D8* mpDirectX8;
  
  UINT mCurrentAdapterNumber;
  UINT mCurrentMode;
  D3DDEVTYPE mD3dDeviceType;
  D3DFORMAT mD3dFormat;
  DWORD mVertexProcessingType;

  /**
   * Initialize the device.
   * @param adapterNumber Which adapter this device belongs to.
   * @param mode Which mode this device controls.
   * @param fullScreen If set to true this device will use full screen mode.
   * @param x x coordinate of upper left hand corner of window
   * @param y y coordinate of upper left hand corner of window
   * @param bpp Bits Per Pixel
   * @param backBufferCount Number of back buffers to use
   * @param useZBuffer If set to true a Z-buffer will be set up and used by this device.
   */
  HRESULT Initialize(int adapterNumber,
                     int mode,
                     bool fullScreen,
                     UINT x,
                     UINT y,
                     UINT width,
                     UINT height,
                     DWORD bpp = 16,
                     DWORD backBufferCount = 1,
                     bool useZBuffer = true);

  /**
   * Change the device options.
   * @param deviceType The type of device to change to
   * @param vertexProcessingType The type of vertex processing to switch to.
   */
  HRESULT ChangeDeviceOptions(D3DDEVTYPE deviceType, DWORD vertexProcessingType);

  bool Initialized() { return mInitialized; }

  /**
   * Constructor.
   * Build a linked list of devices and modes.
   */
  IawD3dWrapper(HWND hWnd);

  /** Destructor */
  virtual ~IawD3dWrapper();

  /** Clear */
  void Clear(DWORD targets);

  /** Begin a frame */
  void StartFrame();

  /** End a frame */
  void EndFrame();

  /** Resize */
  HRESULT Resize();

  //@{
  /** Access function. */
  int GetNumAdapters();
  IawDisplayAdapter* GetAdapter(int adapterNum);
  int GetNumModes();
  IawDisplayMode* GetMode(int modeNum);

  // Timer functions
  double GetTime();
  void SetFrameTime(float frameTime);
  float GetFrameTime() { return mFrameTime; }

  DWORD GetWidth() { return mWidth; }
  DWORD GetHeight() { return mHeight; }
  //@}

private:
  D3DPRESENT_PARAMETERS mD3dPp;
  bool mInitialized;
  bool mHighPerf;
  double mFreq;
  float mFrameTime;
  DWORD mColBackground;
  bool mFullScreen;
  DWORD mBpp;
  HWND mRenderWindow;
  DWORD mWindowWidth;
  DWORD mWindowHeight;
  DWORD mWidth;
  DWORD mHeight;
  DWORD mX;
  DWORD mY;

  DWORD mBackBufferCount;
  bool mUseZBuffer;

  // List of display adapters, each of which contains a list of modes
  IawDisplayAdapter* mpFirstDisplayAdapter;
};

#endif // IawD3dWrapper_h
