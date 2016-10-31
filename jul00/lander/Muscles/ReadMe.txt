3D Real-time Dynamic Muscle Demonstration July, 2000
--------------------------------------------------------

This demonstration builds on the spring-mesh system I built last 
year by making some of the springs active.

In this demo, each Muscle is activated by a periodic function
(a sin wave with 256 emtries in this case).  The muscle has a contraction 
percentage and an offset into the wave table.  The offset allows 
muscles to be activated at different times.

For example, on the dolphin that is included, there are four muscles
controlling the tail.  Two along the top and two on the bottom that
alternate activating.

To change a spring to a muscle, select the two vertices connected by a
spring and selecting "Spring/Muscle Settings" from the Simulation menu.
Make the muscle active and give it a contraction value.

To load new models, load them as an OBJ as described below then connect
it with springs.  

Todo:

Try different activation functions (a square wave would be easy and interesting)
Apply the viscous force to the model as described in the article


Jeff

v. 1.0		Jan 1999
Soft body dynamics

This program simulates soft body dynamics of a particle system
with particles connected with springs.

This program is fairly complicated.  In order to show off the
features I wanted to demonstrate, I needed to create quite a
number of functions and UI.  I appologize for all the extra junk
but it was as clear as I could make it in the time allowed.
All the major functions for the actual dynamic simulation are in
the files:	PhysEnv.h and PhysEnv.cpp.  Some of the timing
and display code is in OGLView.h and OGLView.cpp.  Most of
the UI is passed through MainFrm.cpp to OGLView.  

All of the simulation constants can be set through the 
Simulation Properties dialog.  Any changes to the spring constants
are applied to all springs in the simulation.  There is currently
no way to change the settings for an individual spring.

You can load in one of the pre-made simulations or create a
new one by loading in an OBJ.  The OBJ needs to be scaled 
within a +-5 unit cube world so it fits within the world boundaries.
An OBJ file loads as a point cloud.  You then can connect the dots
by selecting vertices two at a time.  Press "ENTER" to connect the
points with a spring.  The normal length of the spring is the distance
at the time of spring creation.

You can select one or two vertices and apply an user force to
the selected vertex/vertices.  Once selected, a user force is
applied with the arrow keys for X-axis and Z-axis and HOME/END
applies the force to the Y-axis.  The magnitude of the user force
is set in the sim properties dialog.

You can also click and drag the LMB to apply a force in the local
X and Y axis.  The mousespring force is set in the sim properties
dialog.  The force will apply to the one or two vertices selected.

Demos:


Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				
-----------------------------------------------------------

I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL for Windows, Permidia 1 and 2 OpenGL
Drivers, Riva 128, TNT, TNT2, AccelGalaxy, 
and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

Also, depending on how well GLlines are drawn on your system, the
sim should run too fast, too slow, or just right.  I try to lock
it to the clock except to set the timesteps.  If it is strange,
adjust COGLView::RunSim().

