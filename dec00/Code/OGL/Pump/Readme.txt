Volume Preservation			December 2000
--------------------------------------------------------
v. 1.0  

This is a bit of a strange demo.  It is halfway toward a goal I have to do a fast, simple 
soft body using a mass and spring system.  This shows a possible solution to a problem with 
a simple mass system that first appeared in my May 2000 demo that this code is based on.  That 
is volume collapse.  In that demo, the mass points could find a stable state even though the 
volume of the object was not preserved.

Since it did not seem that adding a massive amount of springs to the mesh to do this was a good
idea, I tried something else.  If I have a rough idea of where the mesh points would be if the
object was rigid, I could attempt to make the model adopt that shape.  Problem is the object is
deforming so how can I determine the orientation if it were rigid.  This demo shows one approach
that works in specific cases.

The routine "ComputeObjLocalFrame" in GameSim.cpp looks at the mesh control points and creates
a local transformation axis by averaging the sides of the mesh.  I then use this matrix to transform
the original points to see where the points would be if the object is rigid.  In "ComputeForces" I
can then move the deformed control points towards these rigid ones with springs.

In the demo, you can manipulate the mesh and see the computed axis and ideal points (blue).  As you
pull points around and turn on/off gravity, notice that the calculated axis is pretty representative
of the general orientation of the object.

It doesn't do much else.  However, this idea will become more useful in the months to come particularly
when combined with the FFD method from May.  I will also examine a different method for finding a local 
object frame using the Least Squares method. I also hope to examine Finite Element methods for volume 
preservation as an entirely different approach.

I would love to hear other peoples idea of how to address the problem of determining a coordinate 
frame for a deformable body.  It is such a useful idea that I am sure there is tons of stuff I haven't
seen.

Notes:

Instead of averaging the sides of the mesh, I could have picked representative nodes and
tracked them as it deformed.  This may be more generally useful for odd shaped control meshes but
does not give as good a representative sampling of the outer hull of the object.


How it works.

	Click and drag the Left Mouse Button on one or two control points to 
	change the deformation lattice.

	Click and drag the Right Mouse Button to rotate the view around.

	Menu options to turn on/off gravity.

	View options:
	Draw Springs
	Draw Control Vertices (CVs)
	Draw Orientation Axis and Volume Points ( This shows the computed axis and where the points should be)


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

