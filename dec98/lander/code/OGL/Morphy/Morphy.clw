; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CLoadDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "morphy.h"
LastPage=0

ClassCount=6
Class1=CMainFrame
Class2=CMorphyApp
Class3=CAboutDlg
Class4=COGLView
Class5=CSlider

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class6=CLoadDlg
Resource3=IDD_LOADOBJ

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
LastObject=CMainFrame
Filter=T
VirtualFilter=fWC

[CLS:CMorphyApp]
Type=0
BaseClass=CWinApp
HeaderFile=Morphy.h
ImplementationFile=Morphy.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=Morphy.cpp
ImplementationFile=Morphy.cpp
LastObject=CAboutDlg

[CLS:COGLView]
Type=0
BaseClass=CWnd
HeaderFile=OGLView.h
ImplementationFile=OGLView.cpp

[CLS:CSlider]
Type=0
BaseClass=CSliderCtrl
HeaderFile=Slider.h
ImplementationFile=Slider.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=7
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_APP_EXIT
Command3=ID_VIEW_GEOMETRY
Command4=ID_APP_ABOUT
Command5=ID_HELP_WHICHOPENGL
CommandCount=5

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_LOADOBJ]
Type=1
Class=CLoadDlg
ControlCount=9
Control1=IDC_EDIT1,edit,1350631552
Control2=IDC_BROWSE1,button,1342242816
Control3=IDC_EDIT2,edit,1350631552
Control4=IDC_BROWSE2,button,1342242816
Control5=IDOK,button,1342242817
Control6=IDCANCEL,button,1342242816
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352

[CLS:CLoadDlg]
Type=0
HeaderFile=loaddlg.h
ImplementationFile=loaddlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CLoadDlg

