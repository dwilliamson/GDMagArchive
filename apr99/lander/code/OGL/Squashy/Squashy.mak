# Microsoft Developer Studio Generated NMAKE File, Based on Squashy.dsp
!IF "$(CFG)" == ""
CFG=Squashy - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Squashy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Squashy - Win32 Release" && "$(CFG)" != "Squashy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Squashy.mak" CFG="Squashy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Squashy - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Squashy - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Squashy - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Squashy.exe" "$(OUTDIR)\Squashy.bsc"


CLEAN :
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\Bitmap.sbr"
	-@erase "$(INTDIR)\Squashy.obj"
	-@erase "$(INTDIR)\Squashy.pch"
	-@erase "$(INTDIR)\Squashy.res"
	-@erase "$(INTDIR)\Squashy.sbr"
	-@erase "$(INTDIR)\LoadOBJ.obj"
	-@erase "$(INTDIR)\LoadOBJ.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\MathDefs.sbr"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\OGLView.sbr"
	-@erase "$(INTDIR)\PickObj.obj"
	-@erase "$(INTDIR)\PickObj.sbr"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Skeleton.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Squashy.bsc"
	-@erase "$(OUTDIR)\Squashy.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Squashy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Squashy.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Squashy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Bitmap.sbr" \
	"$(INTDIR)\Squashy.sbr" \
	"$(INTDIR)\LoadOBJ.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\MathDefs.sbr" \
	"$(INTDIR)\OGLView.sbr" \
	"$(INTDIR)\PickObj.sbr" \
	"$(INTDIR)\Skeleton.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Squashy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Squashy.pdb" /machine:I386 /out:"$(OUTDIR)\Squashy.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\Squashy.obj" \
	"$(INTDIR)\LoadOBJ.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\PickObj.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Squashy.res"

"$(OUTDIR)\Squashy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Squashy - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Squashy.exe" "$(OUTDIR)\Squashy.bsc"


CLEAN :
	-@erase "$(INTDIR)\Bitmap.obj"
	-@erase "$(INTDIR)\Bitmap.sbr"
	-@erase "$(INTDIR)\Squashy.obj"
	-@erase "$(INTDIR)\Squashy.pch"
	-@erase "$(INTDIR)\Squashy.res"
	-@erase "$(INTDIR)\Squashy.sbr"
	-@erase "$(INTDIR)\LoadOBJ.obj"
	-@erase "$(INTDIR)\LoadOBJ.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\MathDefs.sbr"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\OGLView.sbr"
	-@erase "$(INTDIR)\PickObj.obj"
	-@erase "$(INTDIR)\PickObj.sbr"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Skeleton.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Squashy.bsc"
	-@erase "$(OUTDIR)\Squashy.exe"
	-@erase "$(OUTDIR)\Squashy.ilk"
	-@erase "$(OUTDIR)\Squashy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Squashy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Squashy.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Squashy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Bitmap.sbr" \
	"$(INTDIR)\Squashy.sbr" \
	"$(INTDIR)\LoadOBJ.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\MathDefs.sbr" \
	"$(INTDIR)\OGLView.sbr" \
	"$(INTDIR)\PickObj.sbr" \
	"$(INTDIR)\Skeleton.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\Squashy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Squashy.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Squashy.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Bitmap.obj" \
	"$(INTDIR)\Squashy.obj" \
	"$(INTDIR)\LoadOBJ.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\PickObj.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Squashy.res"

"$(OUTDIR)\Squashy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Squashy.dep")
!INCLUDE "Squashy.dep"
!ELSE 
!MESSAGE Warning: cannot find "Squashy.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Squashy - Win32 Release" || "$(CFG)" == "Squashy - Win32 Debug"
SOURCE=.\Bitmap.cpp

"$(INTDIR)\Bitmap.obj"	"$(INTDIR)\Bitmap.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\Squashy.cpp

"$(INTDIR)\Squashy.obj"	"$(INTDIR)\Squashy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\Squashy.rc

"$(INTDIR)\Squashy.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\LoadOBJ.cpp

"$(INTDIR)\LoadOBJ.obj"	"$(INTDIR)\LoadOBJ.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\MainFrm.cpp

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\MathDefs.cpp

"$(INTDIR)\MathDefs.obj"	"$(INTDIR)\MathDefs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\OGLView.cpp

"$(INTDIR)\OGLView.obj"	"$(INTDIR)\OGLView.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\PickObj.cpp

"$(INTDIR)\PickObj.obj"	"$(INTDIR)\PickObj.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\Skeleton.cpp

"$(INTDIR)\Skeleton.obj"	"$(INTDIR)\Skeleton.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Squashy.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "Squashy - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Squashy.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Squashy.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Squashy - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Squashy.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Squashy.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

