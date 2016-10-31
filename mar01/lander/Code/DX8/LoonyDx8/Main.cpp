///////////////////////////////////////////////////////////////////////////////
//
// Main.cpp : implementation file
//
// Purpose:	Main Program Entry for Vertex Shader Demo
//
// Created:
//		JL 12/15/00
//
// Notes:	Modified from the Original Nividia (Microsoft?) Vertex Shader Demo
//			Original code doesn't appear to be available anymore
//			Portions Copyright (C) 1999, 2000 NVIDIA Corporation
//			
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2001 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "VShader.h"

CVShader	m_App;
bool		m_Dragging = false;
int			m_DragX,m_DragY;
D3DXVECTOR3	m_HitRot;

//------------------------------------------------------------------------------
LRESULT WINAPI MessageHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if((Msg == WM_KEYDOWN && wParam == VK_ESCAPE) ||
		(Msg == WM_DESTROY))
	{
		PostQuitMessage(0);
	}
	else if(Msg == WM_LBUTTONDOWN)
	{
		m_Dragging = true;
		m_DragX = LOWORD(lParam);
		m_DragY = HIWORD(lParam);
		m_HitRot = m_App.m_Camera.rot;
		SetCapture(hWnd);
	}
	else if(Msg == WM_LBUTTONUP)
	{
		m_Dragging = false;
		ReleaseCapture();
	}
	else if(Msg == WM_MOUSEMOVE)
	{
		if ( m_Dragging)
		{
			if (LOWORD(lParam) != m_DragX)
			{
				m_App.m_Camera.rot.y = m_HitRot.y - ((LOWORD(lParam) - m_DragX) * 0.2f);
			}
			if (HIWORD(lParam) != m_DragY)
			{
				m_App.m_Camera.rot.x = m_HitRot.x - ((HIWORD(lParam) - m_DragY) * 0.2f);
			}
			m_App.UpdateCamera();
		}
	}
	else if(Msg == WM_KEYDOWN)
	{
		switch(wParam)
		{
			case 'W':
				//flip fill mode
				m_App.m_fillMode = (m_App.m_fillMode == D3DFILL_SOLID) ? D3DFILL_WIREFRAME : D3DFILL_SOLID;
				break;
			case VK_PRIOR:
				m_App.m_Camera.pos.z -= 2.0f;
				m_App.UpdateCamera();
				break;
			case VK_NEXT:
				m_App.m_Camera.pos.z += 2.0f;
				m_App.UpdateCamera();
				break;
		}
	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

//------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	RECT rect;
	rect.left   = 0;
	rect.top    = 0;
	rect.bottom = 600;
	rect.right  = 600;

	m_App.name = "Vertex Shader Demo";
	m_App.m_bWindowed = true;
	m_App.m_bRefRast  = false;
	m_App.m_dwBackBufferWidth = rect.right;
	m_App.m_dwBackBufferHeight = rect.bottom;
	m_App.m_fillMode = D3DFILL_SOLID;

	WNDCLASSEX classex;
	ZeroMemory(&classex, sizeof(classex));

	classex.cbSize          = sizeof(WNDCLASSEX);
	classex.style           = CS_CLASSDC;
	classex.lpfnWndProc     = MessageHandler;
	classex.hInstance       = hInstance;
	classex.lpszClassName   = m_App.name;
	RegisterClassEx(&classex);

	HWND hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, m_App.name, m_App.name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		m_App.m_dwBackBufferWidth, m_App.m_dwBackBufferHeight,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	ShowCursor(true);

	// initialize
	HRESULT hr;

	// Some test objects
	// TODO: Add menu to load these but will then need to recreate vertex buffers and such
	if(SUCCEEDED(hr = m_App.Init(hwnd, "donut.obj")))
//	if(SUCCEEDED(hr = m_App.Init(hwnd, "lemon.obj")))
//	if(SUCCEEDED(hr = m_App.Init(hwnd, "sphere.obj")))
//	if(SUCCEEDED(hr = m_App.Init(hwnd, "chess.obj")))
	{
		MSG msg;
		bool bQuit = false;

		while(!bQuit)
		{
			if(FAILED(hr = m_App.DXRun()))
				bQuit = true;

			// pump messages
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				bQuit |= (msg.message == WM_QUIT);

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	// release everything
	m_App.Release();

	if(FAILED(hr))
	{
		static char szErr[512];

		int cch = _snprintf(szErr, sizeof(szErr), "failure(0x%08lx): ", hr);
		D3DXGetErrorString(hr, &szErr[cch], sizeof(szErr));

		MessageBox(hwnd, szErr, m_App.name, MB_OK | MB_ICONEXCLAMATION);
	}

	return EXIT_SUCCESS;
}

