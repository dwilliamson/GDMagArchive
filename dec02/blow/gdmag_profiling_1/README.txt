This is sample code for a simple interactive profiler.  The
profiler is self-contained in the subdirectory "profiler",
so that you can use it in your own code.  The rendering of
the profiler is separated into its own file, so that you can
easily make a Direct3D or GX version or something.

So that the profile has something to measure, there's a
simple game world full of crates that you can move around in.
Crates in the world are frustum-culled, so as you turn the
viewpoint you will see a variation in the frame time
required to render the scene, and in the proportion of the
frame time that is used to render entities.  You'll also
see that one of the other game systems, "ai_pathfind", has
highly variable CPU usage.


Controls:
    '1': Display "self time" profile.
    '2': Display "self stdev" profile.
    '3': Display "hierarchical time" profile.
    '4': Display "hierarchical stdev" profile.
    '0': Toggle the fluctuation in CPU usage of the ai_pathfind
         routine.  This will help show how the profiler responds
         to changing CPU usages.

    Spacebar: Toggle mouselook.
    W, S, A, D: Move forward, backward, left, right.
    Arrow keys: Move forward, backward, left, right.

