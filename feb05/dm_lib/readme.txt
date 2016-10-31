dm_lib - A late-binding data manager       Sean Barrett, 2005-03-15

This is the late-binding data manager I described in my March
2005 Game Developer Inner Product column. It has been extracted
from a game of mine and as such doesn't have any sample code;
however, it was written to be a library so it has minimal coupling
to the game and should be easy to fit in. (Even easier if you just
want to browse the code.) However, it probably doesn't do
everything you want; and it's in C style, not C++ style, so you
might prefer starting over from scratch or at least rewriting it
in a more familiar paradigm.


MANIFEST

  library:
    dm.h         --   external-use header file
    dm_hand.*    --   format handler / loaders interface
    dm_names.*   --   string -> "Name *" mapping
    dm_swap.*    --   public data-access interface
    dm_track.*   --   specify directories, track files there
    dm_win32.c   --   win32 directory-monitoring code

  app:
    dmtypes.h    --   enumerated list of datatypes, type-safe wrappers
    image.*      --   sample image loaders, OpenGL texture loader (no DDS)
    

USAGE

1. Define the datatypes you want to load.

     See "dmtypes.h", which is not part of the library.

2. Write loaders for all those data types. 

     See "image.c" for some sample image loaders and
     a texture-from-image loader. Note that compiling this
     requires "jpeg.lib", "libpng.lib", and "zlib.lib". You
     can get rid of these by removing the #defines of LOAD_JPEG
     and LOAD_PNG at the top of the file.

3. Initialize the data manager.

     Call dmInit(), then install all the loaders
     (the latter is demonstrated at the end of image.c)

4. Specify some directories that contain data.

     Call dmOpenDirectory() repeatedly with each directory
     name. [If you want it to be recursive, you should write
     a wrapper or modify the internal scanning code since it's
     already spotting the subdirectories (and ignoring them).]

5. Access your data by name right when it's about to be used.

     Call dmFind() with the string name of the data (the name
     of the file on disk, with no extension), and the data type.
     "dmtypes.h" has macro wrappers around this for each data
     type to make it more convenient. You might wrap real functions
     or use C++ templates.

6. Unloading data is incompletely handled.

     You can unload an individual resource with dmFree(). You
     can unload all the resources of a given type with dmFreeType().
     There's no actual "keep track of total storage used and unload
     assets that haven't been loaded lately". I'm not going to need
     that for the game I wrote this for, but it's an obvious addition.

     Currently you can't un-"track" a directory--that is,
     dmCloseDirectory() isn't implemented. This is needed to
     actually do namespace-y stuff with, say, storing separate levels
     in separate directories, or storing separate languages in
     separate directories and changing them on the fly. I just
     haven't gotten there yet in my game so I haven't implemented
     it. Shouldn't be too hard.


BEHAVIOR

When you call dmOpenDirectory(), the library immediately scans
the directory and remembers all the files it's scene so it doesn't
have to go looking for them on disk later. This may use more
storage than you're comfortable with.

Every time you call dmFind(), the string name you pass in is
looked-up in a hash table, and then a list of currently-loaded
resources with that name is scanned. If a match is found, it
is returned to you. Generally this is an O(1) operation if the
object is in cache. If not, then the table of tracked resources
created in dmOpenDirectory is scanned. (This merely looks at
a linked list of all resources with this name, so it's quite
fast.) If one is found, it is loaded.

When a file changes on disk, the library notices (because it called
ReadChangesW on each of the directories in dmOpenDirectory). It then
unloads the resource. It doesn't actually reload it when you change it;
it simply flushes the cached entry, and will reload it from scratch the
next time you reference it. So if it's something that is slow to load
(there's a lot of processing), you will see that performance hit the
first time it is used after you save it, rather than the moment you
save it.
