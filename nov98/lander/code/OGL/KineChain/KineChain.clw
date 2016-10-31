; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "kinechain.h"
LastPage=0

ClassCount=5
Class1=CKineChainApp
Class2=CAboutDlg
Class3=CMainFrame
Class4=COGLView

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CRestrict
Resource3=IDD_SETRESTRICT

[CLS:CKineChainApp]
Type=0
BaseClass=CWinApp
HeaderFile=KineChain.h
ImplementationFile=KineChain.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=KineChain.cpp
ImplementationFile=KineChain.cpp
LastObject=CAboutDlg

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
VirtualFilter=fWC
LastObject=ID_OPTIONS_SETRESTRICTIONS

[CLS:COGLView]
Type=0
BaseClass=CWnd
HeaderFile=OGLView.h
ImplementationFile=OGLView.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=6
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308480
Control6=IDC_STATIC,static,1342308480

[DLG:IDD_SETRESTRICT]
Type=1
Class=CRestrict
ControlCount=25
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_DAMP0,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_MINROT0,edit,1350631552
Control9=IDC_MAXROT0,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_DAMP1,edit,1350631552
Control12=IDC_MINROT1,edit,1350631552
Control13=IDC_MAXROT1,edit,1350631552
Control14=IDC_STATIC,static,1342308352
Control15=IDC_DAMP2,edit,1350631552
Control16=IDC_MINROT2,edit,1350631552
Control17=IDC_MAXROT2,edit,1350631552
Control18=IDC_STATIC,static,1342308352
Control19=IDC_DAMP3,edit,1350631552
Control20=IDC_MINROT3,edit,1350631552
Control21=IDC_MAXROT3,edit,1350631552
Control22=IDC_STATIC,static,1342308352
Control23=IDC_DAMP4,edit,1350631552
Control24=IDC_MINROT4,edit,1350631552
Control25=IDC_MAXROT4,edit,1350631552

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_VIEW_GEOMETRY
Command8=ID_OPTIONS_DAMPING
Command9=ID_OPTIONS_DOF
Command10=ID_OPTIONS_SETRESTRICTIONS
Command11=ID_APP_ABOUT
Command12=ID_HELP_WHICHOPENGL
CommandCount=12

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

[CLS:CRestrict]
Type=0
HeaderFile=Restrict.h
ImplementationFile=Restrict.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_MAXROT5

