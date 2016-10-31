#if !defined(AFX_NEWCLOTH_H__F770F420_F0FE_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_NEWCLOTH_H__F770F420_F0FE_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewCloth.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NewCloth dialog

class NewCloth : public CDialog
{
// Construction
public:
	NewCloth(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NewCloth)
	enum { IDD = IDD_MAKECLOTH };
	float	m_BendCoef;
	float	m_BendDamp;
	float	m_ShearCoef;
	float	m_ShearDamp;
	float	m_StructCoef;
	float	m_StructDamp;
	int		m_USize;
	int		m_VSize;
	BOOL	m_Vertical;
	BOOL	m_UseBend;
	BOOL	m_UseShear;
	BOOL	m_UseStruct;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewCloth)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NewCloth)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWCLOTH_H__F770F420_F0FE_11D2_8A1C_00105A124906__INCLUDED_)
