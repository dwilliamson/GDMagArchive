3D Real-time Blob Modelling Demonstration Nov 20, 1999
--------------------------------------------------------
v. 1.0

This is the sample application that accompanies the November 99
Game Developer magazine.  It is meant as a demonstration of
a method for 3D Blob Modelling in OpenGL.  

I really wanted to work on this more but I am so behind on demos,
I will just put it out there and try to update. 

Lots of things you can add to this to make it better.

1. Fix the isosurface generation.  It has some issues marked with TODO
	in the code

2. Blob triangles and vertices do not have normals, create them from 
   the field gradients so you can light the objects

3. Isosurface generation can be optimized like crazy.  Add the
   surface tracking idea I talked about it the article

4. Add a color component to each blob and blend the colors with
   the strength function and use vertex coloring to draw it.

5. Add support for non-sphere objects or non-uniform scaling of blobs

6. Add animation support

All the blob routines are in the MetaGoop.h/cpp files so it could
be adapted to non-MFC apps easy.

There is also an example of using OpenGL name picking in the
OGLView.cpp routine.

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				Nov 5, 1999
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, Riva 128 (New Beta OGL Drivers), AccelGalaxy, 
and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

