/***************************************************************************
	XSplat Cross-Platform Game Development Library
	Platform-independent timer functions

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

inline long unsigned GetMillisecondTime(void)
{
#if defined(_WINDOWS)

	return timeGetTime();

#elif defined(_MACINTOSH)

	return (long unsigned)TickCount() * 1000 / 60;

#endif
}
