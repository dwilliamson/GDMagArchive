#ifndef BRDFPARAMHALFDIFF_HH
#define BRDFPARAMHALFDIFF_HH

#include "Global.hh"
#include "Types.hh"
#include "BRDFParamBase.hh"
#include "BRDFMap.hh"

#include "stream.h"

class Polygon;

class BRDFParamHalfDiff : public BRDFParamBase {
public:
    BRDFParamHalfDiff();
    ~BRDFParamHalfDiff();

protected:
    virtual void internalCalcTexCoords( const Vector &ve, const Vector &vl,
					const Vector &n, 
					const Vector &f, const Vector &f2,
					Double &first_x, Double &first_y,
					Double &second_x, Double &second_y );
};

#endif
