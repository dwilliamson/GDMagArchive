/****************************************************************************
	System message processing
	Macintosh Version

	DispatchEvents hides system-specific event processing methods from the
	game itself.

	On the Macintosh, we're responsible for handling most events and
	passing them on to the appropriate handler or handling them by
	default.
 ****************************************************************************/

#include "XSplat.hpp"

#ifdef _MACINTOSH

/****************************************************************************
	Global Dependencies
*/

extern int GameRunningFlag;

// For mouse control
static CXSplatWindow *pMouseCapture = 0;
static Point LastMouse = { 0, 0 };

/****************************************************************************
	DispatchEvents
*/

static void HandleMouseDown(EventRecord *pEvent);

void DispatchEvents(void)
{
	EventRecord	Event;
	WindowPtr Window;
	CXSplatWindow *pXSplatWindow = 0;

	// If you want your application to multitask better, pass non-zero
	// as the yield time to WaitNextEvent
	while (WaitNextEvent(everyEvent, &Event, 0, 0))
	{
		switch(Event.what)
		{
			case mouseDown:
				HandleMouseDown(&Event);
				break;
			
			case mouseUp:
				// Send mouse-ups to the CXSplatWindow that handled the
				// mouse-down
				if (pMouseCapture)
				{
					Point pt = Event.where;
					// However, GlobalToLocal will only work if the
					// mouse-down receiving window is the active GrafPort.
					GlobalToLocal(&pt);
					pMouseCapture->MouseUp(pt.h, pt.v);
					
					// Release mouse input
					pMouseCapture = 0;
				}
				break;
			

			case autoKey:
				break;

			case keyUp:
			case keyDown:
				Window = FrontWindow();
				if (Window)
					pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
				// else pXSplatWindow has been initialized to 0
				if (pXSplatWindow)
				{
					if ((Event.modifiers & cmdKey) == 0)
					{
						if (Event.what == keyDown)
							pXSplatWindow->KeyDown((char)Event.message);
						else
							pXSplatWindow->KeyUp((char)Event.message);
					}
					else
					{
						// TODO: We'll add menus in a future article
						// For now, we'll include Command-Q, though
						if ((Event.message & 0xFF) == 'Q' ||
							(Event.message & 0xFF) == 'q')
						{
							GameRunningFlag = 0;
						}
					}
				}
				break;
			
			case updateEvt:
				Window = (WindowPtr)Event.message;
				BeginUpdate(Window);
				
				pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
				if (pXSplatWindow)
				{
					COffscreenBuffer *pBuffer =
						pXSplatWindow->GetOffscreenBuffer();
					if (pBuffer)
					{
						pBuffer->Lock();
						pBuffer->SwapBuffer();
						pBuffer->Unlock();
					}
				}
					
				EndUpdate(Window);
				break;

			case activateEvt:
				Window = (WindowPtr)Event.message;
				pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
				if (pXSplatWindow)
				{
					if (Event.modifiers & activeFlag)
						pXSplatWindow->Activate();
					else
						pXSplatWindow->Deactivate();
				}
				break;
			
			case osEvt:
				// Look for suspend/resume events
				switch (Event.message >> 24)
				{
					case suspendResumeMessage:
						// TODO: In a multiple-window application, this
						// TODO: should call Deactivate for ALL windows
						// TODO: in the app
						Window = FrontWindow();
						if (Window)
							pXSplatWindow =
								(CXSplatWindow *)GetWRefCon(Window);
						// else pXSplatWindow has been initialized to 0
						if (pXSplatWindow)
						{
							if (Event.message & resumeFlag)
								pXSplatWindow->Activate();
							else
								pXSplatWindow->Deactivate();
						}
						break;
					
					default:
						// Ignore others
						break;
				}
			
		}
	}

	// Look to see if the mouse has moved anywhere in the system.
	// Send the mouse move to the CXSplatWindow where a mouse down
	// occured, or else send the mouse move to the front window.
	// Only do this if the application is active.
	Point ThisMouse;
	GetMouse(&ThisMouse);
	
	if (ThisMouse.h != LastMouse.h || ThisMouse.v != LastMouse.v)
	{
		pXSplatWindow = 0;

		if (pMouseCapture)
			pXSplatWindow = pMouseCapture;
		else
		{
			Window = FrontWindow();
			if (Window)
				pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
		}
		
		if (pXSplatWindow)
		{
			pXSplatWindow->MouseMove(ThisMouse.h, ThisMouse.v);
			LastMouse = ThisMouse;
		}
	}
}

/****************************************************************************
	Standard Event Handlers
*/

static void HandleMouseDown(EventRecord *pEvent)
{
	WindowPtr Window;
	CXSplatWindow *pXSplatWindow = 0;

	int WindowCode = FindWindow(pEvent->where, &Window);

	switch (WindowCode)
	{
		case inSysWindow:
			SystemClick(pEvent, Window);
			break;
			
		case inMenuBar:
			// TODO: We'll add menus in a future article
			// For now, just enable the Apple menu
			long menuResult;
			short menuID, menuItem;
			Str255 daName;
			
			menuResult = MenuSelect(pEvent->where);
			menuID = menuResult >> 16;
			menuItem = menuResult & 0xFFFF;
			if (menuID == 128)
			{
				GetItem(GetMHandle(128), menuItem, daName);
				OpenDeskAcc(daName);
			}
			HiliteMenu(0);
			break;
		
		case inDrag:
			DragWindow(Window, pEvent->where, &qd.screenBits.bounds);
			break;

		case inGrow:
		case inZoomIn:
		case inZoomOut:
			// For now, no window resizing
			break;
		
		case inContent:
			if (Window != FrontWindow())
				SelectWindow(Window);
			else if (Window)
			{
				pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
				if (pXSplatWindow)
				{
					// Restrict mouse events to this CXSplatWindow until
					// the next mouse up
					pMouseCapture = pXSplatWindow;
					
					Point pt = pEvent->where;
					GlobalToLocal(&pt);
					pXSplatWindow->MouseDown(pt.h, pt.v);
				}
			}
			break;
		
		case inGoAway:
			pXSplatWindow = (CXSplatWindow *)GetWRefCon(Window);
			if (pXSplatWindow &&
				TrackGoAway(Window, pEvent->where))
			{
				// Avoid confusion when deleting the CXSplatWindow
				DisposeWindow(Window);
				pXSplatWindow->Window = 0;
				
				// For now, we'll assume there's only one game window
				// and that the game should end when it's removed
				GameRunningFlag = 0;
			}
			break;
	}
}

#endif // _MACINTOSH
