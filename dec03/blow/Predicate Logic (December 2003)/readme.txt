This month's code implements a simple predicate logic query system.
Eventually, we will expand this in some interesting ways.
This month, though, we're laying down the basics.

By default, the program reads the database from the file "db.logic" and
then reads a bunch of queries from "queries.logic".  You can override
these filenames with command-line arguments (argument 1, if specified,
is the database file; argument 2 is the query file).

The database facts and queries use a simple syntax.  You might
think that I went overboard with regard to the implementation, writing a
lexical analyzer and parser and all that stuff.  That might be so if we
were limiting ourselves to what we see this month.  But in the next few
months this project will expand in scope, into a full language with
complicated syntax.  At that point we will be really happy that we have
a decently-structured lexer and parser.

This program leaks memory voraciously.  If you were to give it a big
database and lots of complex queries, it would conceivably die at some point
due to using too much VM.  I am not bothering to be careful about memory
because, in the future system, these allocations are going to be garbage
collected.  (For the memory usage patterns we see here, if you don't
garbage collect, you have to do all kinds of icky stuff that directly 
complicates the main code.)  


    Jonathan Blow (jon@number-none.com)
    October 11, 2003
    JP's Java, Austin, Texas
