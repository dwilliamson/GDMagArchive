// IawImageLoader.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawImageLoader_h)
#define IawImageLoader_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This class is passed to the IawTexture class for loading image data.
 * Classes are derived from this one to accomodate different image formats.
 * @see IawBmpLoader
 */
class IawImageLoader
{
public:
  IawImageLoader();
  virtual ~IawImageLoader();
  
  /**
   * Derived classes must implement this method.
   * When this method is called, the class should load its image data and
   * create the image to be passed back.  If an alpha channel is available in
   * the image format, it should also be returned.
   * @param pImage The image to return.
   * @param pAlpha The alpha image to return
   */
  virtual HRESULT LoadImage(HBITMAP* pImage, HBITMAP* pAlpha) = 0;

protected:
  HBITMAP mImage;       /**< Bitmap image handle. */
  HBITMAP mImageAlpha;  /**< Bitmap image handle for alpha info. */
};

#endif // !defined(IawImageLoader_h)

