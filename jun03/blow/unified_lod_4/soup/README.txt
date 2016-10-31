Here's the source code for the triangle soup part of 
Unified Rendering LOD #4.

It consists of two programs: "preprocess_world" reads
in a mesh and builds a hierarchical LOD set, writing
out the result; "soup" renders that result.

Both of these programs have some hardcoded defaults 
about what files they try to load.  When you
run "preprocess_world", it is hardcoded to read the
file "data/bunny16000.txt" and chop it with some
also-hardcoded splitting planes.  It writes
out the file "output.world_lod", which may take
significant time to generate on a slow processor, or
if you don't have much RAM (especially if you are
running the debug build).

When you want to display a preprocessed world mesh,
run "soup".  If you run "soup" without arguments, it
tries to load "data/bunny.world_lod".  You can override
that by passing an alternative filename as the first
argument.

Still don't pay too much attention to the frame rate
as "soup" still doesn't use vertex buffers to store the
geometry.  I will get off my butt and do this for next
time.  Hopefully.

The user interface to "soup" is not fully baked, it's the same
interface as for the big terrain, which is not necessarily
what you want when looking at a bunny.  Also, the
bunny won't hierarchically LOD nicely as you move away from
it... that is being saved for next month's code.  The main
point of 'soup' is to visualize the output of the preprocessor,
which has chopped up some pieces, recombined them, and used the
seam database to ensure continuity.

* Hit 'E' to explode the pieces apart to clearly show the seams,
as in the pictures in the article.  You may want to turn on
obvious seams ('O').

* To see all the seams without exploding the pieces apart,
turn on wireframe ('W') then turn off drawing of solid blocks ('S').

* The main source files to look at for an executive summary are:
preprocess_world.cpp (main part of preprocess_world.exe) and
main.cpp (main part of soup.exe).

    Jonathan Blow
    jon@number-none.com
    May 27, 2003
    Spider House Cafe in Austin, Texas, USA



