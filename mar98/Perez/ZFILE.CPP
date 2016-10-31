/*
the file object

Zfile.cpp
*/

//why isn't this commented?  because a) it's not origionally my code and
//b) you shouldn't even be reading this, go look in zportal.cpp!!!

#include "ZFile.h"
#include <crtdbg.h>

ZFile::ZFile() {

	hFile = NULL;
}

ZFile::~ZFile() {

	// Make sure to close the file handle

	if(hFile != NULL) Close();
}

void ZFile::Open(LPCSTR lpszPath) {

	// Create the file handle

	hFile = CreateFile(lpszPath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) _RPT0(_CRT_ERROR, "Couldn't open file");
}

void ZFile::Close() {

	BOOL bErr;

	// Close it and flush any unwritten data.

	bErr = FlushFileBuffers(hFile);
	if(!bErr) _RPT0(_CRT_ERROR, "couldn't flush file buffers");

	bErr = CloseHandle(hFile);
	if(!bErr) _RPT0(_CRT_ERROR, "couldn't close file");

	hFile = NULL;
}

void ZFile::SetPosition(GU32 dwDistanceToMove, GU32 dwMoveMethod) {

	DWORD dwErr;

	dwErr = SetFilePointer(hFile, dwDistanceToMove, NULL, dwMoveMethod);
        if(dwErr == 0xFFFFFFFF) _RPT0(_CRT_ERROR, "invalid file position");
}

void ZFile::Read(LPVOID lpBuffer, GU32 nNumberOfBytesToRead, GU32 *lpNumberOfBytesRead) {

	BOOL bErr;

	// Read from the file.

	bErr = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
        if(!bErr) _RPT0(_CRT_ERROR, "Error on file read");
}

//
//reads a line of text
//

void ZFile::ReadLine(LPVOID lpBuffer, GU32 *lpNumberOfBytesRead)
{
	//basically, keep on reading characters until \n is reached.
	GU8 foo='a';
	GU32 temp,num=0;;
	lpNumberOfBytesRead=0;
	while(foo!='\n')
	{
		ReadByte(&foo,&temp);
		*((GU8 *)lpBuffer+ num)=foo;
		num++;
		lpNumberOfBytesRead++;
	}
}

