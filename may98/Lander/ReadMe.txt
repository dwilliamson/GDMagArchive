Skeletal Mesh Deformation		April, 1998

This application demonstrates the use of weighted skeletal
mesh deformation in a real-time applications.

There are two bones in the system; the upper arm and lower
arm.  Well, really 3, but only two are able to be weighted.
The hand is just there to define the lower arm length.

Controls:

Select Bone to control via the Control menu options or
'1' or '2' to select upper arm or lower arm respectively.

Mouse drag allows you to rotate the currently selected
bone.  Left mouse button controls X and Y rotations and 
Right mouse button controls Z rotation.

CTRL+Drag allows you to move the system around the screen.

SHIFT+Drag allows you select vertices to adjust the weighting
for.  Left mouse button to select and Right mouse button to
deselect.

SPACE to clear all vertex selections.

Once vertices are selected, the Modify weights menu item brings
up a dialog that allows you to set the influence of each
bone on the selected vertices.  Slide from 100% upper arm
control to 100% lower arm control.

View can toggle on and off display of bones, geometry, and outline/
filled polygon mode.

File/Reset resets the system to the default of all vertices controlled
by upper arm.

You can save and load weight files that hold the settings for the
system.

There are three demo files created already:

Arm.wgt		=	Fully weighted arm with vertices set to different percentages
Arm100%.wgt =   Vertices are fully weighted to a bone.  Either upper or lower arm
Arm50%.wgt	=   Vertices are fully weighted except a single row is 50-50 to each bone.

This should allow you to easily see the problems with different setups.

Source Code:

This is the most complicated application I have provided so
far.  There is a lot of code here but most of it is to try
and make a decent UI.  The guts of the technique is quite
small really.  I could make the code simpler but then the
app would be no fun.  So bear with me on all the extra stuff.
Also, I know it is not real optimized but I wanted it very
clear what I was doing.  Believe me when I say, I know most of
the tricks and my actual production code is faster but much
harder to get through.  So please don't mail with comments like
"Your trig routines should use look-up tables."  You guys
can have fun speeding it all up in your own individual projects.

The main deformation routine is DeformBone in OGLView.cpp.  
It is a recursive function that applies the deformation for
each bone to the main model.

The code is set up with a three bones system.  
Upper arm, Lower arm, and hand.
The hand is only there to define the length of the lower arm.
The engine would allow you to weight to the hand but the interface
would be a bunch more clumsy with relative weighting of three bones.
If you code the weightings yourself, you can set up as many bones and
weights as you like.  I have tried up to 25 on a single mesh.

There is also a second deformation that I used for debugging that I
left in there.  It is the DeformModel function.  It is a specific
case for a two bones system.  It calculates the two positions and
linear interpolates based on the weight.  It is faster since it
doesn't recurse through the full hierarchy.  If you restrict your
system to have vertices only influenced by two bones, this may be
the way to go.

To use the test case, remove the DEFORM_GENERAL_SOLUTION define
from the top of OGLView.cpp.

The matrix code to do the non-OpenGL vector by matrix is called
MultVectorByMatrix in Matrix.cpp.

The Quaternion code is also in here because I use it for my animation
system.  It isn't really needed for this project.

Things to do:

You could pretty easily add these things to the source code making
the program more user friendly.

Weighting Color Cues
It would be very easy to ID which bone influences which
vertex.  Softimage does this by changing the colors of 
vertex display to be the color of the bone that influences it.

Weighting Dialog Defaults
Set the weighting dialog slider according to the average
weight of the selected vertices.

DOF Restrictions
Use the Degree of Freedom variables in the bone structure
to store restrictions. In the User controls, use these 
values to limit rotations.

Any questions, email to jeffl@darwin3d.com

Have fun.
