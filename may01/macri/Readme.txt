// Copyright (c) 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative
// works of this software for any purpose and without fee, provided, that
// the above copyright notice and this statement appear in all copies.
// Intel makes no representations about the suitability of this software
// for any purpose.  This software is provided "AS IS."
//
// Intel specifically disclaims all warranties, express or implied, and
// all liability, including consequential and other indirect damages, for
// the use of this software, including liability for infringement of any
// proprietary rights, and including the warranties of merchantability and
// fitness for a particular purpose.  Intel does not assume any
// responsibility for any errors which may appear in this software nor any
// responsibility to update it.

AGP Performance Testing Application
Written by Dean P. Macri (dean.p.macri@intel.com)

This program demonstrates the effects of partial writes of write-combining
buffers.  To use the program, run "AGP_Performance.exe" on a system with a
graphics card that supports hardware transformation and lighting.  You'll
see a sphere in the center with some text describing how many floats are
being touched every frame and whether or not every vertex is being modified.
The current frame rate (FPS) is displayed as well.

By default, zero bytes should be touched and you should see a maximum frame
rate.  Press the numbers 1 through 8 to see the effect of touching 1 through
8 floats of data at every frame.  Performance, as indicated by the frames per second
counter, should drop at every odd value up to 7.  When you press 8,
performance should reach the same point as 1 or 2, possibly higher.  Press
1 through 8 again, this time holding the Shift key while pressing them.
Again, frame rate should drop until you get to 16.  (Note:  If you're testing
on an Intel(4) Pentium(4) 4 processor-based system, the frame rate will not
pick up at the value of 8 because the write combining buffers are 64 bytes
wide, not 32 like on the P6 family processors.

You can press the 'V' key to toggle between modifying every other vertex
and every vertex.  If you modify more than 8 floats per vertex, the toggle
will automatically switch to "every other vertex".


