#include <assert.h>

#include "glsphere.h"
#include "../geometry.h"
#include "../triangle_model.h"

Param__Theta_Phi_In__Theta_Phi_Out__Spheremap::Param__Theta_Phi_In__Theta_Phi_Out__Spheremap() {
}

Param__Theta_Phi_In__Theta_Phi_Out__Spheremap::~Param__Theta_Phi_In__Theta_Phi_Out__Spheremap() {
}

inline void fix_coordinates(Vector *v) {
    assert(v->z < 0.0);

    Double norm = v->x * v->x + v->y * v->y;
    if (norm == 0.0) return;

    norm = 1.0 / sqrt(norm);

    /*
      coordi[0] /= norm;
      coordi[1] /= norm;
      
      coordi[0] += (1-norm)*coordi[0];
      coordi[1] += (1-norm)*coordi[1];
      */

    v->x = 2 * v->x * norm - v->x;
    v->y = 2 * v->y * norm - v->y;

    // c*1/norm + c*1/norm - c;
}

inline void do_model_frame_dammit(Triangle_Model *model, int index,
				  const Vector &eye, const Vector &light) {
    Vector coordi, coordo;

    Vector *pos = &model->condensed_vertices[index];
    Vector *n = &model->condensed_normals[index];
    Vector *t = &model->condensed_tangents[index];
    Vector *b = &model->condensed_cross_tangents[index];

    Vector to_eye = ((Vector &)eye).subtract(*pos);
    Vector to_light = light;
    //    Vector to_light = ((Vector &)light).subtract(*pos);


    coordi.x = dot_product(to_light, *t);
    coordi.y = dot_product(to_light, *b);
    coordi.z = dot_product(to_light, *n);
    coordi.normalize();

    coordo.x = dot_product(to_eye, *t);
    coordo.y = dot_product(to_eye, *b);
    coordo.z = dot_product(to_eye, *n);
    coordo.normalize();

    if (coordi.z < 0.0) fix_coordinates(&coordi);
    if (coordo.z < 0.0) fix_coordinates(&coordo);

    // Not really necessary to do this anymore if backfacing

    coordi.x = coordi.x * 0.5 + 0.5;
    coordi.y = coordi.y * 0.5 + 0.5;
    coordo.x = coordo.x * 0.5 + 0.5;
    coordo.y = coordo.y * 0.5 + 0.5;

    model->face_brdf_channel_0[index] = coordi;
    model->face_brdf_channel_1[index] = coordo;
}

void Param__Theta_Phi_In__Theta_Phi_Out__Spheremap::update_coords(Triangle_Model *model, const Vector &ve, const Vector &vl) {

    Vector light = vl;
    light.normalize();

    int i;
    for (i = 0; i < model->num_condensed_items; i++) {
	do_model_frame_dammit(model, i, ve, light);
    }
}
