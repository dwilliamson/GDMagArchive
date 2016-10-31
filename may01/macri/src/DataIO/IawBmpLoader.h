// IawBmpLoader.h App Wizard Version 2.0 Beta 1
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


#if !defined(IawBmpLoader_h)
#define IawBmpLoader_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * A bitmap loading class.
 * Loads a Windows bitmap.
 */
class IawBmpLoader : public IawImageLoader
{
public:
  /**
   * Constructor.
   * @param rSource A properly initialized and/or casted IawDataStream.
   */
  IawBmpLoader(IawDataStream& rSource);

  /** Default destructor. */
  virtual ~IawBmpLoader();

  /**
   * Implements the parent class virtual method IawImageLoader::LoadImage().
   * The hAlpha bitmap will never be set because .BMP files don't support alpha
   * channels.
   */
  HRESULT LoadImage(HBITMAP* pImage, HBITMAP* pAlpha);

protected:
  /** The input data stream. */
  IawDataStream& mrDataSource;
};

#endif // !defined(IawBmpLoader_h)

