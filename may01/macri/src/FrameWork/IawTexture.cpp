// IawTexture.cpp App Wizard Version 2.0 Beta 1
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
// IawTexture.cpp: implementation of the CIawTexture class.
// Originally adapted from the DX6 D3DFrame sample code. Added some robustness 
// and extra functionality (alpha bitmaps, creating inverse heightmaps for embossing, 
// automatic resize for HW requirements, support for envbump, support for rendering 
// to textures, etc)
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include <tchar.h>
#include "..\IawFrameWork.h"

// Constants...
const int IawTexture::IAW_TEXTR_TRANSPARENT_WHITE = 0x00000002;
const int IawTexture::IAW_TEXTR_TRANSPARENT_BLACK = 0x00000004;
const int IawTexture::IAW_TEXTR_32_BITS_PER_PIXEL = 0x00000008;
const int IawTexture::IAW_TEXTR_RENDER_TARGET     = 0x00000010;
const int IawTexture::IAW_TEXTR_CREATE_MIP_MAPS   = 0x00000020;
const int IawTexture::IAW_TEXTR_FORCE_IN_SYS_MEM  = 0x00000040;
const int IawTexture::IAW_TEXTR_FORCE_IN_VID_MEM  = 0x00000080;

const int IawTexture::IAW_TEXTR_CREATE_INV_HEIGHT_MAP = 0x00000100;
const int IawTexture::IAW_TEXTR_CREATE_BUMP_MAP_DUDV  = 0x00000200;

const int IawTexture::IAW_TEXTR_MUST_HAVE_ALPHA = 0x00000400;
const int IawTexture::IAW_TEXTR_16_BITZ         = 0x00000800;
const int IawTexture::IAW_TEXTR_32_BITZ         = 0x00001000;

const int IawTexture::IAW_TEXTR_CREATE_FONT     = 0x00002000;
const int IawTexture::FONT_TEXTURE_SIZE         = 512;

const int IawTexture::MAX_TEXTURE_FORMAT_TRIES = 10;

const int IawTexture::FONT_TEXT_CHARS_PER_SIDE = 16;
const int IawTexture::FONT_TEXT_CHARS_PER_ROW  = 16;
const int IawTexture::BOTTOM_COLOR             = 0xFF3333FF; // blue
const int IawTexture::TOP_COLOR                = 0xFFFFFFFF; // white


// Constructor...
IawTexture::IawTexture(IawD3dWrapper* pWrapper)
{
  mBitmap   = NULL;
  mAlphaBitmap = NULL;
  mpTexture  = NULL;
  mStage    = 0;
  mHasAlpha  = false;
  mCreateFlags = 0;
  mpWrapper  = pWrapper;
}

// Release allocated resources
void IawTexture::Cleanup()
{
  if (mBitmap)
  {
    DeleteObject(mBitmap);
    mBitmap = NULL;
  }

  if (mAlphaBitmap)
  {
    DeleteObject(mAlphaBitmap);
    mAlphaBitmap = NULL;
  }

  SAFE_RELEASE(mpTexture);

  mStage = 0;
  mHasAlpha = false;
  mCreateFlags = 0;
}

// Destructor
IawTexture::~IawTexture()
{
  Cleanup();
}

// Methods to create a texture...
HRESULT IawTexture::CreateTexture(IawImageLoader& rTextureLoader,
                                  DWORD textureStage,
                                  DWORD textureFlags)
{
  Cleanup();

  // Copy parameters into member variables
  mpTextureLoader   = &rTextureLoader;
  mpAlphaTextureLoader = NULL;// No alpha in this case
  mStage        = textureStage;
  mCreateFlags     = textureFlags;

  // Create a bitmap and load the texture file into it,
  if (FAILED(LoadTextureImage()))
  {
    mpTextureLoader = NULL;
    return E_FAIL;
  }

  // This may not be usable after returning from this function
  // so set it to NULL
  mpTextureLoader = NULL;

  return S_OK;
}

// Create a texture with alpha info...
HRESULT IawTexture::CreateTexture(IawImageLoader& rTextureLoader,
                                  IawImageLoader& rAlphaTextureLoader,
                                  DWORD textureStage,
                                  DWORD textureFlags)
{
  Cleanup();

  // Copy parameters into member variables
  mpTextureLoader = &rTextureLoader;
  mpAlphaTextureLoader = &rAlphaTextureLoader;
  mStage = textureStage;
  mCreateFlags = textureFlags;

  // Because this function got called, we are trying to load an alpha texture
  mHasAlpha = true;

  // Create a bitmap and load the texture file into it.
  if (FAILED(LoadTextureImage()))
  {
    mpTextureLoader   = NULL;
    mpAlphaTextureLoader = NULL;
    return E_FAIL;
  }

  // These may not be usable after returning from this function
  // so set them to NULL
  mpTextureLoader   = NULL;
  mpAlphaTextureLoader = NULL;

  return S_OK;
}

