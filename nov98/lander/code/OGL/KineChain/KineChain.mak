# Microsoft Developer Studio Generated NMAKE File, Based on KineChain.dsp
!IF "$(CFG)" == ""
CFG=KineChain - Win32 Debug
!MESSAGE No configuration specified. Defaulting to KineChain - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "KineChain - Win32 Release" && "$(CFG)" != "KineChain - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KineChain.mak" CFG="KineChain - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KineChain - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "KineChain - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "KineChain - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\KineChain.exe"

!ELSE 

ALL : "$(OUTDIR)\KineChain.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\KineChain.obj"
	-@erase "$(INTDIR)\KineChain.pch"
	-@erase "$(INTDIR)\KineChain.res"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\Quatern.obj"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\KineChain.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\KineChain.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\KineChain.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KineChain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)\KineChain.pdb" /machine:I386\
 /out:"$(OUTDIR)\KineChain.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\KineChain.obj" \
	"$(INTDIR)\KineChain.res" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Quatern.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\KineChain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\KineChain.exe"

!ELSE 

ALL : "$(OUTDIR)\KineChain.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\KineChain.obj"
	-@erase "$(INTDIR)\KineChain.pch"
	-@erase "$(INTDIR)\KineChain.res"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\Quatern.obj"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\KineChain.exe"
	-@erase "$(OUTDIR)\KineChain.ilk"
	-@erase "$(OUTDIR)\KineChain.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\KineChain.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\KineChain.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KineChain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)\KineChain.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\KineChain.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\KineChain.obj" \
	"$(INTDIR)\KineChain.res" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\Quatern.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\KineChain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "KineChain - Win32 Release" || "$(CFG)" == "KineChain - Win32 Debug"
SOURCE=.\Bitmap.cpp
DEP_CPP_BITMA=\
	".\Bitmap.h"\
	

"$(INTDIR)\Bitmap.obj" : $(SOURCE) $(DEP_CPP_BITMA) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


SOURCE=.\KineChain.cpp

!IF  "$(CFG)" == "KineChain - Win32 Release"

DEP_CPP_KINE3=\
	".\KineChain.h"\
	".\MainFrm.h"\
	".\MathDefs.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\KineChain.obj" : $(SOURCE) $(DEP_CPP_KINE3) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

DEP_CPP_KINE3=\
	".\KineChain.h"\
	".\MainFrm.h"\
	".\MathDefs.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\KineChain.obj" : $(SOURCE) $(DEP_CPP_KINE3) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ENDIF 

SOURCE=.\KineChain.rc
DEP_RSC_KineChain=\
	".\res\KineChain.ico"\
	".\res\KineChain.rc2"\
	

"$(INTDIR)\KineChain.res" : $(SOURCE) $(DEP_RSC_KineChain) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "KineChain - Win32 Release"

DEP_CPP_MAINF=\
	".\KineChain.h"\
	".\MainFrm.h"\
	".\MathDefs.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

DEP_CPP_MAINF=\
	".\KineChain.h"\
	".\MainFrm.h"\
	".\MathDefs.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ENDIF 

SOURCE=.\MathDefs.cpp
DEP_CPP_MATHD=\
	".\MathDefs.h"\
	

"$(INTDIR)\MathDefs.obj" : $(SOURCE) $(DEP_CPP_MATHD) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


SOURCE=.\OGLView.cpp

!IF  "$(CFG)" == "KineChain - Win32 Release"

DEP_CPP_OGLVI=\
	".\Bitmap.h"\
	".\KineChain.h"\
	".\MathDefs.h"\
	".\Model.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\OGLView.obj" : $(SOURCE) $(DEP_CPP_OGLVI) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

DEP_CPP_OGLVI=\
	".\Bitmap.h"\
	".\KineChain.h"\
	".\MathDefs.h"\
	".\Model.h"\
	".\OGLView.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\OGLView.obj" : $(SOURCE) $(DEP_CPP_OGLVI) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ENDIF 

SOURCE=.\Quatern.cpp

!IF  "$(CFG)" == "KineChain - Win32 Release"

DEP_CPP_QUATE=\
	".\MathDefs.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\Quatern.obj" : $(SOURCE) $(DEP_CPP_QUATE) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

DEP_CPP_QUATE=\
	".\MathDefs.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\Quatern.obj" : $(SOURCE) $(DEP_CPP_QUATE) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ENDIF 

SOURCE=.\Skeleton.cpp

!IF  "$(CFG)" == "KineChain - Win32 Release"

DEP_CPP_SKELE=\
	".\MathDefs.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\Skeleton.obj" : $(SOURCE) $(DEP_CPP_SKELE) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

DEP_CPP_SKELE=\
	".\MathDefs.h"\
	".\Quatern.h"\
	".\Skeleton.h"\
	

"$(INTDIR)\Skeleton.obj" : $(SOURCE) $(DEP_CPP_SKELE) "$(INTDIR)"\
 "$(INTDIR)\KineChain.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "KineChain - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\KineChain.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\KineChain.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KineChain - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)\KineChain.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\KineChain.pch" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

