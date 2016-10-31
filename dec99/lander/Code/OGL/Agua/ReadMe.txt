Agua Application		12/99
----------------------------------------------

This is the Companion application to the December 1999
Game Developer magazine.

It implements an image processing method for creating
a rippling water surface.

This app does not require 3D hardware.

If it runs too slow, make the size of the height field
smaller by changing the WATER_SIZE define in AguaDlg.cpp
Try 128.  Especially if you consider turning this into
a 3D height field, you will want a smaller field.  

The app is a Dialog based MFC app.

Suggestions for playing around
------------------------------

Experiment with changes to the Process water routine
in AguaDlg.cpp.  This is the actual convolution that 
creates the effect.  There are a lot of variations that
produce different effects.

Optimize...  I didn't spend much time making it fast.
Indexing tricks (especially in a 256x256 field) would
help as well as some assembly here and there.

Take the height field and convert it to a 3D mesh. I
would use the 256x256 as a texture but make the resolution
of the 3D mesh something like 64x64.


Credits
-------------------------------

Since I could not effectively figure out a original 
source for this technique, I have received some mail
from people claiming to be the first.  I will compile
a list of these on my website www.darwin3d.com/gamedev.htm.

Should be fun to try different people's variations.


Jeff
jeffl@darwin3d.com


