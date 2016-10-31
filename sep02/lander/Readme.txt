Rope Bridge Sim				August 2002
--------------------------------------------------------
Jeff Lander
Darwin 3D, LCC
jeffl@darwin3d.com
v. 1.0  

Right-mouse click and drag along screen to set ground depth.
Left-mouse Click to add a sim point or select an existing point

Menu options
------------

File:
New					-	Start with a new simulation field
Load Simulation		-	Load simulation file and settings
Save Simulation		-	Save simulation file and settings

View:
Springs				-	Display the springs
CVs					-	View control vertices

Physics:
Friction			-	Not used in this model, for boarder
Gravity				-	Is gravity active (it should be :)
Grid Snap			-	Aids in creation of meshes, snapped to 20x20 pixels
Sim Running "S"		-	Start/Stop the Sim
Reset "R"			-	Start the sim over
Add Spring "Enter"	-	Adds a spring between two selected points
Properties	"P"		-	Enter simulation properties


About the Sim
-------------

This application simulates point masses connected by springs.  
The spring stiffness and damping can be set along with the Young modulus
which is used to determine the stress on any given spring.  A threshold 
can be specified at which the springs will break.

Computed stress is marked by "reddenning" of the spring lines.

By adjusting the system properties, it is possible to make objects with
a great variety of behaviors, from stiff and brittle to flexible and forgiving.

About the Code
--------------

Simple Windows Opengl application.  The main physics part of the sim is in
SpringSim.cpp.


