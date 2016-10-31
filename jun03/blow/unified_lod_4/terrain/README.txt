Here's the source code for the terrain rendering part of 
Unified Rendering LOD #4.

It consists of two programs: "preprocess_terrain" reads
in a .bt file and builds a hierarchical LOD set, writing
out the result; "terrain" renders that result.

When you run "preprocess_terrain", give it as a command-line
argument the name of the bt file to load (for example,
"data/bt_files/crater_0513.bt".  It will produce as output
a file called "output.terrain_tree" (there is no option
to change the output filename).

A preprocessed version of crater_0513.bt has already been
provided; it's "data/crater.terrain_tree".  If you run
"terrain" with this as the argument, it'll load it up and
render it for you.  You can navigate the viewpoint around
and see how it looks.

Don't pay too much attention to the frame rate since
this renderer still doesn't use vertex buffers to store the
geometry.  That will happen in a future version.

Most of the interesting commands are documented with on-screen
prompts...

The main source files to look at for an overview of what's
happening are: preprocess_terrain.cpp (main part of 
preprocess_terrain.exe) and main.cpp (main part of terrain.exe).

    Jonathan Blow
    jon@number-none.com
    May 28, 2003
    Flight Path Cafe in Austin, Texas, USA



