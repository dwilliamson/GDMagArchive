Dx8 Vertex Shaders Sample Application
-------------------------------------
Graphic Content Column, March 2001
Game Developer Magazine


This application is a recoding of my OpenGL "Loony" cartoon shader application
using DirectX8 and the new vertex shaders.

There are a few .OBJ models that you can try out included.  I didn't add a load
menu option so changing models requires a recompile.  The load menu was just a pain
because I would need to rebuild the vertex buffers so I will leave that as an exercise
for the user.  Any triangulated model with normal info should work in the obj reader.
However, my reader isn't very robust so it may need some debugging to make work with 
any old model.


I ripped a lot of the base DX8 setup code from a NVidia demo that used the same method
(I think Cem Cebenoyan wrote it though it may have been based on DX8 sample code).  I
changed it a bunch to suit where I wanted to start building and to make it more compatible
with my other work.  This will serve as the base application for next months vertex shader
skinning application.

Obviously the app requires DX8 be installed on your machine. www.microsoft.com to get it.


Email any problems to me as I will be trying to make it a more robust stable app to build
other DX8 projects.

-Jeff
jeffl@darwin3d.com


