/*
header to the file object

Zfile.h
*/

#pragma once

#include "ZTypes.h"
#include <windows.h>

typedef class ZFile {

public:

	// Constructors / Destructors

	ZFile();
	~ZFile();

	// Creation functions

	void Open(LPCSTR lpszPath);
	void Close();

	// Random access functions

	void SetPosition(GU32 dwDistanceToMove, GU32 dwMoveMethod);
	void Read(LPVOID lpBuffer, GU32 nNumberOfBytesToRead, GU32 *lpNumberOfBytesRead);

	void ReadByte(LPVOID lpBuffer, GU32 *lpNumberOfBytesRead) {
		BOOL bErr;
		// Read from the file.
		bErr = ReadFile(hFile, lpBuffer, 1, lpNumberOfBytesRead, NULL);
                //if(!bErr) _RPT0(_CRT_ERROR, "error reading file");
	}
	void ReadLine(LPVOID lpBuffer, GU32 *lpNumberOfBytesRead);

protected:

	HANDLE hFile;

} ZFile, *LPZFile;
