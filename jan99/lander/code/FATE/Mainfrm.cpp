///////////////////////////////////////////////////////////////////////////////
//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Purpose:	Implementation of 2D Collision System
//
// Created:
//		JL 11/1/98		
//
// Notes:  THIS CONTAINS SOME USEFUL COMPUTATIONAL GEOMETRY ROUTINES AT THE
//	END OF THIS FILE.
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <math.h>
#include <mmsystem.h>
#include <Direct.h>
#include "Fate.h"

#include "MainFrm.h"

//IGNORE THE DOUBLE TO FLOAT CONVERSION WARNING
#pragma warning ( disable : 4244 )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_OPTIONS_GRID_DOWN, OnOptionsGridDown)
	ON_COMMAND(ID_OPTIONS_GRIDUP, OnOptionsGridup)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_OPTIONS_ZOOMIN, OnOptionsZoomin)
	ON_COMMAND(ID_OPTIONS_ZOOMOUT, OnOptionsZoomout)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
	// Global help commands
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,				// status line indicator
	ID_INDICATOR_INFO,          // status line INFO
	ID_INDICATOR_SNAP,          // status line SNAP
	ID_INDICATOR_GRID,          // status line GRIDSIZE
	ID_INDICATOR_SC2,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_offX = 0;
	m_offY = 0;
	m_scale = 0.4;
	m_gridsize = 64;
	m_snap = TRUE;
	m_point = NULL;
	// SET UP THE WORLD VARIABLES
	m_cursector = NULL;
	m_curedge = NULL;
	m_firstedge = NULL;		// FIRST EDGE IN A SECTOR
	m_edgecnt = 0;
	m_sectorcnt = 0;
	m_nearest_edge = -1;
	m_nearest_pnt.x = 0;
	m_nearest_pnt.y = 0;
	m_old_pnt.x = -2;
	m_old_pnt.y = -2;

	m_cam_pos.x = 0;
	m_cam_pos.y = 0;
	m_cam_pos.z = 0;
	m_cam_yaw = 0;
	m_cam_pitch = 0;
	m_cam_sector = -1;

	InitTrig();
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	char str[80];

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	GetWindowRect(&m_window_rect);

	sprintf(str,"Zoom = %2.1f",m_scale);
	SetStatusText(4,str);
	sprintf(str,"Grid = %3d",m_gridsize);
	SetStatusText(3,str);
	SetStatusText(2,"Snap On");
	sprintf(str,"Sectors = %3d  Edges = %3d",m_sectorcnt,m_edgecnt);
	SetStatusText(1,str);

	return 0;
}

