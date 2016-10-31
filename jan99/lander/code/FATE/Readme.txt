2D Collsion Detection in Real-time Demonstration Dec 5, 1998
----------------------------------------------------------------------------

This is the sample application that accompanies the January 99
Game Developer magazine.  It is meant as a demonstration of
a method for 2D collision detection.

This code is pulled from a portion of a "Duke Nukem" style
3D Engine level editor I wrote a while back. There are a bunch 
of routines that editor writers may find useful.  The math is
handled in Fixed Point trig which I don't use much anymore.
There is also some leftovers of sector structure for floor
and ceiling height that are not real important for the demo.

Most of the interesting Computational Geometry routines are in
"MainFrame.cpp" at the bottom of the file.  There are routines to
find the nearest point to a line segment and to check if a point
is in a concave polygon.  All of the Drawing code just uses WinGDI
calls so no extra libraries are required.

Load one of the sample files to check it out.

Controls for the Application:
-----------------------------------------------------------------------------
Left/Right Arrow Keys -		Rotate the player viewpoint on the map
Up/Down Arrow Keys -		Move player forward and backward.
Right Mouse Button -		Places the player on the map

Ctrl + Left Mouse Button	Adds a point to the current polygon.

	Create closed polygon sectors by CTRL+LMB on the map.  Create the
	polygons clockwise and hit on the first vertex to close the sector.
	A new sector can be attached to the old one.  Shared edges are 
	considered "passable" by the player and are colored red.

Click and Drag a vertex with the Left mouse button to move a vertex.

+ / - (Keypad)	Increases and Decreases the Grid Resolution
A / Z			Zoom the map in and out
S				Turns Grid Snap On and Off

Double Click Mouse	-		Tests whether the clicked point is inside a sector.
-----------------------------------------------------------------------------

There is a bunch of other functions that may be useful to level editor 
creators.  It is largely leftover from production tools so it is up to you
to figure it out or not.  The point was really the collision and the 
inside sector test.

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm	
-----------------------------------------------------------

