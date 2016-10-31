// MusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Muscles.h"
#include "MusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMusDlg dialog


CMusDlg::CMusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMusDlg)
	m_ContractPercent = 0.0f;
	m_ContractOff = 0;
	m_IsMuscle = FALSE;
	m_Spring_Ks = 0.0f;
	m_Spring_Kd = 0.0f;
	//}}AFX_DATA_INIT
}


void CMusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMusDlg)
	DDX_Text(pDX, IDC_CONTRACT, m_ContractPercent);
	DDX_Text(pDX, IDC_CONTRACTOFF, m_ContractOff);
	DDX_Check(pDX, IDC_ISMUSCLE, m_IsMuscle);
	DDX_Text(pDX, IDC_SPRINGKS, m_Spring_Ks);
	DDX_Text(pDX, IDC_SPRINGKD, m_Spring_Kd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMusDlg, CDialog)
	//{{AFX_MSG_MAP(CMusDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMusDlg message handlers
