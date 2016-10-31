#if !defined(AFX_LoadDlg_H__81F04AC0_7EFD_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_LoadDlg_H__81F04AC0_7EFD_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LoadDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadDlg dialog

class CLoadDlg : public CDialog
{
// Construction
public:
	CLoadDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadDlg)
	enum { IDD = IDD_LOADOBJ };
	CString	m_File1;
	CString	m_File2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadDlg)
	afx_msg void OnBrowse1();
	afx_msg void OnBrowse2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LoadDlg_H__81F04AC0_7EFD_11D2_8A1C_00105A124906__INCLUDED_)
