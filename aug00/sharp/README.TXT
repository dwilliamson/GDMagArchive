Fluid Demo, release 1.0
Brian Sharp
brian@maniacal.org
www.maniacal.org/implicits.html

This file contains some information on how to run the demo, as well as info on how to build the demo (which, if you downloaded the binary-only distribution, you should ignore.)  Finally, there's some license information at the bottom, of interest to you if you want to use this source for your own programs.

======================
== RUNNING THE DEMO ==
======================

The zipfile contains paths, and it's important to preserve them; unzip the demo to some starting base directory (which I'll refer to as MyBaseDirectory.)

Run the demo from the fluid directory; so run something like this:

cd \MyBaseDirectory\fluid
release\fluid

Command-line arguments include:

-window : run windowed.
-vidmode WIDTHxHEIGHT:COLORDEPTH : specify a fullscreen mode to run in.
-demo : start in looping demo mode.

So:

fluid -vidmode 1152x864:32 -demo

... will start the demo running fullscreen at 1152x864, 32bpp, running in auto-looping demo mode.

All controls in the demo can be accessed from the menus; hit ~ (tilde) to open the menus.

INCOMPLETE: Demo #5, the mercury fountain, is still slightly incomplete.  The mobile arm on the fountain is supposed to move, but I never got around to implementing the joint constraints needed to do this.  Hence, it doesn't move.  Mea culpa.

=======================
== BUILDING THE DEMO ==
=======================

The source comes with MSVC5 projects and workspaces.  There are a couple things you need to do to build it from MSVC5/6:

- Add include directories.  You need MyBaseDirectory (the directory you unzipped the demo to) in the path.  So, if you unzipped the demo and source to c:\projects, for example, you'd add that to your include directories (Tools/Options/Directories/Include Files in MSVC):

- Add library directories.  You need MyBaseDirectory\plib in your library path.  So, as per the example above, if the source base dir is c:\projects, add c:\projects\plib to your library path (Tools/Options/Directories/Library Files in MSVC)

- Open the workspace.  It's in fluid\fluid.dsw.

- Build.

- Run: make sure to specify that its working directory is the fluid directory, not the fluid\debug or fluid\release directories.  Otherwise, it won't be able to find any of its textures.

=========================
== LICENSE INFORMATION ==
=========================

This source code is in the public domain.  You can use it for anything you want, including commercial projects, and you don't owe me a red cent.  There are, however, a few snags:

- PLIB: I've kept PLIB separated as its own module, and it's only included in this distribution as a few libraries and a couple headers, so I believe under its terms (the LGPL) it doesn't taint my source.  Somebody let me know if this is incorrect, and how I should be stricter about keeping it separate.

- Marching cubes tables: In fluid/tess/MarchingCubeTable.h, I use tables that Paul Bourke / Cory Gene Bloyd provided and allowed me to redistribute, but for non-commercial uses only.  Here's the comment I added at the top of that file:

//
// These tables are taken verbatim from Paul Bourke's website:
//
//  http://www.swin.edu.au/astronomy/pbourke/modelling/polygonise/
//
// Original credit for the tables goes to Cory Gene Bloyd, cbloyd@megsinet.com
//
// Use of these tables is granted for non-commercial purposes only.  If you want to use
// the implicit code for commercial uses, I created my own (unencumbered) tables in
// MarchingCubeTableNew.h, but they're still buggy (a few cases have the winding wrong, argh!)
// If I ever get those bug-free, I'll drop these tables and use them, but until then I'm using
// these.  This is the only file in the entire fluid project that has this restriction, all the
// other code is in the public domain and freely usable.  Sucks that this one file makes the rest
// of the code unusable commercially, but if you feel like bugfixing the other tables, that's your
// ticket out.
//
//    -- Brian (brian@maniacal.org)
//

... so, that still stands: MarchingCubeTableNew.h is still buggy, but I'd love it if someone would fix it, as, if you use MarchingCubeTable.h as part of my source, you can only use it in non-commercial projects.
