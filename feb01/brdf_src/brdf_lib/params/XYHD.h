#ifndef PARAM_XYHALF
#define PARAM_XYHALF

#include "../global.h"
#include "../types.h"
#include "../Brdf_Map.h"

struct Triangle_Model;

class Param__X_Y_Half__X_Y_Diff : public Param {
public:
    Param__X_Y_Half__X_Y_Diff();
    ~Param__X_Y_Half__X_Y_Diff();

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

#endif // PARAM_XYHALF
