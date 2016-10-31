# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Fate - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Fate - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Fate - Win32 Release" && "$(CFG)" != "Fate - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fate.mak" CFG="Fate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fate - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fate - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "Fate - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "Fate - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Fate.exe" "$(OUTDIR)\Fate.hlp"

CLEAN : 
	-@erase "$(INTDIR)\Fate.hlp"
	-@erase "$(INTDIR)\Fate.obj"
	-@erase "$(INTDIR)\Fate.pch"
	-@erase "$(INTDIR)\Fate.res"
	-@erase "$(INTDIR)\FateDoc.obj"
	-@erase "$(INTDIR)\FateView.obj"
	-@erase "$(INTDIR)\Loadpcx.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SecAttr.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\Fate.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/Fate.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Fate.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Fate.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Fate.pdb" /machine:I386 /out:"$(OUTDIR)/Fate.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Fate.obj" \
	"$(INTDIR)\Fate.res" \
	"$(INTDIR)\FateDoc.obj" \
	"$(INTDIR)\FateView.obj" \
	"$(INTDIR)\Loadpcx.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SecAttr.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Fate.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Fate - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Fate.exe" "$(OUTDIR)\Fate.hlp"

CLEAN : 
	-@erase "$(INTDIR)\Fate.hlp"
	-@erase "$(INTDIR)\Fate.obj"
	-@erase "$(INTDIR)\Fate.pch"
	-@erase "$(INTDIR)\Fate.res"
	-@erase "$(INTDIR)\FateDoc.obj"
	-@erase "$(INTDIR)\FateView.obj"
	-@erase "$(INTDIR)\Loadpcx.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\SecAttr.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Fate.exe"
	-@erase "$(OUTDIR)\Fate.ilk"
	-@erase "$(OUTDIR)\Fate.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Fate.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/Fate.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Fate.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Fate.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Fate.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Fate.obj" \
	"$(INTDIR)\Fate.res" \
	"$(INTDIR)\FateDoc.obj" \
	"$(INTDIR)\FateView.obj" \
	"$(INTDIR)\Loadpcx.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\SecAttr.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Fate.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Fate - Win32 Release"
# Name "Fate - Win32 Debug"

!IF  "$(CFG)" == "Fate - Win32 Release"

!ELSEIF  "$(CFG)" == "Fate - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "Fate - Win32 Release"

!ELSEIF  "$(CFG)" == "Fate - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fate.cpp
DEP_CPP_FATE_=\
	".\AmpDefs.h"\
	".\engine.h"\
	".\Fate.h"\
	".\FateDoc.h"\
	".\FateView.h"\
	".\loadpcx.h"\
	".\MainFrm.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Fate.obj" : $(SOURCE) $(DEP_CPP_FATE_) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Fate - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/Fate.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Fate.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "Fate - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Fate.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Fate.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\Fate.h"\
	".\MainFrm.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\FateDoc.cpp
DEP_CPP_FATED=\
	".\Fate.h"\
	".\FateDoc.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\FateDoc.obj" : $(SOURCE) $(DEP_CPP_FATED) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\FateView.cpp
DEP_CPP_FATEV=\
	".\AmpDefs.h"\
	".\engine.h"\
	".\Fate.h"\
	".\FateDoc.h"\
	".\FateView.h"\
	".\loadpcx.h"\
	".\MainFrm.h"\
	".\SecAttr.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\FateView.obj" : $(SOURCE) $(DEP_CPP_FATEV) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Fate.rc
DEP_RSC_FATE_R=\
	".\res\Fate.ico"\
	".\res\Fate.rc2"\
	".\res\FateDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\Fate.res" : $(SOURCE) $(DEP_RSC_FATE_R) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\hlp\Fate.hpj

!IF  "$(CFG)" == "Fate - Win32 Release"

# Begin Custom Build - Making help file...
OutDir=.\Release
ProjDir=.
TargetName=Fate
InputPath=.\hlp\Fate.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   "$(ProjDir)\makehelp.bat"

# End Custom Build

!ELSEIF  "$(CFG)" == "Fate - Win32 Debug"

# Begin Custom Build - Making help file...
OutDir=.\Debug
ProjDir=.
TargetName=Fate
InputPath=.\hlp\Fate.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   "$(ProjDir)\makehelp.bat"

# End Custom Build

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SecAttr.cpp
DEP_CPP_SECAT=\
	".\engine.h"\
	".\Fate.h"\
	".\loadpcx.h"\
	".\SecAttr.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\SecAttr.obj" : $(SOURCE) $(DEP_CPP_SECAT) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Loadpcx.cpp
DEP_CPP_LOADP=\
	".\loadpcx.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Loadpcx.obj" : $(SOURCE) $(DEP_CPP_LOADP) "$(INTDIR)"\
 "$(INTDIR)\Fate.pch"


# End Source File
# End Target
# End Project
################################################################################
