//  IawTexture.h  App Wizard Version 2.0 Beta 1
//  ----------------------------------------------------------------------
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

//  ----------------------------------------------------------------------
//  Authors:  Kim Pallister,Dean Macri  - Intel Technology Diffusion Team
//  ----------------------------------------------------------------------

#if  !defined(IawTexture_h)
#define  IawTexture_h

#if _MSC_VER  >=  1000
#pragma once
#endif //  _MSC_VER  >=  1000

/**
 * A texture class.
 * This class encapsulates the idea of a texture.  As such, it handles texture
 * creation, restoration, and deletion.
 */
class IawTexture
{
public:
  /**
   * Constructor.
   * @param pWrapper A pointer to an initialized D3D wrapper.
   */
  IawTexture(IawD3dWrapper* pWrapper);

  /**
   * Create a texture with no alpha image source.
   * Load an image using the referenced ImageLoader and create a texture from
   * it. The texture cannot be used until it is restored.
   *
   * <i>Note that alpha can come from IAWTEXTR_TRANSPARENT_BLACK/WHITE flags.</i>
   */
  HRESULT CreateTexture(IawImageLoader& rTextureLoader,
                        DWORD textureStage = 0L,
                        DWORD textureFlags = 0L);

  /**
   * Create a texture with an alpha channel from another image source.
   */
  HRESULT CreateTexture(IawImageLoader& rTextureLoader,
                        IawImageLoader& rAlphaTextureLoader,
                        DWORD textureStage = 0L,
                        DWORD textureFlags = 0L);

  /**
   * Create a blank texture, or one from a straight array of bits.
   */
  HRESULT CreateTexture(DWORD hRes,
                        DWORD vRes,
                        DWORD bpp,
                        DWORD textureStage = 0L,
                        DWORD textureFlags = 0L,
                        DWORD* pBits = NULL);

  /**
   * Invalidate the current texture object and rebuild it with the given device
   *
   * Destroy the current device texture and create a new one from the hbitmap
   * and flags member variables, and device info. Be aware that even if the
   * Restore worked once, it may fail later.  For example if you restore after
   * switching to a device with different caps, or switch to a higher
   * resolution and run out of video memory.
   * <b>Always check return values!</b>
   * @return Success/fail code of function call.
   */
  HRESULT Restore();

  /**
   * Similar to Restore().
   * Change the stage that the texture was originally created for. This is used
   * when falling back from a multi-texture approach to a multi-pass approach,
   * or similar.
   * @param newStage The stage for which the texture was originially created.
   * @see Restore()
   */
  HRESULT Restore(DWORD newStage);

  /** Default destructor. */
  virtual  ~IawTexture();

  /** Release allocated resources */
  void  Cleanup();

  //  Constants...
  static const int IAW_TEXTR_TRANSPARENT_WHITE;//  0x00000002
  static const int IAW_TEXTR_TRANSPARENT_BLACK;//  0x00000004
  static const int IAW_TEXTR_32_BITS_PER_PIXEL;//  0x00000008

  /** Create a texture with DDSCAPS_3DDEVICE in order to render to it. */
  static const int IAW_TEXTR_RENDER_TARGET;//  0x00000010

  /** Create full mip maps for the texture chain. */
  static const int IAW_TEXTR_CREATE_MIP_MAPS;//  0x00000020

  /** This specifies to create the texture in system memory. */
  static const int IAW_TEXTR_FORCE_IN_SYS_MEM;//  0x00000040

  /** This specifies to create the texture in video (or AGP) memory. */
  static const int IAW_TEXTR_FORCE_IN_VID_MEM;// 0x00000080

  /**
   * Used for embossing and bump mapping.
   * This says to create a texture which is (0.5 - src texture).
   */
  static const int IAW_TEXTR_CREATE_INV_HEIGHT_MAP;// 0x00000100

