// IawWindow3d.cpp App Wizard Version 2.0 Beta 1
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
// IawWindow3d.cpp: implementation of the CIawWindow3d class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "stdafx.h"
#include "DxErr8.h"
#pragma comment(lib,"DxErr8")

// Init static member variable
WndList* IawWindow3d::mpWindows = NULL;

// This is a global message handler routine for dispatching messages to the appropriate
// instance of the IawWindow3d class
LRESULT WINAPI g_MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  WndList *pWindows = IawWindow3d::mpWindows;
  while (pWindows)
  {
    if (pWindows->hWnd == hWnd)
      return pWindows->pInst->MessageHandler(Msg, wParam, lParam);
    pWindows = pWindows->pNext;
  }
  
  return DefWindowProc(hWnd, Msg, wParam, lParam);
}

IawWindow3d::IawWindow3d()
{
  mWindowX = 50;
  mWindowY = 50;
  mWindowWidth = 400;
  mWindowHeight = 300;
  
  mpWrapper = NULL;
  mIsFullScreen = false;
  mWindow.hWnd = 0;
  mWindow.pInst = NULL;
  mWindow.pNext = NULL;
  
  mpApp = NULL;
  mIsActive = false;
  mAnimationIsPaused = false;
  mUseWireFrame = false;
  mIsAlive = false; 
  mbShift = false;
}

IawWindow3d::~IawWindow3d()
{
  WndList* windows_ptr = IawWindow3d::mpWindows;
  WndList* prev_ptr = NULL;
  
  while (windows_ptr)
  {
    if (windows_ptr->pInst == this)
    {
      if (prev_ptr)
      {
        prev_ptr->pNext = windows_ptr->pNext;
      }
      else
        mpWindows = windows_ptr->pNext;
      break;
    }
    prev_ptr = windows_ptr;
    windows_ptr = windows_ptr->pNext;
  }
}

bool IawWindow3d::Init(HINSTANCE hInst, int nCmdShow)
{
  WNDCLASSEX classex; 
  
  //
  // Register a Window Class
  ZeroMemory(&classex, sizeof(classex));
  classex.cbSize = sizeof(WNDCLASSEX);
  classex.style = CS_CLASSDC;
  classex.lpfnWndProc = g_MessageHandler;
  classex.hInstance = hInst;
  classex.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
  classex.lpszClassName = "Dx8Shell";
  classex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  classex.hCursor = LoadCursor(NULL, IDC_ARROW);
  ATOM wndclass = RegisterClassEx(&classex);
  mWindow.hWnd = CreateWindow("Dx8Shell", "AGP Write Combining Performance Tester", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    mWindowWidth, mWindowHeight,
    NULL, NULL, hInst, NULL);
  // Return false if window creation failed
  if (!mWindow.hWnd) 
    return FALSE;
  
  //
  // Put the window in the list of windows so it can get messages dispatched appropriately
  //
  mWindow.pInst = this;
  mWindow.hInst = hInst;
  mWindow.pNext = mpWindows;
  mpWindows = &mWindow;
  
  //
  // Determine where the window was created
  //
  RECT rc;
  GetWindowRect(mWindow.hWnd, &rc);
  mWindowX = rc.left;
  mWindowY = rc.top;
  
  //
  // Say that we're alive
  //
  mIsAlive = true;
  
  //
  // Show and update the window
  //
  ShowWindow(mWindow.hWnd, SW_SHOW);
  UpdateWindow(mWindow.hWnd);
  
  
  //
  // Create the Direct3D device and attach it to this window
  //
  mpWrapper = new IawD3dWrapper(mWindow.hWnd);
  
  mIsFullScreen = false;
  HRESULT hr = mpWrapper->Initialize(mpWrapper->mCurrentAdapterNumber, 
    mpWrapper->mCurrentMode, 
    mIsFullScreen, 
    mWindowX, mWindowY, 
    mWindowWidth, mWindowHeight);
  
  if (FAILED(hr))
  {
    char szMsg[100];
    sprintf(szMsg, "3D Device failed to initialize.\r\nError: %s\r\n\r\n"
				   "This demo requires a graphics card that supports\r\n"
				   "hardware transformation & lighting.", DXGetErrorString8(hr) );
    MessageBox(mWindow.hWnd, szMsg, "Initialize 3D Device", MB_ICONEXCLAMATION | MB_OK);
    DestroyWindow(mWindow.hWnd);
	return false;
  }
  else
  {
    //
    // Build some menus
    //
    if (!mIsFullScreen)
    {
      if (!BuildAdapterMenu())
      {
        MessageBox(NULL, "Couldn't create the device menu!", "DX8Shell", MB_ICONEXCLAMATION | MB_OK);
	    DestroyWindow(mWindow.hWnd);
        return false;
      }
      
      if (!BuildDeviceMenu())
      {
        MessageBox(NULL, "Couldn't create the mode menu!", "DX8Shell", MB_ICONEXCLAMATION | MB_OK);
	    DestroyWindow(mWindow.hWnd);
        return false;
      }   
      
      if (!BuildModeMenu())
      {
        MessageBox(NULL, "Couldn't create the mode menu!", "DX8Shell", MB_ICONEXCLAMATION | MB_OK);
	    DestroyWindow(mWindow.hWnd);
        return false;
      }
    }
  }
  
  //
  // Initialize stuff for the message processing
  //
  ZeroMemory(&mMsg, sizeof(mMsg));
  
  return true;
}

