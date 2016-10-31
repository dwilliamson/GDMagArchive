Inverse Kinematics in OpenGL Demonstration Program  Oct 1, 1998
----------------------------------------------------------------
v. 1.0

This is the sample application that accompanies the November 98
Game Developer magazine.  It is meant as a demonstration of
an general inverse kinematics solution.  The main function
is in OGLView.cpp at the very bottom.  The function is called
"ComputeCCDLink" and it takes a point on the screen for the 
articulated arm to try and reach.

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, Riva 128, Riva TNT, and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

There are instructions in the Help/About dialog.  

Jeff