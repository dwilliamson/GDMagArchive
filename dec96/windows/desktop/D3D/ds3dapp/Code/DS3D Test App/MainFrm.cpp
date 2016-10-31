// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DS3D Test App.h"

#define INITGUID
#include <mmsystem.h>
#include "dsound.h"

#include "MainFrm.h"
#include "dsutil.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MY_TIMER	1
#define FILE_MENU_INDEX 0
#define RADIUS		1.0
#define PI			3.1415926535
#define TWOPI		(2*PI)
#define STEP		(TWOPI/180)

/////////////////////////////////////////////////////////////////////////////
// CMainFrame


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_FILE_PLAY, OnFilePlay)
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_STOP, OnFileStop)
	ON_WM_TIMER()
	ON_WM_INITMENUPOPUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	Create(NULL, "DS3D Test App", WS_OVERLAPPEDWINDOW,
		rectDefault, NULL, MAKEINTRESOURCE(IDR_MAINFRAME));

	m_pDirectSound = NULL;
	m_pDirectSoundBuffer = NULL;
	m_pDirectSound3DBuffer = NULL;
	m_pDirectSound3DListener = NULL;
	m_pDirectSoundPrimaryBuffer = NULL;
	m_bPlaying = FALSE;
}

CMainFrame::~CMainFrame()
{
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnFilePlay() 
{
	// set the 3D parameters
	m_x	= (float)0.0;	// zero left/right position
	m_y	= (float)0.0;	// always zero (no verticle component in this app)
	m_z	= (float)RADIUS;// RADIUS meters in front of listener
	// load the file if necessary
	if(m_pDirectSound == NULL)
		LoadFile();
	// create and start a timer 
	SetTimer(MY_TIMER, 50, NULL); 
    // Play the sound
	m_pDirectSoundBuffer->SetCurrentPosition(0);
	m_pDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
	m_bPlaying = TRUE;
}


void CMainFrame::OnClose() 
{
	// if file is playing call OnFileStop to kill timer
	if(m_bPlaying == TRUE)
		OnFileStop();
	ReleaseDirectSound();
	CFrameWnd::OnClose();
}


void CMainFrame::OnFileStop() 
{
	m_pDirectSoundBuffer->Stop();
	m_bPlaying = FALSE;
	KillTimer(MY_TIMER);
}


void CMainFrame::OnTimer(UINT nIDEvent) 
{
	static double stepcounter = 0.0;

	ASSERT(m_pDirectSound3DBuffer != NULL);
	stepcounter += STEP;
	if(stepcounter>=TWOPI)
		stepcounter = 0.0;
	m_x = (float)(RADIUS * cos(stepcounter));
	m_z = (float)(RADIUS * sin(stepcounter));
	m_pDirectSound3DBuffer->SetPosition(m_x,m_y,m_z,DS3D_IMMEDIATE);
}


void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	if (!bSysMenu && (nIndex == FILE_MENU_INDEX))
	{
		if (m_bPlaying == TRUE)
		{
			pPopupMenu->EnableMenuItem(ID_FILE_PLAY,MF_GRAYED);
			pPopupMenu->EnableMenuItem(ID_FILE_STOP,MF_ENABLED);
		}
		else
		{
			pPopupMenu->EnableMenuItem(ID_FILE_PLAY,MF_ENABLED);
			pPopupMenu->EnableMenuItem(ID_FILE_STOP,MF_GRAYED);
		}
	}
	
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame member functions

void CMainFrame::LoadFile() 
{
	if (m_pDirectSound == NULL)
		InitDirectSound();

	m_pDirectSoundBuffer = DSLoadSoundBuffer(m_pDirectSound,MAKEINTRESOURCE(IDR_WAVE1));
	ASSERT(m_pDirectSoundBuffer != NULL);
	// Query for the 3D Sound Buffer interface.
	m_pDirectSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer,
		(void**) &m_pDirectSound3DBuffer);
	ASSERT(m_pDirectSound3DBuffer != NULL);
	m_bPlaying = FALSE;
}

BOOL CMainFrame::InitDirectSound(void)
{
    DSBUFFERDESC dsBD = {0};

	if(DS_OK == DirectSoundCreate(NULL, &m_pDirectSound,NULL))
	{
		m_pDirectSound->SetCooperativeLevel(m_hWnd, DSSCL_NORMAL);
		// create primary 3D buffer
        ZeroMemory( &dsBD, sizeof(DSBUFFERDESC));
        dsBD.dwSize = sizeof(dsBD);
	    dsBD.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
        dsBD.dwBufferBytes = 0;  //must be zero for primary buffer..
        if (m_pDirectSound->CreateSoundBuffer(&dsBD,
			&m_pDirectSoundPrimaryBuffer, NULL) != DS_OK)
		{
            MessageBox("Not able to create DirectSound3D primary buffer");
			ReleaseDirectSound();
			return FALSE;
		}

		// obtain interface pointers to IDirectDound3DListener
        if(DS_OK != m_pDirectSoundPrimaryBuffer->QueryInterface(
			IID_IDirectSound3DListener, (void**)&m_pDirectSound3DListener))
        {
            MessageBox("Not able to create Direct 3D Sound Listener object");
			ReleaseDirectSound();
			return FALSE;
        }
	}
	else
	{
        MessageBox("Not able to create DirectSound object");
		m_pDirectSound = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL CMainFrame::ReleaseDirectSound(void)
{

    // Destroy DirectSound3D buffer
    if (m_pDirectSound3DBuffer)
    {       
        m_pDirectSound3DBuffer->Release();
        m_pDirectSound3DBuffer = NULL;
    }
        
    // Destroy DirectSound3D Listener object
    if (m_pDirectSound3DListener)
    {
        m_pDirectSound3DListener->Release();
        m_pDirectSound3DListener = NULL;
    }

    // Destroy DirectSound buffer
    if (m_pDirectSoundBuffer)
    {       
        m_pDirectSoundBuffer->Release();
        m_pDirectSoundBuffer = NULL;
    }
        
    // Destroy DirectSound3D Primary buffer
    if (m_pDirectSoundPrimaryBuffer)
    {
        m_pDirectSoundPrimaryBuffer->Release();
        m_pDirectSoundPrimaryBuffer = NULL;
    } 

    // Destroy DirectSound object
    if (m_pDirectSound)
    {
        m_pDirectSound->Release();
        m_pDirectSound = NULL;
    }
    
	return TRUE;
}




