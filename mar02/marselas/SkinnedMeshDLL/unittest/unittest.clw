; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CUnittestDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "unittest.h"

ClassCount=4
Class1=CUnittestApp
Class2=CUnittestDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_UNITTEST_DIALOG

[CLS:CUnittestApp]
Type=0
HeaderFile=unittest.h
ImplementationFile=unittest.cpp
Filter=N

[CLS:CUnittestDlg]
Type=0
HeaderFile=unittestDlg.h
ImplementationFile=unittestDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=unittestDlg.h
ImplementationFile=unittestDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_UNITTEST_DIALOG]
Type=1
Class=CUnittestDlg
ControlCount=3
Control1=IDCANCEL,button,1342242816
Control2=IDC_BUTTON_STARTGAME,button,1342242816
Control3=IDC_BUTTON_STOPGAME,button,1342242816

