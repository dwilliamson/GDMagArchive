# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Pool - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Pool - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Pool - Win32 Release" && "$(CFG)" != "Pool - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Pool.mak" CFG="Pool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Pool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Pool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "Pool - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Pool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Pool.exe"

CLEAN : 
	-@erase ".\Release\vc40.pdb"
	-@erase ".\Release\Pool.exe"
	-@erase ".\Release\glutils.obj"
	-@erase ".\Release\initworld.obj"
	-@erase ".\Release\CenterWnd.obj"
	-@erase ".\Release\Pool.obj"
	-@erase ".\Release\CompassWnd.obj"
	-@erase ".\Release\ViewWnd.obj"
	-@erase ".\Release\RadarWnd.obj"
	-@erase ".\Release\RESOURCE.res"
	-@erase ".\Release\RenderWorld.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/Pool.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/RESOURCE.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pool.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib  glaux.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib opengl32.lib glu32.lib  glaux.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/Pool.pdb" /machine:I386\
 /out:"$(OUTDIR)/Pool.exe" 
LINK32_OBJS= \
	".\Release\glutils.obj" \
	".\Release\initworld.obj" \
	".\Release\CenterWnd.obj" \
	".\Release\Pool.obj" \
	".\Release\CompassWnd.obj" \
	".\Release\ViewWnd.obj" \
	".\Release\RadarWnd.obj" \
	".\Release\RenderWorld.obj" \
	".\Release\RESOURCE.res"

"$(OUTDIR)\Pool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Pool.exe" "$(OUTDIR)\Pool.bsc"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\Pool.bsc"
	-@erase ".\Debug\CompassWnd.sbr"
	-@erase ".\Debug\ViewWnd.sbr"
	-@erase ".\Debug\glutils.sbr"
	-@erase ".\Debug\RadarWnd.sbr"
	-@erase ".\Debug\Pool.sbr"
	-@erase ".\Debug\initworld.sbr"
	-@erase ".\Debug\CenterWnd.sbr"
	-@erase ".\Debug\RenderWorld.sbr"
	-@erase ".\Debug\Pool.exe"
	-@erase ".\Debug\CompassWnd.obj"
	-@erase ".\Debug\ViewWnd.obj"
	-@erase ".\Debug\glutils.obj"
	-@erase ".\Debug\RadarWnd.obj"
	-@erase ".\Debug\Pool.obj"
	-@erase ".\Debug\initworld.obj"
	-@erase ".\Debug\CenterWnd.obj"
	-@erase ".\Debug\RESOURCE.res"
	-@erase ".\Debug\RenderWorld.obj"
	-@erase ".\Debug\Pool.pdb"
	-@erase ".\Debug\Pool.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fr"$(INTDIR)/" /Fp"$(INTDIR)/Pool.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/RESOURCE.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Pool.bsc" 
BSC32_SBRS= \
	".\Debug\CompassWnd.sbr" \
	".\Debug\ViewWnd.sbr" \
	".\Debug\glutils.sbr" \
	".\Debug\RadarWnd.sbr" \
	".\Debug\Pool.sbr" \
	".\Debug\initworld.sbr" \
	".\Debug\CenterWnd.sbr" \
	".\Debug\RenderWorld.sbr"

"$(OUTDIR)\Pool.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib  glaux.lib /nologo /subsystem:windows /incremental:no /map /debug /debugtype:both /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib opengl32.lib glu32.lib  glaux.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/Pool.pdb" /map:"$(INTDIR)/Pool.map" /debug\
 /debugtype:both /machine:I386 /out:"$(OUTDIR)/Pool.exe" 
LINK32_OBJS= \
	".\Debug\CompassWnd.obj" \
	".\Debug\ViewWnd.obj" \
	".\Debug\glutils.obj" \
	".\Debug\RadarWnd.obj" \
	".\Debug\Pool.obj" \
	".\Debug\initworld.obj" \
	".\Debug\CenterWnd.obj" \
	".\Debug\RenderWorld.obj" \
	".\Debug\RESOURCE.res"

"$(OUTDIR)\Pool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "Pool - Win32 Release"
# Name "Pool - Win32 Debug"

!IF  "$(CFG)" == "Pool - Win32 Release"

!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Pool.c
DEP_CPP_Pool_=\
	".\glutils.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\Pool.obj" : $(SOURCE) $(DEP_CPP_Pool_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\Pool.obj" : $(SOURCE) $(DEP_CPP_Pool_) "$(INTDIR)"

"$(INTDIR)\Pool.sbr" : $(SOURCE) $(DEP_CPP_Pool_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RESOURCE.RC

"$(INTDIR)\RESOURCE.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\glutils.c
DEP_CPP_GLUTI=\
	".\glutils.h"\
	".\externs.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\glutils.obj" : $(SOURCE) $(DEP_CPP_GLUTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\glutils.obj" : $(SOURCE) $(DEP_CPP_GLUTI) "$(INTDIR)"

"$(INTDIR)\glutils.sbr" : $(SOURCE) $(DEP_CPP_GLUTI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CenterWnd.c
DEP_CPP_CENTE=\
	".\externs.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\CenterWnd.obj" : $(SOURCE) $(DEP_CPP_CENTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\CenterWnd.obj" : $(SOURCE) $(DEP_CPP_CENTE) "$(INTDIR)"

"$(INTDIR)\CenterWnd.sbr" : $(SOURCE) $(DEP_CPP_CENTE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ViewWnd.c
DEP_CPP_VIEWW=\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	{$(INCLUDE)}"\gl\glaux.h"\
	".\externs.h"\
	".\glutils.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\ViewWnd.obj" : $(SOURCE) $(DEP_CPP_VIEWW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\ViewWnd.obj" : $(SOURCE) $(DEP_CPP_VIEWW) "$(INTDIR)"

"$(INTDIR)\ViewWnd.sbr" : $(SOURCE) $(DEP_CPP_VIEWW) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RadarWnd.c
DEP_CPP_RADAR=\
	".\glutils.h"\
	".\externs.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\RadarWnd.obj" : $(SOURCE) $(DEP_CPP_RADAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\RadarWnd.obj" : $(SOURCE) $(DEP_CPP_RADAR) "$(INTDIR)"

"$(INTDIR)\RadarWnd.sbr" : $(SOURCE) $(DEP_CPP_RADAR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CompassWnd.c
DEP_CPP_COMPA=\
	".\externs.h"\
	".\glutils.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\CompassWnd.obj" : $(SOURCE) $(DEP_CPP_COMPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\CompassWnd.obj" : $(SOURCE) $(DEP_CPP_COMPA) "$(INTDIR)"

"$(INTDIR)\CompassWnd.sbr" : $(SOURCE) $(DEP_CPP_COMPA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\initworld.c
DEP_CPP_INITW=\
	".\glutils.h"\
	".\externs.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\initworld.obj" : $(SOURCE) $(DEP_CPP_INITW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\initworld.obj" : $(SOURCE) $(DEP_CPP_INITW) "$(INTDIR)"

"$(INTDIR)\initworld.sbr" : $(SOURCE) $(DEP_CPP_INITW) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\RenderWorld.c
DEP_CPP_RENDE=\
	{$(INCLUDE)}"\gl\GL.H"\
	{$(INCLUDE)}"\gl\GLU.H"\
	{$(INCLUDE)}"\gl\glaux.h"\
	".\externs.h"\
	".\glutils.h"\
	

!IF  "$(CFG)" == "Pool - Win32 Release"


"$(INTDIR)\RenderWorld.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Pool - Win32 Debug"


"$(INTDIR)\RenderWorld.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"

"$(INTDIR)\RenderWorld.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
