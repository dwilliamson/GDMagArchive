#include <stdio.h>
#include <assert.h>
#include "standard.h"
#include "../geometry.h"

static void calcThetaAndPhi( Double *theta_01, Double *phi_01, 
			     const Vector &v, const Vector &n, 
			     const Vector &f, const Vector &f2 );

#define P Param__Theta_Phi_In__Theta_Phi_Out

P::P() {
}

P::~P() {
    return;
}

void P::update_coords(Triangle_Model *model,
		      const Vector &eye, const Vector &light) {
    assert(0);
}

void P::convert_to_theta_phi(const Vector &value1, 
			     const Vector &value2,
			     Vector *theta_phi_in_result,
			     Vector *theta_phi_out_result) {
    *theta_phi_in_result = value1;
    *theta_phi_out_result = value2;
}


void P::convert_from_theta_phi(const Vector &theta_phi_in,
			       const Vector &theta_phi_out,
			       Vector *result1,
			       Vector *result2) {
    *result1 = theta_phi_in;
    *result2 = theta_phi_out;
}
