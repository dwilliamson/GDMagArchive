Photorealistic Real-Time Outdoor Light Scattering.
Game Developer Magazine, August 2002

Naty Hoffman (Westwood Studios) & 
Arcot Preetham (ATI Research)
August 2002.

----------------------------------------------------------------------

Acknowledgements:
We would like to thank Kenny Mitchell (Westwood Studios) for the terrain 
engine and Solomon Srinivasan (Paraform Inc) for help with the creating 
the demo.

----------------------------------------------------------------------

Requirements:
1. A Vertex Shader 1.0 & Pixel Shader 1.0 compliant hardware.
2. DX8

----------------------------------------------------------------------

To Run:
Launch the app and hit Play on the menu to run the demo.
Alternatively, you could also play with the sliders to get a feel and 
understand the features of the scattering model.

----------------------------------------------------------------------
Controls:

Sliders:
 Sun
  Position: Move the sun in the east-west plane. Slider midpoint denotes 
            zenith position.
  Intensity: Scale factor for sun intensity.
 Mie Scattering 
  Scattering Coefficient: Scale factor for Mie scattering coefficient
  Directional nature (g): Henyey - Greenstein factor for Mie scattering
 Rayleigh Scattering
  Scattering Coefficient: Scale factor for Rayleigh scattering coefficient
 Inscattering: Scale factor for amount of inscattered light into viewing 
               ray.

Note: You probably would want to move the sun slider first to a midday 
position to see something on the screen.

Keys: 
Maneuver camera with the following keys.
 Up, Down - Move forward/backward
 Left,Right - Yaw
 PgUp,PgDn - Pitch
 Home,End - Move up/down
 F9,F11 - Roll
 F5,F6 - Stepsize decr/incr

----------------------------------------------------------------------

Bugs:
The cursor keys for camera movement affect the sliders.


----------------------------------------------------------------------