  /**
   * Used for embossing and bump mapping.
   * This says to create a texture with DUDV pixel format.
   */
  static const int IAW_TEXTR_CREATE_BUMP_MAP_DUDV;// 0x00000200

  /**
   * Use this to force an alpha chanel even if not supplying a bitmap for it.
   */
  static const int IAW_TEXTR_MUST_HAVE_ALPHA;// 0x00000400 
  static const int IAW_TEXTR_16_BITZ;// 0x00000800
  static const int IAW_TEXTR_32_BITZ;// 0x00001000

  /** This creates a 16x16 grid of chars laid out in ASCII order */
  static const int IAW_TEXTR_CREATE_FONT;// 0x00002000

  //@{
  /** Text related constant used for building procedural fonts */
  static const int FONT_TEXTURE_SIZE;
  static const int FONT_TEXT_CHARS_PER_ROW;
  static const int BOTTOM_COLOR;
  static const int TOP_COLOR;
  //@}

  static const int MAX_TEXTURE_FORMAT_TRIES;// 10

  static const int FONT_TEXT_CHARS_PER_SIDE;

  // Variables...
  /** Windows bitmap containing texture image. */
  HBITMAP mBitmap;

  /** Windows bitmap containing alpha image. */
  HBITMAP mAlphaBitmap;

  /** Direct3D texture. */
  LPDIRECT3DTEXTURE8 mpTexture;

  /** Texture stage (for multi-textures). */
  DWORD mStage;

  /** This is used to signify 4 or 8 bit alpha plane. */
  bool mHasAlpha;

  IawImageLoader* mpTextureLoader;
  IawImageLoader* mpAlphaTextureLoader;
  DWORD mCreateFlags;

  TCHAR *mTexturePath;
  IawD3dWrapper *mpWrapper;

  /** Texture width, height, and bits per pixel */
  DWORD mWidth, mHeight, mBpp;

protected:
  /**
   * Select the parameters for a texture.
   * @param rFormat The Direct3D format of the input.
   * @param rPool The Direct3D pool.
   * @param rLevels An unsigned integer representing the number of levels.
   * @param rUsage An unsigned integer representing the usage.
   */
  virtual HRESULT SelectParameters(D3DFORMAT& rFormat,
                                   D3DPOOL& rPool,
                                   UINT& rLevels,
                                   UINT& rUsage);

  /** Create attached surfaces. */
  virtual HRESULT CreateAttachedSurfaces();

  /** Load a texture map file (or resource) into a BITMAP surface. */
  HRESULT LoadTextureImage();

  /** Copy an image of a bitmap into a surface. */
  HRESULT CopyBitmapToSurface();

  /**
   * Restore a texture from a bitmap.
   * Invalidate the current texture objects and rebuild new ones using the
   * new device.
   */
  HRESULT RestoreFromBitmap();

  /**
   * Copy bits from an array to a texture surface.
   * Assumes that <b><i>pBits</i></b> contains the information in the correct
   * format for the texture surface
   *
   * @param pTexture A pointer to the Direct3D texture to copy from.
   * @param level The level in a mip map chain.
   * @param pBits A pointer to the block of memory the texture will copy into.
   */
  virtual HRESULT CopyBitsStraight(LPDIRECT3DTEXTURE8 pTexture,
                                   int level,
                                   DWORD* pBits);

  /**
   * Copy bits doing some color conversion on the fly.
   * The source data is assumed to be 24-bit RGB. For the alpha channel, the
   * red component is the only one used.
   */
  virtual HRESULT CopyBits(LPDIRECT3DTEXTURE8 pTexture,
                           int level,
                           DWORD bitsPitch,
                           DWORD bitsWidth,
                           DWORD bitsHeight,
                           BYTE* pBits,
                           BYTE* pAlphaBits);

  /**
   * Write a Windows font to texture format.
   */
  HRESULT WriteFontToTexture();
};

#endif // !defined(IawTexture_h)

