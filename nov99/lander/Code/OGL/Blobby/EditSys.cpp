// EditSys.cpp : implementation file
//

#include "stdafx.h"
#include "blobby.h"
#include "EditSys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditSys dialog


CEditSys::CEditSys(CWnd* pParent /*=NULL*/)
	: CDialog(CEditSys::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditSys)
	m_SubDiv = 0;
	m_Threshold = 0.0f;
	//}}AFX_DATA_INIT
}


void CEditSys::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditSys)
	DDX_Text(pDX, IDC_SUBDIV, m_SubDiv);
	DDV_MinMaxInt(pDX, m_SubDiv, 5, 100);
	DDX_Text(pDX, IDC_THRESHOLD, m_Threshold);
	DDV_MinMaxFloat(pDX, m_Threshold, 0.f, 100.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditSys, CDialog)
	//{{AFX_MSG_MAP(CEditSys)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditSys message handlers
