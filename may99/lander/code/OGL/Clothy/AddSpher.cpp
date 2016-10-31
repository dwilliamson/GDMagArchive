// AddSpher.cpp : implementation file
//

#include "stdafx.h"
#include "clothy.h"
#include "AddSpher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddSpher dialog


CAddSpher::CAddSpher(CWnd* pParent /*=NULL*/)
	: CDialog(CAddSpher::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddSpher)
	m_Radius = 0.0f;
	m_XPos = 0.0f;
	m_YPos = 0.0f;
	m_ZPos = 0.0f;
	//}}AFX_DATA_INIT
}


void CAddSpher::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddSpher)
	DDX_Text(pDX, IDC_RADIUS, m_Radius);
	DDV_MinMaxFloat(pDX, m_Radius, 1.e-003f, 10.f);
	DDX_Text(pDX, IDC_XPOS, m_XPos);
	DDX_Text(pDX, IDC_YPOS, m_YPos);
	DDX_Text(pDX, IDC_ZPOS, m_ZPos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddSpher, CDialog)
	//{{AFX_MSG_MAP(CAddSpher)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddSpher message handlers
