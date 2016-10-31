#include "BRDFParamHDD1D2.hh"
#include "Polygon.hh"

#include "VisTransform.hh"
#include "Helper.hh"

BRDFParamHDD1D2::BRDFParamHDD1D2() {
    return;
}

BRDFParamHDD1D2::~BRDFParamHDD1D2() {
    return;
}

void 
BRDFParamHDD1D2::internalCalcTexCoords( const Vector &ve, const Vector &vl,
					  const Vector &n, 
					  const Vector &t, const Vector &b,
					  Double &first_x, Double &first_y,
					  Double &second_x, Double &second_y ) {
    // TODO: check if correct!

    Double coord1h, coord1d;
    Double coord2h, coord2d;

    Vector h = vl + ve;
    h.normalize();

    Vector tprime = t-h*(t|h);  // t' Gram-Schmidt
    tprime.normalize();
    Vector bprime = h^tprime;   // b' = h x t'
    bprime.normalize();

    coord1h = h | t;
    coord2h = h | b;
    coord1d = vl | tprime;
    coord2d = vl | bprime;
    coord1h = asin(coord1h) / M_PI + 0.5;
    coord2h = asin(coord2h) / M_PI + 0.5;
    coord1d = asin(coord1d) / M_PI + 0.5;
    coord2d = asin(coord2d) / M_PI + 0.5;

    first_x = coord2d;
    first_y = coord2h;
    second_x = coord1d;
    second_y = coord1h;
}
