Welcome to the sample code for the 2nd article in
the Arithmetic Coder series.


The Intent Behind The Code
--------------------------
The sample code this time is a file compressor.  It uses
some training data (located in "training.txt") to build
a static data model.  Then it applies those statistics
to a different file (located in "input.txt") to compress
that file.  

The code builds two models: a static order 0 model, and
a static order 1 model.  The order 1 model gets better
compression but takes more memory and is a little slower.

Compression would be significantly better if we used the
same file for training and compression.  But the reason
I am using two different files, and a static data model,
is so that we have some kind of analogy with the situation
we face in a networked game.

In a high-performance networked game, we are sending most
of our data via unreliable transmission, so the server
can't predict which information the client will receive,
and which it won't.  Thus the straightforward ways of
implementing an adaptive data model just won't work, since
the client would quickly fall out of synch with the server.

The simplest solution is to stick to a static model, which
is why we use a static model here.  Also, static models
are the simplest, and I wanted to keep this code example
as simple as possible.  There are a few arithmetic coder
samples to be found on the web, but they are generally
more opaque than this.

In our analogy, "input.txt" represents some data that the
server sends to the client.  Since the client doesn't know
what's in this message until it successfully decodes it,
we cannot derive the data model from this message.  It must
be derived from a source known _a priori_ to both client
and server; hence the separate file "training.txt".

Compression Efficacy
--------------------
Because this is a simple example, the compression is not
nearly so good as what we could achieve with sophisticated
means.  Amusingly, though, both probability models beat
the performance of WinZip 8.0 when compressing input.txt.
Probably this is because WinZip is not optimized for small
isolated files, and stores significant amounts of header
data to tell you how to decode the file.

However, the case of small isolated messages is exactly
what we're interested in for networking.  (We want to be
sending packets somewhere between 100 and 1000 bytes in size;
any bigger than that and we're asking for severe trouble).
So if, instead of using an arithmetic coder like this,
you were to just use 'zlib' or something like that to
compress your packets, you might get worse performance.
Simply because zlib is not designed for this kind of
application.

Code Limitations
----------------
Because I wrote this code to be simple, the arithmetic coder
has a maximum buffer size of 16kbytes.  If you attempt to process
an input.txt that would be more than 16kbytes in compressed form,
the debug build will assert.  (And the release build will just
crash).  You can go into arithmetic_coder.h and increase this
limit, or you can modify the class to dynamically allocate
storage.

CPU Usage
---------
This code was written for simplicity, not speed.  It's slower
than it needs to be; I have placed a few comments in the code
describing how to speed it up.  On a server, where you're 
encoding a lot of messages to send to a lot of clients, 
encoding speed can be pretty important.

Running The Code
----------------
Just run the code from the command line; there are no arguments.
It reads the files "training.txt" and "input.txt" so if you want
to play with its behavior, just modify the contents of those two
files.  (Just remember not to make input.txt too big, unless you
go modify that static buffer size).

Have fun with it.  Next month we'll look at adaptive modeling,
and coping with lost network messages.

   -Jonathan (jon@number-none.com)
    Wednesday, July 16, 2003
    Mojo's and Metro Coffeehouses, Austin, Texas
