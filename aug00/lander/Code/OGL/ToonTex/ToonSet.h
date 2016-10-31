#if !defined(AFX_TOONSET_H__9FB3CCD9_2BF9_440F_9B7A_90910E067CF7__INCLUDED_)
#define AFX_TOONSET_H__9FB3CCD9_2BF9_440F_9B7A_90910E067CF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToonSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToonSet dialog

class CToonSet : public CDialog
{
// Construction
public:
	CToonSet(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CToonSet)
	enum { IDD = IDD_CARTOON_SET };
	float	m_Sil_Blue;
	float	m_Sil_Green;
	float	m_Sil_Red;
	float	m_LineWidth;
	float	m_Light_X;
	float	m_Light_Y;
	float	m_Light_Z;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToonSet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CToonSet)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOONSET_H__9FB3CCD9_2BF9_440F_9B7A_90910E067CF7__INCLUDED_)
