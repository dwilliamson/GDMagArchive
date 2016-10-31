// TimeProps.cpp : implementation file
//

#include "stdafx.h"
#include "Clothy.h"
#include "TimeProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimeProps dialog


CTimeProps::CTimeProps(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeProps::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTimeProps)
	m_FixedTimeSteps = FALSE;
	m_Iterations = 0;
	m_MaxTimeStep = 0.0f;
	//}}AFX_DATA_INIT
}


void CTimeProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimeProps)
	DDX_Check(pDX, IDC_FIXEDTIME, m_FixedTimeSteps);
	DDX_Text(pDX, IDC_ITERATIONS, m_Iterations);
	DDV_MinMaxInt(pDX, m_Iterations, 1, 100);
	DDX_Text(pDX, IDC_MAXTIMESTEP, m_MaxTimeStep);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimeProps, CDialog)
	//{{AFX_MSG_MAP(CTimeProps)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeProps message handlers
