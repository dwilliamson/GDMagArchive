#if !defined(AFX_SIMPROPS_H__6D91A591_983A_11D2_9D88_00105A124906__INCLUDED_)
#define AFX_SIMPROPS_H__6D91A591_983A_11D2_9D88_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimProps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSimProps dialog

class CSimProps : public CDialog
{
// Construction
public:
	CSimProps(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSimProps)
	enum { IDD = IDD_SIMPROP };
	float	m_CoefRest;
	float	m_Damping;
	float	m_GravX;
	float	m_GravY;
	float	m_GravZ;
	float	m_SpringConst;
	float	m_SpringDamp;
	float	m_UserForceMag;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSimProps)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPROPS_H__6D91A591_983A_11D2_9D88_00105A124906__INCLUDED_)
