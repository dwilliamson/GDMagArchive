Okay, here we are with the second installment of this
environmental LOD series, "unified_lod_2".  If you didn't
download and look at "unified_lod_1" first, you might want
to do that, since it is a little bit simpler.

This month the code drops you straight into terrain rendering
mode.  All the keyboard commands are documented on-screen.

The main difference between this month's code and last month's
is that alpha blending is now being used to eliminate popping.
However, there are some other minor differences -- the file 
format is slightly different, for example.  So the two demos
don't interoperate.

I did some refactoring of this code after writing
the corresponding article.  Mainly, I changed the order in
which things are drawn.  Now the solid version of each block
in the terrain is drawn first; then in a second pass the
transitioning blocks get mixed into the final image.  If you 
want to see the alpha-blending work very clearly, then hit
'S' to turn off the rendering of solid blocks.  This will
also show you how much fill-rate overhead there is in the
blending step; and really, there isn't all that much.  
I think in the article I might have given the impression
that a lot of extra fill was required (an impression aided
by the fact that there was a layout error in the article,
and the wrong image was given in one of the figures, so it
looks like more pixels are being filled than actually are).


Don't pay too much attention to the frame rate yet, since
absolutely no optimization has been done (and the system
will still change significantly before it is complete).


[Below are some notes pasted from unified_lod_1, but which
are just as relevant here]:


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
    March 19, 2003
    Bouldin Creek Cafe in Austin, Texas



