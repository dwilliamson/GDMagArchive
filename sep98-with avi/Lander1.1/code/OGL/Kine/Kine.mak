# Microsoft Developer Studio Generated NMAKE File, Based on Kine.dsp
!IF "$(CFG)" == ""
CFG=Kine - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Kine - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Kine - Win32 Release" && "$(CFG)" != "Kine - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Kine.mak" CFG="Kine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Kine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Kine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Kine - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Kine.exe"

!ELSE 

ALL : "$(OUTDIR)\Kine.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\Quatern.obj"
	-@erase "$(INTDIR)\SetRot.obj"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Kine.obj"
	-@erase "$(INTDIR)\Kine.pch"
	-@erase "$(INTDIR)\Kine.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\Kine.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Kine.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c\
 
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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Kine.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Kine.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)\Kine.pdb" /machine:I386\
 /out:"$(OUTDIR)\Kine.exe" 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Quatern.obj" \
	"$(INTDIR)\SetRot.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\Kine.obj" \
	"$(INTDIR)\Kine.res" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Kine.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Kine - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\Kine.exe"

!ELSE 

ALL : "$(OUTDIR)\Kine.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\Quatern.obj"
	-@erase "$(INTDIR)\SetRot.obj"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Kine.obj"
	-@erase "$(INTDIR)\Kine.pch"
	-@erase "$(INTDIR)\Kine.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\Kine.exe"
	-@erase "$(OUTDIR)\Kine.ilk"
	-@erase "$(OUTDIR)\Kine.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Kine.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c\
 
CPP_OBJS=.\Debug/
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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Kine.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Kine.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)\Kine.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\Kine.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Quatern.obj" \
	"$(INTDIR)\SetRot.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\Kine.obj" \
	"$(INTDIR)\Kine.res" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\Kine.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "Kine - Win32 Release" || "$(CFG)" == "Kine - Win32 Debug"
SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	".\Kine.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\OGLView.cpp
DEP_CPP_OGLVI=\
	".\Model.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\SetRot.h"\
	".\Skeleton.h"\
	".\Kine.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

"$(INTDIR)\OGLView.obj" : $(SOURCE) $(DEP_CPP_OGLVI) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\Quatern.cpp
DEP_CPP_QUATE=\
	".\Quatern.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Quatern.obj" : $(SOURCE) $(DEP_CPP_QUATE) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\SetRot.cpp
DEP_CPP_SETRO=\
	".\SetRot.h"\
	".\Kine.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\SetRot.obj" : $(SOURCE) $(DEP_CPP_SETRO) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\Skeleton.cpp
DEP_CPP_SKELE=\
	".\Quatern.h"\
	".\Skeleton.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Skeleton.obj" : $(SOURCE) $(DEP_CPP_SKELE) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\Kine.cpp
DEP_CPP_Kine=\
	".\MainFrm.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	".\Kine.h"\
	".\StdAfx.h"\
	{$(INCLUDE)}"GL\gl.h"\
	{$(INCLUDE)}"GL\glu.h"\
	

"$(INTDIR)\Kine.obj" : $(SOURCE) $(DEP_CPP_Kine) "$(INTDIR)"\
 "$(INTDIR)\Kine.pch"


SOURCE=.\Kine.rc
DEP_RSC_Kine_=\
	".\res\Kine.ico"\
	".\res\Kine.rc2"\
	".\res\KineDoc.ico"\
	

"$(INTDIR)\Kine.res" : $(SOURCE) $(DEP_RSC_Kine_) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "Kine - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\Kine.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c\
 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Kine.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Kine - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)\Kine.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Kine.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

