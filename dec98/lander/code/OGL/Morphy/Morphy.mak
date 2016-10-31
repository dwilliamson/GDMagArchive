# Microsoft Developer Studio Generated NMAKE File, Based on Morphy.dsp
!IF "$(CFG)" == ""
CFG=Morphy - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Morphy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Morphy - Win32 Release" && "$(CFG)" != "Morphy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Morphy.mak" CFG="Morphy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Morphy - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Morphy - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Morphy - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SpotMast.exe"

!ELSE 

ALL : "$(OUTDIR)\SpotMast.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\Morphy.obj"
	-@erase "$(INTDIR)\Morphy.pch"
	-@erase "$(INTDIR)\Morphy.res"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\SpotMast.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Morphy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Morphy.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Morphy.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SpotMast.pdb" /machine:I386\
 /out:"$(OUTDIR)\SpotMast.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Morphy.obj" \
	"$(INTDIR)\Morphy.res" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\SpotMast.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SpotMast.exe" "$(OUTDIR)\Morphy.bsc"

!ELSE 

ALL : "$(OUTDIR)\SpotMast.exe" "$(OUTDIR)\Morphy.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\OGLView.sbr"
	-@erase "$(INTDIR)\Morphy.obj"
	-@erase "$(INTDIR)\Morphy.pch"
	-@erase "$(INTDIR)\Morphy.res"
	-@erase "$(INTDIR)\Morphy.sbr"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Skeleton.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\Morphy.bsc"
	-@erase "$(OUTDIR)\SpotMast.exe"
	-@erase "$(OUTDIR)\SpotMast.ilk"
	-@erase "$(OUTDIR)\SpotMast.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Morphy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Morphy.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Morphy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\OGLView.sbr" \
	"$(INTDIR)\Morphy.sbr" \
	"$(INTDIR)\Skeleton.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Morphy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SpotMast.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\SpotMast.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Morphy.obj" \
	"$(INTDIR)\Morphy.res" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\SpotMast.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "Morphy - Win32 Release" || "$(CFG)" == "Morphy - Win32 Debug"
SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "Morphy - Win32 Release"

DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\mathdefs.h"\
	".\OGLView.h"\
	".\Morphy.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Morphy.pch"


!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"

DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\mathdefs.h"\
	".\OGLView.h"\
	".\Morphy.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF)\
 "$(INTDIR)" "$(INTDIR)\Morphy.pch"


!ENDIF 

SOURCE=.\OGLView.cpp

!IF  "$(CFG)" == "Morphy - Win32 Release"

DEP_CPP_OGLVI=\
	".\mathdefs.h"\
	".\OGLView.h"\
	".\Morphy.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

"$(INTDIR)\OGLView.obj" : $(SOURCE) $(DEP_CPP_OGLVI) "$(INTDIR)"\
 "$(INTDIR)\Morphy.pch"


!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"

DEP_CPP_OGLVI=\
	".\mathdefs.h"\
	".\OGLView.h"\
	".\Morphy.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\OGLView.obj"	"$(INTDIR)\OGLView.sbr" : $(SOURCE) $(DEP_CPP_OGLVI)\
 "$(INTDIR)" "$(INTDIR)\Morphy.pch"


!ENDIF 

SOURCE=.\Morphy.cpp
DEP_CPP_ROSCO=\
	".\MainFrm.h"\
	".\mathdefs.h"\
	".\OGLView.h"\
	".\Morphy.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

!IF  "$(CFG)" == "Morphy - Win32 Release"


"$(INTDIR)\Morphy.obj" : $(SOURCE) $(DEP_CPP_ROSCO) "$(INTDIR)"\
 "$(INTDIR)\Morphy.pch"


!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"


"$(INTDIR)\Morphy.obj"	"$(INTDIR)\Morphy.sbr" : $(SOURCE) $(DEP_CPP_ROSCO)\
 "$(INTDIR)" "$(INTDIR)\Morphy.pch"


!ENDIF 

SOURCE=.\Morphy.rc
DEP_RSC_Morphy=\
	".\res\Morphy.rc2"\
	".\res\SpotMaster.ico"\
	

"$(INTDIR)\Morphy.res" : $(SOURCE) $(DEP_RSC_Morphy) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\Skeleton.cpp

!IF  "$(CFG)" == "Morphy - Win32 Release"

DEP_CPP_SKELE=\
	".\mathdefs.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Skeleton.obj" : $(SOURCE) $(DEP_CPP_SKELE) "$(INTDIR)"\
 "$(INTDIR)\Morphy.pch"


!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"

DEP_CPP_SKELE=\
	".\mathdefs.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\Skeleton.obj"	"$(INTDIR)\Skeleton.sbr" : $(SOURCE) $(DEP_CPP_SKELE)\
 "$(INTDIR)" "$(INTDIR)\Morphy.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Morphy - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Morphy.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Morphy.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Morphy - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Morphy.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Morphy.pch" : \
$(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

