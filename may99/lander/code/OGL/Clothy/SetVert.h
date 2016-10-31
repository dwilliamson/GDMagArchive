#if !defined(AFX_SETVERT_H__5F1BAAC0_BDD8_11D2_8A1C_00105A124906__INCLUDED_)
#define AFX_SETVERT_H__5F1BAAC0_BDD8_11D2_8A1C_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetVert.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetVert dialog

class CSetVert : public CDialog
{
// Construction
public:
	CSetVert(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetVert)
	enum { IDD = IDD_VERTEXDIALOG };
	float	m_VertexMass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetVert)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetVert)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETVERT_H__5F1BAAC0_BDD8_11D2_8A1C_00105A124906__INCLUDED_)
