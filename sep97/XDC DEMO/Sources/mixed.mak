# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=mixed - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to mixed - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mixed - Win32 Release" && "$(CFG)" != "mixed - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "mixed.mak" CFG="mixed - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mixed - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mixed - Win32 Debug" (based on "Win32 (x86) Application")
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
RSC=rc.exe
CPP=xicl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "mixed - Win32 Release"

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

ALL : "$(OUTDIR)\mixed.exe"

CLEAN : 
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dcalls.obj"
	-@erase "$(INTDIR)\d3dmain.obj"
	-@erase "$(INTDIR)\d3dmain.res"
	-@erase "$(INTDIR)\d3dmath.obj"
	-@erase "$(INTDIR)\d3dsphr.obj"
	-@erase "$(INTDIR)\ddcalls.obj"
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\geometry.obj"
	-@erase "$(INTDIR)\lclib.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\procedural.obj"
	-@erase "$(INTDIR)\render.obj"
	-@erase "$(INTDIR)\sphere.obj"
	-@erase "$(INTDIR)\spline.obj"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\tunnel.obj"
	-@erase "$(OUTDIR)\mixed.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "c:\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "c:\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /Fp"$(INTDIR)/mixed.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/d3dmain.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/mixed.bsc" 
BSC32_SBRS= \
	
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib c:\dxsdk\sdk\lib\ddraw.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib c:\dxsdk\sdk\lib\ddraw.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/mixed.pdb" /machine:I386\
 /out:"$(OUTDIR)/mixed.exe" 
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dcalls.obj" \
	"$(INTDIR)\d3dmain.obj" \
	"$(INTDIR)\d3dmain.res" \
	"$(INTDIR)\d3dmath.obj" \
	"$(INTDIR)\d3dsphr.obj" \
	"$(INTDIR)\ddcalls.obj" \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\geometry.obj" \
	"$(INTDIR)\lclib.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\procedural.obj" \
	"$(INTDIR)\render.obj" \
	"$(INTDIR)\sphere.obj" \
	"$(INTDIR)\spline.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\tunnel.obj" \
	".\Mmxoctave.obj" \
	".\Mmxtexture.obj"

"$(OUTDIR)\mixed.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mixed - Win32 Debug"

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

ALL : "$(OUTDIR)\mixed.exe"

CLEAN : 
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dcalls.obj"
	-@erase "$(INTDIR)\d3dmain.obj"
	-@erase "$(INTDIR)\d3dmain.res"
	-@erase "$(INTDIR)\d3dmath.obj"
	-@erase "$(INTDIR)\d3dsphr.obj"
	-@erase "$(INTDIR)\ddcalls.obj"
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\geometry.obj"
	-@erase "$(INTDIR)\lclib.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\procedural.obj"
	-@erase "$(INTDIR)\render.obj"
	-@erase "$(INTDIR)\sphere.obj"
	-@erase "$(INTDIR)\spline.obj"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\tunnel.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\mixed.exe"
	-@erase "$(OUTDIR)\mixed.ilk"
	-@erase "$(OUTDIR)\mixed.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "c:\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "c:\dxsdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)/mixed.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/d3dmain.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/mixed.bsc" 
BSC32_SBRS= \
	
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib c:\dxsdk\sdk\lib\ddraw.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib c:\dxsdk\sdk\lib\ddraw.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/mixed.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/mixed.exe" 
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dcalls.obj" \
	"$(INTDIR)\d3dmain.obj" \
	"$(INTDIR)\d3dmain.res" \
	"$(INTDIR)\d3dmath.obj" \
	"$(INTDIR)\d3dsphr.obj" \
	"$(INTDIR)\ddcalls.obj" \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\geometry.obj" \
	"$(INTDIR)\lclib.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\procedural.obj" \
	"$(INTDIR)\render.obj" \
	"$(INTDIR)\sphere.obj" \
	"$(INTDIR)\spline.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\tunnel.obj" \
	".\Mmxoctave.obj" \
	".\Mmxtexture.obj"

"$(OUTDIR)\mixed.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "mixed - Win32 Release"
# Name "mixed - Win32 Debug"

!IF  "$(CFG)" == "mixed - Win32 Release"

