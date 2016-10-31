# Microsoft Developer Studio Generated NMAKE File, Based on Clothy.dsp
!IF "$(CFG)" == ""
CFG=Clothy - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Clothy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Clothy - Win32 Release" && "$(CFG)" != "Clothy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Clothy.mak" CFG="Clothy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Clothy - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Clothy - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Clothy - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Clothy.exe" "$(OUTDIR)\Clothy.bsc"


CLEAN :
	-@erase "$(INTDIR)\AddSpher.obj"
	-@erase "$(INTDIR)\AddSpher.sbr"
	-@erase "$(INTDIR)\Clothy.obj"
	-@erase "$(INTDIR)\Clothy.pch"
	-@erase "$(INTDIR)\Clothy.res"
	-@erase "$(INTDIR)\Clothy.sbr"
	-@erase "$(INTDIR)\LoadOBJ.obj"
	-@erase "$(INTDIR)\LoadOBJ.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\MathDefs.sbr"
	-@erase "$(INTDIR)\NewCloth.obj"
	-@erase "$(INTDIR)\NewCloth.sbr"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\OGLView.sbr"
	-@erase "$(INTDIR)\PhysEnv.obj"
	-@erase "$(INTDIR)\PhysEnv.sbr"
	-@erase "$(INTDIR)\SetVert.obj"
	-@erase "$(INTDIR)\SetVert.sbr"
	-@erase "$(INTDIR)\SimProps.obj"
	-@erase "$(INTDIR)\SimProps.sbr"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Skeleton.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TimeProps.obj"
	-@erase "$(INTDIR)\TimeProps.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Clothy.bsc"
	-@erase "$(OUTDIR)\Clothy.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Clothy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Clothy.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Clothy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AddSpher.sbr" \
	"$(INTDIR)\Clothy.sbr" \
	"$(INTDIR)\LoadOBJ.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\MathDefs.sbr" \
	"$(INTDIR)\NewCloth.sbr" \
	"$(INTDIR)\OGLView.sbr" \
	"$(INTDIR)\PhysEnv.sbr" \
	"$(INTDIR)\SetVert.sbr" \
	"$(INTDIR)\SimProps.sbr" \
	"$(INTDIR)\Skeleton.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TimeProps.sbr"

"$(OUTDIR)\Clothy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Clothy.pdb" /machine:I386 /out:"$(OUTDIR)\Clothy.exe" 
LINK32_OBJS= \
	"$(INTDIR)\AddSpher.obj" \
	"$(INTDIR)\Clothy.obj" \
	"$(INTDIR)\LoadOBJ.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\NewCloth.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\PhysEnv.obj" \
	"$(INTDIR)\SetVert.obj" \
	"$(INTDIR)\SimProps.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TimeProps.obj" \
	"$(INTDIR)\Clothy.res"

"$(OUTDIR)\Clothy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Clothy - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Clothy.exe" "$(OUTDIR)\Clothy.bsc"


CLEAN :
	-@erase "$(INTDIR)\AddSpher.obj"
	-@erase "$(INTDIR)\AddSpher.sbr"
	-@erase "$(INTDIR)\Clothy.obj"
	-@erase "$(INTDIR)\Clothy.pch"
	-@erase "$(INTDIR)\Clothy.res"
	-@erase "$(INTDIR)\Clothy.sbr"
	-@erase "$(INTDIR)\LoadOBJ.obj"
	-@erase "$(INTDIR)\LoadOBJ.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\MathDefs.obj"
	-@erase "$(INTDIR)\MathDefs.sbr"
	-@erase "$(INTDIR)\NewCloth.obj"
	-@erase "$(INTDIR)\NewCloth.sbr"
	-@erase "$(INTDIR)\OGLView.obj"
	-@erase "$(INTDIR)\OGLView.sbr"
	-@erase "$(INTDIR)\PhysEnv.obj"
	-@erase "$(INTDIR)\PhysEnv.sbr"
	-@erase "$(INTDIR)\SetVert.obj"
	-@erase "$(INTDIR)\SetVert.sbr"
	-@erase "$(INTDIR)\SimProps.obj"
	-@erase "$(INTDIR)\SimProps.sbr"
	-@erase "$(INTDIR)\Skeleton.obj"
	-@erase "$(INTDIR)\Skeleton.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TimeProps.obj"
	-@erase "$(INTDIR)\TimeProps.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Clothy.bsc"
	-@erase "$(OUTDIR)\Clothy.exe"
	-@erase "$(OUTDIR)\Clothy.ilk"
	-@erase "$(OUTDIR)\Clothy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Clothy.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Clothy.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Clothy.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AddSpher.sbr" \
	"$(INTDIR)\Clothy.sbr" \
	"$(INTDIR)\LoadOBJ.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\MathDefs.sbr" \
	"$(INTDIR)\NewCloth.sbr" \
	"$(INTDIR)\OGLView.sbr" \
	"$(INTDIR)\PhysEnv.sbr" \
	"$(INTDIR)\SetVert.sbr" \
	"$(INTDIR)\SimProps.sbr" \
	"$(INTDIR)\Skeleton.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TimeProps.sbr"

"$(OUTDIR)\Clothy.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dsound.lib winmm.lib opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Clothy.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Clothy.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\AddSpher.obj" \
	"$(INTDIR)\Clothy.obj" \
	"$(INTDIR)\LoadOBJ.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\MathDefs.obj" \
	"$(INTDIR)\NewCloth.obj" \
	"$(INTDIR)\OGLView.obj" \
	"$(INTDIR)\PhysEnv.obj" \
	"$(INTDIR)\SetVert.obj" \
	"$(INTDIR)\SimProps.obj" \
	"$(INTDIR)\Skeleton.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TimeProps.obj" \
	"$(INTDIR)\Clothy.res"

"$(OUTDIR)\Clothy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Clothy.dep")
!INCLUDE "Clothy.dep"
!ELSE 
!MESSAGE Warning: cannot find "Clothy.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Clothy - Win32 Release" || "$(CFG)" == "Clothy - Win32 Debug"
SOURCE=.\AddSpher.cpp

"$(INTDIR)\AddSpher.obj"	"$(INTDIR)\AddSpher.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\Clothy.cpp

"$(INTDIR)\Clothy.obj"	"$(INTDIR)\Clothy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\Clothy.rc

"$(INTDIR)\Clothy.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\LoadOBJ.cpp

"$(INTDIR)\LoadOBJ.obj"	"$(INTDIR)\LoadOBJ.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\MainFrm.cpp

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\MathDefs.cpp

"$(INTDIR)\MathDefs.obj"	"$(INTDIR)\MathDefs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\NewCloth.cpp

"$(INTDIR)\NewCloth.obj"	"$(INTDIR)\NewCloth.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\OGLView.cpp

"$(INTDIR)\OGLView.obj"	"$(INTDIR)\OGLView.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\PhysEnv.cpp

"$(INTDIR)\PhysEnv.obj"	"$(INTDIR)\PhysEnv.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\SetVert.cpp

"$(INTDIR)\SetVert.obj"	"$(INTDIR)\SetVert.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\SimProps.cpp

"$(INTDIR)\SimProps.obj"	"$(INTDIR)\SimProps.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\Skeleton.cpp

"$(INTDIR)\Skeleton.obj"	"$(INTDIR)\Skeleton.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "Clothy - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Clothy.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Clothy.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Clothy - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Clothy.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Clothy.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\TimeProps.cpp

"$(INTDIR)\TimeProps.obj"	"$(INTDIR)\TimeProps.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Clothy.pch"



!ENDIF 

