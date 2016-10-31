// IawFileStream.h App Wizard Version 2.0 Beta 1
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

#if !defined(IawFileStream_h)
#define IawFileStream_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * A file stream class.
 */
class IawFileStream : public IawDataStream
{
public:
  /**
   * Constructor.
   * @param file The name of the file to open (a null terminated string).
   * @param flags Any file specific flags (read, write, etc...)
   */
  IawFileStream(char* file, DWORD flags = OPEN_READ);

  /** Default destructor */
  virtual ~IawFileStream();

  HRESULT ReadBytes(void *pBuffer, UINT uiBytesToRead, UINT &uiActualBytesRead);
  HRESULT WriteBytes(void *pBuffer, UINT uiBytesToWrite, UINT &uiActualBytesWritten);
  HRESULT Seek(SeekStart pos, int iDistance, UINT &newPos);
  HRESULT GetSize(UINT &size);

  static const int OPEN_READ;     /**< Open the file for reading. */
  static const int OPEN_WRITE;    /**< Open the file for writing. */
  static const int OPEN_CREATE;   /**< Create the file if it doesn't already exist. */
  static const int OPEN_TRUNCATE; /**< Truncate the file after opening it. */

protected:
  /** Handle to the class file */
  HANDLE mFileHandle;

  /** File specific flags */
  DWORD  mFlags;
};

#endif // !defined(IawFileStream_h)

