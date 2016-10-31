#include <assert.h>

#include "XYHD.h"
#include "../geometry.h"
#include "../triangle_model.h"

#define P Param__X_Y_Half__X_Y_Diff

P::P() {
}

P::~P() {
}

void P::convert_to_theta_phi(const Vector &value1, 
			     const Vector &value2,
			     Vector *theta_phi_in_result,
			     Vector *theta_phi_out_result) {
    assert(0);
}

void P::convert_from_theta_phi(const Vector &theta_phi_in,
			       const Vector &theta_phi_out,
			       Vector *result1,
			       Vector *result2) {
    assert(0);
}

static inline void fix_coordinates(Vector *v) {
    assert(v->z < 0.0);

    Double norm = v->x * v->x + v->y * v->y;
    if (norm == 0.0) return;

    norm = 1.0 / sqrt(norm);

    v->x = 2 * v->x * norm - v->x;
    v->y = 2 * v->y * norm - v->y;
}

inline void do_model_frame(Triangle_Model *model, int index,
			   const Vector &to_eye, const Vector &to_light) {

    Vector pos = model->condensed_vertices[index];

    Vector light = to_light;
    Vector eye = to_eye;
    eye = eye.subtract(pos);

    eye.normalize();

    Vector h = light.add(eye);
    h.normalize();

    Vector t = model->condensed_tangents[index];
    Vector b = model->condensed_cross_tangents[index];
    //    Vector n = model->condensed_normals[index];

    Vector coordd, coordh;
    coordh.x = dot_product(h, t);
    coordh.y = dot_product(h, b);
    //    coordh.z = dot_product(h, n);

    Vector tprime = h.scale(dot_product(t, h));
    tprime = t.subtract(tprime);
    tprime.normalize();
    Vector bprime = cross_product(h, tprime);   // b' = h x t'
    bprime.normalize();

    coordd.x = dot_product(light, tprime);
    coordd.y = dot_product(light, bprime);

    //    coordd.z = dot_product(light, n);
    /*
    if (coordh.z < 0.0) fix_coordinates(&coordh);
    if (coordd.z < 0.0) fix_coordinates(&coordd);
    */

    coordh.x = coordh.x * 0.5 + 0.5;
    coordh.y = coordh.y * 0.5 + 0.5;
    coordd.x = coordd.x * 0.5 + 0.5;
    coordd.y = coordd.y * 0.5 + 0.5;

    model->face_brdf_channel_0[index] = coordh;
    model->face_brdf_channel_1[index] = coordd;
}

void P::update_coords(Triangle_Model *model,
		      const Vector &ve, const Vector &vl) {

    Vector light = vl;
    light.normalize();

    int i;
    for (i = 0; i < model->num_condensed_items; i++) {
	do_model_frame(model, i, ve, light);
    }
}





