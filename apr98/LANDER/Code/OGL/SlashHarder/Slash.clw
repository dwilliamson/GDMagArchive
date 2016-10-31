; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CSliderCtrl
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Slash.h"
LastPage=0

ClassCount=7
Class1=CSlashApp
Class2=CSlashDoc
Class3=CSlashView
Class4=CMainFrame

ResourceCount=3
Resource1=IDD_SETROTATE
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Class6=CSetRot
Class7=CSlider
Resource3=IDD_ABOUTBOX

[CLS:CSlashApp]
Type=0
HeaderFile=Slash.h
ImplementationFile=Slash.cpp
Filter=N

[CLS:CSlashDoc]
Type=0
HeaderFile=SlashDoc.h
ImplementationFile=SlashDoc.cpp
Filter=N

[CLS:CSlashView]
Type=0
HeaderFile=SlashView.h
ImplementationFile=SlashView.cpp
Filter=C

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=ID_CONTROL_HAND



[CLS:CAboutDlg]
Type=0
HeaderFile=Slash.cpp
ImplementationFile=Slash.cpp
Filter=D

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
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_CONTROL_UPPERARM
Command8=ID_CONTROL_LOWERARM
Command9=ID_CONTROL_HAND
Command10=ID_ENDKEY
Command11=ID_APP_ABOUT
Command12=ID_HELP_WHICHOPENGL
CommandCount=12

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
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

[DLG:IDD_SETROTATE]
Type=1
Class=CSetRot
ControlCount=8
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_XAXIS,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_YAXIS,edit,1350631552
Control7=IDC_STATIC,static,1342308352
Control8=IDC_ZAXIS,edit,1350631552

[CLS:CSetRot]
Type=0
HeaderFile=SetRot.h
ImplementationFile=SetRot.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSetRot

[CLS:CSlider]
Type=0
HeaderFile=Slider.h
ImplementationFile=Slider.cpp
BaseClass=CSliderCtrl
Filter=W
LastObject=CSlider
VirtualFilter=KWC

