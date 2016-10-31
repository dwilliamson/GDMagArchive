Curves and Curved surface source
GDC '99 submission
GDMag June/July demo code
Demo information
Release 1.1

Brian Sharp, CogniToy
brian_sharp@cognitoy.com

For more information, check out my web site at http://www.cs.dartmouth.edu/~bsharp/gdmag/


--------------------------------------------------------------------------------------
Release 1.1 Notes:

This release somewhat coincides with the publication of the first of my two GDMag 
articles on curved surfaces.  More importantly, it's an update to fix a few little
things people have sent me mail about.  Still no makefiles, sorry.

Fixed a couple projects that were linking to SGI's OpenGL.dll and Glu.dll instead of
Microsoft's OpenGL32.dll and Glu32.dll.  I saw no benefit from using SGI's libraries
anymore, and a number of people didn't have SGI's DLLs anyway.

Fixed texture usage for cards that supported GL_EXT_texture_edge_clamp.  If you had a
black grid overlayed on the terrain, that should be gone, now.  If not, complain to your
IHV to have their driver writers put GL_EXT_texture_edge_clamp into their driver.

Added snowflakes to the marbles demo.

Added an option to not use glutFullScreen.  If you don't want any of these demos to 
kick into fullscreen, simply run them with the switch /nofullscreen (there's no /? 
support, so just make sure to spell it right. :) )

--------------------------------------------------------------------------------------
Release 1.0 Notes:

These files go along with the GDC '99 talk, Introduction to Parametric Surfaces.
This file is intended to give the different demos some kind of order and explain which
each one is.  Unfortunately, I haven't made makefiles for these projects yet, since I
use MSVC pretty exclusively.  If you really, really want makefiles for them, email me.
If the demand exceeds some arbitrary number, I'll make makefiles and let everyone who
emailed me know.  Until then, I've included the MSVC curves.dsw workspace file along 
with *.dsp project files for each project.

======================================================================================
hermiteFD:
======================================================================================
This demo is a small-source demo of hermite curves using forward differences.  It 
chooses random endpoints and random tangent vectors and draws the curve based on that.
Keys are as follows:

P: toggle between line, and point mode for the curve.
+/-: increase / decrease the tessellation level of the curve.
[/]: increase / decrease the tangent vector lengths.
R: generate another random curve.
F: flip the direction of the tangent vectors.
</>: rotate the curve in space.

======================================================================================
decastel:
======================================================================================
This demo is a small-source demo of bezier curves using de Casteljau's recursive 
algorithm.  It chooses random control points and draws the curve based on that. Keys 
are as follows:

P: toggle between line, and point mode for the curve.
+/-: increase / decrease the tessellation level of the curve.
[/]: increase / decrease the number of control points.
R: generate another random curve.
</>: rotate the curve in space.
V: draw the intermediate recursion steps in blue.

======================================================================================
bezier:
======================================================================================
This demo is a demo of bezier curves using central differences.  It chooses random 
control points and draws the curve based on that. Keys are as follows:

P: toggle between line, and point mode for the curve.
+/-: increase / decrease the tessellation level of the curve.
[/]: increase / decrease the number of control points.
R: generate another random curve.
</>: rotate the curve in space.

======================================================================================
bezsurf:
======================================================================================
This demo is a demo of a single bezier surface using central differences, texturing,
and lightmapping.  It chooses random control points and draws the surface based on 
those. The camera can be controlled by the mouse, with the left mouse button moving
forward and the right mouse button moving backwards.  Keys are as follows:

WARNING: Don't hold down the ) key, as key repeat will cause it to jack the maximum
tessellation depth through the roof, which will generally cause the demo to thrash
for an indefinite period of time as it tries to allocate phenomenal amounts of memory.

P: toggle between filled, line, and point mode for the surface.
(/): decrease / increase the maximum tessellation depth for the surface.
+/-: increase / decrease the tessellation level of the surface.
R: generate another random surface.
T: toggle base texturing.
L: toggle lightmap.

======================================================================================
bezterrain:
======================================================================================
This demo is a demo of a number of connected bezier surfaces using central 
differences, texturing, lightmapping, quadtrees, camera frustum culling, and 
lightmapping.  It chooses random control points and draws the landscape based on 
those. The camera can be controlled by the mouse, with the left mouse button moving
forward and the right mouse button moving backwards.  Keys are as follows:

WARNING: Don't hold down the ) key, as key repeat will cause it to jack the maximum
tessellation depth through the roof, which will generally cause the demo to thrash
for an indefinite period of time as it tries to allocate phenomenal amounts of memory.

P: toggle between filled and line mode for the surface.
+/-: increase / decrease the tessellation level of the surface.
R: generate another random landscape.
T: toggle base texturing.
L: toggle lightmap.

======================================================================================
marbles:
======================================================================================
This demo is a demo of a number of connected bezier surfaces using central 
differences, texturing, lightmapping, quadtrees, camera frustum culling, and 
lightmapping.  It chooses random control points and draws the landscape based on 
those. The camera can be controlled by the mouse, with the left mouse button moving
forward and the right mouse button moving backwards.  

Furthermore, it includes marbles that can be dropped onto the terrain which roll about
in a fairly accurate manner (the physics constants are a little weird, like low
gravity and whatnot).

Keys are as follows:

WARNING: Don't hold down the ) key, as key repeat will cause it to jack the maximum
tessellation depth through the roof, which will generally cause the demo to thrash
for an indefinite period of time as it tries to allocate phenomenal amounts of memory.

P: toggle between filled and line mode for the surface.
+/-: increase / decrease the tessellation level of the surface.
R: generate another random landscape.
T: toggle base texturing.
L: toggle lightmap.
M: drop a marble below the camera.
N: clear all marbles.
J: drop the camera from its current position and roll it around like it were a marble.
K: stop the camera from behaving like a marble.
H: tie the camera to the terrain's height and gradient; this is rather nauseating.
