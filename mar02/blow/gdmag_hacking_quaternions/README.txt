This source code accompanies the article "Hacking Quaternions", 
The Inner Product, by Jonathan Blow, Game Developer Magazine, 
March 2002.  The article may help significantly in figuring
out just what is going on here and why this all works.

The implementation-minded can go directly to quasi_slerp.h, which
contains all the code you need for the fast slerp approximation.
quasi_slerp.cpp uses the approximations and compares their accuracy
to the full slerp.

If you want to see how the numbers in quasi_slerp.h were cooked
up, and maybe tune some new ones for your particular application,
see main.cpp.  This file contains all the numerical optimization
and graphing code.



Here are the controls for this program:

    'A': Start angle-error optimization mode (the default).
    'L': Start length-error optimization mode.
    'Q': Start the interpolator comparison mode.  This compares the
         error performance of slerp, lerp, normalized lerp, and the full
         quasi-slerp.  While in this mode press 'R' to roll up new 
         random quaternions for the interpolation endpoints.

    '1': Draw figure 1.
    '2': Draw figure 2.
    '3': Draw figure 3.
    '4': Draw figure 4.
    '5': Draw figure 5.




Jonathan Blow (jon@bolt-action.com)
February 10, 2002
