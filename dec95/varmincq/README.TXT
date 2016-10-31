                       Using the Example Programs

    EXAMPLE1.C and EXAMPLE2.C where compiled with Watcom C version 10.0
and tested on a 486DX2/66 with 8 megabytes of memory under DOS 6.20.
CEXAMP1.BAT compiles the first example program which reads and writes
files which can be created and viewed by PICLAB.  CEXAMP2.BAT compiles
the second example program which reads and writes .RAW files which can
be created and viewed by our application, PowerView.

    Compiled versions of these files have been included in case you do
not have access to Watcom's compiler.  Both programs require DOS4GW.EXE
in order to execute.  The sources should also be compilable by other
compilers, although some slight modifications to the source files may be
required.

    The example programs are configured to internally use six bits of
precision per component for the histogram and inverse colormap tables.
If you wish to experiment with different values, change the definition
of HIST_BIT in the following source files and recompile: QUANTIZE.C,
EXAMPLE1.C, and EXAMPLE2.C.  The minimum value we've tried is three bits
(which looks quite terrible), the maximum value we've been able to try
due to memory limitations is six bits.  On machines with gobs of real or
virtual memory a setting of seven bits should be feasable.

    It should be noted that neither example program employs error
propagation dithering during the final mapping phase.  A simple direct
mapping is instead used for simplicity.  For best results, especially
when the number of output colors is less than 32 or so, some sort of
dithering should be used.

                      Using EXAMPLE1 with PICLAB

    To use EXAMPLE1 you must have a copy of Lee Daniel Crocker's PICLIB
program, which has been included in case you don't have a copy handy --
see PICLB192.ZIP.  The following steps can be followed in order to use
EXAMPLE1 with PICLAB:

    1.  Execute PICLAB by running PL.EXE.

    2.  Load the image to be quantized into PICLAB's image buffer.  For
GIF files, type "GLOAD <filename>", for TGA files, type "TLOAD
<filename>".  Note the image's resolution, as this will be needed later.
(When loading TGA files, be sure that the image is stored in top-down
format. If not, issue the "REVERSE" command after loading.)

    3.  Convert the image to true color (24-bits per component), if
necessary.  To do this, issue the "UNMAP" command.  EXAMPLE1 only accepts
true color input files.

    4.  Save PICLAB's image buffer to disk.  Type "RSAVE <filename>".
This command will actually create three files, one for each color
component.  The extensions of each file will be .R8, .G8, and .B8.

    5.  Exit PICLAB by typing "EXIT".

    6.  Now it's time to execute EXAMPLE1.  Type the following command:

    EXAMPLE1 <src filename> <dst filename> <x res> <y res> <max colors>

    <src filename> is the filename of the source file which was
previously created by PICLAB.  <dst filename> is the filename of the
destination file.  <x res> and <y res> indicate the resolution of the
source file in pixels.  The final parameter, <max colors>, specifies the
maximum number of colors the image should be quantized to.  Any value
from 1 to 256 is acceptable, although values below four or so are not
recommended.

    EXAMPLE1 actually creates two files, one with a .R8 extension, the
other with a .MAP extension.  The first file contains the raw image data
(8-bits per pixel), while the second file contains the image's palette
(ASCII text).

    7.  After EXAMPLE1 finishes, run PL.EXE once again in order to view
and/or save the file created by EXAMPLE1.

    8.  Load the image created by EXAMPLE1 into PICLAB's image buffer.
Type "RLOAD <filename> <x res> <y res> MONO", where <filename> is the
file created by EXAMPLE1, and <x res> and <y res> are the image's
resolution.  Do not type an extension with the filename.

    9.  Convert the image to colormapped by typing "COLOR".

   10.  Load the image's palette by typing "PLOAD <filename>", where
<filename> is the file created by EXAMPLE1.

   11.  View the quantized image by typing "SHOW".

   12.  If you wish to save the quantized image to disk as a GIF file,
type "GSAVE <filename>".  To save a TGA file, type "TSAVE <filename>".

   13.  When done, exit PICLAB by typing "EXIT".

                     Using EXAMPLE2 with PowerView

    Important note: Your video card must support true color video modes
in order for the following procedure to work.  If not, use EXAMPLE1 and
PICLAB instead.

     The following steps can be followed in order to use EXAMPLE2 with
PowerView:

    1.  Execute PowerView by running PV.EXE.

    2.  Lock a true color video mode, such as 640x480x16m.  First,
select the "Display" pull down menu.  Then select "Video Modes".  Now
select "VESA/16m".  Finally, select the true color video mode to be
locked.

    3.  View the image to be quantized.

    4.  Save the image as a .RAW file by typing Alt+R.  Note: By
default, the resulting output file will be saved to the directory which
you currently have loaded.

    5.  Exit PowerView by typing Alt+Q.

    6.  Run EXAMPLE2:

    EXAMPLE2 <src filename> <dst filename> <max colors>

    Where <src filename> is the filename of the file written by
PowerView, <dst filename> is the filename of the destination file, and
<max colors> is the maximum number of colors the image should be
quantized to.  Neither filename should include an extension.

    7.  Execute PowerView by running PV.EXE.

    8.  View the .RAW image created by EXAMPLE2.  To save the image as a
GIF file, type Alt+G while viewing.  To save a TGA file, type Alt+T
while viewing.

                                 Notes

    If your system does not have a math coprocessor the example programs
will perform rather slowly because of QUANTIZE.C's use of floating point
math.  Our original implementation did not use any floating point math --
but the code which implemented the fixed point calculations was hard to
understand so it was removed.  If you intend to use QUANTIZE.C in a
production program, we highly recommend you remove the floating point
calculations from this module and replace them with fixed point.  Be
sure to preserve enough precision.

    One of the most interesting uses of the quantizer is deriving one
palette from a group of unrelated true-color or palettized images so the
images can be shown on a palettized display at the same time.  In order
to derive the optimum palette for this situation, first enter each image
into one color histogram.  (Do not clear the histogram between images.)
After the histogram is created, create the palette and inverse colormap
in the usual manner.  To display an image, simply map the image through
the inverse colormap as is done in the example programs.  Some sort of
dithering may be employed to increase perceived quality.  For low
resolution displays (320x240 for example), an ordered dither looks good.
For higher resolutions an error propagation dither usually looks better.

    If you modify QUANTIZE.C be sure to *throughly* retest the module as
much as possible.  This module can be very difficult to debug -- a
slight change in the code may not cause a fatal error, but could cause
the resulting image quality to suffer.

