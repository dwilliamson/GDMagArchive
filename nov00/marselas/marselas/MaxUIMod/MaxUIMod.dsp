# Microsoft Developer Studio Project File - Name="MaxUIMod" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MaxUIMod - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MaxUIMod.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MaxUIMod.mak" CFG="MaxUIMod - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MaxUIMod - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MaxUIMod - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MaxUIMod - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "MaxUIMod - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ    /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/MaxUIMod.gup" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\*.gup \3dsmax3\plugins	copy debug\*.gup \max3dbg\exe\plugins
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "MaxUIMod - Win32 Release"
# Name "MaxUIMod - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gup.cpp
# End Source File
# Begin Source File

SOURCE=.\MaxUIMod.cpp
# End Source File
# Begin Source File

SOURCE=.\MaxUIMod.def
# End Source File
# Begin Source File

SOURCE=.\MaxUIMod.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gup.h
# End Source File
# Begin Source File

SOURCE=.\MaxUIMod.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\MaxUIMod.rc2
# End Source File
# End Group
# Begin Group "libs"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\zlibdll.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\bmm.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\client.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\core.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\edmodel.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\expr.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\Flilibd.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\Flilibh.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\Flilibr.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\flt.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\gcomm.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\geom.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\gfx.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\gup.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\helpsys.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\Maxscrpt.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\maxutil.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\mesh.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\MNMath.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\Paramblk2.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\particle.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\tessint.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\viewfile.lib
# End Source File
# Begin Source File

SOURCE=..\3dsmax3\Maxsdk\lib\acap.lib
# End Source File
# End Group
# End Target
# End Project