// Create a blank texture...
HRESULT IawTexture::CreateTexture(DWORD hRes,
                                  DWORD vRes,
                                  DWORD bpp,
                                  DWORD textureStage,
                                  DWORD textureFlags,
                                  DWORD* pBits)
{
  HRESULT   hr;
  D3DFORMAT format;
  D3DPOOL   pool;
  UINT      levels;
  UINT      usage;

  mWidth  = hRes;
  mHeight = vRes;
  mBpp    = bpp;

  // Check size and format
  if (16 == bpp)
    textureFlags &= ~IAW_TEXTR_32_BITS_PER_PIXEL;
  else if (32 == bpp)
    textureFlags |= IAW_TEXTR_32_BITS_PER_PIXEL;

  mStage       = textureStage;
  mCreateFlags = textureFlags;

  if ((mCreateFlags & IAW_TEXTR_MUST_HAVE_ALPHA)   ||
      (mCreateFlags & IAW_TEXTR_TRANSPARENT_WHITE) ||
      (mCreateFlags & IAW_TEXTR_TRANSPARENT_BLACK) ||
      (mCreateFlags & IAW_TEXTR_CREATE_FONT))
  {
    mHasAlpha = true;
  }
  else
  {
    mHasAlpha = false;
  }

  // Get the parameters for the texture to be created.
  if (FAILED(hr = SelectParameters(format, pool, levels, usage)))
    return hr;

  // Create the texture...
  hr = mpWrapper->mpDevice->CreateTexture(hRes, vRes, levels, usage, format, pool, &mpTexture);

  // Now copy the bits if we have any.
  if (SUCCEEDED(hr))
  {
    // Attach a z-buffer if necessary.
    if (FAILED(hr = CreateAttachedSurfaces()))
    {
      SAFE_RELEASE(mpTexture);
      return hr;
    }
  }

  // Create a bitmap to go with the texture.
  BITMAPINFO bmi;
  void* temp_bits = NULL;

  memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biHeight = vRes;
  bmi.bmiHeader.biWidth = hRes;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biPlanes = 1;

  mBitmap = CreateDIBSection(NULL,
                             (const struct tagBITMAPINFO *)&bmi,
                             DIB_RGB_COLORS,
                             &temp_bits,
                             NULL,
                             0);

  if (mHasAlpha)
  {
    mAlphaBitmap = CreateDIBSection(NULL,
                                    (const struct tagBITMAPINFO *)&bmi,
                                    DIB_RGB_COLORS,
                                    &temp_bits,
                                    NULL,
                                    0);
  }

  // Copy the bits if we were given any.
  if (pBits)
  {
    /** @todo Put in code to copy the bits... */
  }

  if (textureFlags &= IAW_TEXTR_CREATE_FONT)
  {
    WriteFontToTexture();
  }

  return hr;
}

