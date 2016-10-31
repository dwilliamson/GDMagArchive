// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#include "mathdefs.h"
#include "engine.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	CMainFrame();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetStatusText(short id,char *string );
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	long m_offX,m_offY;
	long m_sizeX,m_sizeY;
	CRect m_window_rect;
	double m_scale;
	short m_gridsize;
	BOOLEAN m_snap;
	tPoint2D *m_point,m_temppoint;
	tSector	m_sectorlist[1024],*m_cursector;
	tEdge	m_edgelist[8192],*m_curedge,*m_firstedge;
	short	m_sectorcnt,m_edgecnt,m_nearest_edge;
	tPoint2D m_nearest_pnt,m_old_pnt;
	tPoint3D m_cam_pos;
	short m_cam_yaw;
	short m_cam_pitch;
	short m_cam_sector;
	long m_sin[4096];
	long m_cos[4096];
	CPoint m_clickpoint;		// POINT WHERE MOUSE WAS CLICKED

	void Draw2DView(CDC* pDC);
	void Draw3DView(CDC* pDC);
	void SnapPoint(tPoint2D *point);
	short InsideSector(tPoint2D *hitPos);
	short GetNearestEdge(CPoint point);
	void GetNearestPoint(tPoint2D *a,tPoint2D *b,tPoint2D *c,tPoint2D *near);
	void InsertPoint();
	void CloseSector();
	void DeleteEdge(short which);
	void DeleteSector();
	BOOL OnSamePos(short pos1,short pos2);
	void CheckDoubleSided();
	void CheckDeleteEdges(tPoint2D *a,tPoint2D *b);
	void LoadTextures();
	void TransformPoint(long world_x, long world_z, long *camera_x, long *camera_z);

	long FixMult(long a,long b)
	{
		long retval;
		__asm mov eax,a
		__asm mov ebx,b
		__asm imul ebx
		__asm shrd eax,edx,16
		__asm mov retval,eax
		return (retval);
	}

	BOOL PointInPoly(tSector *sector, tPoint2D *hitPos);
	BOOL CheckCollision(short sector, tPoint3D *test);
	void InitTrig();
	void DrawCurPos(CDC* pDC);

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnOptionsGridDown();
	afx_msg void OnOptionsGridup();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnOptionsZoomin();
	afx_msg void OnOptionsZoomout();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
