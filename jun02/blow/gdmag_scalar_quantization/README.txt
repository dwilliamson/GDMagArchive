Once again, this month's code is pretty straightforward.
You just run it from the command line; there are no command-line
arguments.  There are two distinct parts to this code.

The first part is all about testing the three quantizers
mentioned in the article (TL, TC, RL).  I recommend that you use
RL in the general case, and maybe use TC if you're being super
tweaky.  TL is only there to illustrate how not being careful
about rounding leads to a waste of bandwidth.

The second part of the code is about repacking IEEE floating-point
numbers into smaller, lower-precision and lower-range formats.
If you haven't seen any of the funky IEEE fp bit tricks before,
this code may be difficult to decipher; if you have, then it's pretty
straightforward.  If you don't "get it", see the article for
references.

   -Jonathan Blow (jon@bolt-action.com)