bool IawWindow3d::BuildAdapterMenu()
{
  int num_devices = mpWrapper->GetNumAdapters();
  MENUITEMINFO mii;
  
  HMENU hMenu = GetMenu(mWindow.hWnd);
  hMenu = GetSubMenu(hMenu, 1); // Get the second sub-menu (Assumes File then Adapter)
  DeleteMenu(hMenu, IDM_ADAPTER_ADAPTER0, MF_BYCOMMAND); // Delete the first menu item
  
  memset(&mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_CHECKMARKS | MIIM_ID | MIIM_STATE | MIIM_TYPE;
  mii.fType = MFT_STRING;
  mii.fState = MFS_ENABLED | MFS_UNCHECKED | MFS_UNHILITE;
  
  IawDisplayAdapter* adapter_info;
  
  for (int i=0; i<num_devices; i++)
  {
    adapter_info = mpWrapper->GetAdapter(i);
    mii.wID = IDM_ADAPTER_ADAPTER0 + i;
    mii.dwTypeData = adapter_info->mStrDesc;//pMenuDescription;
    InsertMenuItem(hMenu, i, TRUE, &mii);
  }
  
  CheckMenuItem(hMenu,
    IDM_ADAPTER_ADAPTER0 + mpWrapper->mCurrentAdapterNumber,
    MF_BYCOMMAND | MF_CHECKED );
  
  return true;
}

bool IawWindow3d::BuildDeviceMenu()
{
  int iNumDevices = mpWrapper->GetNumAdapters();
  
  HMENU hMenu = GetMenu(mWindow.hWnd);
  hMenu = GetSubMenu(hMenu, 2); // Get the second sub-menu (Assumes File then Adapter)
  
  D3DDISPLAYMODE  d3d_display_mode;
  
  mpWrapper->mpDirectX8->GetAdapterDisplayMode(mpWrapper->mCurrentAdapterNumber,
    &d3d_display_mode);
  
  HRESULT hr;
  
  EnableMenuItem(hMenu, IDM_DEVICE_NULL,      MF_GRAYED);
  EnableMenuItem(hMenu, IDM_DEVICE_SWRASTERIZER, MF_GRAYED );
  EnableMenuItem(hMenu, IDM_DEVICE_HALSW,   MF_GRAYED );
  EnableMenuItem(hMenu, IDM_DEVICE_HALMIXED,    MF_GRAYED );
  EnableMenuItem(hMenu, IDM_DEVICE_HALHW,   MF_GRAYED );
  EnableMenuItem(hMenu, IDM_DEVICE_HALPURE,   MF_GRAYED );
  EnableMenuItem(hMenu, IDM_DEVICE_REFRAST,   MF_GRAYED );
  
  /*
  if (D3D_OK == mpWrapper->mpDirectX8->CheckDeviceType(mpWrapper->mCurrentAdapterNumber, 
  D3DDEVTYPE_NULL,
  d3d_display_mode.Format))
    EnableMenuItem(hMenu, IDM_DEVICE_NULL, MF_ENABLED );
    */
  
  if (D3D_OK == mpWrapper->mpDirectX8->CheckDeviceType(mpWrapper->mCurrentAdapterNumber, 
    D3DDEVTYPE_SW ,
    d3d_display_mode.Format,
    d3d_display_mode.Format,
    false))
    EnableMenuItem(hMenu, IDM_DEVICE_SWRASTERIZER, MF_ENABLED );
    
  if (D3D_OK == mpWrapper->mpDirectX8->CheckDeviceType(mpWrapper->mCurrentAdapterNumber, 
    D3DDEVTYPE_HAL ,
    d3d_display_mode.Format,
    d3d_display_mode.Format,
    false))
  {
    //now we have some additional checking to do in this case
    D3DCAPS8  d3d_caps_8;
    hr = mpWrapper->mpDirectX8->GetDeviceCaps(mpWrapper->mCurrentAdapterNumber,
      D3DDEVTYPE_HAL ,
      &d3d_caps_8);
    
    if (d3d_caps_8.DevCaps & D3DDEVCAPS_HWRASTERIZATION)
      EnableMenuItem(hMenu, IDM_DEVICE_HALSW, MF_ENABLED );
    
    if (d3d_caps_8.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    {
      EnableMenuItem(hMenu, IDM_DEVICE_HALMIXED, MF_ENABLED );
      EnableMenuItem(hMenu, IDM_DEVICE_HALHW, MF_ENABLED );
    }
    if (d3d_caps_8.DevCaps & D3DDEVCAPS_PUREDEVICE)
      EnableMenuItem(hMenu, IDM_DEVICE_HALPURE, MF_ENABLED );
  }
  if (D3D_OK == mpWrapper->mpDirectX8->CheckDeviceType(mpWrapper->mCurrentAdapterNumber, 
    D3DDEVTYPE_REF ,
    d3d_display_mode.Format,
    d3d_display_mode.Format,
    false))
    EnableMenuItem(hMenu, IDM_DEVICE_REFRAST, MF_ENABLED );
  
  
  ///////////////////////////
  // uncheck all menu items
  CheckMenuItem(hMenu, IDM_DEVICE_SWRASTERIZER, MF_BYCOMMAND | MF_UNCHECKED );
  CheckMenuItem(hMenu, IDM_DEVICE_REFRAST, MF_BYCOMMAND | MF_UNCHECKED );
  
  
  ////////////////////////////////
  // check the appropriate menuitem
  
  if (mpWrapper->mD3dDeviceType == D3DDEVTYPE_SW)
    CheckMenuItem(hMenu, IDM_DEVICE_SWRASTERIZER, MF_BYCOMMAND | MF_CHECKED );
  
  if (mpWrapper->mD3dDeviceType == D3DDEVTYPE_HAL)
  {
    if (mpWrapper->mVertexProcessingType & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
      CheckMenuItem(hMenu, IDM_DEVICE_HALSW, MF_BYCOMMAND | MF_CHECKED );
    
    if (mpWrapper->mVertexProcessingType & D3DCREATE_MIXED_VERTEXPROCESSING)
      CheckMenuItem(hMenu, IDM_DEVICE_HALMIXED, MF_BYCOMMAND | MF_CHECKED );
    
    if (mpWrapper->mVertexProcessingType & D3DCREATE_HARDWARE_VERTEXPROCESSING)
      CheckMenuItem(hMenu, IDM_DEVICE_HALHW, MF_BYCOMMAND | MF_CHECKED );
    
    if (mpWrapper->mVertexProcessingType & D3DCREATE_PUREDEVICE)
      CheckMenuItem(hMenu, IDM_DEVICE_HALPURE, MF_BYCOMMAND | MF_CHECKED );
    
  }
  
  if (mpWrapper->mD3dDeviceType == D3DDEVTYPE_REF)
    CheckMenuItem(hMenu, IDM_DEVICE_REFRAST, MF_BYCOMMAND | MF_CHECKED );
  
  
  return true;
}


bool IawWindow3d::BuildModeMenu()
{
  int num_modes = mpWrapper->GetNumModes();
  MENUITEMINFO mii;
  
  HMENU hMenu = GetMenu(mWindow.hWnd);
  hMenu = GetSubMenu(hMenu, 3); // Get the second sub-menu (Assumes File, Adapter, Device, then Mode)
  
  int iItems = GetMenuItemCount(hMenu);
  int i;
  for (i=2; i<iItems; i++)
    DeleteMenu(hMenu, 2, MF_BYPOSITION); 
  
  
  memset(&mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_CHECKMARKS | MIIM_ID | MIIM_STATE | MIIM_TYPE;
  mii.fType = MFT_STRING;
  mii.fState = MFS_ENABLED | MFS_UNCHECKED | MFS_UNHILITE;
  
  IawDisplayMode* mode_info_ptr;
  char str[255];
  
  for (i=0; i<num_modes; i++)
  {
    mode_info_ptr = mpWrapper->GetMode(i);
    mii.wID = IDM_MODE_FULL0 + i;
    
    char strfmt[255];
    switch (mode_info_ptr->mD3dDisplayMode.Format) {
    case D3DFMT_UNKNOWN:
      sprintf(strfmt,"Unknown         "); break;      
      //    case D3DFMT_UNKNOWN_C5 :
      //      sprintf(strfmt,"D3DFMT_UNKNOWN_C5 "); break;
      //    case D3DFMT_UNKNOWN_C5A1: 
      //      sprintf(strfmt,"D3DFMT_UNKNOWN_C5A1 "); break;
      //    case D3DFMT_UNKNOWN_C8 :
      //      sprintf(strfmt," D3DFMT_UNKNOWN_C8"); break;
      //    case D3DFMT_UNKNOWN_C8A8: 
      //      sprintf(strfmt,"D3DFMT_UNKNOWN_C8A8 "); break;
      //    case D3DFMT_UNKNOWN_D16 :
      //      sprintf(strfmt," D3DFMT_UNKNOWN_D16"); break;
      //    case D3DFMT_UNKNOWN_D15S1: 
      //      sprintf(strfmt," D3DFMT_UNKNOWN_D15S1"); break;
      //    case D3DFMT_UNKNOWN_D24 :
      //      sprintf(strfmt," D3DFMT_UNKNOWN_D24"); break;
      //    case D3DFMT_UNKNOWN_D24S8: 
      //      sprintf(strfmt," D3DFMT_UNKNOWN_D24S8"); break;
      //    case D3DFMT_UNKNOWN_D32 :
      //      sprintf(strfmt,"D3DFMT_UNKNOWN_D32 "); break;
    case D3DFMT_R8G8B8 :
      sprintf(strfmt,"32-Bit          "); break;
    case D3DFMT_A8R8G8B8:
      sprintf(strfmt,"32-Bit (Alpha)  "); break;
    case D3DFMT_X8R8G8B8: 
      sprintf(strfmt,"32-Bit          "); break;
    case D3DFMT_R5G6B5 :
      sprintf(strfmt,"16-Bit (5-6-5)  "); break;
    case D3DFMT_X1R5G5B5: 
      sprintf(strfmt,"16-Bit (5-5-5)  "); break;
      
    case D3DFMT_A1R5G5B5:
      sprintf(strfmt,"16-Bit (1-5-5-5)"); break;
    case D3DFMT_A4R4G4B4: 
      sprintf(strfmt,"16-Bit (4-4-4-4)"); break;
    case D3DFMT_R3G3B2 :
      sprintf(strfmt," 8-Bit (3-3-2)  "); break;
    case D3DFMT_A8R3G3B2 :
      sprintf(strfmt,"16-Bit (8-3-3-2)"); break;
    case D3DFMT_X4R4G4B4 :
      sprintf(strfmt,"16-Bit (4-4-4)  "); break;
    }
    
    if (mode_info_ptr->mD3dDisplayMode.RefreshRate > 0)
      sprintf(str,"%4d x %4d - %s - %d Hz",mode_info_ptr->mD3dDisplayMode.Width,
      mode_info_ptr->mD3dDisplayMode.Height, strfmt, mode_info_ptr->mD3dDisplayMode.RefreshRate);
    else
      sprintf(str,"%4d x %4d - %s ",mode_info_ptr->mD3dDisplayMode.Width,
      mode_info_ptr->mD3dDisplayMode.Height, strfmt);
    
    mii.dwTypeData = str;//pMenuDescription;
    InsertMenuItem(hMenu, i+2, TRUE, &mii);
  }
  
  if (!mIsFullScreen)
    CheckMenuItem(hMenu, IDM_MODE_WINDOWED, MF_BYCOMMAND | MF_CHECKED );
  
  return true;
}


bool IawWindow3d::Enable(bool bEnable)
{
  mIsActive = bEnable;
  TargetChanged();
  if (mIsActive)
    mCurTime = mpWrapper->GetTime();
  
  return mIsActive;
}

bool IawWindow3d::ProcessMessages()
{
  if (!mIsAlive)
    return true;
  
  BOOL got_message;
  
  got_message = PeekMessage(&mMsg, NULL, 0, 0, PM_REMOVE);
  
  if (got_message)  // only process 1 message per frame
  { 
    TranslateMessage(&mMsg);
    DispatchMessage(&mMsg);
  }
    
  //update the timer vars
  mLastTime = mCurTime;
  if (mpWrapper)
  {
    mCurTime = mpWrapper->GetTime();
    
    // Update the FPS counter
	double diff = mCurTime - mLastTime;
	float tim = (float)diff;
    mpWrapper->SetFrameTime(tim);
  }
  
  if (mIsActive && mpApp)
  {
    mpWrapper->Clear(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER);
    if (!mAnimationIsPaused)
      mpApp->UpdateWorld();
    
    mpWrapper->StartFrame();
    
    if (mUseWireFrame)
      mpWrapper->mpDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
    
    mpApp->RenderWorld();
    
    
    if (mUseWireFrame)
      mpWrapper->mpDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
    
    mpWrapper->EndFrame(); //Does Endscene and flip/blit
    
  }
  return (mMsg.message == WM_QUIT);
}

BOOL CALLBACK DialogProc( HWND hDialog, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uiMsg )
	{
	case WM_INITDIALOG:
		return true;
	case WM_COMMAND:
		if( LOWORD( wParam ) == IDOK )
		{
			EndDialog( hDialog, TRUE );
			return true;
		}
		break;
	}
	return false;
}


//
// Handle messages specific to this window
//
LRESULT IawWindow3d::MessageHandler(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  HRESULT hr = S_OK;
  PAINTSTRUCT ps;
  int Id;
  
  HMENU hMenu = GetMenu(mWindow.hWnd); //we use this to check the wireframe option
  //if someone preses W
  
  switch (Msg)
  {
  case WM_MENUSELECT:
		if( LOWORD( wParam ) == 4 )
		{
			  HMENU hMenu = GetMenu(mWindow.hWnd);
			  hMenu = GetSubMenu(hMenu, 4); // Get the fourth sub-menu (Assumes File, Adapter, Device, Mode, Options)
			  for( int i=ID_OPTIONS_0; i<= ID_OPTIONS_16; i++ )
				  if( i == (ID_OPTIONS_0 + mpApp->mFloatsToTouch) )
					CheckMenuItem( hMenu, i, MF_BYCOMMAND | MF_CHECKED );
				  else
					CheckMenuItem( hMenu, i, MF_BYCOMMAND | MF_UNCHECKED );
		}
		break;
    case WM_SIZE:
      if ((wParam == SIZE_MAXHIDE) || (wParam == SIZE_MINIMIZED))
        mIsActive = false;
      
      if (mIsActive)
      {
        if (!mIsFullScreen)
        {
          RECT rc;
          
          if (!IsZoomed(mWindow.hWnd))
          {
            GetWindowRect(mWindow.hWnd, &rc);
            mWindowX = rc.left;
            mWindowY = rc.top;
            mWindowWidth = rc.right - rc.left;
            mWindowHeight = rc.bottom - rc.top;
          }
          
          mIsActive = false;
          
          
          if (mpApp)
            mpApp->TargetChanging();
          
          if (mpWrapper)
          {
            HRESULT hr;
            if (FAILED(hr = mpWrapper->Resize()))
            {
              char szMsg[100];
              sprintf(szMsg, "Device failed to re-initialize.  Error: %8lx", hr);
              MessageBox(mWindow.hWnd, szMsg, "Resize", MB_ICONEXCLAMATION | MB_OK);
              DestroyWindow(mWindow.hWnd);
            }
            else
              TargetChanged();
          }
          
          
          mIsActive = true;
        }
      }
      break;
      
    case WM_CLOSE:
      DestroyWindow(mWindow.hWnd);
      break;
      
    case WM_DESTROY:
      if (mpApp) 
      {
        mpApp->DestroyWorld();
        delete mpApp;
        mpApp = NULL;
      }
      if (mpWrapper)
      {
        delete mpWrapper;
        mpWrapper = NULL;
      }
      mIsAlive = false;
      //      PostQuitMessage(0);
      break;
      
    case WM_PAINT:
      BeginPaint(mWindow.hWnd, &ps);
      EndPaint(mWindow.hWnd, &ps);
      ValidateRect(mWindow.hWnd, NULL);
      break;
      
    case WM_ACTIVATEAPP:
      mIsActive = wParam ? true : false;
      break;
      
    case WM_MOVE:
      if (mIsActive && mpWrapper && !mIsFullScreen)
      {
        RECT rc;
        if (!IsZoomed(mWindow.hWnd))
        {
          GetWindowRect(mWindow.hWnd, &rc);
          mWindowX = rc.left;
          mWindowY = rc.top;
        }
      }
      break;
      
    case WM_KEYDOWN:
      
      switch (wParam) 
      {
	  case VK_SHIFT:
		  mbShift = true;
		  break;
      case 'X': //place your code here for what to do if X is pressed (exit for now).
        DestroyWindow(mWindow.hWnd);
        break;
      case 'W':
        mUseWireFrame = !mUseWireFrame;
        
        hMenu = GetSubMenu(hMenu, 4); // Get the second sub-menu (Assumes File, Adapter, Device, Mode, Options)
        
        if (mUseWireFrame)
          CheckMenuItem(hMenu, IDM_OPTIONS_WIREFRAME, MF_BYCOMMAND | MF_CHECKED );
        else
          CheckMenuItem(hMenu, IDM_OPTIONS_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED );
        break;
      case 'F': // Toggle between full screen and windowed
        if (mIsActive && mpWrapper && mpApp)
        {
          mIsActive = false;
          mIsFullScreen = !mIsFullScreen;
          if (mpApp)
            mpApp->TargetChanging();
          
          hr = mpWrapper->Initialize(mpWrapper->mCurrentAdapterNumber, mpWrapper->mCurrentMode, mIsFullScreen, 
            mWindowX, mWindowY, mWindowWidth, mWindowHeight);
          
          if (FAILED(hr))
          {
            char szMsg[100];
            sprintf(szMsg, "Device failed to re-initialize.  Error: %8lx", hr);
            MessageBox(mWindow.hWnd, szMsg, "Toggle Fullscreen", MB_ICONEXCLAMATION | MB_OK);
            DestroyWindow(mWindow.hWnd);
          }
          else /*if ((m_pIAWWrapper->Initialized())*/
          {
            TargetChanged();
            mIsActive = true;
          }
        }
        break;
      case 'P':
        {
          mAnimationIsPaused = !mAnimationIsPaused;
        }
	  default:
		  //now that keypress handled here, pass to the app class
		  mpApp->KeyDown(wParam, mbShift);
		  {
			  HMENU hMenu = GetMenu(mWindow.hWnd);
			  hMenu = GetSubMenu(hMenu, 4); // Get the second sub-menu (Assumes File, Adapter, Device, Mode, Options)
			  for( int i=ID_OPTIONS_0; i<= ID_OPTIONS_16; i++ )
				  if( i == (ID_OPTIONS_0 + mpApp->mFloatsToTouch) )
					CheckMenuItem( hMenu, i, MF_BYCOMMAND | MF_CHECKED );
				  else
					CheckMenuItem( hMenu, i, MF_BYCOMMAND | MF_UNCHECKED );
		  }
        break;
        
      }
      break;
      case WM_KEYUP:
		if( wParam == VK_SHIFT )
			mbShift = false;
		else
			mpApp->KeyUp(wParam);
        break;
        
      case WM_MOUSEMOVE:
        if (mpWrapper && mpWrapper->Initialized())
          mpApp->MouseMove(wParam,lParam);
        break;
        
      case WM_LBUTTONDOWN:
        if (mpWrapper && mpWrapper->Initialized())
          mpApp->MouseLeftDown(wParam,lParam);
        break;
        
      case WM_LBUTTONUP:
        if (mpWrapper && mpWrapper->Initialized())
          mpApp->MouseLeftUp(wParam,lParam);
        break;
        
      case WM_RBUTTONDOWN:
        if (mpWrapper && mpWrapper->Initialized())
          mpApp->MouseRightDown(wParam,lParam);
        break;
        
      case WM_RBUTTONUP:
        if (mpWrapper && mpWrapper->Initialized())
          mpApp->MouseRightUp(wParam,lParam);
        break;
        
        
      case WM_COMMAND:
        //need to handle menus here
        Id = LOWORD(wParam);
        
        // If the FILE->EXIT menu option was chosen then exit the program
        if (Id == IDM_FILE_EXIT)
        {
          DestroyWindow(mWindow.hWnd);
          break;
        }

		if (Id == ID_HELP_ABOUT )
		{
			DialogBox( mWindow.hInst, MAKEINTRESOURCE( IDD_ABOUT ), mWindow.hWnd, DialogProc );
			break;
		}
        
        if (mpWrapper)
        {
          // If Adapter->(an adapter) was chosen, we need to check if it's
          // different from the current adapter, and if so, change adapters
          if ((Id >= IDM_ADAPTER_ADAPTER0) && 
            (Id < IDM_ADAPTER_ADAPTER0 + mpWrapper->GetNumAdapters()))
          {
            SwitchAdapter(Id - IDM_ADAPTER_ADAPTER0);
          } //end if Adapter menu item selected
          //now we need to check if one of the device types was picked
          else if ((Id >= IDM_DEVICE_NULL) && 
            (Id <= IDM_DEVICE_REFRAST))
          {
            SwitchRasterizer(Id );
          }
          else if ((Id >= IDM_MODE_WINDOWED) && 
            (Id < IDM_MODE_FULL0 + mpWrapper->GetNumModes()))
          {
            SwitchMode(Id);
          }
          else if (Id == IDM_OPTIONS_WIREFRAME)
          {
            mUseWireFrame = !mUseWireFrame;
            
            HMENU hMenu = GetMenu(mWindow.hWnd);
            hMenu = GetSubMenu(hMenu, 4); // Get the second sub-menu (Assumes File, Adapter, Device, Mode, Options)
            
            if (mUseWireFrame)
              CheckMenuItem(hMenu, IDM_OPTIONS_WIREFRAME, MF_BYCOMMAND | MF_CHECKED );
            else
              CheckMenuItem(hMenu, IDM_OPTIONS_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED );
          }     
		  else if ((Id >= ID_OPTIONS_0) && (Id <= ID_OPTIONS_16))
		  {
              HMENU hMenu = GetMenu(mWindow.hWnd);
              hMenu = GetSubMenu(hMenu, 4); // Get the second sub-menu (Assumes File, Adapter, Device, Mode, Options)
			  for( int i=ID_OPTIONS_0; i<= ID_OPTIONS_16; i++ )
				CheckMenuItem( hMenu, i, MF_BYCOMMAND | MF_UNCHECKED );
			  CheckMenuItem( hMenu, Id, MF_BYCOMMAND | MF_CHECKED );
			  if( Id > ID_OPTIONS_8 )
				  mpApp->KeyDown(Id - ID_OPTIONS_0 - 8 + '0', true);
			  else
				mpApp->KeyDown(Id - ID_OPTIONS_0 + '0', false);
		  }
		  else if (Id == ID_OPTIONS_TOGGLE_VERTICES)
		  {
			  mpApp->KeyDown('V', false);
		  }
        }
        break;
        
        
        
      default:
        break;
    }
    
    return DefWindowProc(mWindow.hWnd, Msg, wParam, lParam);
}

//
// Switch to the appropriate adapter. adapter is 0-indexed for the list on the
// adapter menu.
//
bool IawWindow3d::SwitchAdapter(int adapter)
{
  HRESULT hr;
  HMENU hMenu = GetMenu(mWindow.hWnd);
  hMenu = GetSubMenu(hMenu, 1); // Get the second sub-menu (Assumes File then Adapter)
  
  if (adapter != (int)mpWrapper->mCurrentAdapterNumber)
  {
    mIsActive = false;
    
    
    CheckMenuItem(hMenu, IDM_ADAPTER_ADAPTER0+mpWrapper->mCurrentAdapterNumber,
      MF_BYCOMMAND | MF_UNCHECKED );
    
    if (mpApp)
      mpApp->TargetChanging();
    
    mpWrapper->mCurrentAdapterNumber = adapter;
    hr = mpWrapper->Initialize(adapter, mpWrapper->mCurrentMode, mIsFullScreen, 
      mWindowX, mWindowY, mWindowWidth, mWindowHeight);
    
        
    if (FAILED(hr))
    {
      char szMsg[100];
      sprintf(szMsg, "Device failed to re-initialize.  Error: %8lx", hr);
      MessageBox(mWindow.hWnd, szMsg, "Switch Adapter", MB_ICONEXCLAMATION | MB_OK);
      
      DestroyWindow(mWindow.hWnd);
      return false;
    }
    else
    {
      TargetChanged();
      
      mIsActive = true;
      CheckMenuItem(hMenu, IDM_ADAPTER_ADAPTER0+mpWrapper->mCurrentAdapterNumber, MF_BYCOMMAND | MF_CHECKED );
      BuildDeviceMenu();
      BuildModeMenu();
    }
  } //end if adapter selected is different
  return true;
}

//
// Switch to the appropriate mode.  Id is the Menu item command 
//
bool IawWindow3d::SwitchMode(int Id)
{
  int mode;
  HRESULT hr;
  HMENU hMenu = GetMenu(mWindow.hWnd);
  hMenu = GetSubMenu(hMenu, 3); // Get the second sub-menu (Assumes File, Adapter, Device, Mode, Options)
  
  
  //first, handle windowed mode
  if (Id == IDM_MODE_WINDOWED)
  {
    if (!mIsFullScreen)
      return true;
    
    mIsFullScreen = false;
    mode = mpWrapper->mCurrentMode;
  }
  else
  {
    mIsFullScreen = true;
    mode = Id - IDM_MODE_FULL0;
  }
  
  if (mpApp)
    mpApp->TargetChanging();
  
  hr = mpWrapper->Initialize(mpWrapper->mCurrentAdapterNumber, mode, mIsFullScreen, 
    mWindowX, mWindowY, mWindowWidth, mWindowHeight);
  
  if (FAILED(hr))
  {
    char szMsg[100];
    sprintf(szMsg, "Device failed to re-initialize.  Error: %8lx", hr);
    MessageBox(mWindow.hWnd, szMsg, "Switch Mode", MB_ICONEXCLAMATION | MB_OK);
    DestroyWindow(mWindow.hWnd);
    return false;
  }
  else /*if ((m_pIAWWrapper->Initialized())*/
  {
    TargetChanged();
        
    for (int i=IDM_MODE_WINDOWED; i< IDM_MODE_FULL0 + mpWrapper->GetNumModes(); i++)
      CheckMenuItem(hMenu, IDM_MODE_WINDOWED + i, MF_BYCOMMAND | ((i == Id) ? MF_CHECKED : MF_UNCHECKED) );
    
    mIsActive = true;
  }
  return true;
}


HRESULT IawWindow3d::TargetChanged()
{
  if (mpApp)
    mpApp->TargetChanged();
  
  return S_OK;
}

bool IawWindow3d::SwitchRasterizer(int mode)
{
  D3DDEVTYPE d3d_device_type;
  DWORD vertex_processing_type;
  HMENU hMenu = GetMenu(mWindow.hWnd);
  
  switch (mode) {
  case (IDM_DEVICE_NULL):
    d3d_device_type = D3DDEVTYPE_SW;
    vertex_processing_type = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    break;
  case (IDM_DEVICE_SWRASTERIZER):
    d3d_device_type = D3DDEVTYPE_SW;
    vertex_processing_type = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    break;
  case (IDM_DEVICE_HALSW):
    d3d_device_type = D3DDEVTYPE_HAL;
    vertex_processing_type = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    break;
  case (IDM_DEVICE_HALMIXED):
    d3d_device_type = D3DDEVTYPE_HAL;
    vertex_processing_type = D3DCREATE_MIXED_VERTEXPROCESSING;
    break;
  case (IDM_DEVICE_HALHW):
    d3d_device_type = D3DDEVTYPE_HAL;
    vertex_processing_type = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    break;
  case (IDM_DEVICE_HALPURE):
    d3d_device_type = D3DDEVTYPE_HAL;
    vertex_processing_type = D3DCREATE_PUREDEVICE;
    break;
  case (IDM_DEVICE_REFRAST):
    d3d_device_type = D3DDEVTYPE_REF;
    vertex_processing_type = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    break;
  }
  
  //now set these options in the wrapper
  if (mpApp)
    mpApp->TargetChanging();
  
  HRESULT hr = mpWrapper->ChangeDeviceOptions(d3d_device_type, vertex_processing_type);
  
  if (FAILED(hr))
  {
    char szMsg[100];
    sprintf(szMsg, "Device failed to re-initialize.  Error: %8lx", hr);
    MessageBox(mWindow.hWnd, szMsg, "Switch Rasterizer", MB_ICONEXCLAMATION | MB_OK);
    DestroyWindow(mWindow.hWnd);
    return false;
  }
  else
  {
    TargetChanged();
        
    for (int i=IDM_DEVICE_NULL; i<= IDM_DEVICE_REFRAST; i++)
      CheckMenuItem(hMenu, i, MF_BYCOMMAND | ((i == mode) ? MF_CHECKED : MF_UNCHECKED) );
    
    mIsActive = true;
    return true;
  }
}


void IawWindow3d::SetApp(AGP_Performance* pApp)
{ 
	HMENU hMenu = GetMenu(mWindow.hWnd);

	mpApp = pApp; 
	CheckMenuItem( hMenu, ID_OPTIONS_0, MF_BYCOMMAND | MF_CHECKED );
}
