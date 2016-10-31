
Real-time 3D Painting September 2000
--------------------------------------------------------
v. 1.0

This is the sample application that accompanies the September 2000
Game Developer magazine.  It is meant as a demonstration of
a method for simple painting directly on a 3D object.

Load an OBJ file with texture coordinates and a texture map. (Note: this
method works best with textures 256x256 and below). You should
then see it come up in the view.  You can turn lighting on/off via the 
menu.  Also toggle bilinear filtering with the menu.

Painting
---------------------------------------------------------------------
LMB and drag across the object to paint with the current brush
LMB+SHIFT to select the color under the mouse as the current pen color

Manipulating the View
---------------------------------------------------------------------
RMB to Rotate view in X and Y
RMB+CTRL to Translate in X and Z
RMB+SHIFT to Translate in X and Y

MMB	to Translate in X and Z

You can also change the pen color through the menu option.  Once the 
object is painted the way you like it, save the file out as a TGA to disk.

---------------------------------------------------------------------

Stuff you can do to improve:

This is a pretty simple paint system.  Quite a bit can be done to 
improve it.

1. Make the color selection a swatch that is always active.
2. Allow for different sized brushes and brush types.
3. Track the stroke pattern and then stroke the texture afterwards in a smooth manner.
4. Allow for an alpha component to the brush color then mix with the texture.
5. Add support for multiple UV channels, bump mapping, etc.
6. Improve the selection buffer so it handles large textures better.

October's issue covers mapping coordinate methods.  Part of that code is included in
this build.  You can reset the UV coordinates on the objects based on different mapping
methods.  These will be finished up next month, but for now you can check it out.  The final
will include other mapping methods as well as the ability to save the final OBJ.

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

I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, Permidia 2 OpenGL,  NVidia TNT, TNT2, GeForce256.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX Voodoo or
Voodoo 2.  3DFX Voodoo and Voodoo 2 OpenGL do not support 
OpenGL in a window so will not work with this application.

