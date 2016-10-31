# Microsoft Developer Studio Project File - Name="soup" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=soup - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "soup.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "soup.mak" CFG="soup - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "soup - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "soup - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "soup - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /GX /Zi /O2 /I "..\app_shell" /I "..\app_shell\include" /I "..\mesh" /I "..\framework" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib jpg.lib opengl32.lib glu32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"..\app_shell\lib"

!ELSEIF  "$(CFG)" == "soup - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Gm /GX /ZI /Od /I "..\app_shell" /I "..\app_shell\include" /I "..\mesh" /I "..\framework" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 uuid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib jpg.lib opengl32.lib glu32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\app_shell\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "soup - Win32 Release"
# Name "soup - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app_shell\app_shell.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\auto_array.cpp
# End Source File
# Begin Source File

SOURCE=..\bt_loader.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\data_structures.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh\error_quadric.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\geometry.cpp
# End Source File
# Begin Source File

SOURCE=..\hash_table.cpp
# End Source File
# Begin Source File

SOURCE=..\app_shell\jpeg_load.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\lightweight_proximity_grid.cpp
# End Source File
# Begin Source File

SOURCE=..\main.cpp
# End Source File
# Begin Source File

SOURCE=..\make_world.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh\mesh_builder.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh_chopper.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh\mesh_reducer.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh\mesh_topology_handler.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\priority_queue.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\projector.cpp
# End Source File
# Begin Source File

SOURCE=..\render_state.cpp
# End Source File
# Begin Source File

SOURCE=..\framework\rendering.cpp
# End Source File
# Begin Source File

SOURCE=..\save_and_load.cpp
# End Source File
# Begin Source File

SOURCE=..\seam_database.cpp
# End Source File
# Begin Source File

SOURCE=..\mesh\tangent_frames.cpp
# End Source File
# Begin Source File

SOURCE=..\visibility.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
