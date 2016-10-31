#ifndef BRDFPARAMANGLESPHERE_HH
#define BRDFPARAMANGLESPHERE_HH

#include "Global.hh"
#include "Types.hh"
#include "BRDFParametrizer.hh"
#include "BRDFMap.hh"

#include "stream.h"

class Polygon;

class BRDFParamAngleSphere : public BRDFParametrizer {
public:
    BRDFParamAngleSphere( BRDFInOut f );
    ~BRDFParamAngleSphere();

    virtual void calculatePolygonTexCoords( Polygon *poly, 
					    const Vector &eye, 
					    const Vector &light );

    virtual void calculateTexCoords( const Vector &eye, const Vector &light,
				     const Vector &v, const Vector &n, 
				     const Vector &f, const Vector &f2,
				     Double &first_x, Double &first_y,
				     Double &second_x, Double &second_y );

    virtual BRDFInOut getFirstParam() { return first; }
protected:
    BRDFInOut first;
};

#endif
