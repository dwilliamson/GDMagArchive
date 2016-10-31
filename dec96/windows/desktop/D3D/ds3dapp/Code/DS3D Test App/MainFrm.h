// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include <mmsystem.h>
#include "dsound.h"

class CMainFrame : public CFrameWnd
{

// Implementation
public:
	CMainFrame();
	virtual ~CMainFrame();

// Generated message map functions
protected:
	BOOL m_bPlaying;
	LPDIRECTSOUND m_pDirectSound;
	LPDIRECTSOUNDBUFFER m_pDirectSoundBuffer;
	LPDIRECTSOUNDBUFFER m_pDirectSoundPrimaryBuffer;
	LPDIRECTSOUND3DBUFFER m_pDirectSound3DBuffer;
	LPDIRECTSOUND3DLISTENER m_pDirectSound3DListener;
	D3DVALUE	m_x;	// positive X axis points out the right ear
	D3DVALUE	m_y;	// positive Y axis points up
	D3DVALUE	m_z;	// positive Z axis points away from viewer

	BOOL InitDirectSound(void);
	BOOL ReleaseDirectSound(void);
	void LoadFile(void);
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnFilePlay();
	afx_msg void OnClose();
	afx_msg void OnFileStop();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

