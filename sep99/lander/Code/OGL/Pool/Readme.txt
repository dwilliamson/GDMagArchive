3D Billiard Physicls			Sept 1999
--------------------------------------------------------
v. 1.0  

I know this is very late and to top that off it is a work in progress.
Too much real work got in the way of my demos and I got way behind.
The goal was to create a more game like application instead of the 
standard tool type MFC interface project.  So, I stripped everything out
of the article test app and put a new face on it.

The program uses a dynamic simulation to handle billiard ball collisions
and friction.  One aspect missing right now is the rotational physics.
I decided I wanted to change the matrix method used in the article to
a quaternion model which will be better for future stuff.  I haven't yet
had time to integrate all that code in.  I will post the update as soon 
as all that is in but since people have been asking for what I have...

I wanted to make the geometry format generic but it was too big a pain. 
So it is my own custom.  You would have to provide your own models if
you want to change it.  At least you can change the textures.

The balls and cue stick are in the Model.h file as ascii data.  The 
world is in a proprietary format.  No biggy and not real efficient
but it works.  Handles both tris and quads in two seperate data lists.

Thanks to my sister, Christine Lander, for creating the meshes to give
the app some pizzazz.

I plan on continuing to update this more once I get all caught up.  I
was thinking I may release a full billiards game with source to the
community for Christmas.  At least that gives me a goal.

Hope you enjoy. It seems like a pretty cool start to something.
Email me any problems or suggestions. 

Jeff

How it works.

	Click and drag the Left Mouse Button to line up the cue.

	Click and drag the Right Mouse Button to pull back the stick and drag
	forward to shoot.


Problems for you to explore:

	Make this into a game....  You know with pockets and score and sound...

	Add some more balls.

	Tackle integrating the rotational effects.  It is 90% there already.

	Bumper Collision induced Spin

	Ball to Ball spin transfer

	Add the 3D component to the physics so you can do jumps.

	No solution for tunneling.  If you hit things too hard ball-ball collisions may miss.


Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				
-----------------------------------------------------------

I compiled the code with Visual C++ 6.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL for Windows, Permidia 1 and 2 OpenGL
Drivers, Riva 128, TNT, TNT2, AccelGalaxy, 
and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX Voodoo series. 
3DFX OpenGL that does not support OpenGL in a window will not work
with this application.

