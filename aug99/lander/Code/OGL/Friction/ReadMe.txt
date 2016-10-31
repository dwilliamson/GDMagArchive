3D Real-time Soft Body Friction Demonstration July, 1999
--------------------------------------------------------
v. 1.0  With Friction

This is a quick demonstration to show how the Coulomb method
of dry friction can be applied to a dynamic simulation.

The demo is built on the same soft body code from earlier in
the year.  However, there have been some key changes.

1. A contact model has been added.  This required changing
the particle structure so models from the previous version will
not work.  I create the cube and diamond for this demo.  The 
contact model checks to see if distance from the particle to the
contact surface is below the contactEpsilon threshold (currently 0.01f).
If so, the particle is marked and the contact normal is saved.

2. Friction force is added via the Coulomb model during 
ComputeForces.  If the relative velocity is greater then the
threshold STATIC_THRESHOLD, the kinetic friction is applied.
Otherwise, static friction is used.

Notes:

In order to see the friction force in action, world damping
has been reduced.  You can toggle friction on and off to
see how it halts the object (F key or Menu switch).

Obviously particles are not the best thing for showing off
friction.  Friction works better on rigid bodies with larger
masses.  I will address this next month when I start building
a Billiards simulation with rigid spheres.

In the meantime, you can play with the coefficients and
contact model.  Switch integrators and see the effects.
If you are ambitions, add this into the cloth demo from 
a few months ago to the sphere collision routines.

Also think about creating a list of contacts and storing
friction coefficients with the contact.  I wanted to do
it that way but ran out of time.

Next month, the app will have a totally different look.
Much more like a production game with less MFC style stuff.
Brush up on your billiards...

Jeff

v. 1.0		Jan 1999

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

