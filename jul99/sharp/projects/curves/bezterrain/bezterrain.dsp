# Microsoft Developer Studio Project File - Name="bezterrain" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bezterrain - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bezterrain.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bezterrain.mak" CFG="bezterrain - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bezterrain - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "bezterrain - Win32 Debug" (based on\
 "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bezterrain - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 math.lib opengl32.lib glu32.lib glut32.lib winmm.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "bezterrain - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib glut32.lib math.lib winmm.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bezterrain - Win32 Release"
# Name "bezterrain - Win32 Debug"
# Begin Source File

SOURCE=..\bezier\BezierBasis.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierBasis.h
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierPatch.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierPatch.h
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierPatchTessellator.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierPatchTessellator.h
# End Source File
# Begin Source File

SOURCE=..\bezier\BezierTypedefs.h
# End Source File
# Begin Source File

SOURCE=.\bezterrain.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\CameraMover.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\CameraMover.h
# End Source File
# Begin Source File

SOURCE=..\bezier\CentralPatchTessellator.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\CentralPatchTessellator.h
# End Source File
# Begin Source File

SOURCE=..\bezier\ClipVolume.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\ClipVolume.h
# End Source File
# Begin Source File

SOURCE=..\CurvePoint.h
# End Source File
# Begin Source File

SOURCE=..\GlobalCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\GlobalCamera.h
# End Source File
# Begin Source File

SOURCE=..\main.cpp
# End Source File
# Begin Source File

SOURCE=..\main.h
# End Source File
# Begin Source File

SOURCE=..\bezier\Mountaineer.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\Mountaineer.h
# End Source File
# Begin Source File

SOURCE=..\OpenGL.cpp
# End Source File
# Begin Source File

SOURCE=..\OpenGL.h
# End Source File
# Begin Source File

SOURCE=..\bezier\PatchBox.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\PatchBox.h
# End Source File
# Begin Source File

SOURCE=..\Polynomial.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\Polynomial.h
# End Source File
# Begin Source File

SOURCE=..\bezier\TerrainQuadtree.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\TerrainQuadtree.h
# End Source File
# Begin Source File

SOURCE=..\bezier\TerrainTreeNode.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\TerrainTreeNode.h
# End Source File
# Begin Source File

SOURCE=..\bezier\UniformPatchTessellator.cpp
# End Source File
# Begin Source File

SOURCE=..\bezier\UniformPatchTessellator.h
# End Source File
# End Target
# End Project