// Select texture parameters...
HRESULT IawTexture::SelectParameters(D3DFORMAT& rFormat,
                                     D3DPOOL& rPool,
                                     UINT& rLevels,
                                     UINT& rUsage)
{
  D3DFORMAT d3d_formats[MAX_TEXTURE_FORMAT_TRIES];
  int formats = 0;
  int idx;

  rPool  = D3DPOOL_MANAGED;
  rLevels = 1;
  rUsage = 0;

  if (mCreateFlags & IAW_TEXTR_CREATE_MIP_MAPS)
  {
    rLevels = 0;
  }

  // Change the rPool where the texture is created.
  if (mCreateFlags & IAW_TEXTR_FORCE_IN_VID_MEM)
  {
    rPool = D3DPOOL_DEFAULT;
  }
  else if (mCreateFlags & IAW_TEXTR_FORCE_IN_SYS_MEM)
  {
    rPool = D3DPOOL_SYSTEMMEM;
  }

  // Now pick an appropriate format
  if (mHasAlpha)
  {
    if (mCreateFlags & IAW_TEXTR_32_BITS_PER_PIXEL)
    {
      d3d_formats[formats++] = D3DFMT_A8R8G8B8;
      d3d_formats[formats++] = D3DFMT_A4R4G4B4;
      d3d_formats[formats++] = D3DFMT_A1R5G5B5;
      d3d_formats[formats++] = D3DFMT_X8R8G8B8;
      d3d_formats[formats++] = D3DFMT_R5G6B5;
      d3d_formats[formats++] = D3DFMT_X1R5G5B5;
    }
    else
    {
      if ((mCreateFlags & IAW_TEXTR_TRANSPARENT_WHITE) ||
          (mCreateFlags & IAW_TEXTR_TRANSPARENT_BLACK))
      {
        d3d_formats[formats++] = D3DFMT_A1R5G5B5;
        d3d_formats[formats++] = D3DFMT_A4R4G4B4;
        d3d_formats[formats++] = D3DFMT_A8R8G8B8;
        d3d_formats[formats++] = D3DFMT_R5G6B5;
        d3d_formats[formats++] = D3DFMT_X1R5G5B5;
        d3d_formats[formats++] = D3DFMT_X8R8G8B8;
      }
      else
      {
        d3d_formats[formats++] = D3DFMT_A4R4G4B4;
        d3d_formats[formats++] = D3DFMT_A8R8G8B8;
        d3d_formats[formats++] = D3DFMT_A1R5G5B5;
        d3d_formats[formats++] = D3DFMT_R5G6B5;
        d3d_formats[formats++] = D3DFMT_X1R5G5B5;
        d3d_formats[formats++] = D3DFMT_X8R8G8B8;
      }
    }
  }
  else
  {
    if (mCreateFlags & IAW_TEXTR_32_BITS_PER_PIXEL)
    {
      d3d_formats[formats++] = D3DFMT_X8R8G8B8;
      d3d_formats[formats++] = D3DFMT_R5G6B5;
      d3d_formats[formats++] = D3DFMT_X1R5G5B5;
    }
    else
    {
      d3d_formats[formats++] = D3DFMT_R5G6B5;
      d3d_formats[formats++] = D3DFMT_X1R5G5B5;
      d3d_formats[formats++] = D3DFMT_X8R8G8B8;
    }
  }

  if (mCreateFlags & IAW_TEXTR_RENDER_TARGET)
  {
    // For render targets, we need to force a few specific values.
    rUsage = D3DUSAGE_RENDERTARGET;
    rPool = D3DPOOL_DEFAULT;

    if (mHasAlpha)
    {
      d3d_formats[formats++] = D3DFMT_A8R8G8B8; 
      d3d_formats[formats++] = D3DFMT_X8R8G8B8; 
      d3d_formats[formats++] = D3DFMT_R5G6B5;
      d3d_formats[formats++] = D3DFMT_X1R5G5B5;
    }
    else
    {
      if (mCreateFlags & IAW_TEXTR_32_BITS_PER_PIXEL)
      {
        d3d_formats[formats++] = D3DFMT_X8R8G8B8;
        d3d_formats[formats++] = D3DFMT_A8R8G8B8;
        d3d_formats[formats++] = D3DFMT_R5G6B5;
        d3d_formats[formats++] = D3DFMT_X1R5G5B5;
      }
      else
      {
        d3d_formats[formats++] = D3DFMT_R5G6B5;
        d3d_formats[formats++] = D3DFMT_X1R5G5B5;
        d3d_formats[formats++] = D3DFMT_X8R8G8B8;
        d3d_formats[formats++] = D3DFMT_A8R8G8B8;
      }
    }
  }

  // Check for a supported rFormat
  idx = 0;
  D3DDEVICE_CREATION_PARAMETERS d3d_cp;
  D3DDISPLAYMODE d3d_mode;

  mpWrapper->mpDevice->GetCreationParameters(&d3d_cp);
  mpWrapper->mpDevice->GetDisplayMode(&d3d_mode);
  while ((idx < formats) &&
         (FAILED(mpWrapper->mpDirectX8->CheckDeviceFormat(d3d_cp.AdapterOrdinal,
                                                           d3d_cp.DeviceType,
                                                           d3d_mode.Format,
                                                           rUsage,
                                                           D3DRTYPE_TEXTURE,
                                                           d3d_formats[idx]))))
  {
    idx++;
  }

  if (idx == formats)
  {
    return D3DERR_NOTAVAILABLE;
  }

  rFormat = d3d_formats[idx];

  return S_OK;
}

HRESULT IawTexture::CreateAttachedSurfaces()
{
  return S_OK;
}

