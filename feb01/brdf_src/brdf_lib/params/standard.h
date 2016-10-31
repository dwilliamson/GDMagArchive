#ifndef BRDF_PARAM_STANDARD_H
#define BRDF_PARAM_STANDARD_H

#include "../global.h"
#include "../types.h"
#include "../Brdf_Parameterizer.h"
#include "../Brdf_Map.h"

class Polygon;

class Param__Theta_Phi_In__Theta_Phi_Out : public Param {
public:
    Param__Theta_Phi_In__Theta_Phi_Out();
    ~Param__Theta_Phi_In__Theta_Phi_Out();

    virtual void convert_to_theta_phi(const Vector &value1, 
				      const Vector &value2,
				      Vector *theta_phi_in_result,
				      Vector *theta_phi_out_result); 

    virtual void convert_from_theta_phi(const Vector &theta_phi_in,
					const Vector &theta_phi_out,
					Vector *result1,
					Vector *result2);

    virtual void update_coords(Triangle_Model *model,
			       const Vector &eye, 
			       const Vector &light);
};

#endif BRDF_PARAM_STANDARD_H
