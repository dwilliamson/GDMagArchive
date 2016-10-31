// SetRot.cpp : implementation file
//

#include "stdafx.h"
#include "Slash.h"
#include "SetRot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetRot dialog


CSetRot::CSetRot(CWnd* pParent /*=NULL*/)
	: CDialog(CSetRot::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetRot)
	m_XAxis = 0.0f;
	m_YAxis = 0.0f;
	m_ZAxis = 0.0f;
	//}}AFX_DATA_INIT
}


void CSetRot::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetRot)
	DDX_Text(pDX, IDC_XAXIS, m_XAxis);
	DDX_Text(pDX, IDC_YAXIS, m_YAxis);
	DDX_Text(pDX, IDC_ZAXIS, m_ZAxis);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetRot, CDialog)
	//{{AFX_MSG_MAP(CSetRot)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetRot message handlers