HRESULT IawTexture::LoadTextureImage()
{
  // Load the Texture
  if (mpTextureLoader)
  {
    HBITMAP* pAlpha = NULL;
    if (!mpAlphaTextureLoader)
      pAlpha = &mAlphaBitmap;
    if (FAILED(mpTextureLoader->LoadImage(&mBitmap, pAlpha)))
      return D3DERR_NOTFOUND;
    if (mBitmap == NULL)
      return D3DERR_NOTFOUND;
    if (mAlphaBitmap)
      mHasAlpha = true;
  }

  if (mHasAlpha && mpAlphaTextureLoader && !mAlphaBitmap)
  {
    if (FAILED(mpAlphaTextureLoader->LoadImage(&mAlphaBitmap, NULL)))
      return D3DERR_NOTFOUND;
    if (mAlphaBitmap == NULL)
      return D3DERR_NOTFOUND;
  }

  //last sanity check.
  if (mBitmap)
  {
    if (mHasAlpha)
    {
      if (mAlphaBitmap)
        return D3D_OK; // alpha and bitmap found
      else
      {
        // Free the loaded Bitmap
        DeleteObject(mBitmap);
        mBitmap = NULL;
        return D3DERR_NOTFOUND; //asked for alpha, but alpha not found
      }
    }
    else
      return D3D_OK; //no alpha, bitmap found
  }
  else if (mAlphaBitmap)
  {
    DeleteObject(mAlphaBitmap);
    mAlphaBitmap = NULL;
  }

  return D3DERR_NOTFOUND; //bitmap couldn't load
}

// Restore...
HRESULT IawTexture::Restore()
{
  // Check params
  if (NULL == mpWrapper->mpDevice)
    return D3DERR_INVALIDDEVICE;

  // Release previously created object
  SAFE_RELEASE(mpTexture);

  // Restore the texture surface from the bitmap image. At this point, code
  // can be added to handle other texture formats, such as those created from
  // .dds files, .jpg files, or whatever else.
  return RestoreFromBitmap();
}

HRESULT IawTexture::Restore(DWORD newStage)
{
  mStage = newStage;
  return Restore();
}

// Restore a texture from a bitmap...
HRESULT IawTexture::RestoreFromBitmap()
{
  // Get the device caps
  HRESULT  hr = S_OK;
  D3DFORMAT format;
  D3DPOOL  pool;
  UINT   levels;
  UINT   usage;

  // Get the bitmap structure (to extract width, height, and bpp)
  BITMAP bm;
  GetObject(mBitmap, sizeof(BITMAP), &bm);
  DWORD width = (DWORD)bm.bmWidth;
  DWORD height = (DWORD)bm.bmHeight;

  // Currently, we fail if the alpha bitmap is a different size.
  /** @todo Fix this to scale the smaller to fit the larger. */
  if (mAlphaBitmap)
  {
    BITMAP bmtemp;
    GetObject(mAlphaBitmap, sizeof(BITMAP), &bmtemp);
    if ((width != (DWORD)bmtemp.bmWidth) || (height != (DWORD)bmtemp.bmHeight))
    {
      OutputDebugString ("Alpha and Color bitmaps are different sizes! Failing...\n");
      return E_FAIL;
    }
  }

 if ((mCreateFlags & IAW_TEXTR_MUST_HAVE_ALPHA)  ||
    (mCreateFlags & IAW_TEXTR_TRANSPARENT_WHITE) ||
    (mCreateFlags & IAW_TEXTR_TRANSPARENT_BLACK))
    mHasAlpha = true;

  // Get the parameters for the texture to be created
  if (FAILED(hr = SelectParameters(format, pool, levels, usage)))
   return hr;

  /**
   * @todo Fix the following block of code.
   *
   * //check size
   * D3DCAPS8 caps;
   * memset(&caps, 0, sizeof(D3DCAPS8));
   * //caps.bi.bmiHeader.biSize = sizeof(D3DCAPS8);
   * mpWrapper->mpDevice->GetDeviceCaps(&caps);
   * while (width > caps.MaxTextureWidth)
   *  width /=2;
   * while (height > caps.MaxTextureHeight)
   *  height /=2;
   */

  // Okay, create the texture
  hr = mpWrapper->mpDevice->CreateTexture(width, height, levels, usage, format, pool, &mpTexture);

  // Now copy the bits if we have any
  if (SUCCEEDED(hr))
  {
    // Attach a z-buffer if necessary
    if (FAILED(hr = CreateAttachedSurfaces()))
    {
      SAFE_RELEASE(mpTexture);
      return hr;
    }

    // Copy the bitmap to the texture surface
    return CopyBitmapToSurface();
  }

  return hr;
}

/*
#define MyGetPixel(bits, x, y, width, height) \
 (COLORREF)((*(bits + (height-y-1) * ((width*3+3)&~3) + x*3)) << 16) | \
 ((*(bits + (height-y-1) * ((width*3+3)&~3) + x*3+1)) <<8) | \
 ((*(bits + (height-y-1) * ((width*3+3)&~3) + x*3+2)))
*/

