#ifndef BRDF_PARAMETERIZER_H
#define BRDF_PARAMETERIZER_H

#include <iostream.h>

#include "global.h"
#include "geometry.h"

#undef Polygon

class Polygon;
struct Triangle_Model;

class Param {
public:
    enum Brdf_ParamInOut { in = 0, out = 1, half=2, diff=3 };

    Param() {};
    ~Param() {};
    /*
    virtual void convert_to_theta_phi(const Vector &value1, 
				      const Vector &value2,
				      Vector *theta_phi_in_result,
				      Vector *theta_phi_out_result) = 0; 

    virtual void convert_from_theta_phi(const Vector &theta_phi_in,
					const Vector &theta_phi_out,
					Vector *result1,
					Vector *result2) = 0;
					*/
    virtual void update_coords(Triangle_Model *model,
			       const Vector &eye, 
			       const Vector &light) = 0;
};

typedef Param::Brdf_ParamInOut BRDFInOut;

#endif // BRDF_PARAMETERIZER_H
