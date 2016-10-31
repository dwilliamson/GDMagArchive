// SetVert.cpp : implementation file
//

#include "stdafx.h"
#include "Clothy.h"
#include "SetVert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetVert dialog


CSetVert::CSetVert(CWnd* pParent /*=NULL*/)
	: CDialog(CSetVert::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetVert)
	m_VertexMass = 0.0f;
	//}}AFX_DATA_INIT
}


void CSetVert::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetVert)
	DDX_Text(pDX, IDC_VERTEXMASS, m_VertexMass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetVert, CDialog)
	//{{AFX_MSG_MAP(CSetVert)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetVert message handlers
