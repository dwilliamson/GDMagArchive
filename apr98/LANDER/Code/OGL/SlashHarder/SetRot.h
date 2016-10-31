#if !defined(AFX_SETROT_H__56A308A2_70E2_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_SETROT_H__56A308A2_70E2_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SetRot.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetRot dialog

class CSetRot : public CDialog
{
// Construction
public:
	CSetRot(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetRot)
	enum { IDD = IDD_SETROTATE };
	float	m_XAxis;
	float	m_YAxis;
	float	m_ZAxis;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetRot)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetRot)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETROT_H__56A308A2_70E2_11D1_83A0_004005308EB5__INCLUDED_)
