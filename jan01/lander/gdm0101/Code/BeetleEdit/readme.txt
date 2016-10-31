
Creating a Weighted Skeletal Character		January 2001
----------------------------------------------------------
v. 2.00

This program is based on my 3D paint system from last year and allows you to load
in an OBJ object and an Ascii file describing the skeletal system.  You can then automatically
generate vertex weights based on the size and position of the bone objects.  You can also "paint"
the weights using 3D painting techniques.

In practice most developers will probably use a 3D modeling and animation package to
do this job.  However, some may not have access to this type of tool so need to
develop their own.  This may give you some ideas.

I use a simple ASCII format for the skeletal description.  Each bone is described by
name and then info about that bone is enclosed in brackets.  For example:

Bone Hip
{
4								// Number of children
Spine2  Back1  RLeg1  LLeg1		// Child names
0.0000  3.3923  0.0013			// XYZ transform
-90.0000  0.0000  90.0000		// XYZ rotation
1.0000  1.0000  1.0000			// XYZ scale
1.3000							// Size of bounding sphere
weights 0						// Number of vertex weights to follow in (index weight) pairs
}

This bone describes a hip joint that is connected to 4 child bones.

To make this program more useful, users should be able to create, delete, and change the heirarchy.  
That is a pretty tough interface issue though so it is left as an exercise to the user.

The routine "AutoComputeWeights" in OGLView.cpp is the meat of the weight calculation. 
It calculates a falloff formula for each vertex, sorts the weights, takes the top N weights, and
normalizes the results.  It usually creates a pretty decent set of weights.  However, using the
paint function, you can easily tweak the results.

Controls:

I adopted a "Maya" style camera control system.  You hold down the "CTRL" key to manipulate
the camera.

CTRL+LMB Drag	-	Orbits the camera around XY axes
CTRL+MMB Drag	-	Moves Camera in XY	(for those with MMB)
CTRL+LMB+MMB	-	Moves Camera in Z
CTRL+RMB Drag	-	Moves Camera in XZ
CTRL+RMB+SHIFT	-	Moves Camera in XY

Once a mesh and skeleton is loaded, Skeletal commands are active.

C				-	Calculate Automatic Vertex Weights
+ (Keypad)		-	Make next bone active
- (Keypad)		-	Make previous bone active
<				-	Shrink current Bounding Sphere
>				-	Increase current B-Sphere

LMB Paint		-	Weight Vertices to current bone
LMB+SHIFT Paint	-	Unweight Vertices from current bone
RMB				-	Rotate Current Bone in XY
RMB+SHIFT		-	Rotate Current Bone in Z

You can save out the mesh in a simple Binary format or OBJ and Save the Skeletal system
as an ASCII file.

TODO:	Many things could make this a much better production tool.
1. Ability to create, delete, and change the skeletal system
2. Controls for weight painting shape and falloff options
3. A single save format that saves mesh,skeleton,weights, texture.
4. Animation tools
5. Tons more as you are well aware.

---------------------------------------------------------------------

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

I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, Permidia 2 OpenGL,  NVidia TNT, TNT2, GeForce256.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX Voodoo or
Voodoo 2.  3DFX Voodoo and Voodoo 2 OpenGL do not support 
OpenGL in a window so will not work with this application.


