// Restrict.cpp : implementation file
//

#include "stdafx.h"
#include "kinechain.h"
#include "Restrict.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRestrict dialog


CRestrict::CRestrict(CWnd* pParent /*=NULL*/)
	: CDialog(CRestrict::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRestrict)
	m_MinRot0 = 0;
	m_MinRot1 = 0;
	m_MinRot2 = 0;
	m_MinRot3 = 0;
	m_MinRot4 = 0;
	m_MaxRot0 = 0;
	m_MaxRot1 = 0;
	m_MaxRot2 = 0;
	m_MaxRot3 = 0;
	m_MaxRot4 = 0;
	m_Damp0 = 0.0f;
	m_Damp1 = 0.0f;
	m_Damp2 = 0.0f;
	m_Damp3 = 0.0f;
	m_Damp4 = 0.0f;
	//}}AFX_DATA_INIT
}


void CRestrict::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRestrict)
	DDX_Text(pDX, IDC_MINROT0, m_MinRot0);
	DDX_Text(pDX, IDC_MINROT1, m_MinRot1);
	DDX_Text(pDX, IDC_MINROT2, m_MinRot2);
	DDX_Text(pDX, IDC_MINROT3, m_MinRot3);
	DDX_Text(pDX, IDC_MINROT4, m_MinRot4);
	DDX_Text(pDX, IDC_MAXROT0, m_MaxRot0);
	DDX_Text(pDX, IDC_MAXROT1, m_MaxRot1);
	DDX_Text(pDX, IDC_MAXROT2, m_MaxRot2);
	DDX_Text(pDX, IDC_MAXROT3, m_MaxRot3);
	DDX_Text(pDX, IDC_MAXROT4, m_MaxRot4);
	DDX_Text(pDX, IDC_DAMP0, m_Damp0);
	DDX_Text(pDX, IDC_DAMP1, m_Damp1);
	DDX_Text(pDX, IDC_DAMP2, m_Damp2);
	DDX_Text(pDX, IDC_DAMP3, m_Damp3);
	DDX_Text(pDX, IDC_DAMP4, m_Damp4);
	DDV_MinMaxFloat(pDX, m_Damp0, 0.0f, 359.0f);
	DDV_MinMaxFloat(pDX, m_Damp1, 0.0f, 359.0f);
	DDV_MinMaxFloat(pDX, m_Damp2, 0.0f, 359.0f);
	DDV_MinMaxFloat(pDX, m_Damp3, 0.0f, 359.0f);
	DDV_MinMaxFloat(pDX, m_Damp4, 0.0f, 359.0f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRestrict, CDialog)
	//{{AFX_MSG_MAP(CRestrict)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRestrict message handlers
