
3D Real-time Cartoon Rendering Textured Update August 2000
----------------------------------------------------------
v. 2.0

This demo makes use of multi-texture hardware support in OpenGL to add
texturing to the cartoon renderer.  It was featured in the July 2000 issue
of Game Developer Magazine.

The idea is that the simple cartoon shading from February isn't too much
detail.  I would like to add a texture pass as well.  I could have simply
used multi-texture to modulate the cartoon shade with the texture but to
make things more interesting, I am using two base textures.

The cartoon shade table now has an alpha component for each shade.  This
is used to select between the two base textures.  When ARB_multitexture is
supported, the renderer will make two multi-texture passes.

Pass 1.  
The object is drawn with the "dark" texture modulated by the cartoon shade.
Blend is set to glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA) and alpha
test is set so it only draws when the alpha component is 0.0f.

Pass 2.  
The object is drawn with the "light" texture modulated by the cartoon shade.
Blend is set to glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) and alpha
test is set so it draws when the alpha component is > 0.0f.

Discussion:

The alpha test prevents overdraw and z-fighting. It could be done without the
alpha test or even with alpha values other then 0 and 1.  It may be interesting
to have the two passes smoothly blend together rather then a cut. 

One new hardware (such as the ATI Radeon) that supports 3 texture multitexture,
all these passes could be done in a single pass.  Once I get a Radeon, I will
give it a try.

Each model is in an OBJ file.  The two textures are designated in the .MTL file
for each OBJ.  The Pass 1 texture is the map_Kd and Pass 2 is map_Ks.  To try
different textures, just change the MTL text file.  Since OBJ format varies a
bunch, you may need to massage the file or my code to get your models in.  It 
only handles triangle models right now for ease of code.

Next, I want to work on good ways for automatically generating those sketch style
textures so that they follow the model contours.


v. 1.0

This is the sample application that accompanies the February 2000
Game Developer magazine.  It is meant as a demonstration of
a method for Non-photorealistic cartoon rendering.

The February issue didn't really do any cartoon shading so there
wasn't much to do.  Just the silhouette lines.  Lots of people wanted
the app before the March issue though so I am releasing it.

But since you haven't seen the March issue yet, you won't know
about the shading method.  So, here it is in a nutshell:

I use a 1D texture as a non-linear shading function.  The dot product
of the light vector and the surface normal is used to calculate which
entry to grab out of that table.  This gets rid of the Gouraud shaded
look and allows me to be very flexible on the shading.

For this demo, there are 32 entries in the shade table.  This may not
be enough precision for some shaders.  If you look at the "gradient.shd"
which is a smooth gradient, you will see the banding.  This is also
because the bilinear filtering is turned off also.  If it was on,
the shader wouldn't look like a Toon.

You can edit the .SHD files and create your own settings.  They are
simple text files.  

Model files are Wavefront OBJs.  I get the model base color from the
Diffuse color in the MTL file.

Use settings to change the line color, width and light direction variables.
Anti-Alias may really slow things down on some OpenGL implementations.
This anti-aliases the sihouette lines.

Manipulating the View
---------------------------------------------------------------------
LMB to Rotate view in X and Y
LMB+SHIFT to Rotate in Z

RMB to Translate in Z
RMB+SHIFT to Translate in X and Y

---------------------------------------------------------------------

Stuff you can do to improve:

Add support for multiple materials and then add in material lines.
Add blending with a texture.

---------------------------------------------------------------------

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, Riva 128 (New Beta OGL Drivers), AccelGalaxy, 
and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX Voodoo or
Voodoo 2.  3DFX Voodoo and Voodoo 2 OpenGL do not support 
OpenGL in a window so will not work with this application.

