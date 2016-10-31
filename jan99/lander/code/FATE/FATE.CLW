; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Fate.h"
LastPage=0

ClassCount=6
Class1=CFateApp
Class2=CFateDoc
Class3=CFateView
Class4=CMainFrame

ResourceCount=2
Resource1=IDR_MAINFRAME
Class5=CAboutDlg
Class6=CSecAttr
Resource2=IDD_ABOUTBOX

[CLS:CFateApp]
Type=0
HeaderFile=Fate.h
ImplementationFile=Fate.cpp
Filter=N

[CLS:CFateDoc]
Type=0
HeaderFile=FateDoc.h
ImplementationFile=FateDoc.cpp
Filter=N

[CLS:CFateView]
Type=0
HeaderFile=FateView.h
ImplementationFile=FateView.cpp
Filter=C
LastObject=CFateView
BaseClass=CView
VirtualFilter=VWC

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_FILE_SAVE
BaseClass=CFrameWnd
VirtualFilter=fWC



[CLS:CAboutDlg]
Type=0
HeaderFile=Fate.cpp
ImplementationFile=Fate.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=6
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_APP_EXIT
Command5=ID_OPTIONS_GRIDUP
Command6=ID_OPTIONS_GRID_DOWN
Command7=ID_OPTIONS_ZOOMIN
Command8=ID_OPTIONS_ZOOMOUT
Command9=ID_APP_ABOUT
CommandCount=9

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
Command14=ID_CONTEXT_HELP
Command15=ID_HELP
CommandCount=15

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_CONTEXT_HELP
CommandCount=9

[CLS:CSecAttr]
Type=0
HeaderFile=SecAttr.h
ImplementationFile=SecAttr.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSecAttr

