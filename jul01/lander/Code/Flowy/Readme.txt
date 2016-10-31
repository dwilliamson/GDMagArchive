Flowy Liquid Flow Simulator             July-August 2001
--------------------------------------------------------
v. 1.0  
Jeff Lander             jeffl@darwin3d.com
Darwin 3D, LLC          www.darwin3d.com

This simulator implements the basic fluid flow elements in the July/August Game Developer Magazine.
The sample allows you to build up a set of basic fluid elements and combine them to create a
potential flow field.  Particles can then be traced through the field by dropping them with the mouse or
having them emitted from fluid elements.

This code draws the particles as simple colored tristrips.  For my production work, I combine particles
into streams and use textures and blending to make effects.  However, the interface is much more complicated
and messy then presented here.  This sample can serve as a basis for using potential flow fields to generate
organic looks for particle systems, texture creation, or any other fluid looking effects.


Controls
--------
Left Mouse Button and drag      Drop particles into the simulator
Right Mouse Button              Create a new fluid element at the clicked location or drag an existing element

To change the element types, use the Flow Elements\Edit Element menu.


Flow element types are:

Uniform     (Symbolized by an Arrow in the flow area)
    Creates a uniform potential flow across the entire field.
    Size specifies the force of the flow
    Direction is direction in degrees with 0 = North  90 = East ...

Source      (Yellow Circle)
    Flow that radiates outword from the center
    Size is area of influence 
    Strength is strength of the flow

Vortex      (Cyan Circle)
    Create a flow in a clockwise direction around the center
    Size is area of influence 
    Strength is strength of the flow (make negative for counter-clockwise)

Doublet     (Green Circle)
    Create a doublet flow around the center
    Size is area of influence 
    Strength is strength of the flow


When Emit Particles is checked, particles will be randomly dispersed in the elements radius and then be effected by
the flow.


Samples
-------
Default.sim     Three source emitters, a uniform flow East, with a source and doublet in the center and two vorices.
Bubbles.sim     Many source emitters, a uniform flow North and obstacles along the path
Cyclone.sim     A center source emitter surrounded by 4 vortices
PaintSwirl.sim  Two source emitters with a fast vortex. Drag the vortex with RMB to swirl things around.  Or paint with LMB


Todo
----

Lots of possibilities in this app.  

Add colored and/or textures to the particle quads.  
Connect multiple particles to create complex shapes and blend them together
Extend potential flow elements to 3D and create smoke.  The paintswirl sim shows how an object can be made to
    disrupt a flow field like a person waving their hand through smoke.


Questions or ideas send to jeffl@darwin3d.com
