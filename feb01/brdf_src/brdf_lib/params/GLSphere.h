#include "../global.h"
#include "../types.h"
#include "../Brdf_Map.h"

class Param__Theta_Phi_In__Theta_Phi_Out__Spheremap : public Param {
  public:
    Param__Theta_Phi_In__Theta_Phi_Out__Spheremap();
    ~Param__Theta_Phi_In__Theta_Phi_Out__Spheremap();

    virtual void update_coords(Triangle_Model *model,
			       const Vector &eye, 
			       const Vector &light);
};
