# EASYTEXT.MAK

!if !$d(BCROOT)
BCROOT  = $(MAKEDIR)\..
!endif

# macros
APPNAME = CFIRE
INCPATH = $(BCROOT)\INCLUDE
LIBPATH = $(BCROOT)\LIB

!if $d(WIN16)
COMPILER= BCC.EXE
FLAGS   = -W -ml -v -w4 -I$(INCPATH) -L$(LIBPATH) 
!else
COMPILER= BCC32.EXE
FLAGS   = -W -v -w4 -I$(INCPATH) -L$(LIBPATH)
!endif

# link
$(APPNAME).exe: $(APPNAME).obj $(APPNAME).def
  $(COMPILER) $(FLAGS) $(APPNAME).obj OLE2W32.LIB

# compile
$(APPNAME).obj: $(APPNAME).cpp
  $(COMPILER) -c $(FLAGS) $(APPNAME).cpp
