/*
header file for testing ground program

neweng.h
*/


#pragma once

// -==Headers==-

#include <windows.h>

// -==Classes==-

typedef class NewEng {

public:

	// Constructor / Destructor

	NewEng(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	~NewEng();

	// App flow control

	void Run();

	// Window procs

	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Data

	HWND hMainWindow;
	HINSTANCE hInstance;

//protected:


} NewEng, *LPNewEng;

// -==Prototypes==-
extern LPNewEng _app;

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);