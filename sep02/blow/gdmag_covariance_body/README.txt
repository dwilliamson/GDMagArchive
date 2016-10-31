This program is a simple deomonstration of computing a covariance body
from point samples.  You can add and remove data points and look at
the resulting covariance.

You will have to add 3 points before you see much of interest.

You can toggle between two main display modes, Figure 1 and Figure 2.
Figure 1 just shows the covariance body as a blue ellipse.  Figure 2
shows the same ellipse, but also projects data points down to the 
X axis and computes their 1D distribution and variance (which, 
you should note, matches the projection of the covariance body 
down to the same axis).  

The 1D distribution is computed by convolving a Lanczos-windowed
sinc pulse with a series of spikes representing the data points'
presence or absence at a particular X coordinate.  For correct
results, the width of this pulse should be scaled based on the
number of input points.  This was a quick-hack-type feature, so
I didn't scale the window; thus the distribution will appear
inappropriately spiky for a small number of input points.

There is a limit on the number of input points (200), though you can 
change this by editing MAX_INPUT_POINTS in main.cpp and recompiling.


Controls:
    '1': Enter display mode for Figure 1.
    '2': Enter display mode for Figure 2.
    Spacebar: Add a data point at the position of the mouse ponter.
    Delete: Remove the most recently added data point.

