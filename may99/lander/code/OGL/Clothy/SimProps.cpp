// SimProps.cpp : implementation file
//

#include "stdafx.h"
#include "Clothy.h"
#include "SimProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimProps dialog


CSimProps::CSimProps(CWnd* pParent /*=NULL*/)
	: CDialog(CSimProps::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSimProps)
	m_CoefRest = 0.0f;
	m_Damping = 0.0f;
	m_GravX = 0.0f;
	m_GravY = 0.0f;
	m_GravZ = 0.0f;
	m_SpringConst = 0.0f;
	m_SpringDamp = 0.0f;
	m_UserForceMag = 0.0f;
	//}}AFX_DATA_INIT
}


void CSimProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSimProps)
	DDX_Text(pDX, IDC_COEFREST, m_CoefRest);
	DDX_Text(pDX, IDC_Damping, m_Damping);
	DDX_Text(pDX, IDC_GRAVX, m_GravX);
	DDX_Text(pDX, IDC_GRAVY, m_GravY);
	DDX_Text(pDX, IDC_GRAVZ, m_GravZ);
	DDX_Text(pDX, IDC_SPRINGCONST, m_SpringConst);
	DDX_Text(pDX, IDC_SPRINGDAMP, m_SpringDamp);
	DDX_Text(pDX, IDC_USERFORCEMAG, m_UserForceMag);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSimProps, CDialog)
	//{{AFX_MSG_MAP(CSimProps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimProps message handlers
