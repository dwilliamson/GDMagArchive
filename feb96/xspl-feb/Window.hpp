/***************************************************************************
	X-Splat Cross-Platform Game Development Library
	Game Window class declaration for Mac and Windows

	This code is Copyright (c) 1995 by Jon Blossom. All Rights Reserved.
 ***************************************************************************/

/***************************************************************************
	CXSplatWindow declaration
*/

class CXSplatWindow
{
public:
	CXSplatWindow(char const *Caption, int WindowWidth=0, int WindowHeight=0,
		unsigned char const *Colors = 0);
	virtual ~CXSplatWindow(void);

	virtual void Activate(void) { IsActiveFlag = 1; };
	virtual void Deactivate(void) { IsActiveFlag = 0; };
	virtual void Idle(void) {};

	virtual void KeyDown(char unsigned Key) {};
	virtual void KeyUp(char unsigned Key) {};

	virtual void MouseDown(int x, int y) { MouseDownFlag = 1; };
	virtual void MouseUp(int x, int y) { MouseDownFlag = 0; };
	virtual void MouseMove(int x, int y) {};

	COffscreenBuffer *GetOffscreenBuffer(void)
		{ return pOffscreenBuffer; };

protected:
	int IsActiveFlag;
	int MouseDownFlag;
	COffscreenBuffer *pOffscreenBuffer;

#if defined(_WINDOWS)
// Windows implementation details

	HPALETTE Palette;
	HWND Window;

	friend LONG PASCAL XSplatWindowProc(HWND, UINT, WPARAM, LPARAM);

#elif defined(_MACINTOSH)
// Macintosh implementation details

	WindowPtr Window;
	PaletteHandle WinPalette;

	friend void HandleMouseDown(EventRecord *pEvent);

#endif
};
