So this is a BRDF demo or something.

This program is based on Jan Kautz's research code.  The win32 
implementation (by Jonathan Blow) was made to show that BRDF
rendering can be done very quickly on modern game PCs.  The
program contains some OpenGL initialization code that was written
by Jeff Lander.  Thanks Jeff!


To run this demo you should have recent and stable OpenGL drivers
for your 3D graphics card.  If you want to update your drivers,
go to glsetup.com.  They'll take care of you.

I recommend running this demo on a GeForce card or an ATI Radeon.
The Radeon seems to have slightly higher color quality, but the
GeForce spanks it in terms of rendering speed.  The demo will
run on an ATI Rage 128 but there it's just too slow to be any
fun at high resolutions.  You can run it on a Voodoo 5 but 
the image quality is a tad lacking (you'll see some banding).
It will run on lots of other graphics cards (TNT, Voodoo 3,
Bitchin'fast!3D^2000) but don't expect good results as we really
do need a card with a lot of color accuracy for this demo to
have a point.

There are some sucky things about this demo because it was just
kind of slapped together.  Blame all that on me (Jonathan Blow).
The most obviously suckful are the fact that you can't freely
rotate or control the object and the light source.  But hey,
this is a rendering demo, not a user interface demo.

The demo also spends a long time starting up because it does a
lot of unnecessary and slow things to load texture maps and create
3D models.  These have nothing to do with the BRDF technique
itself; the BRDFs are represented by ordinary-sized texture maps
and will load just as fast as you could make any texture map load.

The demo will crash without warning if it can't find any of its
data files, so just run it from the directory where all those
files are, okay?


Mipmapping is currently disabled because with the glu implementation
I have, it looks like crap.  It's all still in the code though,
so you can turn it on and see what it does.  Look for the variable
"ALLOW_MIPMAPPING".


You can control the width and height of the rendering window
from the command line, like this:  

      brdf_demo.exe width=800 height=600

The default window dimensions are 1152x864 which might be a little
big for your taste.  I don't know.


Comments, questions, etc can be sent to me.  Have another sip of 
thine mead; thank-you and have a nice day.


    Jonathan Blow
    jon@bolt-action.com