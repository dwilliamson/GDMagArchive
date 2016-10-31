# Microsoft Developer Studio Project File - Name="fluid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=fluid - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak" CFG="fluid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluid - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "fluid - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluid - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp16 /W3 /GX /O2 /Ob2 /I "t:\projects\plib\src\sg" /I "t:\projects\plib\src\fnt" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 pui.lib sg.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 pui.lib sg.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# SUBTRACT LINK32 /profile /debug

!ELSEIF  "$(CFG)" == "fluid - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "t:\projects\plib\src\sg" /I "t:\projects\plib\src\fnt" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "fluid - Win32 Release"
# Name "fluid - Win32 Debug"
# Begin Group "Surface Representation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SurfRep\BucketPartition.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfRep\BucketPartition.h
# End Source File
# Begin Source File

SOURCE=.\SurfRep\PhysicalPoint.h
# End Source File
# Begin Source File

SOURCE=.\SurfRep\PotentialPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfRep\PotentialPoints.h
# End Source File
# Begin Source File

SOURCE=.\SurfRep\SurfaceSampler.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfRep\SurfaceSampler.h
# End Source File
# End Group
# Begin Group "Tessellation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Tess\ContinuousTessellator.cpp
# End Source File
# Begin Source File

SOURCE=.\Tess\ContinuousTessellator.h
# End Source File
# Begin Source File

SOURCE=.\Tess\CubePolygonizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Tess\CubePolygonizer.h
# End Source File
# Begin Source File

SOURCE=.\Tess\IntVector.h
# End Source File
# Begin Source File

SOURCE=.\Tess\MarchingCube.cpp
# End Source File
# Begin Source File

SOURCE=.\Tess\MarchingCube.h
# End Source File
# Begin Source File

SOURCE=.\Tess\MarchingCubeTable.h
# End Source File
# Begin Source File

SOURCE=.\Tess\MarchingCubeTableNew.h
# End Source File
# Begin Source File

SOURCE=.\Tess\MarchingTetTable.h
# End Source File
# Begin Source File

SOURCE=.\Tess\SurfaceTessellator.cpp
# End Source File
# Begin Source File

SOURCE=.\Tess\SurfaceTessellator.h
# End Source File
# End Group
# Begin Group "Rendering"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Render\ImplicitMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\Render\ImplicitMesh.h
# End Source File
# End Group
# Begin Group "Main Program"

# PROP Default_Filter ""
# Begin Group "Demos"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Demos\CongealDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\CongealDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\DummyDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\DummyDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\FillDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\FillDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\FluidDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\FluidDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\FountainDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\FountainDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\FountainProtoDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\FountainProtoDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\HeightFieldDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\HeightFieldDemo.h
# End Source File
# Begin Source File

SOURCE=.\Demos\StreamDemo.cpp
# End Source File
# Begin Source File

SOURCE=.\Demos\StreamDemo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AutoOrbitCam.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoOrbitCam.h
# End Source File
# Begin Source File

SOURCE=.\basicFluid.cpp
# End Source File
# Begin Source File

SOURCE=.\IntHashTest.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\PuiPersona.h
# End Source File
# End Group
# Begin Group "Utility"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Utility\IntHash.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Utility\IntHash.h
# End Source File
# End Group
# Begin Group "ParticleSys"

# PROP Default_Filter ""
# Begin Group "Generators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ParticleSys\Fountain.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\Fountain.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\ParticleGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\ParticleGenerator.h
# End Source File
# End Group
# Begin Group "Force Exerters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ParticleSys\AltitudeWind.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\AltitudeWind.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\BasicAccelerator.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\BasicAccelerator.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\BoxBouncer.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\BoxBouncer.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\ConstantExerter.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\ConstantExerter.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\MoleculeAnimator.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\MoleculeAnimator.h
# End Source File
# End Group
# Begin Group "Integrators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ParticleSys\ImplicitMover.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\ImplicitMover.h
# End Source File
# End Group
# Begin Group "Colliders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ParticleSys\CylinderCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\CylinderCollider.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\HeightFieldCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\HeightFieldCollider.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\PlaneCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\PlaneCollider.h
# End Source File
# End Group
# Begin Group "Killers"

# PROP Default_Filter ""
# End Group
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ParticleSys\CompositeFunctor.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSys\CompositeFunctor.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ParticleSys\ParticleFunctor.h
# End Source File
# End Group
# Begin Group "Calder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Calder\FountainVisuals.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\FountainVisuals.h
# End Source File
# Begin Source File

SOURCE=.\Calder\HeightField.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\HeightField.h
# End Source File
# Begin Source File

SOURCE=.\Calder\MultiHeightFieldCollider.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\MultiHeightFieldCollider.h
# End Source File
# Begin Source File

SOURCE=.\Calder\PolygonSampler.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\PolygonSampler.h
# End Source File
# Begin Source File

SOURCE=.\Calder\WaveGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\WaveGenerator.h
# End Source File
# Begin Source File

SOURCE=.\Calder\WavePlane.cpp
# End Source File
# Begin Source File

SOURCE=.\Calder\WavePlane.h
# End Source File
# End Group
# End Target
# End Project
