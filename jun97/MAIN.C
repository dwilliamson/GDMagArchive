//main.c,  mainwindow functions and winmain()

#include "..\\include\\move.h"
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

LRESULT CALLBACK main_window_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam) {
//main window procedure
switch(msg) {
	case WM_DESTROY:
		close_direct_draw();
		PostQuitMessage(0);
	return 0;
	//Mouse messages
	 case WM_LBUTTONDBLCLK:
		mouse_double_click=1;
	 break;
	 case WM_LBUTTONDOWN:
		mouse_code|=1;
	 break;
	 case WM_RBUTTONDOWN:
		mouse_code|=2;
	 break;
	 case WM_LBUTTONUP:
		if (mouse_code & 1)
			{mouse_code^=1;
			 mouse_click=1;
			 }
	 break;
	 case WM_RBUTTONUP:
		if (mouse_code & 2)
			{mouse_code^=2;
			 mouse_click=2;
			 }
	 break;
	case WM_CHAR:
	//A key has been pressed
		key_pressed=wparam;
	break;
	}
return(DefWindowProc(hwnd,msg,wparam,lparam));
}

static BOOL init_instance(HANDLE inst,int show) {
//Initialize instance
HWND hwnd;
hwnd=CreateWindowEx(
		WS_EX_TOPMOST,
		"MOVE_DEMO",
		game_title,
		WS_SYSMENU | WS_OVERLAPPED | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN)/2-320,GetSystemMetrics(SM_CYSCREEN)/2-240,640,480,
		NULL,
		NULL,
		inst,
		NULL);
if(hwnd==NULL) return (FALSE);
mainwindow=hwnd;
ShowWindow(hwnd,show);
UpdateWindow(hwnd);
return( TRUE );
}

BOOL init_app( HANDLE inst ) {
//Initialize application
WNDCLASS wc;
strcpy(game_title,"Demonstration of real time unit movement");
wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)MakeProcInstance((FARPROC)main_window_proc,inst);
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=inst;
wc.hIcon=NULL;
wc.hCursor=LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);//GetStockObject(BLACK_BRUSH);
wc.lpszMenuName=NULL;
wc.lpszClassName="MOVE_DEMO";
if (!RegisterClass(&wc)) return FALSE;
return TRUE;
}

int PASCAL WinMain(HANDLE inst,HANDLE prev_inst,LPSTR cmd_line,int show) {
MSG msg;
//Initialize stuff
//Init application
if(!prev_inst)
	if(!init_app(inst))
		return FALSE;
//Init instance
if (!init_instance(inst,show))
	return FALSE;
//Init direct draw
if (init_direct_draw(mainwindow))
	return FALSE;
//Read and set palette
if (!init_game_palette())
	return FALSE;
//Allocate video memory
if (!init_gfx_mode())
	return FALSE;
//Read the color tables for the shadows and recoloring
if (!read_color_tables())
	return FALSE;
//Read the tiles
if (!read_images())
	return FALSE;
//Fill the map
if (!set_map(CLUTTER_OBSTACLES))
	return FALSE;
//Read a font
if (!init_active_font("static\\font"))
	return FALSE;
//Prevent the windows mouse cursor from popping up
ShowCursor(FALSE);
SetCapture(mainwindow);
//Start looping
while(1)
	{if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{//Handle messages
		 if(!GetMessage(&msg,NULL,0,0))
				return msg.wParam;
		 TranslateMessage(&msg);
		 DispatchMessage(&msg);
		}
	else
		//Go to main game loop
		game_loop();
	}
}
