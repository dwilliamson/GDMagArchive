The program runs on Win95/98 with Glide 2.4 or above. Don't use triple buffering as I use the second buffer to render the shadow texture. I experienced troubles using a PC which has "forced" the triple buffer.

Description of the files of the sample code of the article "Casting shadows on volumes" by Hubert Nguyen :

Project for VC++ 5.x or above:
-----------------------------
GAMEDEV  DSW
GAMEDEV  DSP

Source code:
-----------
TOOLS    CPP    : various functions needed
SAMPLE   CPP    : the "main example code"
RASTER   CPP    : file that contains code which uses Glide
MESHES   CPP    : basic code to handle mesh objects
MATRIX   CPP    : basic code to create rotation matrixes
MAIN     CPP    : file that contains "winmain"
HSINCOS  CPP    : sincos table
HMATRI~1 CPP    : basic code to create rotation matrixes
CAMERA   CPP    : basic functions to handle camera
COPYBUF  CPP    : function that copy a buffer into a window

DATAS    CPP    : 3D mesh in "C code" format
DATAR    CPP    : 3D mesh in "C code" format
DATAMAN  CPP    : 3D mesh in "C code" format

Include files:
-------------
TOOLS    HPP
SAMPLE   HPP
RASTER   HPP
MESHES   HPP
MATRIX   HPP
MAIN     HPP
HVECTOR  HPP
HSINCOS  HPP
HMATRI~1 HPP
COPYBUF  HPP
CAMERA   HPP


I'll post any update of this program to GDmag, and you can reach me at nguyenyoda@aol.com


Thanks,

Hubert Nguyen.
