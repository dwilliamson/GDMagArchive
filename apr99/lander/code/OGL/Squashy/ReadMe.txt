3D Real-time Soft Body Dynamics Demonstration April 5, 1999
--------------------------------------------------------
v. 2.0

Sorry it took some time to release this code.  It took a
while to recover from the GDC andd get my actual job-work 
caught up.  I finally sat down and commented and cleaned
things up.

This is the same demonstration as the February column with 
the addition of better integration.  There are two integrators 
in addition to the Euler one from that issue.

Midpoint Method (RK2)
	This integrator evaluates the function at a half step then
	takes a full step with those values 

Runge-Kutta 4
	This very sophisticated integrator actually evaluates the 
	function four times and is very stable even with very stiff
	springs.

For example the "GonaBlow" test using the Euler integrator blew up.
Using both the Midpoint and RK4 integrators, this works fine.
Try these files with the new engine.

GonaBlow.dps	Spring Stiffness = 20.0  Damping = 0.1	Midpoint & RK4
GonaBlw2.dps	Spring Stiffness = 200.0  Damping = 0.01	RK4 only

Since the new integrators need to recompute the forces multiple
times, there are quite a few extra calculations.  In fact I found
while the collisions were being resolved and the simulator was backing
up to find the exact time of collision, the sim was too slow.  So
I set it up so that it uses the Euler integrator only during collision
rootfinding.  This dramatically speeds up performance while not effecting
stability as far as I can see.

This needs to be optimized like crazy if you want to use it in a
production application.  One obvious point for optimizing would be
to change the way the springs work so that they use the squared-distance
as the baseline so it would avoid the SQRT function in the force 
calculations.  I am sure you will all find a lot of others.  The
RK4 code could probably be organized a lot better I know.  Drop me
a copy of any cool improvements you make.

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

"GonaBlow"
Be sure an check out the "GonaBlow" demo.  It shows what happens
when the integrator suffers from numerical instability.  You can 
prove this by halving the MaxStepSize in the Timing Property Dialog.
We will fix that next month.

"BoxBad"
This box is only connected along the cube edges.  It is not stable in
this form.  Try it out and see why.

"BoxGood"
By adding crossbeam supports, it becomes stable.  This is a good example
of how to create a stable model.

"Diamond"
Another shape with pretty stiff springs.  It is interesting to see how
it balances on an edge and then eventually falls.  I suspect this is rounding
errors.  In theory, it should balance exactly forever.

This is a pretty complex demo.  Any problems or questions, email me.

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

