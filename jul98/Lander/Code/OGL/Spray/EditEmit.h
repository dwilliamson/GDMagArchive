#if !defined(AFX_EDITEMIT_H__F171A980_D718_11D1_83A2_004005308EB5__INCLUDED_)
#define AFX_EDITEMIT_H__F171A980_D718_11D1_83A2_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditEmit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditEmit dialog

class CEditEmit : public CDialog
{
// Construction
public:
	CEditEmit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditEmit)
	enum { IDD = IDD_SETEMITTER };
	float	m_endColorB;
	float	m_endColorBVar;
	float	m_endColorG;
	float	m_endColorGVar;
	float	m_endColorR;
	float	m_endColorRVar;
	int		m_emits;
	int		m_emitVar;
	float	m_forceX;
	float	m_forceY;
	float	m_forceZ;
	int		m_life;
	int		m_lifeVar;
	float	m_pitch;
	float	m_pitchVar;
	float	m_startColorB;
	float	m_startColorBVar;
	float	m_startColorG;
	float	m_startColorGVar;
	float	m_startColorR;
	float	m_startColorRVar;
	float	m_speed;
	float	m_speedVar;
	float	m_yaw;
	float	m_yawVar;
	CString	m_name;
	int		m_TotalParticles;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditEmit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditEmit)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITEMIT_H__F171A980_D718_11D1_83A2_004005308EB5__INCLUDED_)
