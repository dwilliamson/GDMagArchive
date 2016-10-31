# Microsoft Developer Studio Project File - Name="GlobalUtility" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GlobalUtility - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GlobalUtility.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GlobalUtility.mak" CFG="GlobalUtility - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GlobalUtility - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GlobalUtility - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GlobalUtility - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GlobalUtility", ATWAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GlobalUtility - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /D WINVER=0x0500 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 mesh.lib maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /machine:I386 /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /libpath:"..\..\..\..\lib" /release
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GlobalUtility - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /D WINVER=0x0500 /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mesh.lib maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /debug /machine:I386 /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /pdbtype:sept /libpath:"..\..\..\..\lib"

!ELSEIF  "$(CFG)" == "GlobalUtility - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GlobalUtility___Win32_Hybrid"
# PROP BASE Intermediate_Dir "GlobalUtility___Win32_Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MD /W3 /Gm /GX /ZI /Od /I "c:\3dsmax42\maxsdk\include" /I "C:\3dsmax42\Cstudio\Sdk" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /D WINVER=0x0500 /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /debug /machine:I386 /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /pdbtype:sept /libpath:"..\..\..\..\lib"
# ADD LINK32 D3dxof.lib winmm.lib d3dx8.lib d3d8.lib dxguid.lib mesh.lib maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /debug /machine:I386 /out:"hybrid\d3dviewport.gup" /pdbtype:sept /libpath:"c:\3dsmax42\maxsdk\lib" /libpath:"C:\3dsmax42\Cstudio\Sdk"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy hybrid\*.gup \3dsmax42\plugins	copy hybrid\*.gup \3dsmax42debug\exe\plugins
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "GlobalUtility - Win32 Release"
# Name "GlobalUtility - Win32 Debug"
# Name "GlobalUtility - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\d3dapp.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\d3dfont.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\d3dutil.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\dxutil.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\exportxfile.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\GlobalUtility.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\mdraw.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\meshdata.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\mload.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\pch.cpp
# ADD CPP /Yc"pch.h"
# End Source File
# Begin Source File

SOURCE=.\Plugin.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\Plugin.def
# End Source File
# Begin Source File

SOURCE=.\skinnedmesh.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\ViewWindow.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# Begin Source File

SOURCE=.\xskinexp.cpp
# ADD CPP /Yu"pch.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\d3dapp.h
# End Source File
# Begin Source File

SOURCE=.\d3dfont.h
# End Source File
# Begin Source File

SOURCE=.\D3DRES.H
# End Source File
# Begin Source File

SOURCE=.\d3dutil.h
# End Source File
# Begin Source File

SOURCE=.\dxutil.h
# End Source File
# Begin Source File

SOURCE=.\GlobalUtility.h
# End Source File
# Begin Source File

SOURCE=.\meshdata.h
# End Source File
# Begin Source File

SOURCE=.\pch.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\skinnedmesh.h
# End Source File
# Begin Source File

SOURCE=.\xskinexp.h
# End Source File
# Begin Source File

SOURCE=.\xskinexptemplates.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\GlobalUtility.rc
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\skinmesh1.vsh
# End Source File
# Begin Source File

SOURCE=.\skinmesh2.vsh
# End Source File
# Begin Source File

SOURCE=.\skinmesh3.vsh
# End Source File
# Begin Source File

SOURCE=.\skinmesh4.vsh
# End Source File
# End Target
# End Project
