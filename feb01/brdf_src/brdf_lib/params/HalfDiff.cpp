#include "BRDFParamHalfDiff.hh"
#include "Polygon.hh"

#include "VisTransform.hh"
#include "Helper.hh"

BRDFParamHalfDiff::BRDFParamHalfDiff() {
    return;
}

BRDFParamHalfDiff::~BRDFParamHalfDiff() {
    return;
}

void 
BRDFParamHalfDiff::internalCalcTexCoords( const Vector &ve, const Vector &vl,
					  const Vector &n, 
					  const Vector &f, const Vector &b,
					  Double &first_x, Double &first_y,
					  Double &second_x, Double &second_y ) {
    
    Vector coordi, coordo;

    Vector h = vl + ve;
    h.normalize();

    coordo[0] = h|f;
    coordo[1] = h|b;
    coordo[2] = h|n;
    
    Double phi_h = 0.0, th_h = 0.0;

    calcTPFromXY( coordo[0], coordo[1], th_h, phi_h );
    
    VisTransform rotTh  = VisTransform::Rotation( b, -th_h );
    VisTransform rotPhi = VisTransform::Rotation( n, -phi_h );
    
    Vector d = rotTh.transformPoint( rotPhi.transformPoint( vl ) );


    // Just checkin
    /* Still wrong
    Double signD = (d|b);
    Double dt, dp;
    calcTPFromXY( d[0], d[1], dt, dp );
    calcXYFromTP( dt, 0.0, d[0], d[1] );

    coordi[0] = d|Vector(1.0,0.0,0.0)*(signD>0?-1:1);
    coordi[1] = 0.0;
    */

    coordi[0] = d|f;
    coordi[1] = d|b;
    coordi[2] = d|n;
    
    Double norm;
    Double thi = acos(coordi[2]) / M_PI_2;
    Double tho = acos(coordo[2]) / M_PI_2;
    norm = sqrt(coordi[0]*coordi[0]+coordi[1]*coordi[1]);
    if( norm > 0.001 ) {
	coordi[0] *= (1.0/norm) * thi;
	coordi[1] *= (1.0/norm) * thi;
	} else {
	    coordi[0] = coordi[1] = 0.0;
	}
    norm = sqrt(coordo[0]*coordo[0]+coordo[1]*coordo[1]);
    if( norm > 0.001 ) {
	coordo[0] *= (1.0/norm) * tho;
	coordo[1] *= (1.0/norm) * tho;
    } else {
	coordo[0] = coordo[1] = 0.0;
    }
    
    coordi[0] = coordi[0] / 2.0 + 0.5;
    coordi[1] = coordi[1] / 2.0 + 0.5;
    coordo[0] = coordo[0] / 2.0 + 0.5;
    coordo[1] = coordo[1] / 2.0 + 0.5;

    first_x = coordo[0];
    first_y = coordo[1];
    second_x = coordi[0];
    second_y = coordi[1];
}
