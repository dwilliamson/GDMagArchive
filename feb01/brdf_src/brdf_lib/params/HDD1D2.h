#ifndef BRDFPARAMHDD1D2_HH
#define BRDFPARAMHDD1D2_HH

#include "Global.hh"
#include "Types.hh"
#include "BRDFParamBase.hh"
#include "BRDFMap.hh"

#include "stream.h"

class Polygon;

class BRDFParamHDD1D2 : public BRDFParamBase {
public:
    BRDFParamHDD1D2();
    ~BRDFParamHDD1D2();

protected:
    virtual void internalCalcTexCoords( const Vector &ve, const Vector &vl,
					const Vector &n, 
					const Vector &f, const Vector &f2,
					Double &first_x, Double &first_y,
					Double &second_x, Double &second_y );
};

#endif
