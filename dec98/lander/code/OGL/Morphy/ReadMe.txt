3D Real-time Morphing in OpenGL Demonstration Nov 5, 1998
--------------------------------------------------------
v. 1.0

This is the sample application that accompanies the December 98
Game Developer magazine.  It is meant as a demonstration of
a method for 3D morphing in OpenGL.  

To use the program, load in two OBJ models having the exact same
vertex count.  The slider will morph smoothly between the two.

The function morphModel() does the main work.

Note: The normals should probably be normalized after the morph
but in practice it seems to work fine without doing it.

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				Feb 5, 1998
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, Riva 128 (New Beta OGL Drivers), and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

