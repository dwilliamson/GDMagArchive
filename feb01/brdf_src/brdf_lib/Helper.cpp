#include "Global.hh"
#include "Helper.hh"

static const Double epsilon = 0.00001; // for numerical inaccuracy

void calcTPFromXY( Double x, Double y, Double &th, Double &phi ) {
    Double xylen = x*x + y*y;

    if( xylen > 1.0 ) xylen = 1.0; // precision problems!

    Double z = sqrt( 1.0 - xylen );

    if( z > 1.0-epsilon ) th = 0.0;
    else th = acos(z);

    if( xylen != 0.0 ) {
	Double ratio = x/sqrt(xylen);
	if( ABS(ratio) > 1.0-epsilon ) phi = 0.0;
	else phi = acos( ratio );
	if( y < 0.0-epsilon ) phi = 2*M_PI - phi;
    } else {
	phi = 0.0;
    }
}

Vector calcVectorFromXY( Double x, Double y ) {
    /* NO! It's possible but far slower
    Double th, phi;
    calcTPFromXY( x, y, th, phi );

    return Vector( sin(th)*cos(phi), 
		   sin(th)*sin(phi),
		   cos(th) );
		   */

    Double xylen = x*x + y*y;

    if( xylen > 1.0 ) xylen = 1.0; // precision problems!

    Double z = sqrt( 1.0 - xylen );

    return Vector( x, y, z );
}

void calcXYFromTP( Double t, Double p, Double &x, Double &y ) {
    x = sin(t)*cos(p);
    y = sin(t)*sin(p);    
}
