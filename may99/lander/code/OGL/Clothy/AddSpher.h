#if !defined(AFX_ADDSPHER_H__F770F421_F0FE_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_ADDSPHER_H__F770F421_F0FE_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddSpher.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddSpher dialog

class CAddSpher : public CDialog
{
// Construction
public:
	CAddSpher(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddSpher)
	enum { IDD = IDD_ADDSPHERE };
	float	m_Radius;
	float	m_XPos;
	float	m_YPos;
	float	m_ZPos;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddSpher)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddSpher)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDSPHER_H__F770F421_F0FE_11D2_8A1C_00105A124906__INCLUDED_)