HRESULT IawTexture::CopyBitmapToSurface()
{
  // Get the bitmap structure (to extract width, height, and bpp)
  BITMAP bm;
  GetObject(mBitmap, sizeof(BITMAP), &bm);

  //do the same for the alpha bitmap, if there is one
  BITMAP bm_alpha;
  if (mAlphaBitmap)
  {
    GetObject(mAlphaBitmap, sizeof(BITMAP), &bm_alpha);
    if ((bm_alpha.bmWidth != bm.bmWidth) || (bm_alpha.bmHeight != bm.bmHeight))
    {
      OutputDebugString ("color and alpha bitmaps not the same size (this isn't supported yet \n");
      return NULL;
    }
  }

  // Get a Device Context for the bitmap. Use it for blitting.
  HDC bitmap = CreateCompatibleDC(NULL);
  if (NULL == bitmap)
  {
    return NULL;
  }

  HDC alpha_bitmap = CreateCompatibleDC(NULL);
  if (mAlphaBitmap)
  {
    // and get a DC for the alpha bitmap
    if (NULL == alpha_bitmap)
    {
      return NULL;
    }
  }

  // Get the bits of the bitmap and it's alpha channel
  BYTE* bits;
  BYTE* alpha_bits;
  BITMAPINFO bi;
  DWORD pitch;
  memset(&bi, 0, sizeof( BITMAPINFO));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = bm.bmWidth;
  bi.bmiHeader.biHeight = bm.bmHeight;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;
  bi.bmiHeader.biCompression = BI_RGB;
  pitch = ((bi.bmiHeader.biWidth * bi.bmiHeader.biBitCount/8 + 3) & ~3);
  bi.bmiHeader.biSizeImage = pitch * bi.bmiHeader.biHeight;
  bits = new BYTE[pitch * bi.bmiHeader.biHeight];
  GetDIBits(bitmap, mBitmap, 0, bm.bmHeight, bits, &bi, DIB_RGB_COLORS);
  if (alpha_bitmap)
  {
    alpha_bits = new BYTE[pitch * bi.bmiHeader.biHeight];
    GetDIBits(alpha_bitmap, mAlphaBitmap, 0, bm.bmHeight, alpha_bits, &bi, DIB_RGB_COLORS);
  }

  // Now copy the bits to the surface
  HRESULT hr = CopyBits(mpTexture, 0, pitch, bm.bmWidth, bm.bmHeight, bits, alpha_bits);

  //be sure to clean up before you leave
  delete [] bits;
  if (alpha_bitmap)
    delete [] alpha_bits;

  DeleteDC(bitmap);
  DeleteDC(alpha_bitmap);

  return D3D_OK;
}

// Copy bits from an array to a texture surface.
HRESULT IawTexture::CopyBitsStraight(LPDIRECT3DTEXTURE8 pTexture,
                                     int level,
                                     DWORD* pBits)
{
  HRESULT hr;
  D3DSURFACE_DESC surface_desc;
  DWORD x, y;
  DWORD idx   = 0,
        idx_2 = 0;
  D3DLOCKED_RECT locked_rect;
  RECT rc_full;

  pTexture->GetLevelDesc(level, &surface_desc);

  rc_full.left   = 0;
  rc_full.top    = 0;
  rc_full.right  = surface_desc.Width;
  rc_full.bottom = surface_desc.Height;

  hr = mpTexture->LockRect(level, &locked_rect, &rc_full, 0);

  if (SUCCEEDED(hr))
  {
    for (y = 0; y < surface_desc.Height; y++)
    {
      idx_2 = y * locked_rect.Pitch;
      for (x = 0; x < surface_desc.Width; x++)
      {
        ((DWORD *)locked_rect.pBits)[idx_2 + x] = pBits[idx];
        idx++;
      }
    }
  }
  mpTexture->UnlockRect(level);

  return S_OK;
}

