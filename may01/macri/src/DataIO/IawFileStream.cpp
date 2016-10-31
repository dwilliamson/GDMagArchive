// IawFileStream.cpp App Wizard Version 2.0 Beta 1
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
// IawFileStream.cpp: implementation of the CIawFileStream class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

// Constants...
const int IawFileStream::OPEN_READ      = 0x01;
const int IawFileStream::OPEN_WRITE     = 0x02;
const int IawFileStream::OPEN_CREATE    = 0x04;
const int IawFileStream::OPEN_TRUNCATE  = 0x08;

// Constructor
IawFileStream::IawFileStream(char* file, DWORD flags)
{
  DWORD access = 0;
  DWORD share = 0;
  DWORD creation_disposition = 0;

  if (flags & OPEN_READ)
  {
    access |= GENERIC_READ;
    share = FILE_SHARE_READ;
  }
  if (flags & OPEN_WRITE)
  {
    access |= GENERIC_WRITE;
    share = 0;
  }

  if (flags & OPEN_CREATE)
    if (flags & OPEN_TRUNCATE)
      creation_disposition = CREATE_ALWAYS;
    else
      creation_disposition = OPEN_ALWAYS;
    else if (flags & OPEN_TRUNCATE)
      creation_disposition = TRUNCATE_EXISTING;
    else
      creation_disposition = OPEN_EXISTING;

    mFileHandle = CreateFile((LPCTSTR)file, access, share, NULL, creation_disposition, 0, NULL);

    if (INVALID_HANDLE_VALUE == mFileHandle)
    {
      int error = GetLastError();
    }

    mFlags = flags;
}

// Destructor
IawFileStream::~IawFileStream()
{
  CloseHandle(mFileHandle);
}

// Read file
HRESULT IawFileStream::ReadBytes(void* pBuffer, UINT bytesToRead, UINT& rActualBytesRead)
{
  if (INVALID_HANDLE_VALUE == mFileHandle)
    return ERROR_FILE_NOT_FOUND;
  if (ReadFile(mFileHandle, pBuffer, bytesToRead, (DWORD*)&rActualBytesRead, NULL))
    return S_OK;

  return HRESULT_FROM_WIN32(GetLastError());
}

// Write file
HRESULT IawFileStream::WriteBytes(void* pBuffer, UINT bytesToWrite, UINT& rActualBytesWritten)
{
  if (INVALID_HANDLE_VALUE == mFileHandle)
    return ERROR_FILE_NOT_FOUND;
  if (WriteFile(mFileHandle, pBuffer, bytesToWrite, (DWORD*)&rActualBytesWritten, NULL))
    return S_OK;

  return HRESULT_FROM_WIN32(GetLastError());
}

// Seek in file
HRESULT IawFileStream::Seek(SeekStart pos, int distance, UINT& rNewPos)
{
  if (INVALID_HANDLE_VALUE == mFileHandle)
    return ERROR_FILE_NOT_FOUND;

  DWORD move_type;

  move_type = (seek_current == pos) ? FILE_CURRENT : ((seek_beginning == pos) ? FILE_BEGIN : FILE_END);

  SetLastError(NO_ERROR);
  rNewPos = SetFilePointer(mFileHandle, distance, NULL, move_type);

  if (NO_ERROR == GetLastError())
    return S_OK;

  int error = GetLastError();
  return HRESULT_FROM_WIN32(GetLastError());
}

// Get size of file
HRESULT IawFileStream::GetSize(UINT &size)
{
  if (INVALID_HANDLE_VALUE == mFileHandle)
    return ERROR_FILE_NOT_FOUND;

  size = GetFileSize(mFileHandle, NULL);

  if (NO_ERROR == GetLastError())
    return S_OK;

  return HRESULT_FROM_WIN32(GetLastError());
}

