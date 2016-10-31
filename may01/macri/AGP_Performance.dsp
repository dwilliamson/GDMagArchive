# Microsoft Developer Studio Project File - Name="AGP_Performance" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AGP_Performance - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AGP_Performance.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AGP_Performance.mak" CFG="AGP_Performance - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AGP_Performance - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AGP_Performance - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AGP_Performance - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /map /debug /machine:I386

!ELSEIF  "$(CFG)" == "AGP_Performance - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AGP_Performance - Win32 Release"
# Name "AGP_Performance - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AGP_Performance.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Dx8Shell.rc
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawBmpLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\IawD3dWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawFileStream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawImageLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FrameWork\IawMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawObject.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Shaders\IawShader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Shaders\IawShaderMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawSphere.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawTextMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FrameWork\IawTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\IawWindow3d.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stdAfx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\AGP_Performance.h
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawBmpLoader.h
# End Source File
# Begin Source File

SOURCE=.\src\IawD3dWrapper.h
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawDataStream.h
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawFileStream.h
# End Source File
# Begin Source File

SOURCE=.\src\IawFrameWork.h
# End Source File
# Begin Source File

SOURCE=.\src\DataIO\IawImageLoader.h
# End Source File
# Begin Source File

SOURCE=.\src\FrameWork\IawMatrix.h
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawObject.h
# End Source File
# Begin Source File

SOURCE=.\src\Shaders\IawShader.h
# End Source File
# Begin Source File

SOURCE=.\src\Shaders\IawShaderMgr.h
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawSphere.h
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawSuperVertex.h
# End Source File
# Begin Source File

SOURCE=.\src\Objects\IawTextMgr.h
# End Source File
# Begin Source File

SOURCE=.\src\FrameWork\IawTexture.h
# End Source File
# Begin Source File

SOURCE=.\src\IawWindow3d.h
# End Source File
# Begin Source File

SOURCE=.\src\Utils\MissingDX7Stuff.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\stdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resources\icon1.ico
# End Source File
# End Group
# End Target
# End Project
