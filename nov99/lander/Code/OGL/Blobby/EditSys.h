#if !defined(AFX_EDITSYS_H__7EE16A06_49C7_4105_A9C8_6D4BC12DA2FD__INCLUDED_)
#define AFX_EDITSYS_H__7EE16A06_49C7_4105_A9C8_6D4BC12DA2FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditSys.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditSys dialog

class CEditSys : public CDialog
{
// Construction
public:
	CEditSys(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditSys)
	enum { IDD = IDD_EDITSYS };
	int		m_SubDiv;
	float	m_Threshold;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditSys)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditSys)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSYS_H__7EE16A06_49C7_4105_A9C8_6D4BC12DA2FD__INCLUDED_)
