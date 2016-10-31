When you run "unified_lod_1", you get a small demo of the
seam-matching as described in the article.  Two terrain blocks
are drawn next to each other, with a seam fill between them.
You can play with the resolution of one of the blocks and get
a feel for how the seams work.

All the keyboard commands are documented on-screen when you
run the program.

You can switch to terrain rendering mode, where you can move
a viewpoint around a precomputed terrain.  I have supplied
a preprocessed version of the Crater Lake height field,
as the file "data/crater_0513.terrain_tree".  For the terrain
rendering to work, you need to tell it which file to use
on the command line, e.g. 

    unified_lod_1.exe data\crater_0513.terrain_tree

The Visual C++ project is set up to do this already if you
run from the IDE.

This demo is not yet supposed to be impressive.
The terrain is monotextured and you will see popping as you
move around.  Keep in mind that the code will be upgraded
every month until it handles polygon soup environments 
without popping.  The main goal of this first demo is
to provide a starting point, and to provide a simple-as-
possible view of how some of the systems work (like the
detail reducer) before things become cloudy.

Even though the terrain is rendered as static blocks, I
am not using vertex buffers for them.  Right now I am
rendering them in pretty much the slowest possible way,
i.e. calling glVertex() a bunch of times.  You could
fix this pretty easily, but I figure I will wait
until the final version before doing low-level performance
tweaks like that.  The important thing to remember is
that because they are static blocks, they *can* be
rendered very efficiently as vertex buffers should we
choose to do so.

The way I output vertex normals may confuse some people.
You'll see that I don't store a surface normal at every
vertex in the mesh data structure; instead, I store a
tangent frame, represented as a quaternion.  This
quaternion contains all the information you need to 
compute the normal, tangent, and binormal vectors. 
I use quaternions because they are small, they are nice
to interpolate, and they can be used directly in high-end
shaders to produce anisotropic lighting effects.  For
now though, because we're only doing DX8-level stuff,
I extract the vertex normal manually, by rotating the
Z-axis (0, 0, 1) by the quaternion.  This is not very
fast as I've implemented it, but you can make it
much faster using a specialized function made to 
extract the rotated Z-axis from a quaternion.  I talk
about the math behind this in my Game Developer article
"Inverse Kinematics with Joint Limits" [basically, write
out the algebra for (q)z(q*) and simplify].  I will
probably make this upgrade for next month's code.


I have included a utility "preprocess_terrain" that 
generates .terrain_tree files.  If you run it with
no arguments, it will generate a simple sine-wave
terrain.  If you give it the name of a .bt file as
an argument, it will parse that .bt file and preprocess
it appropriately.  For more information on the .bt
file format, see http://vterrain.org.  I have supplied
the original .bt file for the Crater Lake map in case
you want to make changes to the preprocessor or mesh
format.

All this data made the download pretty large; sorry
about that.  At some point maybe I will do an article
about mesh compression.  (Quaternions as tangent frames
compress very nicely, I'll have you know!)

I originally wrote this code in my personal game engine.
To port this into an OpenGL-based open source app,
I extracted some of my game engine source, simplified it
some, and put it in the subdirectories 'framework'
and 'mesh'.  'framework' is basically all supporting code 
that doesn't have much to do with the basic algorithm.  
'mesh' is supporting code that *does* have to do with the
basic algorithm (for example, it contains the mesh
simplification code).


Because Windows XP has lousy scheduling, the frame rate
in this app is a bit skippy sometimes under XP.  I don't
know how to fix this without doing relatively convoluted
and disgusting things that have no place in a demo...
so we get to live with that.  Windows XP ... 10 millisecond
timeslices, my ass.


    Jonathan Blow
    March 14, 2003
    Jo's Cafe in Austin, Texas, where some obnoxious
        DJ is playing loud crap as part of SXSW.


