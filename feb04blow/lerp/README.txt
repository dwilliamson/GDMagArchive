This is the seconddl source code release for the programming language Lerp.
Though it is released in conjunction with the Game Developer Magazine
"Inner Product" column for February 2004, it contains features beyond those
discussed in the article.  In fact, the January soruce release contained all
the features discussed in the February article, so I used the occasion of
this release to add some other features instead, like thread support.

---------------------------------------------------------------------
There are several programs available here.  Try the following commands:

    release\lerp test.lerp
    release\lerp thread_test.lerp
    release\lerp crate_app.lerp
    release\lerp pool\pool.lerp_set

In the first three commands, the .lerp files are self-contained programs.
Used in the fourth, a .lerp_set is a way of setting up a larger project.
It contains directives on files to load and modules to import.


Please keep in mind that this is a work in progress and thus is not robust!
Expect to encounter features that don't work, runtime crashes if you do
weird things, etc.



See below for garbage collection notes [copied from last release; there
have been no gc improvements yet!]:
---------------------------------------------------------------------
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



    Jonathan Blow (jon@number-none.com)
    March 29, 2004
    Royal Ground Coffee and Laundromat, San Francisco, California, USA
