This month's code is an OpenGL app that runs poorly-coded
relaxation processes on points in the plane, and on unit
vectors on the sphere.

The 2D simulation was not a high-effort endeavor; the goal
was just to illustrate the general idea behind the hexagonal
distribution being the "natural" state for points repelling
each other in the plane.  Because the simulation is not very
good (it's unstable around the edges, for reasons that I don't
quite understand right now; also, it doesn't converge very
quickly once it gets near the answer), it never quite draws
a perfect hex grid.  You can see the pattern forming, though.
Hit the 'C' key to toggle the display of heuristic hex boundaries
(this is off by default because it makes the simulation slower,
but turning it on really shows you a lot more about what is
happening).  

For an interesting way of indexing coordinates in the plane into
a hex grid, read the file "hex_indexing.txt".

The 3D unit vector simulation works more smoothly than the 2D
one.  It also converges very slowly once it gets near the right
answer, though.  But at any time you can save out the results
by hitting 's'.  The output of this program is directly usable
for production game purposes (I have done so myself, anyway).

In the subdirectory "unit_vector_test" is a small program that
reads the output of the 3D simulation and tests it against
random input vectors in order to measure how good the quantization
is, and to verify that everything is working.


Controls for the main app: 

'1': Start 2D relaxation subroutine
'2': Start 3D unit vector relaxation subroutine

'Q', 'W': Decrease and increase the simulation timestep.
    I don't actually draw the timestep right now.  Oops sorry. 
    You probably don't want to mess with this too much since
    hitting 'W' a lot will make the simulation unstable.

'C': Toggle drawing hex borders in the 2D simulation.


   -Jonathan (jon@bolt-action.com)
