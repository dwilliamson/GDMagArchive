Dagger Motion Capture File Viewer		Dec 16, 1997
-----------------------------------------------------

Well the issue release snuck up on me fast.

This is the sample application that accompanies the Jan 98
Game Developer magazine.  It is meant as a demonstration of
how to load and display Motion Capture data via standard
file formats.

First off, I am pulling out this code from my existing tools
and converting them to a nice clean OpenGL application.  I am
a little behind in finishing this.  I have only converted the
BVA loader code.  I will convert my BVH and AMC loaders as
soon as I can and upload them to my website as well as
Game Developer.  In any case, this should be a great start to
working with motion data.

Thanks again to House of Moves and Biovision for providing
sample motion files to work with.  Write to me if you have
problems or questions and check the web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				Dec. 18, 1997
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

There are instructions in the Help/About dialog.  

Use Frame/Load Animation to bring in a BVA file.  You can
use the VCR style controls to play it back.  Hold the SHIFT
with the mouse buttons to rotate the view or CTRL to 
translate the view around.

The Hierarchy window (HierWin.CPP/H) can be used to select a Bone.
Double click on it to bring up the edit window.  This allows you 
to change the bone transformation settings.  Playing the animation 
loses those changes.

I started implementing an Add bone command but it is not complete
and commented out.

In the article I described using Windows Timers to play the animation.
When I finished my production tool, I found that it could not play
back as fast as I thought it should.  It turned out the WinTimer was
a real slowdown to the system.  So, I yanked it out and grabbed the
message handler and run the animation from there.  That improved
things greatly.  I know I could even get it a bit faster but it
wouldn't be as windows friendly.

Have some fun.

