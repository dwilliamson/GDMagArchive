Slash II - Slash Harder	

Quaternion Interpolation Demonstration Program March 15, 1998
-------------------------------------------------------------

v. 2.0

This is the sample application that accompanies the April 98
Game Developer magazine.  It is meant as a demonstration of
how use Quaternions in OpenGL.  This is the second article
of the two part series.  This one really deals with
interpolating between Quaternions.  If you want more
explaination of the Euler angle to Quaternion code, see the
March 98 Game Developer magazine and sample code.

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
Drivers, Riva 128 (New Beta OGL Drivers), and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

There are instructions in the Help/About dialog.  

This program is to demonstrate the use of Quaternions for
animation control in OpenGL.

The user interface for creating keyframe positions is a little
strange.  What can I say, you want a good keyframing program,
buy a professional 3D animation package. 

Anyway, it works like this:

The demo skeleton is an arm with three bones; Upper Arm, Lower Arm,
and Hand.  Each bone has a primary rotation position and a secondary
one.  The interpolator slider sets the relative position between these
keys.  I created two keyframe positions for each bone and hard coded
them in as a sample "slash" move.  But, you can create your own
interpolation.

Select the bone you wish to position and then rotate it to the desired
orientation by dragging with the left mouse button for X and Y and
right button for the Z axis.  When you have it set to the final
position, select "Keyframe/Set End Key" this locks it in.  Do that
for all 3 bones.  

After all the keyframes are locked, move them all to the starting
orientation.  Then you can use the slider to interpolate between
them.

Other Functions:

1,2,3			Select Upper Arm, Lower Arm, Hand as current bone. 
Double Click	Brings up edit box for orientation of current bone.
Ctrl+Mouse Drag	Translate the base position of the system.


---------------------------------------

Other Info:

The models were exported by a custom plugin and are Vertex Colored
openGL vertex arrays.  The data is interleaved.  I won't go into
the model format but it is real obvious if you look at the loader
in MainFrame and the skeleton format in the skeleton.h file.

My explaination of the comparison in the SLERP code in the magazine
was wrong.  Thanks to several people for pointing that out.  The
comments in code here I believe is the correct one.  Although, I haven't
created strange cases to test it all out.  It seems very robust for all
the application I have used it for.

This is a pretty big application.  It seemed small when I started the
little demo but each feature added a bunch.  Any questions, let me
know.  I hope to scale back a bit on the demo apps but with
upcoming topics like skeletal deformation, IK, and dynamic LOD,
I doubt they will get much smaller.

If you create cool tools or demos yourself and want me to check
them out.  Send them.  I would love to see what other people are
working on.  Also, if you have things you are interested in
seeing cover, let me know.

Have fun.

Jeff

