// EditEmit.cpp : implementation file
//

#include "stdafx.h"
#include "Spray.h"
#include "EditEmit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditEmit dialog


CEditEmit::CEditEmit(CWnd* pParent /*=NULL*/)
	: CDialog(CEditEmit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditEmit)
	m_endColorB = 0.0f;
	m_endColorBVar = 0.0f;
	m_endColorG = 0.0f;
	m_endColorGVar = 0.0f;
	m_endColorR = 0.0f;
	m_endColorRVar = 0.0f;
	m_emits = 0;
	m_emitVar = 0;
	m_forceX = 0.0f;
	m_forceY = 0.0f;
	m_forceZ = 0.0f;
	m_life = 0;
	m_lifeVar = 0;
	m_pitch = 0.0f;
	m_pitchVar = 0.0f;
	m_startColorB = 0.0f;
	m_startColorBVar = 0.0f;
	m_startColorG = 0.0f;
	m_startColorGVar = 0.0f;
	m_startColorR = 0.0f;
	m_startColorRVar = 0.0f;
	m_speed = 0.0f;
	m_speedVar = 0.0f;
	m_yaw = 0.0f;
	m_yawVar = 0.0f;
	m_name = _T("");
	m_TotalParticles = 0;
	//}}AFX_DATA_INIT
}


void CEditEmit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditEmit)
	DDX_Text(pDX, IDC_ECOLORB, m_endColorB);
	DDV_MinMaxFloat(pDX, m_endColorB, 0.f, 1.f);
	DDX_Text(pDX, IDC_ECOLORBV, m_endColorBVar);
	DDV_MinMaxFloat(pDX, m_endColorBVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_ECOLORG, m_endColorG);
	DDV_MinMaxFloat(pDX, m_endColorG, 0.f, 1.f);
	DDX_Text(pDX, IDC_ECOLORGV, m_endColorGVar);
	DDV_MinMaxFloat(pDX, m_endColorGVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_ECOLORR, m_endColorR);
	DDV_MinMaxFloat(pDX, m_endColorR, 0.f, 1.f);
	DDX_Text(pDX, IDC_ECOLORRV, m_endColorRVar);
	DDV_MinMaxFloat(pDX, m_endColorRVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_EMITS, m_emits);
	DDX_Text(pDX, IDC_EMITVAR, m_emitVar);
	DDX_Text(pDX, IDC_FORCEX, m_forceX);
	DDX_Text(pDX, IDC_FORCEY, m_forceY);
	DDX_Text(pDX, IDC_FORCEZ, m_forceZ);
	DDX_Text(pDX, IDC_LIFE, m_life);
	DDX_Text(pDX, IDC_LIFEVAR, m_lifeVar);
	DDX_Text(pDX, IDC_PITCH, m_pitch);
	DDV_MinMaxFloat(pDX, m_pitch, 0.f, 360.f);
	DDX_Text(pDX, IDC_PITCHVAR, m_pitchVar);
	DDV_MinMaxFloat(pDX, m_pitchVar, 0.f, 360.f);
	DDX_Text(pDX, IDC_SCOLORB, m_startColorB);
	DDV_MinMaxFloat(pDX, m_startColorB, 0.f, 1.f);
	DDX_Text(pDX, IDC_SCOLORBV, m_startColorBVar);
	DDV_MinMaxFloat(pDX, m_startColorBVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_SCOLORG, m_startColorG);
	DDV_MinMaxFloat(pDX, m_startColorG, 0.f, 1.f);
	DDX_Text(pDX, IDC_SCOLORGV, m_startColorGVar);
	DDV_MinMaxFloat(pDX, m_startColorGVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_SCOLORR, m_startColorR);
	DDV_MinMaxFloat(pDX, m_startColorR, 0.f, 1.f);
	DDX_Text(pDX, IDC_SCOLORRV, m_startColorRVar);
	DDV_MinMaxFloat(pDX, m_startColorRVar, 0.f, 1.f);
	DDX_Text(pDX, IDC_SPEED, m_speed);
	DDX_Text(pDX, IDC_SPEEDVAR, m_speedVar);
	DDX_Text(pDX, IDC_YAW, m_yaw);
	DDV_MinMaxFloat(pDX, m_yaw, 0.f, 360.f);
	DDX_Text(pDX, IDC_YAWVAR, m_yawVar);
	DDV_MinMaxFloat(pDX, m_yawVar, 0.f, 360.f);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 80);
	DDX_Text(pDX, IDC_TOTALPARTS, m_TotalParticles);
	DDV_MinMaxInt(pDX, m_TotalParticles, 1, 4000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditEmit, CDialog)
	//{{AFX_MSG_MAP(CEditEmit)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditEmit message handlers
