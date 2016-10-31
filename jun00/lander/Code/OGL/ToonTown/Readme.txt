Accelerated FFD deformation				May 2000
--------------------------------------------------------
v. 1.0  

This is a demonstration of how skeletal deformation techniques can be 
used to accelerate arbitrary deformation functions such as a cubic FFD.

A lattice of control points is created around an object and those control
points are connected with springs.  Any movement of these control points
will deform the mesh.

This demo also uses the cartoon rendering shader from January.

How it works.

	Click and drag the Left Mouse Button on one or two control points to 
	change the deformation lattice.

	Click and drag the Right Mouse Button to rotate the view around.

	Menu options to turn on/off gravity.

	View options:
	Draw Springs
	Draw Control Vertices (CVs)
	Draw Mesh
	Draw Vertex Influences (This displays the rough weighting on the mesh of the 
		first selected CV)

Problems for you to explore:

	The physical simulation is a simple spring system.  It is easy to collapse
	the mesh on itself.  The physical sim really needs a mesh deformation technique
	that preserves volume.

	Collision occurs with the control vertices right now.  They really show collide with
	the base mesh points.  One idea is to store offsets from the CV into the mesh and only
	collide then.  Thanks to Casey of RAD Game Tools for that idea.

	Convert this whole thing to be hardware accelerated with both vertex and pixel shaders
	once DX8 hardware is available to do this. (How about it IHVs??)


Any questions email:

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				
-----------------------------------------------------------

I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL for Windows, Permidia 2 OpenGL
Drivers, Riva 128, TNT, TNT2, AccelGalaxy, ATI Rage LT Pro, 
and Matrox G400.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX Voodoo series. 
3DFX OpenGL that does not support OpenGL in a window will not work
with this application.

The Spring and Particle display make use of line drawing.  Some cards
are slow when displaying lines (ATI Rage LT Pro is an example).  Performance
will increase greatly when line drawing is off.