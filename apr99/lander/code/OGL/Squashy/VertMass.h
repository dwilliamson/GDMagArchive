#if !defined(AFX_VERTMASS_H__A1CA3700_C1F8_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_VERTMASS_H__A1CA3700_C1F8_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VertMass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVertMass dialog

class CVertMass : public CDialog
{
// Construction
public:
	CVertMass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVertMass)
	enum { IDD = IDD_VERTEXMASS };
	float	m_VertexMass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVertMass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVertMass)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERTMASS_H__A1CA3700_C1F8_11D2_8A1C_00105A124906__INCLUDED_)
