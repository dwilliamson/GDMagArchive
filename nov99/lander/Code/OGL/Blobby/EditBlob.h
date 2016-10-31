#if !defined(AFX_EDITBLOB_H__6B58AB36_9FA2_44CA_B404_C35D4E6F5A18__INCLUDED_)
#define AFX_EDITBLOB_H__6B58AB36_9FA2_44CA_B404_C35D4E6F5A18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBlob.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditBlob dialog

class CEditBlob : public CDialog
{
// Construction
public:
	CEditBlob(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditBlob)
	enum { IDD = IDD_EDITBLOB };
	float	m_RadiusSquared;
	float	m_Strength;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBlob)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditBlob)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBLOB_H__6B58AB36_9FA2_44CA_B404_C35D4E6F5A18__INCLUDED_)
