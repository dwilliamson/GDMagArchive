#ifndef HELPER_HH
#define HELPER_HH

#include "Point.hh"
#include "Types.hh"


// calculate theta/phi from x and y (a la GLSphere)
// theta: [0:pi/2], phi: [0:2pi]
void calcTPFromXY( Double x, Double y, Double &th, Double &phi );

// calculate a vector from x and y (a la GLSphere)
Vector calcVectorFromXY( Double x, Double y );

// calculate x and y (a la GLSphere) from theta (->y) and phi (->x)
void calcXYFromTP( Double t, Double p, Double &x, Double &y );

#endif
