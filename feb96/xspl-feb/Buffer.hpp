/***************************************************************************
	XSplat Cross-Platform Game Development Library
	Offscreen Buffering class declaration for Mac and Windows

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

class COffscreenBuffer
{
public:
	// Basic information access
	char unsigned *GetBits(void) { return pBits; };
	long GetStride(void) const { return Stride; };
	int GetWidth(void) const { return Width; };
	int GetHeight(void) const { return Height; };

	// Displaying the buffer
	void SwapBuffer(void) const;
	void SwapRect(int Left, int Top, int Right, int Bottom) const;

	// Pixel access control
	void Lock(void);
	void Unlock(void);

	// Constructor and Destructor
	COffscreenBuffer(void);
	~COffscreenBuffer(void);

private:
	// Common implementation data
	char unsigned *pBits;
	long Stride;
	int Height;
	int Width;

#if defined (_WINDOWS)
// Windows implementation details

	HWND TargetWindow;
	HDC OffscreenDC;
	HBITMAP OffscreenBitmap;
	HBITMAP OriginalMonoBitmap;

#elif defined(_MACINTOSH)
// Macintosh implementation details

	GWorldPtr OffscreenGWorld;

#endif
};

inline void COffscreenBuffer::SwapBuffer(void) const
{
	SwapRect(0, 0, Width, Height);
}

#if defined(_WINDOWS)
// More Windows implementation details
// Lock and Unlock are no-ops in Windows
inline void COffscreenBuffer::Lock(void) {};
inline void COffscreenBuffer::Unlock(void) {};
#endif
