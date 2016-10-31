// IawDataStream.h App Wizard Version 2.0 Beta 1
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


#if !defined(IawDataStream_h)
#define IawDataStream_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * This is a pure virtual interface declaration for derived classes that will
 * implement data loading from various sources.
 * @see IawFileStream
 * @see IawUrlStream
 */
class IawDataStream
{
public:
  /**
   * Used for data stream pointer positioning.
   * @see Seek
   */
  enum SeekStart
  {
    seek_current,
    seek_beginning,
    seek_end
  };

  /* No constructor since you cannot instanciate a pure virtual class. */
  /** virtual destructor */
  virtual ~IawDataStream() {};

  /**
   * Read a specified number of bytes from the data stream of the class and
   * copy them into a buffer.
   *
   * <strong>Implementation is left to derived classes</strong>
   *
   * @param pBuffer The buffer to copy input into.
   * @param bytesToRead The number of bytes to read.
   * @param actualBytesRead The actual number of bytes copied.
   *
   * @return Success code of the read.
   */
  virtual HRESULT ReadBytes(void* pBuffer, UINT bytesToRead, UINT& rActualBytesRead) = 0;

  /**
   * Write a specified number of bytes to the data stream of the class.
   *
   * <strong>Implementation is left to derived classes</strong>
   *
   * @param pBuffer The buffer to write from.
   * @param bytesToRead The number of bytes to write.
   * @param actualBytesRead The actual number of bytes written.
   *
   * @return Success code of the write.
   */
  virtual HRESULT WriteBytes(void* pBuffer, UINT bytesToWrite, UINT& rActualBytesWritten) = 0;

  /**
   * Reposition data stream pointer to desired location.
   * @param pos Location to start seeking from (beginning, end, or current pos)
   * @param distance Number of bytes to seek
   * @param rNewPos New data stream pointer location.
   * @retrun Success code of the seek.
   */
  virtual HRESULT Seek(SeekStart pos, int distance, UINT& rNewPos) = 0;

  /**
   * Get the size of the data stream.
   * @param rSize Returned size of data stream
   * @return Success code of the query.
   */
  virtual HRESULT GetSize(UINT& rSize) = 0;
};

#endif // !defined(IawDataStream_h)

