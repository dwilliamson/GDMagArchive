; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Spray.h"
LastPage=0

ClassCount=6
Class1=CSprayApp
Class2=CSprayDoc
Class3=CSprayView
Class4=CMainFrame

ResourceCount=3
Resource1=IDD_SETEMITTER
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Class6=CEditEmit
Resource3=IDD_ABOUTBOX

[CLS:CSprayApp]
Type=0
HeaderFile=Spray.h
ImplementationFile=Spray.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=CSprayApp

[CLS:CSprayDoc]
Type=0
HeaderFile=SprayDoc.h
ImplementationFile=SprayDoc.cpp
Filter=N

[CLS:CSprayView]
Type=0
HeaderFile=SprayView.h
ImplementationFile=SprayView.cpp
Filter=C
LastObject=CSprayView

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_SETTINGS_SHOWAXIS
BaseClass=CFrameWnd
VirtualFilter=fWC



[CLS:CAboutDlg]
Type=0
HeaderFile=Spray.cpp
ImplementationFile=Spray.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=9
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_APP_EXIT
Command5=ID_SETTINGS_EDITEMITTER
Command6=ID_SETTINGS_ANTIALIASPOINTS
Command7=ID_SETTINGS_SHOWAXIS
Command8=ID_APP_ABOUT
Command9=ID_HELP_WHICHOPENGL
CommandCount=9

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_SETEMITTER]
Type=1
Class=CEditEmit
ControlCount=46
Control1=IDC_NAME,edit,1350631552
Control2=IDC_YAW,edit,1350631552
Control3=IDC_YAWVAR,edit,1350631552
Control4=IDC_PITCH,edit,1350631552
Control5=IDC_PITCHVAR,edit,1350631552
Control6=IDC_SPEED,edit,1350631552
Control7=IDC_SPEEDVAR,edit,1350631552
Control8=IDC_TOTALPARTS,edit,1350631552
Control9=IDC_LIFE,edit,1350631552
Control10=IDC_LIFEVAR,edit,1350631552
Control11=IDC_EMITS,edit,1350631552
Control12=IDC_EMITVAR,edit,1350631552
Control13=IDC_SCOLORR,edit,1350631552
Control14=IDC_SCOLORG,edit,1350631552
Control15=IDC_SCOLORB,edit,1350631552
Control16=IDC_SCOLORRV,edit,1350631552
Control17=IDC_SCOLORGV,edit,1350631552
Control18=IDC_SCOLORBV,edit,1350631552
Control19=IDC_ECOLORR,edit,1350631552
Control20=IDC_ECOLORG,edit,1350631552
Control21=IDC_ECOLORB,edit,1350631552
Control22=IDC_ECOLORRV,edit,1350631552
Control23=IDC_ECOLORGV,edit,1350631552
Control24=IDC_ECOLORBV,edit,1350631552
Control25=IDC_FORCEX,edit,1350631552
Control26=IDC_FORCEY,edit,1350631552
Control27=IDC_FORCEZ,edit,1350631552
Control28=IDOK,button,1342242817
Control29=IDCANCEL,button,1342242816
Control30=IDC_STATIC,static,1342308352
Control31=IDC_STATIC,static,1342308352
Control32=IDC_STATIC,static,1342308352
Control33=IDC_STATIC,static,1342308352
Control34=IDC_STATIC,static,1342308352
Control35=IDC_STATIC,static,1342308352
Control36=IDC_STATIC,static,1342308352
Control37=IDC_STATIC,static,1342308352
Control38=IDC_STATIC,static,1342308352
Control39=IDC_STATIC,static,1342308352
Control40=IDC_STATIC,static,1342308352
Control41=IDC_STATIC,static,1342308352
Control42=IDC_STATIC,static,1342308352
Control43=IDC_STATIC,static,1342308352
Control44=IDC_STATIC,static,1342308352
Control45=IDC_STATIC,static,1342308352
Control46=IDC_STATIC,static,1342308352

[CLS:CEditEmit]
Type=0
HeaderFile=EditEmit.h
ImplementationFile=EditEmit.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_TOTALPARTS