void CMainFrame::SetStatusText(short id,char *string )
{
	m_wndStatusBar.SetPaneText(id,string);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Scale the Grid Size Up
void CMainFrame::OnOptionsGridup() 
{
	char str[80];
	if (m_gridsize <= 1024) m_gridsize *= 2;
	sprintf(str,"Grid = %4d",m_gridsize);
	SetStatusText(3,str);
	Invalidate (TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Scale the Grid Size Down
void CMainFrame::OnOptionsGridDown() 
{
	char str[80];
	if (m_gridsize > 2) 
	{
		m_gridsize /= 2;
		sprintf(str,"Grid = %4d",m_gridsize);
	}
	else
	{
		m_gridsize = 1;
		sprintf(str,"Grid OFF");
	}
	SetStatusText(3,str);
	Invalidate (TRUE);
}

void CMainFrame::OnOptionsZoomin() 
{
	char str[80];
	if (m_scale <= 10.0) m_scale += .1;
	sprintf(str,"Zoom = %2.1f",m_scale);
	SetStatusText(4,str);
	Invalidate (TRUE);
}

void CMainFrame::OnOptionsZoomout() 
{
	char str[80];
	if (m_scale > 0.2)
	{
		m_scale -= 0.1;
	}
	sprintf(str,"Zoom = %2.1f",m_scale);
	SetStatusText(4,str);
	Invalidate (TRUE);
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	tPoint3D	temp;
	tPoint2D	temp2D;
	int cx, cy;
	CRect rect;
	cx = (GetSystemMetrics(SM_CXFULLSCREEN) - (640 + 8)) / 2;
	cy = (GetSystemMetrics(SM_CYFULLSCREEN) - (480 + 94)) / 2;
	
	switch (nChar)
	{
		case VK_INSERT:
			InsertPoint();
			Invalidate(TRUE);
			break;
		case VK_DELETE:		
			DeleteSector();
			Invalidate(TRUE);
			break;
		case VK_RETURN:		
			CloseSector();
			Invalidate(TRUE);
			break;
		case VK_ADD:
			OnOptionsGridup();
			break;
		case VK_SUBTRACT:
			OnOptionsGridDown();
			break;
		case 'A':
			OnOptionsZoomin();
			break;
		case 'M':
			break;
		case 'Z':
			OnOptionsZoomout();
			break;
		case 'S':
			m_snap = TRUE - m_snap;
			if (m_snap)
				SetStatusText(2,"Snap On");
			else
				SetStatusText(2,"Snap Off");
			break;
		case VK_UP:
			temp.x = m_cam_pos.x + (m_sin[m_cam_yaw]>>13);
			temp.z = m_cam_pos.z + (m_cos[m_cam_yaw]>>13);
			if (m_cam_sector == -1)
			{
				m_cam_pos.x = temp.x;
				m_cam_pos.z = temp.z;
				temp2D.x = temp.x;
				temp2D.y = temp.z;
				m_cam_sector = InsideSector(&temp2D);
			}
			else
			{
				if (CheckCollision(m_cam_sector,&temp))
				{
					m_cam_pos.x = temp.x;
					m_cam_pos.z = temp.z;
				}
			}
			Invalidate(TRUE);
			break;
		case VK_DOWN:
			temp.x = m_cam_pos.x - (m_sin[m_cam_yaw]>>13);
			temp.z = m_cam_pos.z - (m_cos[m_cam_yaw]>>13);
			if (m_cam_sector == -1)
			{
				m_cam_pos.x = temp.x;
				m_cam_pos.z = temp.z;
				temp2D.x = temp.x;
				temp2D.y = temp.z;
				m_cam_sector = InsideSector(&temp2D);
			}
			else
			{
				if (CheckCollision(m_cam_sector,&temp))
				{
					m_cam_pos.x = temp.x;
					m_cam_pos.z = temp.z;
				}
			}
			Invalidate(TRUE);
			break;
		case VK_LEFT:
			m_cam_yaw += (4096 - 128);
			m_cam_yaw %= 4096;
			Invalidate(TRUE);
			break;
		case VK_RIGHT:
			m_cam_yaw += 128;
			m_cam_yaw %= 4096;
			Invalidate(TRUE);
			break;
	}
	
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////
// Procedure:	OnRButtonUp
// Purpose:		Right Mouse Button Handler
//////////////////////////////////////////////////////////////////
void CMainFrame::OnRButtonUp(UINT nFlags, CPoint point) 
{
	tPoint2D temp;
	// MOVE THE CAMERA POSITION TO WHERE THE USER CLICKS
	m_cam_pos.x = (long)((point.x + m_offX - (m_sizeX / 2)) / m_scale);
	m_cam_pos.z = -(long)((point.y + m_offY - (m_sizeY / 2)) / m_scale);
	temp.x = m_cam_pos.x;
	temp.y = m_cam_pos.z;
	m_cam_sector = InsideSector(&temp);
	Invalidate(TRUE);
	CFrameWnd::OnRButtonUp(nFlags, point);
}

#define VERTEX_SNAP_PROXIMITY		100

void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point) 
{
	short loop,t_sector,t_edge;	
	tPoint2D temp;
	long dist;
	char message[80];

	temp.x = (long)((point.x + m_offX - (m_sizeX / 2)) / m_scale);
	temp.y = (long)-((point.y + m_offY - (m_sizeY / 2)) / m_scale);
	if ((nFlags & MK_CONTROL) > 0)
	{
		t_sector = -1;
		for (loop = 0; loop < m_edgecnt; loop++)
		{
			dist = (temp.x - m_edgelist[loop].pos.x) * 		// SQUARED DISTANCE
					(temp.x - m_edgelist[loop].pos.x) +
					(temp.y - m_edgelist[loop].pos.y) * 
					(temp.y - m_edgelist[loop].pos.y); 
			if (dist < VERTEX_SNAP_PROXIMITY)
			{
				temp.x = m_edgelist[loop].pos.x;
				temp.y = m_edgelist[loop].pos.y;
				t_sector = m_edgelist[loop].sector;
				t_edge = loop;
			}
		}
		if (t_sector == -1)	// IF I DIDN'T CLICK ON AN EXISTING POINT, SNAP IT
		{
			SnapPoint(&temp);
			// CHECK ALL POINTS AGAIN ONCE I HAVE SNAPPED
			for (loop = 0; loop < m_edgecnt; loop++)
			{
				dist = (temp.x - m_edgelist[loop].pos.x) *		// SQUARED DISTANCE
						(temp.x - m_edgelist[loop].pos.x) +
						(temp.y - m_edgelist[loop].pos.y) * 
						(temp.y - m_edgelist[loop].pos.y); 
				if (dist < VERTEX_SNAP_PROXIMITY)
				{
					temp.x = m_edgelist[loop].pos.x;
					temp.y = m_edgelist[loop].pos.y;
					t_sector = m_edgelist[loop].sector;
					t_edge = loop;
				}
			}
		}
		if (m_cursector == NULL)	// haven't created a sector
		{
			m_cursector = &m_sectorlist[m_sectorcnt];
			m_cursector->edge = m_edgecnt;
			m_cursector->edgecnt = 1;
			m_cursector->flags = 0;
			m_curedge = &m_edgelist[m_edgecnt];
			m_curedge->pos.x = temp.x;
			m_curedge->pos.y = temp.y;
			m_curedge->sector = m_sectorcnt;
			m_curedge->backsector = -1;
			m_curedge->backedge = -1;
			m_curedge->nextedge = -1;
			m_curedge->prevedge = -1;
			m_firstedge = m_curedge;
			m_sectorcnt++;
			m_edgecnt++;
			Invalidate(TRUE);
		}
		else
		{
			if (&m_sectorlist[t_sector] == m_cursector)
			{
				if (t_edge == m_cursector->edge)	// CLOSED THE SECTOR
				{
					m_curedge->nextedge = m_cursector->edge;
					m_firstedge->prevedge = m_edgecnt - 1;	// LINK BACK THE START TO THE END
					m_cursector->flags = SECTOR_FLAGS_CLOSED;
					m_cursector->ceil_tex = 0;
					m_cursector->floor_tex = 0;
					m_cursector->floor_height = 0;
					m_cursector->ceil_height = 2048;
					m_cursector = NULL;
					CheckDoubleSided();
				}
			}
			else
			{
				m_curedge->nextedge = m_edgecnt;
				m_cursector->edgecnt++;
				m_curedge = &m_edgelist[m_edgecnt];
				m_curedge->pos.x = temp.x;
				m_curedge->pos.y = temp.y;
				m_curedge->sector = m_sectorcnt - 1;
				m_curedge->backsector = -1;
				m_curedge->backedge = -1;
				m_curedge->nextedge = -1;
				m_curedge->prevedge = m_edgecnt - 1;
				m_edgecnt++;
			}
			Invalidate(TRUE);
		}
		m_point = NULL;
		sprintf(message,"Sectors = %d  Edges = %d",m_sectorcnt,m_edgecnt);
		SetStatusText(1,message);
	}
	else if ((nFlags & MK_SHIFT) > 0)
	{
		m_clickpoint = point;
	}
	else	// MOVE A SET POINT
	{
		m_point = NULL;
		for (loop = 0; loop < m_edgecnt; loop++)
		{
			dist = (temp.x - m_edgelist[loop].pos.x) * 
					(temp.x - m_edgelist[loop].pos.x) +
					(temp.y - m_edgelist[loop].pos.y) * 
					(temp.y - m_edgelist[loop].pos.y); 
			if (dist < 100)
			{
				m_temppoint.x = m_edgelist[loop].pos.x;
				m_temppoint.y = m_edgelist[loop].pos.y;
				m_point = &m_edgelist[loop].pos;
				Invalidate(TRUE);
				break;
			}
		}
	}
	CFrameWnd::OnLButtonDown(nFlags, point);
}

void CMainFrame::OnLButtonUp(UINT nFlags, CPoint point) 
{
	short loop;
	char message[80];
	short t_edge,next;
	if (m_point != NULL)
	{
		SnapPoint(m_point);
		// Check if I need to delete similar edges
		CheckDeleteEdges(&m_temppoint,m_point);
		// MOVE ALL POINTS THAT WERE AT THE SAME LOCATION
		for (loop = 0; loop < m_edgecnt; loop++)
		{
			if (m_temppoint.x == m_edgelist[loop].pos.x &&
				m_temppoint.y == m_edgelist[loop].pos.y)
			{
				m_edgelist[loop].pos.x = m_point->x;
				m_edgelist[loop].pos.y = m_point->y;
			}
			// STORE OFF THE MOVED POINT
			if (m_point == &m_edgelist[loop].pos)
				t_edge = loop;
		}
		// CHECK IF I NEED TO DELETE THIS EDGE
		next = m_edgelist[t_edge].nextedge;
		if (m_point->x == m_edgelist[next].pos.x &&
			m_point->y == m_edgelist[next].pos.y)
		{
			DeleteEdge(t_edge);
		}

		CheckDoubleSided();

		sprintf(message,"Sectors = %d  Edges = %d",m_sectorcnt,m_edgecnt);
		SetStatusText(1,message);
		Invalidate(TRUE);
	}
	CFrameWnd::OnLButtonUp(nFlags, point);
}

void CMainFrame::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	tPoint2D temp;
	char message[80];
	short sec;

	temp.x = (long)((point.x + m_offX - (m_sizeX / 2)) / m_scale);
	temp.y = (long)-((point.y + m_offY - (m_sizeY / 2)) / m_scale);

	sec = InsideSector(&temp);

	if (sec == -1)
	{
		MessageBox("Not inside any..","Inside Sector Test",MB_OK);
	}
	else
	{
		sprintf(message,"Inside Sector %d",sec);
		MessageBox(message,"Inside Sector Test",MB_OK);
	}
	CFrameWnd::OnLButtonDblClk(nFlags, point);
}

void CMainFrame::OnMouseMove(UINT nFlags, CPoint point) 
{
	long startX,startY;
	short nearest;
	CClientDC dc(this); // device context for painting
	if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
	{

		if ((nFlags & MK_SHIFT) > 0)
		{
			m_offY += m_clickpoint.y - point.y;
			m_offX += m_clickpoint.x - point.x;
			Invalidate(TRUE);
			m_clickpoint = point;
		}
		else if (m_point != NULL)
		{
			dc.SetROP2(R2_NOT);
			// ERASE OLD POINT
			startX = (m_point->x * m_scale) - m_offX + (m_sizeX / 2);
			startY = (-m_point->y * m_scale) - m_offY + (m_sizeY / 2);
			dc.MoveTo(startX-2,startY-2);
			dc.LineTo(startX+2,startY-2);
			dc.LineTo(startX+2,startY+2);
			dc.LineTo(startX-2,startY+2);
			dc.LineTo(startX-2,startY-2);

			m_point->x = (long)((point.x + m_offX - (m_sizeX / 2)) / m_scale);
			m_point->y = -(long)((point.y + m_offY - (m_sizeY / 2)) / m_scale);

			startX = (m_point->x * m_scale) - m_offX + (m_sizeX / 2);
			startY = (-m_point->y * m_scale) - m_offY + (m_sizeY / 2);
			dc.MoveTo(startX-2,startY-2);
			dc.LineTo(startX+2,startY-2);
			dc.LineTo(startX+2,startY+2);
			dc.LineTo(startX-2,startY+2);
			dc.LineTo(startX-2,startY-2);
		}
	}
	else
	{
		dc.SetROP2(R2_NOT);
		nearest = GetNearestEdge(point);
		if (nearest >= 0)
		{
			m_nearest_edge = nearest;
			// TODO: m_old_pnt is not defined at start
			dc.MoveTo(m_old_pnt.x-1,m_old_pnt.y-1);
			dc.LineTo(m_old_pnt.x+1,m_old_pnt.y-1);
			dc.LineTo(m_old_pnt.x+1,m_old_pnt.y+1);
			dc.LineTo(m_old_pnt.x-1,m_old_pnt.y+1);
			dc.LineTo(m_old_pnt.x-1,m_old_pnt.y-1);
			startX = (m_nearest_pnt.x * m_scale) - m_offX + (m_sizeX / 2);
			startY = (-m_nearest_pnt.y * m_scale) - m_offY + (m_sizeY / 2);
			dc.MoveTo(startX-1,startY-1);
			dc.LineTo(startX+1,startY-1);
			dc.LineTo(startX+1,startY+1);
			dc.LineTo(startX-1,startY+1);
			dc.LineTo(startX-1,startY-1);
			m_old_pnt.x = startX;
			m_old_pnt.y = startY;
		}
	}
	CFrameWnd::OnMouseMove(nFlags, point);
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	Draw2DView(&dc);
	// Do not call CFrameWnd::OnPaint() for painting messages
}



void CMainFrame::OnFileNew() 
{
	// RESET THE WORLD TO NOTHING
	char message[80];
	m_sectorcnt = 0;
	m_edgecnt = 0;
	sprintf(message,"Sectors = %d  Edges = %d",m_sectorcnt,m_edgecnt);
	SetStatusText(1,message);
	Invalidate(TRUE);
}

// LOAD A SET OF SECTORS FROM A FILE
void CMainFrame::OnFileOpen() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Fate Files (*.fte)|*.FTE|All Files (*.*)|*.*||";
	char directory[80];
	CFileDialog	*dialog;
	FILE	*fp;
///////////////////////////////////////////////////////////////////////////////		
	// HAD TO ADD DIRECTORY STUFF SINCE DIALOG CHANGES DIRECTORY
	_getcwd(directory,80);
	dialog = new CFileDialog(TRUE,"FTE",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		fp = fopen(dialog->GetPathName(),"rb");
		if (fp != NULL)
		{
			fread(&m_sectorcnt,sizeof(short),1,fp);
			fread(m_sectorlist,sizeof(tSector),m_sectorcnt,fp);
			fread(&m_edgecnt,sizeof(short),1,fp);
			fread(m_edgelist,sizeof(tEdge),m_edgecnt,fp);
			fread(&m_cam_pos,sizeof(tPoint3D),1,fp);
			fclose(fp);
		}
	}
	// RESET THE MAIN DIRECTORY
	_chdir(directory);
	Invalidate(TRUE);
}

// SAVE THE CURRENT SETUP TO A FILE
void CMainFrame::OnFileSave() 
{
/// Local Variables ///////////////////////////////////////////////////////////
	char BASED_CODE szFilter[] = "Fate Files (*.fte)|*.FTE|All Files (*.*)|*.*||";
	char directory[80];
	CFileDialog	*dialog;
	FILE	*fp;
///////////////////////////////////////////////////////////////////////////////		
	// HAD TO ADD DIRECTORY STUFF SINCE DIALOG CHANGES DIRECTORY
	_getcwd(directory,80);
	dialog = new CFileDialog(FALSE,"PFTE",NULL,OFN_OVERWRITEPROMPT,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		fp = fopen(dialog->GetPathName(),"wb");
		if (fp != NULL)
		{
			fwrite(&m_sectorcnt,sizeof(short),1,fp);
			fwrite(m_sectorlist,sizeof(tSector),m_sectorcnt,fp);
			fwrite(&m_edgecnt,sizeof(short),1,fp);
			fwrite(m_edgelist,sizeof(tEdge), m_edgecnt,fp);
			fwrite(&m_cam_pos,sizeof(tPoint3D),1,fp);
			fclose(fp);
		}
	}
	// RESET THE MAIN DIRECTORY
	_chdir(directory);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame Implementation Functions

// CREATE A FIXED POINT TRIG TABLE
void CMainFrame::InitTrig()
{
	int     angle;
	float   f_angle;

	for(angle = 0; angle < MAX_ANGLE; angle++)
	{
		f_angle = ((float)angle * M_PI) / (MAX_ANGLE / 2);      // Convert degrees to radians
		m_sin[angle] = (long)(sin(f_angle) * (1 << PRECISION));           // Get fixed point sine 2^16
		m_cos[angle] = (long)(cos(f_angle) * (1 << PRECISION));         // Get fixed point cosine 2^16
	}
}

/////////////////////////////////////////////////////////////////////////////
// Procedure:	DrawCurPos
// Purpose:		Draw the current POV Position on the Map
void CMainFrame::DrawCurPos(CDC* pDC)
{
	CPen penPOV;
	long startX,startY,endX,endY;
	double lenX,lenY;
	short left,right;

	penPOV.CreatePen(PS_SOLID,1,0x00005000);
	pDC->SelectObject(&penPOV);

	lenX = (double)((10 * m_sin[m_cam_yaw])/PRECMULT);
	lenY = (double)((15 * m_cos[m_cam_yaw])/PRECMULT);

	startX = (m_cam_pos.x * m_scale) - m_offX + (m_sizeX / 2);
	startY = (-m_cam_pos.z * m_scale) - m_offY + (m_sizeY / 2);

	endX = startX + lenX;
	endY = startY - lenY;

	pDC->MoveTo(startX,startY);
	pDC->LineTo(endX,endY);

	// DO THE ARROWHEAD PART

	startX = endX;
	startY = endY;

	left = m_cam_yaw + 4096 - 512;
	left %= 4096;

	lenX = (m_sin[left]>>13);
	lenY = (m_cos[left]>>13);

	endX = startX - lenX;
	endY = startY + lenY;

	pDC->MoveTo(startX,startY);
	pDC->LineTo(endX,endY);

	right = m_cam_yaw + 512;
	right %= 4096;

	lenX = (m_sin[right]>>13);
	lenY = (m_cos[right]>>13);

	endX = startX - lenX;
	endY = startY + lenY;

	pDC->MoveTo(startX,startY);
	pDC->LineTo(endX,endY);

}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame drawing
void CMainFrame::Draw2DView(CDC* pDC)
{
	long startX,startY,tempX,tempY;
	long loop,loop2,pos;
	tPoint2D *pnt;
	CPen pengrid,penclosedline,pendoublesided,penblue,penltblue,*curpen;
	CRect aRect;
	pDC->SetROP2(R2_COPYPEN);

	pengrid.CreatePen(PS_SOLID,1,0x00c0c0c0);
	penclosedline.CreatePen(PS_SOLID,1,0x00505050);
	pendoublesided.CreatePen(PS_SOLID,1,0x000000FF);
	penblue.CreatePen(PS_SOLID,1,0x00FF0000);
	penltblue.CreatePen(PS_SOLID,1,0x00FF4040);
	pDC->SelectObject(&pengrid);


	GetWindowRect(&aRect);
	m_sizeX = aRect.right - aRect.left;
	m_sizeY = aRect.bottom - aRect.top;


	if (m_gridsize > 1)
	{
		startX = -65536;
		startY = -65536;
		// DO ALL THE COLUMNS
		for (loop = startX; loop < 65536; loop += m_gridsize)	
		{
			pos = (long)((double)loop * m_scale);
			pos = pos - m_offX + (m_sizeX / 2);
			if (pos >=0 && pos < m_sizeX)
			{
				pDC->MoveTo(pos,0);
				pDC->LineTo(pos,m_sizeY);
			}
		}
		// DO ALL THE COLUMNS
		for (loop = startY; loop < 65536; loop += m_gridsize)	
		{
			pos = (long)((double)loop * m_scale);
			pos = pos - m_offY + (m_sizeY / 2);
			if (pos >=0 && pos < m_sizeY)
			{
				pDC->MoveTo(0,pos);
				pDC->LineTo(m_sizeX,pos);
			}
		}
	}
	
	// DRAW ALL THE LINES
	for (loop = 0; loop < m_sectorcnt; loop++)	// FOR ALL THE SECTORS
	{
		if ((m_sectorlist[loop].flags & SECTOR_FLAGS_CLOSED) == 0)
		{
			curpen = &penltblue;
		}
		else
		{
			curpen = &penclosedline;
		}
		pos = m_sectorlist[loop].edge;
		for (loop2 = 0; loop2 < m_sectorlist[loop].edgecnt; loop2++)
		{
			pnt = &m_edgelist[pos].pos;
			tempX = (pnt->x * m_scale) - m_offX + (m_sizeX / 2);
			tempY = (-pnt->y * m_scale) - m_offY + (m_sizeY / 2);
			if (loop2 == 0)	// FIRST EDGE SAVE POINT
			{
				pDC->MoveTo(tempX,tempY);
				startX = tempX;
				startY = tempY;
			}
			else
			{
				pDC->LineTo(tempX,tempY);
			}
			if (m_edgelist[pos].backedge > -1)
			{
				pDC->SelectObject(&pendoublesided);
			}
			else
			{
				pDC->SelectObject(curpen);
			}
			pos = m_edgelist[pos].nextedge;
		}
		if ((m_sectorlist[loop].flags & SECTOR_FLAGS_CLOSED) > 0)
		{
			pDC->LineTo(startX,startY);
		}
	}

	// DRAW THE POINTS
	for (loop = 0; loop < m_sectorcnt; loop++)	// FOR ALL THE SECTORS
	{
		pDC->SelectObject(&penblue);
		pos = m_sectorlist[loop].edge;
		for (loop2 = 0; loop2 < m_sectorlist[loop].edgecnt; loop2++)
		{
			pnt = &m_edgelist[pos].pos;
			startX = (pnt->x * m_scale) - m_offX + (m_sizeX / 2);
			startY = (-pnt->y * m_scale) - m_offY + (m_sizeY / 2);
			pDC->MoveTo(startX-2,startY-2);
			pDC->LineTo(startX+2,startY-2);
			pDC->LineTo(startX+2,startY+2);
			pDC->LineTo(startX-2,startY+2);
			pDC->LineTo(startX-2,startY-2);
			pos = m_edgelist[pos].nextedge;
		}
	}

	if (m_nearest_edge > -1)
	{
		pDC->SelectObject(&penltblue);
		startX = (m_nearest_pnt.x * m_scale) - m_offX + (m_sizeX / 2);
		startY = (-m_nearest_pnt.y * m_scale) - m_offY + (m_sizeY / 2);
		pDC->MoveTo(startX-1,startY-1);
		pDC->LineTo(startX+1,startY-1);
		pDC->LineTo(startX+1,startY+1);
		pDC->LineTo(startX-1,startY+1);
		pDC->LineTo(startX-1,startY-1);
	}

	DrawCurPos(pDC);
/*
	sprintf(message,"Nearest #%d",m_nearest_edge);
	m_MainFrame->SetStatusText(0,message);*/
}

void CMainFrame::TransformPoint(long world_x, long world_z, long *camera_x, long *camera_z)
{
	long move_x, move_z;

	/* DO POSITION. TRANSLATE ABOUT LOOK AT POINT.. */
	move_x = world_x - m_cam_pos.x;
	move_z = world_z - m_cam_pos.z;

	/* DO YAW... */
	*camera_x = (FixMult(m_cos[m_cam_yaw],move_x)) -
				(FixMult(m_sin[m_cam_yaw],move_z));

	*camera_z = (FixMult(m_cos[m_cam_yaw],move_z)) +
				(FixMult(m_sin[m_cam_yaw],move_x));

}

BOOL CMainFrame::OnSamePos(short pos1,short pos2)
{
	if (m_edgelist[pos1].pos.x == m_edgelist[pos2].pos.x &&
		m_edgelist[pos1].pos.y == m_edgelist[pos2].pos.y)
		return TRUE;
	else
		return FALSE;
}

// CHECK FOR DOUBLE SIDED EDGES
void CMainFrame::CheckDoubleSided()
{
	short inner,outer;
	for (outer = 0; outer < m_edgecnt; outer++)
	{
		for (inner = 0; inner < m_edgecnt; inner++)
		{
			// THEY SHARE AN EDGE IF
			if (OnSamePos(outer,m_edgelist[inner].nextedge) &&
				OnSamePos(inner,m_edgelist[outer].nextedge))
			{
				m_edgelist[inner].backedge = outer;
				m_edgelist[inner].backsector = m_edgelist[outer].sector;
				m_edgelist[outer].backedge = inner;
				m_edgelist[outer].backsector = m_edgelist[inner].sector;
			}
		}		
	}
}

void CMainFrame::InsertPoint()
{
	tEdge	*this_edge,*new_edge;
	tPoint2D t_pnt;
	if (m_nearest_edge >= 0)
	{
		t_pnt.x = m_nearest_pnt.x;
		t_pnt.y = m_nearest_pnt.y;
		SnapPoint(&t_pnt);
		this_edge = &m_edgelist[m_nearest_edge];
		new_edge = &m_edgelist[m_edgecnt];
		new_edge->pos.x = t_pnt.x;
		new_edge->pos.y = t_pnt.y;
		new_edge->sector = this_edge->sector;
		new_edge->backsector = -1;
		new_edge->backedge = -1;
		new_edge->prevedge = m_nearest_edge;
		m_edgelist[this_edge->nextedge].prevedge = m_edgecnt;
		new_edge->nextedge = this_edge->nextedge;
		this_edge->nextedge = m_edgecnt;
		m_edgecnt++;
		m_sectorlist[this_edge->sector].edgecnt++;
	}
}

void CMainFrame::DeleteEdge(short which)
{
	short loop;	
	tEdge *t_edge;
	t_edge = &m_edgelist[which];
	char message[80];
	sprintf(message,"Delete # %d",which);
	MessageBox(message,"Delete Edge",MB_OK);
	// FIX THE POINTERS TO THIS EDGE
	// DO IT FOR THE SECTORS
	for (loop = 0; loop < m_sectorcnt; loop ++)
	{
		if (m_sectorlist[loop].edge == which)
		{
			m_sectorlist[loop].edge = t_edge->nextedge;
			m_edgelist[m_edgelist[loop].nextedge].prevedge = 
				m_edgelist[m_sectorlist[loop].edge].prevedge;
		}
	}
	// DO IT FOR THE EDGES
	for (loop = 0; loop < m_edgecnt; loop++)
	{
		if (m_edgelist[loop].nextedge == which)
		{
			m_edgelist[loop].nextedge = t_edge->nextedge;
			m_edgelist[m_edgelist[loop].nextedge].prevedge = 
				m_edgelist[loop].prevedge;
		}
	}

	// RESET THE MEMORY AND THE POINTERS
	for (loop = which; loop < m_edgecnt; loop++)
	{
		memcpy( &m_edgelist[loop],&m_edgelist[loop+1],sizeof(tEdge) );
	}
	m_edgecnt--;	// DECREMENT THE EDGECNT
	// RESET THE POINTERS IN THE SECTORS
	for (loop = 0; loop < m_sectorcnt; loop ++)
	{
		if (m_sectorlist[loop].edge >= which)
			m_sectorlist[loop].edge--;
	}
	// RESET THE POINTERS IN THE EDGES
	for (loop = 0; loop < m_edgecnt; loop ++)
	{
		if (m_edgelist[loop].nextedge >= which)
			m_edgelist[loop].nextedge--;
		if (m_edgelist[loop].prevedge>= which)
			m_edgelist[loop].prevedge--;
	}
}

void CMainFrame::DeleteSector()
{
	char message[80];
	short loop, t_sector;	
	if (m_nearest_edge > -1)
	{
		t_sector = m_edgelist[m_nearest_edge].sector;
		if (m_cursector == &m_sectorlist[t_sector])
		{
			m_cursector = NULL;
		}
		// DELETE IN THE EDGES
		for (loop = 0; loop < m_edgecnt; loop ++)
		{
			if (m_edgelist[loop].sector == t_sector)
			{
				DeleteEdge(loop);
				loop --;
			}
		}
		// DELETE THE ACTUAL SECTOR
		for (loop = t_sector; loop < m_sectorcnt; loop ++)
		{
			memcpy( &m_sectorlist[loop],&m_sectorlist[loop+1],sizeof(tSector) );
		}
		m_sectorcnt--;	
	}
	sprintf(message,"Sectors = %d  Edges = %d",m_sectorcnt,m_edgecnt);
	SetStatusText(1,message);
}

// CLOSES AN EDITING SECTOR IF IT IS OPEN
void CMainFrame::CloseSector()
{
	char message[80];
	sprintf(message,"Close Sector %d?",m_sectorcnt);
	if (MessageBox(message,"Delete Edge",MB_YESNO))
	{
		if (m_cursector != NULL)	// I AM WORKING ON ONE
		{
			m_curedge->nextedge = m_cursector->edge;
			m_firstedge->prevedge = m_edgecnt - 1;	// LINK BACK THE START TO THE END
			m_cursector->flags = SECTOR_FLAGS_CLOSED;
			m_cursector->ceil_tex = 0;
			m_cursector->floor_tex = 0;
			m_cursector->floor_height = 0;
			m_cursector->ceil_height = 2048;
			m_cursector = NULL;
			CheckDoubleSided();
		}
	}
}

void CMainFrame::SnapPoint(tPoint2D *point)
{
	if (m_snap && m_gridsize > 1)
	{
		if (point->x > 0) point->x += (m_gridsize / 2);
		else point->x -= (m_gridsize / 2);
		point->x = (point->x / m_gridsize) * m_gridsize;
		if (point->y > 0) point->y += (m_gridsize / 2);
		else point->y -= (m_gridsize / 2);
		point->y = (point->y / m_gridsize) * m_gridsize;
	}
}

//////////////////////////////////////////////////////////////////
// CheckDeleteEdges
// Purpose:	To find if moving a point caused an edge to be deleted
//////////////////////////////////////////////////////////////////
void CMainFrame::CheckDeleteEdges(tPoint2D *a,tPoint2D *b)
{
	short loop,next;
	for (loop = 0; loop < m_edgecnt; loop++)
	{
		next = m_edgelist[loop].nextedge;
		if (next > -1)
		{
			if (a->x == m_edgelist[loop].pos.x && a->y == m_edgelist[loop].pos.y &&
				b->x == m_edgelist[next].pos.x && 
				b->y == m_edgelist[next].pos.y)
			{
				DeleteEdge(loop);
				loop--;
			}
			if (b->x == m_edgelist[loop].pos.x && b->y == m_edgelist[loop].pos.y &&
				a->x == m_edgelist[next].pos.x && 
				a->y == m_edgelist[next].pos.y)
			{
				DeleteEdge(loop);
				loop--;
			}
			// IF IT IS A SINGLE POINT, DELETE IT
			if (m_edgelist[loop].pos.x == 
				m_edgelist[next].pos.x && 
				m_edgelist[loop].pos.y == 
				m_edgelist[next].pos.y )
			{
				DeleteEdge(loop);
				loop--;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	GetNearestPoint
// Purpose:		Find the nearest point on a line segment
// Arguments:	Two endpoints to a line segment a and b,
//				and a test point c
// Returns:		Sets the nearest point on the segment in nearest
///////////////////////////////////////////////////////////////////////////////
void CMainFrame::GetNearestPoint(tPoint2D *a,tPoint2D *b,tPoint2D *c,tPoint2D *nearest)
{
/// Local Variables ///////////////////////////////////////////////////////////
	long dot_ta,dot_tb;
///////////////////////////////////////////////////////////////////////////////
	// SEE IF a IS THE NEAREST POINT - ANGLE IS OBTUSE
	dot_ta = (c->x - a->x)*(b->x - a->x) + (c->y - a->y)*(b->y - a->y);
	if (dot_ta <= 0)	// It is off the a vertex
	{
		nearest->x = a->x;
		nearest->y = a->y;
		return;
	}
	dot_tb = (c->x - b->x)*(a->x - b->x) + (c->y - b->y)*(a->y - b->y);
	// SEE IF b IS THE NEAREST POINT - ANGLE IS OBTUSE
	if (dot_tb <= 0)
	{
		nearest->x = b->x;
		nearest->y = b->y;
		return;
	}
	// FIND THE REAL NEAREST POINT ON THE LINE SEGMENT - BASED ON RATIO
	nearest->x = a->x + ((b->x - a->x) * dot_ta)/(dot_ta + dot_tb);
	nearest->y = a->y + ((b->y - a->y) * dot_ta)/(dot_ta + dot_tb);
}

short CMainFrame::GetNearestEdge(CPoint point) 
{
	short loop,winner = -1,next;
	long distance = 20000,tempdist;
	tPoint2D *a,*b,temp,nearest;
	temp.x = (long)((point.x + m_offX - (m_sizeX / 2)) / m_scale);
	temp.y = -(long)((point.y + m_offY - (m_sizeY / 2)) / m_scale);
	// CHECK ALL POINTS
	for (loop = 0; loop < m_edgecnt; loop++)
	{
		a = &m_edgelist[loop].pos;
		next = m_edgelist[loop].nextedge;
		if (next >= 0)
		{
			b = &m_edgelist[next].pos;
			GetNearestPoint(a,b,&temp,&nearest); 
			tempdist =  (nearest.x - temp.x) * (nearest.x - temp.x) + 
						(nearest.y - temp.y) * (nearest.y - temp.y);
			if (tempdist < distance)
			{
				distance = tempdist;
				winner = loop;
				m_nearest_pnt.x = nearest.x;
				m_nearest_pnt.y = nearest.y;
			}
		}
	}
	return winner;
}

// DISTANCE THRESHOLD TO WALLS - SORT OF ARBITRARY -  ABOUT A FOOT
#define DISTANCE_THRESH		256

//////////////////////////////////////////////////////////////////
// Procedure:	CheckCollision
// Purpose:		Make Sure view is not too close to wall
// Returns:		TRUE if Valid Position, else FALSE
//////////////////////////////////////////////////////////////////
BOOL CMainFrame::CheckCollision(short sector, tPoint3D *test) 
{
	short loop,next,cur;
	long tempdist,temp2;
	tPoint2D *a,*b,temp,nearest;
	// CHECK ALL POINTS

	temp.x = test->x;
	temp.y = test->z;

	// IF MY TEST POINT IS OUTSIDE THE CURRENT SECTOR, GET A NEW ONE
	// TODO: COULD BE OPTIMISED BASED ON WHEN YOU CROSS A DOUBLE SIDED EDGE
	//       NO NEED NOW IT IS FAST ENOUGH
	if (!PointInPoly(&m_sectorlist[sector], &temp))
	{
		// GET THE SECTOR THAT THE TEST POINT IS IN
		sector = m_cam_sector = InsideSector(&temp);
	}

	if (sector > -1)
	{
		cur = m_sectorlist[sector].edge;

		for (loop = 0; loop < m_sectorlist[sector].edgecnt; loop++)
		{
			a = &m_edgelist[cur].pos;
			next = m_edgelist[cur].nextedge;
			if (next >= 0)		// IF THERE IS A NEXT
			{
				b = &m_edgelist[next].pos;
				// IF THE EDGE IS DOUBLE SIDED JUST CHECK ENDPOINTS
				if (m_edgelist[cur].backedge > -1)
				{
					// CHECK ONLY THE SQUARED DISTANCE - NO NEED FOR A SQRT
					tempdist =  (a->x - test->x) * (a->x - test->x) + 
								(a->y - test->z) * (a->y - test->z);
					temp2 =		(b->x - test->x) * (b->x - test->x) + 
								(b->y - test->z) * (b->y - test->z);
					// WHICH IS THE CLOSEST
					if (temp2 < tempdist)
						tempdist = temp2;
				}
				else	// EDGE IS NOT DOUBLE SIDED SO FIND NEAREST POINT ON LINE
				{
					GetNearestPoint(a,b,&temp,&nearest); 
					// CHECK ONLY THE SQUARED DISTANCE - NO NEED FOR A SQRT
					tempdist =  (nearest.x - test->x) * (nearest.x - test->x) + 
								(nearest.y - test->z) * (nearest.y - test->z);
				}
				if (tempdist < DISTANCE_THRESH)
				{
					return FALSE;		// TOO CLOSE
				}
			}
			cur = next;
		}

		return TRUE;	// VALID POSITION
	}
	else
		return FALSE;	// OUTSIDE SECTOR

}

#ifdef USE_VERSION2
// FIGURE OUT WHICH QUADRANT THE VERTEX IS RELATIVE TO THE HIT POINT
#define WHICH_QUAD(vertex, hitPos) \
  ( (vertex.x > hitPos->x) ? ((vertex.y > hitPos->y) ? 1 : 4) : ( (vertex.y > hitPos->y) ? 2 : 3) )
// GET THE X INTERCEPT OF THE LINE FROM THE CURRENT VERTEX TO THE NEXT
#define X_INTERCEPT(point1, point2,  hitY) \
  (point2.x - ( ((point2.y - hitY) * (point1.x - point2.x)) / (point1.y - point2.y) ) )

//////////////////////////////////////////////////////////////////
// Procedure:	PointInPoly (SUM OF ANGLES CROSSING VERSION)
// Purpose:		Check if a point is inside a polygon
// Returns:		TRUE if Point is inside polygon, else FALSE
//////////////////////////////////////////////////////////////////
BOOL CMainFrame::PointInPoly(tSector *sector, tPoint2D *hitPos)
{
	short	edge, first, next;
	short	quad, next_quad, delta, total;

	edge = first = sector->edge;    
	quad = WHICH_QUAD(m_edgelist[edge].pos, hitPos);
	total = 0;		// COUNT OF ABSOLUTE SECTORS CROSSED
	/* LOOP THROUGH THE VERTICES IN A SECTOR */
	do {
		next = m_edgelist[edge].nextedge;					
		next_quad = WHICH_QUAD(m_edgelist[next].pos, hitPos);
		delta = next_quad - quad;		// HOW MANY QUADS HAVE I MOVED
		// SPECIAL CASES TO HANDLE CROSSINGS OF MORE THEN ONE QUAD
		switch (delta) {							
			case  2: // IF WE CROSSED THE MIDDLE, FIGURE OUT IF IT WAS CLOCKWISE OR COUNTER
			case -2: 
				// US THE X POSITION AT THE HIT POINT TO DETERMINE WHICH WAY AROUND
				if (X_INTERCEPT(m_edgelist[edge].pos, m_edgelist[next].pos, hitPos->y) > hitPos->x)	
					delta =  - (delta);					
				break;							
			case  3:			// MOVING 3 QUADS IS LIKE MOVING BACK 1
				delta = -1; 
				break;					
			case -3:			// MOVING BACK 3 IS LIKE MOVING FORWARD 1
				delta =	 1; 
				break;				
		}
		/* ADD IN THE DELTA */
		total += delta;
		quad = next_quad;		// RESET FOR NEXT STEP
		edge = next;
	} while (edge != first);

	/* AFTER ALL IS DONE IF THE TOTAL IS 4 THEN WE ARE INSIDE */
	if ((total == +4) || (total == -4)) return TRUE; else return FALSE;
}

#else
//////////////////////////////////////////////////////////////////
// Procedure:	PointInPoly (EDGE CROSSING VERSION)
// Purpose:		Check if a point is inside a polygon
// Returns:		TRUE if Point is inside polygon, else FALSE
//////////////////////////////////////////////////////////////////
BOOL CMainFrame::PointInPoly(tSector *sector, tPoint2D *hitPos)
{
	short  edge, first, next;
	tPoint2D *pnt1,*pnt2;
	BOOL	inside = FALSE;		// INITIAL TEST CONDITION
	BOOL  flag1,flag2;

	edge = first = sector->edge;		// SET UP INITIAL CONDITIONS
	pnt1 = &m_edgelist[edge].pos;
    flag1 = ( hitPos->y >= pnt1->y ) ;	// IS THE FIRST VERTEX OVER OR UNDER THE LINE
	/* LOOP THROUGH THE VERTICES IN A SECTOR */
	do {
		next = m_edgelist[edge].nextedge;	// CHECK THE NEXT VERTEX
		pnt2 = &m_edgelist[next].pos;
	    flag2 = ( hitPos->y >= pnt2->y );	// IS IT OVER OR UNDER

		if (flag1 != flag2)		// MAKE SURE THE EDGE ACTUALLY CROSSES THE TEST X AXIS
		{
			// CALCULATE WHETHER THE SEGMENT ACTUALLY CROSSES THE X TEST AXIS
			// A TRICK FROM GRAPHIC GEMS IV TO GET RID OF THE X INTERCEPT DIVIDE
			if (((pnt2->y - hitPos->y) * (pnt1->x - pnt2->x) >=
				(pnt2->x - hitPos->x) * (pnt1->y - pnt2->y)) == flag2 )
				inside = !inside;	// IF IT CROSSES TOGGLE THE INSIDE FLAG (ODD IS IN, EVEN OUT)
		}
		pnt1 = pnt2;	// RESET FOR NEXT STEP
		edge = next;
		flag1 = flag2;
	} while (edge != first);
	return inside;
}
#endif

//////////////////////////////////////////////////////////////////
// Procedure:	InsideSector
// Purpose:		Find What sector a Point is in
// Returns:		Number of Sector that point is in or -1
// Notes:		Could be optimized based on adjacent sectors
//////////////////////////////////////////////////////////////////
short CMainFrame::InsideSector(tPoint2D *hitPos)
{
	unsigned short loop;
	short ret = -1;
	char message[80];
	for (loop = 0; loop < m_sectorcnt; loop++)	// FOR ALL THE SECTORS
	{
		if ((m_sectorlist[loop].flags & SECTOR_FLAGS_CLOSED) == 1)
		{
			if (PointInPoly(&m_sectorlist[loop],hitPos))
			{
				ret = loop;
				break;
			}
		}
	}

	sprintf(message,"Cam in Sector = %d",ret);
	SetStatusText(1,message);
	return ret;
}
