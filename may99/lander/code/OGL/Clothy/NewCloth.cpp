// NewCloth.cpp : implementation file
//

#include "stdafx.h"
#include "clothy.h"
#include "NewCloth.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NewCloth dialog


NewCloth::NewCloth(CWnd* pParent /*=NULL*/)
	: CDialog(NewCloth::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewCloth)
	m_BendCoef = 0.0f;
	m_BendDamp = 0.0f;
	m_ShearCoef = 0.0f;
	m_ShearDamp = 0.0f;
	m_StructCoef = 0.0f;
	m_StructDamp = 0.0f;
	m_USize = 0;
	m_VSize = 0;
	m_Vertical = FALSE;
	m_UseBend = FALSE;
	m_UseShear = FALSE;
	m_UseStruct = FALSE;
	//}}AFX_DATA_INIT
}


void NewCloth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewCloth)
	DDX_Text(pDX, IDC_BENDCOEF, m_BendCoef);
	DDV_MinMaxFloat(pDX, m_BendCoef, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_BENDDAMP, m_BendDamp);
	DDV_MinMaxFloat(pDX, m_BendDamp, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_SHEARCOEF, m_ShearCoef);
	DDV_MinMaxFloat(pDX, m_ShearCoef, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_SHEARDAMP, m_ShearDamp);
	DDV_MinMaxFloat(pDX, m_ShearDamp, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_STRUCTCOEF, m_StructCoef);
	DDV_MinMaxFloat(pDX, m_StructCoef, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_STRUCTDAMP, m_StructDamp);
	DDV_MinMaxFloat(pDX, m_StructDamp, 1.e-004f, 1000.f);
	DDX_Text(pDX, IDC_USIZE, m_USize);
	DDV_MinMaxInt(pDX, m_USize, 1, 128);
	DDX_Text(pDX, IDC_VSIZE, m_VSize);
	DDV_MinMaxInt(pDX, m_VSize, 1, 128);
	DDX_Check(pDX, IDC_VERTICAL, m_Vertical);
	DDX_Check(pDX, IDC_USEBEND, m_UseBend);
	DDX_Check(pDX, IDC_USESHEAR, m_UseShear);
	DDX_Check(pDX, IDC_USESTRUCT, m_UseStruct);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewCloth, CDialog)
	//{{AFX_MSG_MAP(NewCloth)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NewCloth message handlers
