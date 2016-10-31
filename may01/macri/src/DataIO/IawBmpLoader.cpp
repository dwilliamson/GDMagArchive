// IawBmpLoader.cpp App Wizard Version 2.0 Beta 1
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
// IawBmpLoader.cpp: implementation of the CIawBmpLoader class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// How wide, in bytes, would this many bits be, DWORD aligned? 
#define WIDTHBYTES(bits) ((((bits) + 31)>>5)<<2)

// Constructor
IawBmpLoader::IawBmpLoader(IawDataStream& rSource) :
  mrDataSource(rSource)
{
}

// Destructor
IawBmpLoader::~IawBmpLoader()
{
}

// Implementation of parent class virtual method LoadImage().
HRESULT IawBmpLoader::LoadImage(HBITMAP* pImage, HBITMAP* pAlpha)
{
  HDC device = NULL;  // Is this okay?  May need to test on a multi-monitor configuration
  BITMAPFILEHEADER bfh;
  struct 
  {
    BITMAPINFOHEADER bmih;
    RGBQUAD	pal[256];
  } bmi;
  void* pBits = NULL;
  UINT bytes_read;
  DWORD bytes_per_line;
  DWORD bits_size;

  // Set the bitmaps to NULL initially (And the BMP loader will NEVER load an Alpha)
  if (pImage)
    *pImage = NULL;
  if (pAlpha)
    *pAlpha = NULL;

  // Read the BMP Header from the DataStream
  memset(&bfh, 0, sizeof(bfh));
  if (FAILED(mrDataSource.ReadBytes(&bfh, sizeof(bfh), bytes_read)) || 
      (bytes_read != sizeof(bfh)))
  {
    return ERROR_INVALID_DATA;
  }

  // Check to see if this is a BMP file
  if ((bfh.bfType != 0x4d42) || (bfh.bfReserved1!=0) || (bfh.bfReserved2!=0))
  {
    //  It doesn't look familiar, so bail out
    return ERROR_INVALID_DATA;
  }

  // Read in the BITMAPINFOHEADER
  if (FAILED(mrDataSource.ReadBytes(&bmi.bmih, sizeof(BITMAPINFOHEADER), bytes_read)) ||
      (bytes_read != sizeof(BITMAPINFOHEADER)))
  {
    // Houston, we have a problem
    return ERROR_INVALID_DATA;
  }

  // Verify that we understand this BMP format (not supporting OS/2 BMPs for now)
  if (bmi.bmih.biSize != sizeof(BITMAPINFOHEADER))
  {
    // This could be a corrupt file, an OS/2 format, or some new format we
    // don't know about.  Just bail...
    return ERROR_INVALID_DATA;
  }

  // Do we need to read in a palette?
  DWORD palette_size = 0;
  if (bmi.bmih.biClrUsed)
    palette_size = bmi.bmih.biClrUsed;
  else
  {
    switch(bmi.bmih.biBitCount)
    {
    case 1:
      palette_size = 2;
      break;
    case 4:
      palette_size = 16;
      break;
    case 8:
      palette_size = 256;
      break;
    }
  }

  // We can't handle this
  if (palette_size > 256)
    return ERROR_INVALID_DATA;

  if (palette_size)
  {
    if (FAILED(mrDataSource.ReadBytes(bmi.pal, palette_size * sizeof(RGBQUAD), bytes_read)) || 
        (bytes_read != palette_size * sizeof(RGBQUAD)))
    {
      // Something's fishy
      return ERROR_INVALID_DATA;
    }
  }

  // Calculate the number of bytes in a line of the image.  This needs to be DWORD aligned.
  bytes_per_line = WIDTHBYTES(bmi.bmih.biWidth * bmi.bmih.biPlanes * bmi.bmih.biBitCount);

  // How many total bytes will we need?
  bits_size = bmi.bmih.biHeight * bytes_per_line;

  // Okay, we should have everything necessary to create our DIB Section now, so do it
  mImage = CreateDIBSection(device,
                            (const struct tagBITMAPINFO *)&bmi,
                            DIB_RGB_COLORS,
                            &pBits,
                            NULL,
                            0);
  if (mImage == NULL)
  {
    // Who knows why, but we couldn't create the DIB Section -- we must bail
    return ERROR_INVALID_DATA;
  }

  // Added this for Windows 2000
  GdiFlush();

  // If the "File" header has an offset to the bits, we'll seek to that offset.
  // Otherwise, we assume the bits of the image immediately follow the
  // header/palette.
  if (bfh.bfOffBits != 0)
  {
    UINT new_pos;
    if (FAILED(mrDataSource.Seek(IawFileStream::seek_beginning, bfh.bfOffBits, new_pos)) ||
        (new_pos != bfh.bfOffBits))
    {
      // The seek failed.  Destroy our DIB Section and get out of here
      DeleteObject(mImage);
      return ERROR_INVALID_DATA;
    }
  }

  // We're coming in for a landing.  Read in the image bits and we're done
  if (FAILED(mrDataSource.ReadBytes(pBits, bits_size, bytes_read)) ||
      (bytes_read != bits_size))
  {
    // Darn it, we were almost there.  Oh well, cleanup and get out
    DeleteObject(mImage);
    return ERROR_INVALID_DATA;
  }

  // We're done.  Return the DIB Section 
  if (pImage)
    *pImage = mImage;

  return S_OK;
}

