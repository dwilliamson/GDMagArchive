// VertMass.cpp : implementation file
//

#include "stdafx.h"
#include "Friction.h"
#include "VertMass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVertMass dialog


CVertMass::CVertMass(CWnd* pParent /*=NULL*/)
	: CDialog(CVertMass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVertMass)
	m_VertexMass = 0.0f;
	//}}AFX_DATA_INIT
}


void CVertMass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVertMass)
	DDX_Text(pDX, IDC_VERTEXMASS, m_VertexMass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVertMass, CDialog)
	//{{AFX_MSG_MAP(CVertMass)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVertMass message handlers