// Copy bits doing some color conversion on the fly.
HRESULT IawTexture::CopyBits(LPDIRECT3DTEXTURE8 pTexture,
                             int level,
                             DWORD bitsPitch,
                             DWORD bitsWidth,
                             DWORD bitsHeight,
                             BYTE* pBits,
                             BYTE* pBitsAlpha)
{
  HRESULT hr;
  D3DSURFACE_DESC surface_desc;
  DWORD x, y;
  DWORD idx   = 0,
        idx_2 =0 ;
  D3DLOCKED_RECT locked_rect;
  RECT rc_full;
  DWORD r_shift_l, r_shift_r, r_mask;
  DWORD g_shift_l, g_shift_r, g_mask;
  DWORD b_shift_l, b_shift_r, b_mask;
  DWORD a_shift_l, a_shift_r, a_mask;
  DWORD data, red, green, blue, alpha;
  DWORD dest_bpp   = 32,
        dest_bytes = 4;
  bool calc_luminance = false;

  pTexture->GetLevelDesc(level, &surface_desc);

  switch(surface_desc.Format)
  {
  case D3DFMT_R8G8B8:
    r_shift_l = 0;
    r_shift_r = 0;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x0000FF00;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x000000FF;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    break;
  case D3DFMT_A8R8G8B8:
    // Fall through here...
  case D3DFMT_X8R8G8B8:
    r_shift_l = 0;
    r_shift_r = 0;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x0000FF00;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x000000FF;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0xFF000000;
    break;
  case D3DFMT_R5G6B5:
    r_shift_l = 0;
    r_shift_r = 8;
    r_mask = 0x00F80000;
    g_shift_l = 0;
    g_shift_r = 5;
    g_mask = 0x0000FC00;
    b_shift_l = 0;
    b_shift_r = 3;
    b_mask = 0x000000F8;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    dest_bpp = 16;
    break;
  case D3DFMT_X1R5G5B5:
    // Fall through here...
  case D3DFMT_A1R5G5B5:
    r_shift_l = 0;
    r_shift_r = 9;
    r_mask = 0x00F80000;
    g_shift_l = 0;
    g_shift_r = 6;
    g_mask = 0x0000F800;
    b_shift_l = 0;
    b_shift_r = 3;
    b_mask = 0x000000F8;
    a_shift_l = 0;
    a_shift_r = 16;
    a_mask = 0x80000000;
    dest_bpp = 16;
    break;
  case D3DFMT_A4R4G4B4:
    // Fall through here...
  case D3DFMT_X4R4G4B4:
    r_shift_l = 0;
    r_shift_r = 12;
    r_mask = 0x00F00000;
    g_shift_l = 0;
    g_shift_r = 8;
    g_mask = 0x0000F000;
    b_shift_l = 0;
    b_shift_r = 4;
    b_mask = 0x000000F0;
    a_shift_l = 0;
    a_shift_r = 16;
    a_mask = 0xF0000000;
    dest_bpp = 16;
    break;
  case D3DFMT_R3G3B2:
    r_shift_l = 0;
    r_shift_r = 16;
    r_mask = 0x00E00000;
    g_shift_l = 0;
    g_shift_r = 11;
    g_mask = 0x0000E000;
    b_shift_l = 0;
    b_shift_r = 6;
    b_mask = 0x000000C0;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    dest_bpp = 8;
    break;
  case D3DFMT_A8:
    r_shift_l = 0;
    r_shift_r = 0;
    r_mask = 0x00000000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x00000000;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x00000000;
    a_shift_l = 0;
    a_shift_r = 24;
    a_mask = 0xFF000000;
    dest_bpp = 8;
    break;
  case D3DFMT_A8R3G3B2:
    r_shift_l = 0;
    r_shift_r = 16;
    r_mask = 0x00E00000;
    g_shift_l = 0;
    g_shift_r = 11;
    g_mask = 0x0000E000;
    b_shift_l = 0;
    b_shift_r = 6;
    b_mask = 0x000000C0;
    a_shift_l = 0;
    a_shift_r = 16;
    a_mask = 0xFF000000;
    dest_bpp = 16;
    break;
  case D3DFMT_L8:
    calc_luminance = true;
    r_shift_l = 0;
    r_shift_r = 12;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x00000000;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x00000000;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    dest_bpp = 8;
    break;
  case D3DFMT_A8L8:
    calc_luminance = true;
    r_shift_l = 0;
    r_shift_r = 12;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x00000000;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x00000000;
    a_shift_l = 0;
    a_shift_r = 16;
    a_mask = 0xFF000000;
    dest_bpp = 16;
    break;
  case D3DFMT_A4L4:
    calc_luminance = true;
    r_shift_l = 0;
    r_shift_r = 20;
    r_mask = 0x00F00000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x00000000;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x00000000;
    a_shift_l = 0;
    a_shift_r = 24;
    a_mask = 0xF0000000;
    dest_bpp = 8;
    break;
  case D3DFMT_V8U8:
    r_shift_l = 0;
    r_shift_r = 8;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 8;
    g_mask = 0x0000FF00;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x00000000;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    dest_bpp = 16;
    break;
  case D3DFMT_L6V5U5:
    r_shift_l = 0;
    r_shift_r = 8;
    r_mask = 0x00FC0000;
    g_shift_l = 0;
    g_shift_r = 6;
    g_mask = 0x0000F800;
    b_shift_l = 0;
    b_shift_r = 3;
    b_mask = 0x000000F8;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    dest_bpp = 16;
    break;
  case D3DFMT_X8L8V8U8:
    r_shift_l = 0;
    r_shift_r = 0;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x0000FF00;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x000000FF;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0xFF000000;
    break;
  case D3DFMT_Q8W8V8U8:
    r_shift_l = 0;
    r_shift_r = 0;
    r_mask = 0x00FF0000;
    g_shift_l = 0;
    g_shift_r = 0;
    g_mask = 0x0000FF00;
    b_shift_l = 0;
    b_shift_r = 0;
    b_mask = 0x000000FF;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0xFF000000;
    break;
  case D3DFMT_W11V11U10:
    r_shift_l = 8;
    r_shift_r = 0;
    r_mask = 0x00FF0000;
    g_shift_l = 5;
    g_shift_r = 0;
    g_mask = 0x0000FF00;
    b_shift_l = 2;
    b_shift_r = 0;
    b_mask = 0x000000FF;
    a_shift_l = 0;
    a_shift_r = 0;
    a_mask = 0x00000000;
    break;
  default: // All the rest are currently unsupported by us
    return E_FAIL;
  }

  dest_bytes     = dest_bpp / 8;
  rc_full.left   = 0;
  rc_full.top    = 0;
  rc_full.right  = surface_desc.Width;
  rc_full.bottom = surface_desc.Height;

  hr = mpTexture->LockRect(level, &locked_rect, &rc_full, 0);
  if (SUCCEEDED(hr))
  {
    for (y = 0; y < surface_desc.Height; y++)
    {
      idx_2 = y * locked_rect.Pitch;
      idx = y * bitsPitch;
      for (x = 0; x < surface_desc.Width; x++)
      {
        data = ((*(DWORD *)&(pBits[idx+x*3])) & 0xFFFFFF);
        if (pBitsAlpha)
        data |= ((DWORD)pBitsAlpha[idx+x*3+2] << 24);
        if (calc_luminance)
        {
          red = (((data & 0xFF0000) >> 16) * 30 +
                 ((data & 0x00FF00) >> 8) * 59+
                 ((data & 0x0000FF) * 11) / 100) << 16;
          red = (((red & r_mask) >> r_shift_r) << r_shift_l);
          green = 0;
          blue  = 0;
          alpha = ((data & a_mask) >> a_shift_r) << a_shift_l;
        }
        else
        {
          red   = ((data & r_mask) >> r_shift_r) << r_shift_l;
          green = ((data & g_mask) >> g_shift_r) << g_shift_l;
          blue  = ((data & b_mask) >> b_shift_r) << b_shift_l;
          alpha = ((data & a_mask) >> a_shift_r) << a_shift_l;
        }
        data = alpha | red | green | blue;
        if (8 == dest_bpp)
        {
          ((BYTE *)locked_rect.pBits)[idx_2+x * dest_bytes ] = (BYTE)(data & 0xFF);
        }
        else if (16 == dest_bpp)
        {
          *((WORD*)&(((BYTE *)locked_rect.pBits)[idx_2+x * dest_bytes ])) = (WORD)(data & 0xFFFF);
        }
        else
        {
          *((DWORD*)&(((BYTE *)locked_rect.pBits)[idx_2+x * dest_bytes])) = data;
        }
      }
    }
  }
  mpTexture->UnlockRect(level);

  return S_OK;
}


