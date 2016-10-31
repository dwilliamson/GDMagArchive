Inverse Kinematics in OpenGL Demonstration Program  July 1, 1998
----------------------------------------------------------------
v. 1.1		Sept 20, 1998

There were a couple of typos in the magazine.
Eq.1 should have been a Point P2. There was also an error
in Equation 6.  It should have reflected the equation in the code

           y(L2 cos(O2) + L1) - x(L2 sin(O2))
O1 =       -----------------------------------
           x(L2 cos(O2) + L1) + y(L2 sin(O2))

However, Eran Gottlieb sent me a message saying that the source
in the Watt & Watt Advanced Animation book had an error.
The result should have been the tan(O1). As in:

           b(L2 cos(O2) + L1) - a(L2 sin(O2))
O1 = atan  ----------------------------------
           a(L2 cos(O2) + L1) + b(L2 sin(O2))

Eran offered a pretty long algebraic proof but we can work out
an easier geometric proof of this also.

I am working from my Figure 4 in the article.

tan(O4) = L2 sin(O2) / L2 cos(O2) + L1

tan(O3) = b/a

O1 = O3 - O4

Using the Tan identity 

tan(O3 - O4) = tan(O3) - tan (O4) / (1 + tan(O3)tan(O4)

          (b/a) - (L2 sin(O2) / (L2 cos(O2) + L1)
tan(O1) = -----------------------------------------
          1 + (b/a)((L2 sin(O2) / (L2 cos(O2) + L1))

Multiply out the a

           b - a((L2 sin(O2) / (L2 cos(O2) + L1))
tan(O1) =  --------------------------------------
           a + b((L2 sin(O2) / (L2 cos(O2) + L1))

Multiply out the tan(O4)

           b(L2 cos(O2) + L1) - a(L2 sin(O2))
tan(O1) =  --------------------------------------
           a(L2 cos(O2) + L1) + b(L2 sin(O2))


           b(L2 cos(O2) + L1) - a(L2 sin(O2))
O1 = atan  --------------------------------------
           a(L2 cos(O2) + L1) + b(L2 sin(O2))


What was most interesting to me was that my application seemed to
work.  There is a very easy reason why.  It turns out that we were
working in radians.  If you compare the radian graph to the tan graph
you will see that for angle from -30 to 30 degrees or so, the values
are almost equivalent.

Since my application by the shear restrictions of the screen never
really showed the problem with the approximation.  You could see
the points diverge as the hand reached the bottom of the screen a bit.

This brings up an interesting optimization.  You can skip the ATAN under
certain circumstances.  Because of the repeating nature of TAN it would be
something like if you joint has a DOF restriction that it stays in the
range of 0-90 degrees.  When the value of tan(O1) is < 0.5 then use that
value directly and skip the ATAN call.  Similar steps can be used in
other quadrants.

Thanks to Eran Gottlieb for pointing this out. I am continually impressed
by the amount I am learning from the readers of Game Developer.

Also, thanks to Alan Watt for confirming the problem in the book.


----------------------------------------------------------------
v. 1.0

This is the sample application that accompanies the September 98
Game Developer magazine.  It is meant as a demonstration of
an analytical 2D inverse kinematics solution.  The main function
is in OGLView.cpp at the very bottom.  The function is called
"ComputeIK" and it takes a point on the screen for the articulated
arm to try and reach.

Write to me if you have problems or questions and check 
the web site or Game Developer's web site for updates.

Jeff Lander 
jeffl@darwin3d.com
www.darwin3d.com/gamedev.htm				July 1, 1998
-----------------------------------------------------------

I know this code could be optimized for maximum performance
but it was written to be a clean example without a lot of
tricks.  It should be easy to learn and build from.  

Here are the details.

I compiled the code with Visual C++ 5.0.  It has been tested
with Microsoft OpenGL, SGI OpenGL, Permidia 1 and 2 OpenGL
Drivers, Riva 128, Riva TNT, and Symetra Ultra FX Pro.

It should run on any OpenGL fully complient driver.  This 
DOES NOT include the mini-QuakeGL driver for 3DFX.  3DFX
OpenGL does not support OpenGL in a window so will not work
with this application.

There are instructions in the Help/About dialog.  

It also has an example of working with Vertex Arrays as well as
Display Lists.  I have been playing around with different variations
of interleaved arrays and found this a pretty good way to do things.

Have some fun.

Jeff