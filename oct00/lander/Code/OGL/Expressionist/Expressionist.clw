; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "expressionist.h"
LastPage=0

ClassCount=4
Class1=CExpressionistApp
Class2=CAboutDlg
Class3=CMainFrame
Class4=COGLView

ResourceCount=2
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME

[CLS:CExpressionistApp]
Type=0
BaseClass=CWinApp
HeaderFile=Expressionist.h
ImplementationFile=Expressionist.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=Expressionist.cpp
ImplementationFile=Expressionist.cpp
LastObject=ID_FILE_SAVEOBJ

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
VirtualFilter=fWC

[CLS:COGLView]
Type=0
BaseClass=CWnd
HeaderFile=OGLView.h
ImplementationFile=OGLView.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=11
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342177283

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_SAVEOBJ
Command3=ID_FILE_SAVETEXTURE
Command4=ID_APP_EXIT
Command5=ID_SETTINGS_USELIGHTING
Command6=ID_SETTINGS_BILINEARFILTER
Command7=ID_UVCOORDINATES_SPHERICAL
Command8=ID_UVCOORDINATES_CYLINDRICAL
Command9=ID_UVCOORDINATES_PLANAR_XAXIS
Command10=ID_UVCOORDINATES_PLANAR_YAXIS
Command11=ID_UVCOORDINATES_PLANAR_ZAXIS
Command12=ID_UVCOORDINATES
Command13=ID_PAINT_SETCOLOR
Command14=ID_APP_ABOUT
Command15=ID_WHICHOGL
CommandCount=15

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_SAVEOBJ
Command5=ID_FILE_SAVETEXTURE
Command6=ID_EDIT_PASTE
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_NEXT_PANE
Command10=ID_PREV_PANE
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_EDIT_CUT
Command14=ID_EDIT_UNDO
CommandCount=14