HRESULT IawTexture::WriteFontToTexture()
{
  //create a font with the right attribs we need
  HFONT fontTempFont = CreateFont((mWidth/16),
                                  (mWidth/16),
                                  0,
                                  0,
                                  FW_BOLD,
                                  false,
                                  false,
                                  false,
                                  ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS,
                                  DEFAULT_QUALITY,
                                  DEFAULT_PITCH | FF_MODERN,
                                  NULL);

  // This is done twice, once for the color channel, once for alpha
  // maybe I could have just blitted it?

  //FIRST TIME = COLOR

  // Test writing on the bitmap, and then restoring the texture
  HDC bitmap = CreateCompatibleDC(NULL);

  SelectObject(bitmap, fontTempFont);

  if (NULL == bitmap)
  {
    OutputDebugString("couldn't get device context \n");
    return E_FAIL;
  }

  SelectObject(bitmap, mBitmap);
  SetTextColor(bitmap,0x00ffffff);
  SetBkColor(bitmap,0x00000000);

  SetTextAlign(bitmap, TA_CENTER);

  for (int y = 0; y < FONT_TEXT_CHARS_PER_SIDE; y++)
  {
    for (int x = 0;x < FONT_TEXT_CHARS_PER_SIDE; x++)
    {
      int xpos = mWidth/(2*FONT_TEXT_CHARS_PER_SIDE) + (x * mWidth/FONT_TEXT_CHARS_PER_SIDE);
      int ypos = y * mWidth/FONT_TEXT_CHARS_PER_SIDE;
      char str[2];
      sprintf(str,"%c",(y*FONT_TEXT_CHARS_PER_SIDE+x));
      //if (y ==3)
      // OutputDebugString(str);
      TextOut(bitmap,xpos,ypos,str,1);
    }
  }

  // Draw some lines to make sure there will be no char bleeding 
  if (mWidth > 64) //if the size is too small, I don't want to do this.
  {
    LOGBRUSH long_brush; 
    HPEN pen;

    long_brush.lbStyle = BS_SOLID; 
    //long_brush.lbColor = RGB(255,0,255); //use this for debugging text
    long_brush.lbColor = RGB(0,0,0); 
    long_brush.lbHatch = 0; 

    pen = ExtCreatePen(PS_COSMETIC | PS_SOLID,
                       mWidth/*FONTTEXTURESIZE*/ / 128,
                       &long_brush,
                       0,
                       NULL);

    SelectObject(bitmap, pen);

    int height = mHeight/FONT_TEXT_CHARS_PER_SIDE;
    for (int j = 0; j < (int)mHeight; j+=height)
    {
      //moving down across rows, do one on first pixel, one on second
      MoveToEx(bitmap, 1, j, NULL); 
      LineTo(bitmap, mWidth-1, j); 

      MoveToEx(bitmap, 1, j+height-1, NULL); 
      LineTo(bitmap, mWidth-1, j+height-1); 
    }

    int width = mWidth/FONT_TEXT_CHARS_PER_SIDE;
    for (int i = 0; i < (int)mWidth; i+=width)
    {
      //moving down across rows, do one on first pixel, one on second
      MoveToEx(bitmap, i, 1, NULL); 
      LineTo(bitmap, i, mWidth-1); 

      MoveToEx(bitmap, i+width-1, 1, NULL); 
      LineTo(bitmap, i+width-1, mWidth-1); 
    }
    DeleteObject(pen);
  }

  DeleteDC(bitmap); //bitmap->ReleaseDC(bitmap);

  // Second pass for alpha...

  //test writing on the bitmap, and then restoring the texture
  HDC alpha_bitmap = CreateCompatibleDC(NULL);

  SelectObject(alpha_bitmap, fontTempFont);

  if (NULL == alpha_bitmap)
  {
    OutputDebugString("couldn't  get  device  context  \n");
    return  E_FAIL;
  }

  SelectObject(alpha_bitmap, mAlphaBitmap);
  SetTextColor(alpha_bitmap, 0x00ffffff);
  SetBkColor(alpha_bitmap, 0x00000000);

  SetTextAlign(alpha_bitmap, TA_CENTER);

  for (y = 0; y < FONT_TEXT_CHARS_PER_SIDE; y++)
  {
    for (int x = 0; x < FONT_TEXT_CHARS_PER_SIDE; x++)
    {
      int xpos = mWidth/(2*FONT_TEXT_CHARS_PER_SIDE) + (x * mWidth/FONT_TEXT_CHARS_PER_SIDE);
      int ypos = y * mWidth/FONT_TEXT_CHARS_PER_SIDE;
      char str[2];
      sprintf(str,"%c",(y*FONT_TEXT_CHARS_PER_SIDE+x));
      //if (y  ==3)
      //  OutputDebugString(str);
      TextOut(alpha_bitmap, xpos, ypos, str, 1);
    }
  }

  // If the size is not too small, draw some lines to make sure there will be
  // no char bleeding 
  if (mWidth > 64)
  {
    LOGBRUSH long_brush;
    HPEN pen;

    long_brush.lbStyle = BS_SOLID;
    //long_brush.lbColor = RGB(255,0,255);  //use  this  for  debugging  text
    long_brush.lbColor = RGB(0,0,0); 
    long_brush.lbHatch = 0; 
    pen = ExtCreatePen(PS_COSMETIC | PS_SOLID, mWidth/*FONTTEXTURESIZE*/ /128, &long_brush, 0, NULL); 
    SelectObject(alpha_bitmap, pen);

    int height = mHeight/FONT_TEXT_CHARS_PER_SIDE;
    for (int j=0;j<(int)mHeight;j+=height)
    {
      //moving down across rows, do one on first pixel, one on second
      MoveToEx(alpha_bitmap, 1, j, NULL); 
      LineTo(alpha_bitmap, mWidth-1, j); 

      MoveToEx(alpha_bitmap, 1, j+height-1, NULL); 
      LineTo(alpha_bitmap, mWidth-1, j+height-1); 
    }

    int width = mWidth/FONT_TEXT_CHARS_PER_SIDE;
    for (int i=0;i<(int)mWidth;i+=width)
    {
      //moving down across rows, do one on first pixel, one on second
      MoveToEx(alpha_bitmap, i, 1, NULL); 
      LineTo(alpha_bitmap, i, mWidth-1); 

      MoveToEx(alpha_bitmap, i+width-1, 1, NULL); 
      LineTo(alpha_bitmap, i+width-1, mWidth-1); 
    }
    DeleteObject(pen);
  }

  DeleteDC(alpha_bitmap);

  return S_OK;
}