!ELSEIF  "$(CFG)" == "mixed - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\tunnel.cpp
DEP_CPP_TUNNE=\
	".\d3dapp.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\ddutil.h"\
	".\main.h"\
	

"$(INTDIR)\tunnel.obj" : $(SOURCE) $(DEP_CPP_TUNNE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dcalls.c
DEP_CPP_D3DCA=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3dmacs.h"\
	".\lclib.h"\
	

"$(INTDIR)\d3dcalls.obj" : $(SOURCE) $(DEP_CPP_D3DCA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dmain.cpp
DEP_CPP_D3DMA=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\ddutil.h"\
	".\lclib.h"\
	".\main.h"\
	

"$(INTDIR)\d3dmain.obj" : $(SOURCE) $(DEP_CPP_D3DMA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dmain.rc
DEP_RSC_D3DMAI=\
	".\d3d.ico"\
	".\d3dres.h"\
	

"$(INTDIR)\d3dmain.res" : $(SOURCE) $(DEP_RSC_D3DMAI) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dmath.c

"$(INTDIR)\d3dmath.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dsphr.c

"$(INTDIR)\d3dsphr.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ddcalls.c
DEP_CPP_DDCAL=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3dmacs.h"\
	".\lclib.h"\
	

"$(INTDIR)\ddcalls.obj" : $(SOURCE) $(DEP_CPP_DDCAL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ddutil.cpp
DEP_CPP_DDUTI=\
	".\ddutil.h"\
	

"$(INTDIR)\ddutil.obj" : $(SOURCE) $(DEP_CPP_DDUTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\geometry.c
DEP_CPP_GEOME=\
	".\d3dapp.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\procedural.h"\
	".\render.h"\
	

"$(INTDIR)\geometry.obj" : $(SOURCE) $(DEP_CPP_GEOME) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\lclib.c
DEP_CPP_LCLIB=\
	".\lclib.h"\
	

"$(INTDIR)\lclib.obj" : $(SOURCE) $(DEP_CPP_LCLIB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\misc.c
DEP_CPP_MISC_=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3dmacs.h"\
	".\lclib.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\procedural.c
DEP_CPP_PROCE=\
	".\procedural.h"\
	

"$(INTDIR)\procedural.obj" : $(SOURCE) $(DEP_CPP_PROCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\render.c
DEP_CPP_RENDE=\
	".\d3dapp.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\procedural.h"\
	".\render.h"\
	

!IF  "$(CFG)" == "mixed - Win32 Release"


"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"
   $(CPP) /nologo /ML /W3 /GX /O2 /I "c:\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /Fp"$(INTDIR)/mixed.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "mixed - Win32 Debug"

# ADD CPP /O2

"$(INTDIR)\render.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"
   $(CPP) /nologo /MLd /W3 /Gm /GX /Zi /O2 /I "c:\dxsdk\sdk\inc" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)/mixed.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sphere.c
DEP_CPP_SPHER=\
	".\d3dapp.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\main.h"\
	".\procedural.h"\
	".\render.h"\
	

"$(INTDIR)\sphere.obj" : $(SOURCE) $(DEP_CPP_SPHER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\spline.c

"$(INTDIR)\spline.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\stats.cpp
DEP_CPP_STATS=\
	".\d3dapp.h"\
	".\d3ddemo.h"\
	".\d3dmacs.h"\
	".\d3dmain.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dsphr.h"\
	".\main.h"\
	

"$(INTDIR)\stats.obj" : $(SOURCE) $(DEP_CPP_STATS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\texture.c
DEP_CPP_TEXTU=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3dmacs.h"\
	".\lclib.h"\
	".\main.h"\
	

"$(INTDIR)\texture.obj" : $(SOURCE) $(DEP_CPP_TEXTU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\d3dapp.c
DEP_CPP_D3DAP=\
	".\d3dapp.h"\
	".\d3dappi.h"\
	".\d3dmacs.h"\
	".\lclib.h"\
	

"$(INTDIR)\d3dapp.obj" : $(SOURCE) $(DEP_CPP_D3DAP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Mmxtexture.obj

!IF  "$(CFG)" == "mixed - Win32 Release"

!ELSEIF  "$(CFG)" == "mixed - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Mmxoctave.obj

!IF  "$(CFG)" == "mixed - Win32 Release"

!ELSEIF  "$(CFG)" == "mixed - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
