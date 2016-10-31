
3D Real-time Cave Painting Style Renderer 	May 2001
----------------------------------------------------------
v. 1.0

This demo application uses the techniques I used for cartoon rendering
to achieve a rendering in the style of a cave painting.

Like the Cartoon style renderer, a 1D texture table is used to select
regions for shading.  The table contains alpha channel values so the shading
is also blended with the background.

For the ink lines, a more sketched style is achieved by drawing the silhouette
multiple times, randomly jittering the endpoints.  To make the random lines into
continuous strokes, edges are precalculated and combined into potential strokes.
The edges are sorted in object space from top-bottom and left-right to allow them 
to be rendered in a more artistic style.  As all these calculations are made
at startup, when rendered, it is simply a matter of walking the edge list and drawing
any edges that are visible.

Manipulating the View
---------------------------------------------------------------------
LMB to Rotate view in X and Y
LMB+SHIFT to Rotate in Z

RMB to Translate in Z
RMB+SHIFT to Translate in X and Y

