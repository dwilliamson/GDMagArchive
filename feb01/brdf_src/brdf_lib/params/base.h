#ifndef BRDF_PARAMS_BASE_H
#define BRDF_PARAMS_BASE_H

#include "Global.hh"
#include "Point.hh"
#include "BRDFParametrizer.hh"

#include <stream.h>

class Polygon;

class BRDFParamBase : public BRDFParametrizer {
public:
    BRDFParamBase() {} ;
    ~BRDFParamBase() {} ;

    virtual void calculatePolygonTexCoords( Polygon *poly, 
					    const Vector &eye, 
					    const Vector &light );

    virtual void calculateTexCoords( const Vector &eye, const Vector &light,
				     const Vector &v, const Vector &n,
				     const Vector &f, const Vector &f2,
				     Double &first_x, Double &first_y,
				     Double &second_x, Double &second_y );
protected:

    virtual void internalCalcTexCoords( const Vector &ve, const Vector &vl,
					const Vector &n, 
					const Vector &f, const Vector &f2,
					Double &first_x, Double &first_y,
					Double &second_x, Double &second_y );
};

#endif  // BRDF_PARAMS_BASE_H
