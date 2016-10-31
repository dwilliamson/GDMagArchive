# Microsoft Developer Studio Generated NMAKE File, Based on GlobalUtility.dsp
!IF "$(CFG)" == ""
CFG=GlobalUtility - Win32 Hybrid
!MESSAGE No configuration specified. Defaulting to GlobalUtility - Win32 Hybrid.
!ENDIF 

!IF "$(CFG)" != "GlobalUtility - Win32 Release" && "$(CFG)" != "GlobalUtility - Win32 Debug" && "$(CFG)" != "GlobalUtility - Win32 Hybrid"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GlobalUtility.mak" CFG="GlobalUtility - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GlobalUtility - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GlobalUtility - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GlobalUtility - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GlobalUtility - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\..\plugin\GlobalUtility_skeleton.gup"


CLEAN :
	-@erase "$(INTDIR)\GlobalUtility.obj"
	-@erase "$(INTDIR)\GlobalUtility.res"
	-@erase "$(INTDIR)\Plugin.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\ViewWindow.obj"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.exp"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.lib"
	-@erase "..\..\..\..\plugin\GlobalUtility_skeleton.gup"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MD /W3 /GX /O2 /I "..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /Fp"$(INTDIR)\GlobalUtility.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\GlobalUtility.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GlobalUtility.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /incremental:no /pdb:"$(OUTDIR)\GlobalUtility_skeleton.pdb" /machine:I386 /def:".\Plugin.def" /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /implib:"$(OUTDIR)\GlobalUtility_skeleton.lib" /libpath:"..\..\..\..\lib" /release 
DEF_FILE= \
	".\Plugin.def"
LINK32_OBJS= \
	"$(INTDIR)\GlobalUtility.obj" \
	"$(INTDIR)\Plugin.obj" \
	"$(INTDIR)\ViewWindow.obj" \
	"$(INTDIR)\GlobalUtility.res"

"..\..\..\..\plugin\GlobalUtility_skeleton.gup" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GlobalUtility - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\..\..\plugin\GlobalUtility_skeleton.gup"


CLEAN :
	-@erase "$(INTDIR)\GlobalUtility.obj"
	-@erase "$(INTDIR)\GlobalUtility.res"
	-@erase "$(INTDIR)\Plugin.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ViewWindow.obj"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.exp"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.lib"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.pdb"
	-@erase "..\..\..\..\plugin\GlobalUtility_skeleton.gup"
	-@erase "..\..\..\..\plugin\GlobalUtility_skeleton.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /Fp"$(INTDIR)\GlobalUtility.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\GlobalUtility.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GlobalUtility.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /incremental:yes /pdb:"$(OUTDIR)\GlobalUtility_skeleton.pdb" /debug /machine:I386 /def:".\Plugin.def" /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /implib:"$(OUTDIR)\GlobalUtility_skeleton.lib" /pdbtype:sept /libpath:"..\..\..\..\lib" 
DEF_FILE= \
	".\Plugin.def"
LINK32_OBJS= \
	"$(INTDIR)\GlobalUtility.obj" \
	"$(INTDIR)\Plugin.obj" \
	"$(INTDIR)\ViewWindow.obj" \
	"$(INTDIR)\GlobalUtility.res"

"..\..\..\..\plugin\GlobalUtility_skeleton.gup" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GlobalUtility - Win32 Hybrid"

OUTDIR=.\Hybrid
INTDIR=.\Hybrid

ALL : "..\..\..\..\plugin\GlobalUtility_skeleton.gup"


CLEAN :
	-@erase "$(INTDIR)\GlobalUtility.obj"
	-@erase "$(INTDIR)\GlobalUtility.res"
	-@erase "$(INTDIR)\Plugin.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ViewWindow.obj"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.exp"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.lib"
	-@erase "$(OUTDIR)\GlobalUtility_skeleton.pdb"
	-@erase "..\..\..\..\plugin\GlobalUtility_skeleton.gup"
	-@erase "..\..\..\..\plugin\GlobalUtility_skeleton.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MD /W3 /Gm /GX /ZI /Od /I "..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLOBALUTILITY_EXPORTS" /Fp"$(INTDIR)\GlobalUtility.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\GlobalUtility.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GlobalUtility.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=maxutil.lib geom.lib gfx.lib gup.lib core.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x08C70000" /dll /incremental:yes /pdb:"$(OUTDIR)\GlobalUtility_skeleton.pdb" /debug /machine:I386 /def:".\Plugin.def" /out:"..\..\..\..\plugin\GlobalUtility_skeleton.gup" /implib:"$(OUTDIR)\GlobalUtility_skeleton.lib" /pdbtype:sept /libpath:"..\..\..\..\lib" 
DEF_FILE= \
	".\Plugin.def"
LINK32_OBJS= \
	"$(INTDIR)\GlobalUtility.obj" \
	"$(INTDIR)\Plugin.obj" \
	"$(INTDIR)\ViewWindow.obj" \
	"$(INTDIR)\GlobalUtility.res"

"..\..\..\..\plugin\GlobalUtility_skeleton.gup" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("GlobalUtility.dep")
!INCLUDE "GlobalUtility.dep"
!ELSE 
!MESSAGE Warning: cannot find "GlobalUtility.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GlobalUtility - Win32 Release" || "$(CFG)" == "GlobalUtility - Win32 Debug" || "$(CFG)" == "GlobalUtility - Win32 Hybrid"
SOURCE=.\GlobalUtility.cpp

"$(INTDIR)\GlobalUtility.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Plugin.cpp

"$(INTDIR)\Plugin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ViewWindow.cpp

"$(INTDIR)\ViewWindow.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GlobalUtility.rc

"$(INTDIR)\GlobalUtility.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

