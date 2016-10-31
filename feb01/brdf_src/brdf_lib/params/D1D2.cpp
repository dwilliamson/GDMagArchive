#include "BRDFParamD1D2.hh"
#include "Polygon.hh"

#include "VisTransform.hh"
#include "Helper.hh"

BRDFParamD1D2::BRDFParamD1D2() {
    return;
}

BRDFParamD1D2::~BRDFParamD1D2() {
    return;
}

void 
BRDFParamD1D2::internalCalcTexCoords( const Vector &ve, const Vector &vl,
					  const Vector &n, 
					  const Vector &t, const Vector &b,
					  Double &first_x, Double &first_y,
					  Double &second_x, Double &second_y ) {
    Double coord1i, coord1o;
    Double coord2i, coord2o;

    coord1i = vl | t;
    coord2i = vl | b;
    coord1o = ve | t;
    coord2o = ve | b;
    coord1i = asin(coord1i) / M_PI + 0.5;
    coord2i = asin(coord2i) / M_PI + 0.5;
    coord1o = asin(coord1o) / M_PI + 0.5;
    coord2o = asin(coord2o) / M_PI + 0.5;

    first_x = coord2o;
    first_y = coord2i;
    second_x = coord1o;
    second_y = coord1i;
}
