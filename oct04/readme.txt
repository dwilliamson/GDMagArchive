Inner Product Source Code -- October 2004 

This is the source and executable accompanying
"Hybrid Procedural Texturing". The source textures
are all in Photoshop PSD format, in some cases still
with their layers intact. (I flattened some to reduce
the size.)

The executable requires NVIDIA hardware (GF3+) to run,
since it uses register combiners rather than pixel
shaders. 

The user interface for maneuvering the camera
is an extremely painful one (one that I've been
using for 10 years and seems easy to me). Each
of the keys below imparts a little translational
or angular momentum, so hit them repeatedly (or hold
them down for auto key repeat) until you're moving
as fast as you want. Space bar stops all movement.

  W    forward
  X    backward
  Z    sidestep left
  C    sidestep right
  Q    up
  E    down

  A    turn left
  D    turn right
  R    tilt up
  V    tilt down

You begin facing the brick wall, hovering over the
asphalt. To the left you will find a horribly unsuccessful
attempt at creating a mud-crack texture hybrid.
To the right are two attempts at synthesizing grass
textures; there are still obvious patterns in the
base grass texturing showing. 180 degrees behind the initial
position is another brick wall, this one done for
older hardware--it uses two textures and no special
pixel blending operations (just modulate).

To compile you'll need NVIDIA's nvparse library.
