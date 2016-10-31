#if !defined(AFX_RESTRICT_H__2023DCA0_611E_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_RESTRICT_H__2023DCA0_611E_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Restrict.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRestrict dialog

class CRestrict : public CDialog
{
// Construction
public:
	CRestrict(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRestrict)
	enum { IDD = IDD_SETRESTRICT };
	int	m_MinRot0;
	int	m_MinRot1;
	int	m_MinRot2;
	int	m_MinRot3;
	int	m_MinRot4;
	int	m_MaxRot0;
	int	m_MaxRot1;
	int	m_MaxRot2;
	int	m_MaxRot3;
	int	m_MaxRot4;
	float	m_Damp0;
	float	m_Damp1;
	float	m_Damp2;
	float	m_Damp3;
	float	m_Damp4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRestrict)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRestrict)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESTRICT_H__2023DCA0_611E_11D2_8A1C_00105A124906__INCLUDED_)
