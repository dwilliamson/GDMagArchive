This is the initial source code release for the programming language Lerp.
Though it is released in conjunction with the Game Developer Magazine
"Inner Product" column for January 2004, it contains features beyond those
discussed in the article, including features featured in future articles.  
The main reason for this is, I had to do a lot of experimentation in order
to determine what to write about, and that experimentation involved 
implementing various features to see what kind of cohesive whole can be made,
and what potential further features would then become visible as good ideas.
It'd e counterproductive to hold the source back just to synchronize it with
the articles.


Since a language system like this is a rather large project, invariably
many aspects of the project are in a rough unfinished state, or else are
temporary modules meant to be replaced later.  The most prominent of these
is the garbage collector....

The garbage collection system is not yet complete.  Lerp uses a copying
garbage collector; for such a collector to work well with games, it needs
to collect asynchronously with the main language interpreter, so that the
running program does not stall for collections.  Such asynchronous operation
requires a little more engineering to be invested than has been done yet.
Also, the garbage collector ought to invoke itself automatically when the
program wants more memory but none is available.  But in order for this to work
with a copying collector, some constraints need to be made in the implementation
of the C++ callback code, and none of that has been done yet.  So, all programs
must explicitly garbage-collect from within their main loops by calling
the function "gc()".  If they don't do this before running out of memory,
the program exits.  Currently programs get 16 megabytes of memory to work
with.  You can change this by modifying the following line:

    int arena_size = 16 * 1024 * 1024;

in memory_manager.cpp.  Note that the garbage collection system will allocate
two of these arenas, but the program can only use one arena's worth of memory.


There are two programs available here.  The first is a text-only demonstration
and test of language features ("test.lerp").  The second is a simple 3D game-like 
program, a port of the "Interactive Profiling" app from the December 2002 column
("crate_app.lerp").

To run these, just run lerp.exe with the filename as the first argument.  For
example, in Windows:

    lerp.exe test.lerp

I have made two Windows shortcuts to do this, called "Run test.lerp"
and "Run crate_app.lerp".



High on the list for next month's version is a more organized system for
creating and loading import libraries...


    Jonathan Blow (jon@number-none.com)
    December 31, 2003
    Transcontinental Hotel, Paris, France
