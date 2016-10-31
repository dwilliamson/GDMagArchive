// AguaDlg.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "Agua.h"
#include "AguaDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// TODO: Adjust this to make the field smaller and it will run faster
//			128 is pretty good for speed
#define WATER_SIZE				256

#define RGB32(r,g,b)	((r << 16) + (g << 8) + b)
#define GetR32(color)	(0x000000ff & (color >> 16))
#define GetG32(color)	(0x000000ff & (color >> 8))
#define GetB32(color)	(0x000000ff & color)

#define SETBUFFER(buf,x,y,value)	(buf[((y) * WATER_SIZE) + (x)] = value)
#define READBUFFER(buf,x,y)			(buf[((y) * WATER_SIZE) + (x)])

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAguaDlg dialog

CAguaDlg::CAguaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAguaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAguaDlg)
	m_Drip_Radius = 12;
	m_DampingFactor = 0.002f;
	m_ImageFile = _T("Reflect.tga");
	m_Green = 128;
	m_Red = 128;
	m_Blue = 128;
	m_UseImage = TRUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAguaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAguaDlg)
	DDX_Control(pDX, IDC_DISPLAY, m_Display);
	DDX_Text(pDX, IDC_DRIP_RADIUS, m_Drip_Radius);
	DDX_Text(pDX, IDC_DAMPING, m_DampingFactor);
	DDX_Text(pDX, IDC_IMAGEFILE, m_ImageFile);
	DDX_Text(pDX, IDC_GREEN, m_Green);
	DDV_MinMaxUInt(pDX, m_Green, 0, 255);
	DDX_Text(pDX, IDC_RED, m_Red);
	DDV_MinMaxUInt(pDX, m_Red, 0, 255);
	DDX_Text(pDX, IDC_BLUE, m_Blue);
	DDV_MinMaxUInt(pDX, m_Blue, 0, 255);
	DDX_Check(pDX, IDC_USEIMAGE, m_UseImage);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAguaDlg, CDialog)
	//{{AFX_MSG_MAP(CAguaDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_UPDATE(IDC_DAMPING, OnUpdateDamping)
	ON_EN_UPDATE(IDC_DRIP_RADIUS, OnUpdateDripRadius)
	ON_BN_CLICKED(IDC_IMAGE_BROWSE, OnImageBrowse)
	ON_EN_UPDATE(IDC_IMAGEFILE, OnUpdateImagefile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAguaDlg message handlers

BOOL CAguaDlg::OnInitDialog()
{
//// Local Variables ////////////////////////////////////////////////////////////////
    BITMAPINFO	bmi;
    CDC			*hdc;
/////////////////////////////////////////////////////////////////////////////////////
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    memset ( &bmi, 0, sizeof(BITMAPINFO) );
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WATER_SIZE;		//wndRect.right;	// 1024 mode
    bmi.bmiHeader.biHeight = WATER_SIZE; //wndRect.bottom;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdc = GetDC ();
    if ( !hdc )
        return 0;

    m_Bitmap = CreateDIBSection ( hdc->m_hDC , &bmi, DIB_RGB_COLORS, (void **)&m_Buffer, NULL, 0 );
    ReleaseDC ( hdc);

    if ( !m_Bitmap )
        return 0;

	m_Display.SetBitmap(m_Bitmap);

	m_ReadBuffer = (char *)malloc(WATER_SIZE * WATER_SIZE);
	m_WriteBuffer = (char *)malloc(WATER_SIZE * WATER_SIZE);

	for (int i = 0; i < WATER_SIZE * WATER_SIZE; i++) 
	{
		m_ReadBuffer[i] = 0;
		m_WriteBuffer[i] = 0;
	}

	LoadTGAFile("Reflect.tga",&m_ReflectImage);

	SetDisplay();
	
	SetTimer(1,5,NULL);		// Set it up to animate via messages

	m_Drip_Radius_Sqr = m_Drip_Radius * m_Drip_Radius;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAguaDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAguaDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAguaDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CAguaDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int		xpos,ypos;
	CRect	rect;
	CPoint	pnt;
	static bool	dragging;
/////////////////////////////////////////////////////////////////////////////////////
	switch (message)
	{
		case WM_LBUTTONDOWN:
			dragging = true;
			pnt.x = LOWORD(lParam);  // horizontal position of cursor 
			pnt.y = HIWORD(lParam);  // vertical position of cursor 
			ClientToScreen(&pnt);
			m_Display.GetWindowRect(&rect);
			if (rect.PtInRect(pnt))
			{
				xpos = pnt.x - rect.left;
				ypos = (WATER_SIZE - 1) - (pnt.y - rect.top);
				MakeDrip(xpos,ypos, 16);
			}
			break;
		case WM_LBUTTONUP:
			dragging = false;
			break;
		case WM_MOUSEMOVE:
			if (dragging)
			{
				pnt.x = LOWORD(lParam);  // horizontal position of cursor 
				pnt.y = HIWORD(lParam);  // vertical position of cursor 
				ClientToScreen(&pnt);
				m_Display.GetWindowRect(&rect);
				if (rect.PtInRect(pnt))
				{
					xpos = pnt.x - rect.left;
					ypos = (WATER_SIZE - 1) - (pnt.y - rect.top);
					MakeDrip(xpos,ypos, 16);
				}
			}
			break;
		case WM_TIMER:
			ProcessWater();
			break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////////
//	Handle Control UI like text boxes and buttons
////////////////////////////////////////////////////////////////////////////////

void CAguaDlg::OnUpdateDamping() 
{
	UpdateData(TRUE);		
}

void CAguaDlg::OnUpdateDripRadius() 
{
	UpdateData(TRUE);	
}

void CAguaDlg::OnImageBrowse() 
{
	char BASED_CODE szFilter[] = "Targa Files (*.tga)|*.tga||";
	CFileDialog	*dialog;
///////////////////////////////////////////////////////////////////////////////
	dialog = new CFileDialog(TRUE,"TGA",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
	if (dialog->DoModal() == IDOK)
	{
		m_ImageFile = dialog->GetPathName();
	}
	UpdateData(FALSE);	
	OnUpdateImagefile();
}

void CAguaDlg::OnUpdateImagefile() 
{
	UpdateData(TRUE);	
	LoadTGAFile((char *)(LPCSTR)m_ImageFile,&m_ReflectImage);
}

////////////////////////////////////////////////////////////////////////////////
//	Actual Code for creation of the water height field
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	SetDisplay
// Purpose:		To convert the height field into something to look at in the dialog
/////////////////////////////////////////////////////////////////////////////////////
void CAguaDlg::SetDisplay() 
{
//// Local Variables ////////////////////////////////////////////////////////////////
	char	*temp;
	long	color;
	int		r,g,b,xoff,yoff;
/////////////////////////////////////////////////////////////////////////////////////
	// Swap the buffers
	temp = m_ReadBuffer;
	m_ReadBuffer = m_WriteBuffer;
	m_WriteBuffer = temp;

	int cnt = 0;
	for (int j = 0; j < WATER_SIZE; j++) 
	{
		for (int i = 0; i < WATER_SIZE; i++, cnt++) 
		{
			xoff = i;
			if (i > 0 && i < WATER_SIZE - 1)
			{
				xoff -= (m_ReadBuffer[cnt - 1]);
				xoff += (m_ReadBuffer[cnt + 1]);
			}

			yoff = j;
			if (j > 0 && j < WATER_SIZE - 1)
			{
				yoff -= m_ReadBuffer[cnt - WATER_SIZE];
				yoff += m_ReadBuffer[cnt + WATER_SIZE];
			}

			// Clamp the offset to actual picture range
			if (xoff < 0) xoff = 0;
			if (yoff < 0) yoff = 0;
			if (xoff >= m_ReflectImage.header.d_width) xoff = m_ReflectImage.header.d_width - 1;
			if (yoff >= m_ReflectImage.header.d_height) yoff = m_ReflectImage.header.d_height - 1;

			if (m_UseImage)
			{
				color = GetRGBFromTGA(&m_ReflectImage,xoff,yoff);
				r = GetR32(color);
				g = GetG32(color);
				b = GetB32(color);
			}
			else
			{
				r = m_Red;
				g = m_Green;
				b = m_Blue;
			}

			
			r += m_ReadBuffer[cnt];
			g += m_ReadBuffer[cnt];
			b += m_ReadBuffer[cnt];
			// Clamp Color to the valid range
			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;
			m_Buffer[cnt] = RGB32(r,g,b);
		}
	}
	m_Display.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	SquaredDist
// Purpose:		Find the Squared distance between two 2D points
/////////////////////////////////////////////////////////////////////////////////////
int inline CAguaDlg::SquaredDist(int sx, int sy, int dx, int dy)
{
	return ((dx - sx) * (dx - sx)) + ((dy - sy) * (dy - sy));
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	Make Drip
// Purpose:		Creates an initial drip in the water field
//				Usually caused by mouse press
/////////////////////////////////////////////////////////////////////////////////////
void CAguaDlg::MakeDrip(int x, int y, int depth) 
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int i,j,dist,finaldepth;
	CRect bounds(0,0,WATER_SIZE - 1,WATER_SIZE - 1);
/////////////////////////////////////////////////////////////////////////////////////
	for (j = y - m_Drip_Radius; j < y + m_Drip_Radius; j++)
	{
		for (i = x - m_Drip_Radius; i < x + m_Drip_Radius; i++)
		{
			if (bounds.PtInRect(CPoint(i,j)))
			{
				dist = SquaredDist(x,y,i,j);
				if (dist < m_Drip_Radius_Sqr)
				{
					finaldepth = (int)((float)depth * ((float)(m_Drip_Radius - sqrt(dist))/(float)m_Drip_Radius));
					if (finaldepth > 127) finaldepth = 127;
					if (finaldepth < -127) finaldepth = -127;
					SETBUFFER(m_WriteBuffer,i,j,finaldepth);
				}
			}
		}
	}
	m_Display.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////
// Procedure:	ProcessWater
// Purpose:		Calculate new values for the water height field
/////////////////////////////////////////////////////////////////////////////////////
void CAguaDlg::ProcessWater() 
{
//// Local Variables ////////////////////////////////////////////////////////////////
	int i,j;
	float value;
/////////////////////////////////////////////////////////////////////////////////////
	UpdateData(TRUE);						// Get Value from Dialog

	for (j = 2; j < WATER_SIZE - 2; j++)
	{
		for (i = 2; i < WATER_SIZE - 2; i++)
		{
			// Sample a "circle" around the center point
			value = (float)(
				READBUFFER(m_ReadBuffer,i-2,j) +
				READBUFFER(m_ReadBuffer,i+2,j) +
				READBUFFER(m_ReadBuffer,i,j-2) +
				READBUFFER(m_ReadBuffer,i,j+2) +
				READBUFFER(m_ReadBuffer,i-1,j) +
				READBUFFER(m_ReadBuffer,i+1,j) +
				READBUFFER(m_ReadBuffer,i,j-1) +
				READBUFFER(m_ReadBuffer,i,j+1) +
				READBUFFER(m_ReadBuffer,i-1,j-1) +
				READBUFFER(m_ReadBuffer,i+1,j-1) +
				READBUFFER(m_ReadBuffer,i-1,j+1) +
				READBUFFER(m_ReadBuffer,i+1,j+1));
			value /= 6.0f;		// Average * 2
			value -= (float)READBUFFER(m_WriteBuffer,i,j);
			// Values for damping from 0.04 - 0.0001 look pretty good
			value -= (value * m_DampingFactor);
			SETBUFFER(m_WriteBuffer,i,j,(int)value);
		}
	}
	SetDisplay();		// Draw and Swap Buffers
}

