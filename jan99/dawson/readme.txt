This directory contains three test applications and an structured exception
handling routine in a library. You can examine the source to the test
applications to see how to hook in a structured exception handler to
catch errors, and you can run the programs to see the types of error
logs that are created.

The simplest application to run in order to generate error logs is the
MFC application, mfctest.

All code and executables are copyright © 1998 Bruce Dawson.
You may use them in your own projects as long as the
copyright notice is retained in the source code.

The exception handler and the associated test programs were created
with VisualC++ 5.0 and can be loaded as a single workspace. The code,
particularly the exception handler, should be reasonably portable to
other compilers for the Win32 x86 platform.
