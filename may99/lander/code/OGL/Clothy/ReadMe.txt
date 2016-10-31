3D Cloth Simulation Demonstration April 10, 1999
--------------------------------------------------------
v. 1.0

This is the sample application that accompanies the May 99
Game Developer magazine.  It is meant as a demonstration of
a method for 3D cloth simulation in OpenGL.  

Cloth simulation is a very complex process.  This column and
demo just scratches the surface.  It is simply a way of building
a form of cloth using the mass and spring system from last 
month.  It is sort of a macro function (CreateClothPatch in OGLView.cpp)
that builds all the various springs and attaches them.
There are many places it can be improved and modified.  
Please read the article and check the references in the 
"For Further Info" section if you are really interested 
in cloth simulation.

Ideas for Optimization and expansion:
1. Change the springs to work on the Squared distance to avoid
the sqrt for each spring.

2. The collision detection really slows things down.  The model
could be changed so that it didn't back up the sim for cloth->sphere
collisions.  You could simply move the vertex outside the sphere.
I have tried this and it makes things a bunch faster at the expense
of some accuracy.

3. Optimize the integration code.  Especially the RK4 needs some
work.  

4. Add a fix to the "super-elastic" effect I described in the 
article.

5. Add an implicit integrator like in the Witkin and Baraff 
reference.

6. Add in cloth->box collision.

Hopefully this will get you started.  If you make any 
improvements send them to me and I will post them with a
credit to you.
 
Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.
I hope to expand on this demo some more in the future.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm			
-----------------------------------------------------------


I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL for Windows, Permidia 1 and 2 OpenGL
Drivers, Riva 128 (New Beta OGL Drivers), AccelGalaxy, 
and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

Also, depending on how well GLlines are drawn on your system, the
sim should run too fast, too slow, or just right.  I try to lock
it to the clock except to set the timesteps.  If it is strange,
adjust COGLView::RunSim().

