// Slider.cpp : implementation file
//

#include "stdafx.h"
#include "Slash.h"
#include "Slider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSlider

CSlider::CSlider()
{
	// FLAG WHETHER I AM DRAGGING THE SLIDER
	m_pressed = FALSE;
}

CSlider::~CSlider()
{
}


BEGIN_MESSAGE_MAP(CSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CSlider)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlider message handlers

int CSlider::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSliderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// SET UP THE SLIDER SO IT GOES FROM 0 - 1024
	SetRange(0,1024,TRUE);
	SetPos(0);
	return 0;
}

// GET A FLOAT REPRESENTING THE RELATIVE POSITION OF THE SLIDER
// Returns: float from 0-1
float CSlider::GetSetting() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	int position;
///////////////////////////////////////////////////////////////////////////////
	position = GetPos();
	return position / 1024.0f;
}

void CSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// MARK THE DRAG
	m_pressed = TRUE;
	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CSlider::OnMouseMove(UINT nFlags, CPoint point) 
{
	// IF I AM DRAGGING, REDRAW MY MAIN WINDOW
	if (m_pressed)
		GetParent()->InvalidateRect(NULL);
	CSliderCtrl::OnMouseMove(nFlags, point);
}

void CSlider::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// NOT DRAGGING ANY MORE
	m_pressed = FALSE;
	CSliderCtrl::OnLButtonUp(nFlags, point);
}
