// EditBlob.cpp : implementation file
//

#include "stdafx.h"
#include "blobby.h"
#include "EditBlob.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditBlob dialog


CEditBlob::CEditBlob(CWnd* pParent /*=NULL*/)
	: CDialog(CEditBlob::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditBlob)
	m_RadiusSquared = 0.0f;
	m_Strength = 0.0f;
	//}}AFX_DATA_INIT
}


void CEditBlob::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBlob)
	DDX_Text(pDX, IDC_RADIUS, m_RadiusSquared);
	DDV_MinMaxFloat(pDX, m_RadiusSquared, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_STRENGTH, m_Strength);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBlob, CDialog)
	//{{AFX_MSG_MAP(CEditBlob)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBlob message handlers
