// unittestDlg.h : header file
//

#if !defined(AFX_UNITTESTDLG_H__E947DC44_0F2B_4CBB_A63D_831A5A2875AB__INCLUDED_)
#define AFX_UNITTESTDLG_H__E947DC44_0F2B_4CBB_A63D_831A5A2875AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CUnittestDlg dialog

class CUnittestDlg : public CDialog
{
// Construction
public:
	CUnittestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CUnittestDlg)
	enum { IDD = IDD_UNITTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnittestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CUnittestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonStartgame();
	afx_msg void OnButtonStopgame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITTESTDLG_H__E947DC44_0F2B_4CBB_A63D_831A5A2875AB__INCLUDED_)
