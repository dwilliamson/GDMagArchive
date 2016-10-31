#if !defined(AFX_MUSDLG_H__732C52E9_74F0_461D_BBA8_52C583BA6173__INCLUDED_)
#define AFX_MUSDLG_H__732C52E9_74F0_461D_BBA8_52C583BA6173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMusDlg dialog

class CMusDlg : public CDialog
{
// Construction
public:
	CMusDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMusDlg)
	enum { IDD = IDD_MUSCLESET };
	float	m_ContractPercent;
	int		m_ContractOff;
	BOOL	m_IsMuscle;
	float	m_Spring_Ks;
	float	m_Spring_Kd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMusDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MUSDLG_H__732C52E9_74F0_461D_BBA8_52C583BA6173__INCLUDED_)
