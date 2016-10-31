// ToonSet.cpp : implementation file
//

#include "stdafx.h"
#include "loony.h"
#include "ToonSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToonSet dialog


CToonSet::CToonSet(CWnd* pParent /*=NULL*/)
	: CDialog(CToonSet::IDD, pParent)
{
	//{{AFX_DATA_INIT(CToonSet)
	m_Sil_Blue = 0.0f;
	m_Sil_Green = 0.0f;
	m_Sil_Red = 0.0f;
	m_LineWidth = 0.0f;
	m_Light_X = 0.0f;
	m_Light_Y = 0.0f;
	m_Light_Z = 0.0f;
	//}}AFX_DATA_INIT
}


void CToonSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CToonSet)
	DDX_Text(pDX, IDC_SIL_BLUE, m_Sil_Blue);
	DDV_MinMaxFloat(pDX, m_Sil_Blue, 0.f, 1.f);
	DDX_Text(pDX, IDC_SIL_GREEN, m_Sil_Green);
	DDV_MinMaxFloat(pDX, m_Sil_Green, 0.f, 1.f);
	DDX_Text(pDX, IDC_SIL_RED, m_Sil_Red);
	DDV_MinMaxFloat(pDX, m_Sil_Red, 0.f, 1.f);
	DDX_Text(pDX, IDC_SIL_WIDTH, m_LineWidth);
	DDV_MinMaxFloat(pDX, m_LineWidth, 1.f, 10.f);
	DDX_Text(pDX, IDC_LIGHT_X, m_Light_X);
	DDX_Text(pDX, IDC_LIGHT_Y, m_Light_Y);
	DDX_Text(pDX, IDC_LIGHT_Z, m_Light_Z);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToonSet, CDialog)
	//{{AFX_MSG_MAP(CToonSet)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToonSet message handlers
