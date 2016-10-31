Five Example Programs are included here.

Simple1: Example of creating a scene with a simple background. In short,
         it shows how to blit a bitmap to the screen using DirectX. You
         will probably have to use the elipses button in the BackGroundFile
         property for the TSpeedDraw component to browse for the file
         called BackGrnd1.bmp. It should be in the root directory of the
         projects sent with this package.

NoBmp1:  Shows how to create a DirectX scene that has no bitmap. Draw on
         a hidden blank background using the GDI, then flip backgrounds
         using DirectDraw.

Simple2: Use two bitmaps in one scene. The first is a background bitmap,
         the second a very simple minded form of sprite. You
         will probably have to use the elipses button in the BackGroundFile
         property for the TSpeedDraw component to browse for the file
         called BackGrnd1.bmp. It should be in the root directory of the
         projects sent with this package.

COMDLL:  Make TSpeedDraw into a COM object. Don't forget to register the
         object with the included SPEDDRAW.REG file. Type START SPEDDRAW.REG
         or run REGEDIT (found in the Windows directory) and register
         SPEDDRAW.REG. You might need to edit the INPROCSERVER32 statement
         in the REG file.

CFIRE:   Simple minded C program that calls your Delphi COM object.

For possible updates or further information on this code see this website:
  http://users.aol.com/charliecal
        
