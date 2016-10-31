// AguaDlg.h : header file
//

#if !defined(AFX_AGUADLG_H__BB47BA75_6E1A_40E8_86EE_E5F79B1C91C6__INCLUDED_)
#define AFX_AGUADLG_H__BB47BA75_6E1A_40E8_86EE_E5F79B1C91C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LoadTGA.h"

/////////////////////////////////////////////////////////////////////////////
// CAguaDlg dialog

class CAguaDlg : public CDialog
{
// Construction
public:
	CAguaDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAguaDlg)
	enum { IDD = IDD_AGUA_DIALOG };
	CStatic	m_Display;
	int		m_Drip_Radius;
	float	m_DampingFactor;
	CString	m_ImageFile;
	UINT	m_Green;
	UINT	m_Red;
	UINT	m_Blue;
	BOOL	m_UseImage;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAguaDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HBITMAP m_Bitmap;
	long	*m_Buffer;
	char	*m_ReadBuffer, *m_WriteBuffer;			// Water height map
	tTGAFile_s m_ReflectImage;
	int		m_Drip_Radius_Sqr;		// Squared Radius

	void	SetDisplay();		// Draw the height field
	void	MakeDrip(int x, int y, int depth);
	int		SquaredDist(int sx, int sy, int dx, int dy);
	void	ProcessWater();

	// Generated message map functions
	//{{AFX_MSG(CAguaDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnUpdateDamping();
	afx_msg void OnUpdateDripRadius();
	afx_msg void OnImageBrowse();
	afx_msg void OnUpdateImagefile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AGUADLG_H__BB47BA75_6E1A_40E8_86EE_E5F79B1C91C6__INCLUDED_)
