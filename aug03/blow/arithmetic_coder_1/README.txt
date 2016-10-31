This month's app is pretty simple.  Just run it from
the command line; there are no command-line arguments.
It uses the Arithmetic_Encoder and Arithmetic_Decoder
classes to pack and unpack values.

The particular arithmetic coder I use is a form of
"range coder".  See arithmetic_coder.cpp for details.

This month's code doesn't do any statistical modeling
of the data; all values are considered equally possible.
Modeling will come next month.

   -Jonathan (jon@number-none.com)
    Tuesday, July 1, 2003
    Metro Coffeehouse, Austin, Texas
