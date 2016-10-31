// LoadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "morphy.h"
#include "LoadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadDlg dialog


CLoadDlg::CLoadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadDlg)
	m_File1 = _T("");
	m_File2 = _T("");
	//}}AFX_DATA_INIT
}


void CLoadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadDlg)
	DDX_Text(pDX, IDC_EDIT1, m_File1);
	DDX_Text(pDX, IDC_EDIT2, m_File2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadDlg, CDialog)
	//{{AFX_MSG_MAP(CLoadDlg)
	ON_BN_CLICKED(IDC_BROWSE1, OnBrowse1)
	ON_BN_CLICKED(IDC_BROWSE2, OnBrowse2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadDlg message handlers

void CLoadDlg::OnBrowse1() 
{
	char szFilter[] = "OBJ files (*.obj)|*.obj||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( TRUE, ".obj", NULL, NULL, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		name = dialog.GetFileName( ) ;
		m_File1 = name;
		UpdateData(FALSE);		// UPDATE THE CONTROL
	}
}

void CLoadDlg::OnBrowse2() 
{
	char szFilter[] = "OBJ files (*.obj)|*.obj||";  // WILL INCLUDE Biovision Hierarchy BVH (*.bvh)|*.bvh|
	CFileDialog	dialog( TRUE, ".obj", NULL, NULL, szFilter, this);
	CString name;		
	if (dialog.DoModal())
	{
		name = dialog.GetFileName( ) ;
		m_File2 = name;
		UpdateData(FALSE);		// UPDATE THE CONTROL
	}
}
