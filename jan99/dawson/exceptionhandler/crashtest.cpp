#include <string.h>		// For strcpy

// Copyright © 1998 Bruce Dawson.

/*
This source file causes a number of different crashes in order to exercise
the exception handler.
*/

// Disable the optimizer, otherwise it might 'fix' some of the 'bugs'
// that I've placed in my code for test purposes.
#pragma optimize("g", off)

typedef void (*tBogusFunction)();

void __cdecl CrashTestFunction(int CrashCode)
{
	char *p = 0;	// Null pointer.
	char x = 0;
	int y = 0;
	switch (CrashCode)
	{
		case 0:
		default:
			*p = x;	// Illegal write.
			break;
		case 1:
			x = *p;	// Illegal read.
			break;
		case 2:
			strcpy(0, 0);	// Illegal read in C run time.
			break;
		case 3:
		{
			tBogusFunction	BadFunc = (tBogusFunction)0;
			BadFunc();	// Illegal code read - jump to address zero.
			break;
		}
		case 4:
			y = y / y;	// Divide by zero.
			break;
	}
}
