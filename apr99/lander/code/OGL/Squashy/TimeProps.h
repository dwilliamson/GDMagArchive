#if !defined(AFX_TIMEPROPS_H__A1CA3701_C1F8_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_TIMEPROPS_H__A1CA3701_C1F8_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeProps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTimeProps dialog

class CTimeProps : public CDialog
{
// Construction
public:
	CTimeProps(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTimeProps)
	enum { IDD = IDD_SIMTIMING };
	BOOL	m_FixedTimeSteps;
	int		m_Iterations;
	float	m_MaxTimeStep;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTimeProps)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMEPROPS_H__A1CA3701_C1F8_11D2_8A1C_00105A124906__INCLUDED_)
