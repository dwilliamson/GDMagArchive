#if !defined(AFX_SLIDER_H__708CA800_BE37_11D1_83A0_004005308EB5__INCLUDED_)
#define AFX_SLIDER_H__708CA800_BE37_11D1_83A0_004005308EB5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Slider.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSlider window

class CSlider : public CSliderCtrl
{
// Construction
public:
	CSlider();
	float GetSetting();

// Attributes
private:
	BOOL m_pressed;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSlider();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSlider)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDER_H__708CA800_BE37_11D1_83A0_004005308EB5__INCLUDED_)
