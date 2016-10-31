; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSimProps
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "friction.h"
LastPage=0

ClassCount=7
Class1=CFrictionApp
Class2=CAboutDlg
Class3=CMainFrame
Class4=COGLView
Class5=CSimProps
Class6=CTimeProps
Class7=CVertMass

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_SIMPROP
Resource4=IDD_VERTEXMASS
Resource5=IDD_SIMTIMING

[CLS:CFrictionApp]
Type=0
BaseClass=CWinApp
HeaderFile=Friction.h
ImplementationFile=Friction.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=Friction.cpp
ImplementationFile=Friction.cpp

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp

[CLS:COGLView]
Type=0
BaseClass=CWnd
HeaderFile=OGLView.h
ImplementationFile=OGLView.cpp

[CLS:CSimProps]
Type=0
BaseClass=CDialog
HeaderFile=SimProps.h
ImplementationFile=SimProps.cpp
Filter=D
VirtualFilter=dWC
LastObject=CSimProps

[CLS:CTimeProps]
Type=0
BaseClass=CDialog
HeaderFile=TimeProps.h
ImplementationFile=TimeProps.cpp

[CLS:CVertMass]
Type=0
BaseClass=CDialog
HeaderFile=VertMass.h
ImplementationFile=VertMass.cpp

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=8
Control1=IDC_STATIC,static,1342308480
Control2=IDC_STATIC,static,1342308352
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308353

[DLG:IDD_SIMPROP]
Type=1
Class=CSimProps
ControlCount=22
Control1=IDC_GRAVX,edit,1350631552
Control2=IDC_GRAVY,edit,1350631552
Control3=IDC_GRAVZ,edit,1350631552
Control4=IDC_COEFREST,edit,1350631552
Control5=IDC_Damping,edit,1350631552
Control6=IDC_SPRINGCONST,edit,1350631552
Control7=IDC_SPRINGDAMP,edit,1350631552
Control8=IDC_USERFORCEMAG,edit,1350631552
Control9=IDC_MOUSESPRING,edit,1350631552
Control10=IDC_KINETICFRICTION,edit,1350631552
Control11=IDC_STATICFRICTION,edit,1350631552
Control12=IDOK,button,1342242817
Control13=IDCANCEL,button,1342242816
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_STATIC,static,1342308352

[DLG:IDD_SIMTIMING]
Type=1
Class=CTimeProps
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_FIXEDTIME,button,1342242819
Control4=IDC_STATIC,static,1342308352
Control5=IDC_MAXTIMESTEP,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_ITERATIONS,edit,1350631552

[DLG:IDD_VERTEXMASS]
Type=1
Class=CVertMass
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308353
Control4=IDC_STATIC,static,1342308352
Control5=IDC_VERTEXMASS,edit,1350631552

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_SAVE
Command3=ID_FILE_NEWSYSTEM
Command4=ID_APP_EXIT
Command5=ID_VIEW_SHOWSPRINGS
Command6=ID_VIEW_SHOWGEOMETRY
Command7=ID_VIEW_SHOWVERTICES
Command8=ID_SIMULATION_RUNNING
Command9=ID_SIMULATION_RESET
Command10=ID_SIMULATION_SETSIMPROPERTIES
Command11=ID_SIMULATION_SETTIMINGPROPERTIES
Command12=ID_SIMULATION_SETVERTEXMASS
Command13=ID_SIMULATION_USEGRAVITY
Command14=ID_SIMULATION_USEFRICTION
Command15=ID_INTEGRATOR_EULER
Command16=ID_INTEGRATOR_MIDPOINT
Command17=ID_INTEGRATOR_RUNGEKUTTA4
Command18=ID_APP_ABOUT
Command19=ID_HELP_WHICHOPENGL
CommandCount=19

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_SAVE
CommandCount=2

