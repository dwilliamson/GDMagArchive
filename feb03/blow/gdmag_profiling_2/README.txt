Okay, first we'll start with the README from the first
version of the profiling code, since it's all still true:


----------------------------------

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

----------------------------------


This newer version uses a Kohonen Self-Organizing Feature
Map to classify behaviors in the system.  This classification
work is experimental.  I wouldn't tout it as "production
quality", but if you are interested in visualizing the
behavior of your game you might find it a useful first
step.

In addition to the main program zones from last month's
code, there's also a "draw_lens_flare" zone.  Point the
camera toward the sun in order to spike activity in
this zone.



The rdtsc call in profiler_lowlevel.h has been modified to
go faster, thanks to feedback from Paul Du Bois.

Paul, as well as Sean Barrett, pointed out that the
lowlevel profiling code doesn't correctly deal with
recursive / reentrant routines.  If you try to use it
on such routines, the numbers will get all screwy.
I had wanted to fix this before putting up the code,
but as often happens in life, I ran out of time.  Sometime
in the future, I'd like to do a follow-up to these columns,
and it'll be fixed then (and there will be lots of other
improvements as well).

Sean Barrett used some of this code and made a profiling
system that handles reentrancy correctly.  It also contains 
some nice declaration features like easier-to-define
"private" profiling zones, and supports some extra
hierarchical data-gathering abilities.  And for an extra bonus 
it can be used from C as well as C++.  Check it out at
http://silverspaceship.com/src/iprof/.




How to use the feature map:

When you press the spacebar to enable mouselook and start
moving around, a blue grid will appear and start filling up.
This is the Feature Map.  Pressing spacebar again (to turn
mouselook off) will pause the feature map's data gathering.
You can mouse over the individual squares in the map to
see information about the "typical profile" each square
represents.  Clicking on a map square will give you two 
color-gradient versions of the map; these represent the map
as seen from the relative point of view of the square you
clicked on.  The grayscale one just talks about how far
away the other nodes are from the candidate; the colored
one finds an approximation to the principal components of
the other squares' spread with respect to the candidate,
then assigns the three highest components to red, green,
and blue, then mixes the colors.  (If there aren't 3
principal components with big magnitudes, as often happens,
then fewer colors will be used).  Below this readout,
in the same colors used for the principal component graph,
you'll get a colored profile line telling you what each
gradient vector is.  Not sure how useful these are yet.

At the same time you clicked on the map, a bunch of
grayscale maps appeared at the top of the screen.  
Each one of these maps corresponds to a single program
zone (the name of the zone is drawn below the map).  Each
square is colored by the importance a particular zone
plays in the profile for that map square.  So in the
grayscale map for "draw_entities", white squares represent
behaviors that spent a lot of time drawing entities; black
squares represent behaviors that spent no time drawing
entities.  

You can search for correlations between behaviors by
clicking on these grayscale maps.  For example to find
a set of behaviors that spent a lot of time in
"draw_entities" and also a lot of time in "draw_lens_flare",
just click on both of the appropriate grayscale maps
and you'll get a composite map representing the joint
occurrence.  Clicking toggles the presence of a zone
in the composite, so you can add and remove zones at will.

The intensities of these grayscale maps are normalized,
so they can't be compared to each other; the behavior
that draws the most entities will always be white in the
draw_entities map, and the behavior that draws the most
lens flare will always be white in the draw_lens_flare
map, even if draw_lens_flare takes twice as much time
as draw_entities.



Lens flare textures by Jim Scott:
http://www.blackpawn.com/texts/lensflare/default.html



    Jonathan Blow
    jon@number-none.com
