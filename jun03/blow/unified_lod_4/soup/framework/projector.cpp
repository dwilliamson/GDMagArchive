#include "../framework.h"
#include "projector.h"

#include <math.h>

const double YON_RANGE = 4000.0;

Projector::Projector() {
    fov = M_PI * 0.5f;
    z_viewport = 0;
    i_z_viewport = 1;
    offset_x = 0;
    offset_y = 0;
    center_x = 0;
    center_y = 0;
    viewport_width = 0;
    viewport_height = 0;
    pixels_per_unit = 0;
    offset_x = offset_y = 0;
}

Projector::~Projector() {
}

void Projector::auto_compute_offsets() {
    double h = viewport_height;

    offset_x = center_x;
    offset_y = h - center_y;

    my_frustum.recalibrate(this);
}

void Projector::get_cop(float *x, float *y) {
    *x = center_x;
    *y = center_y;
}

void Projector::set_cop(float x, float y) {
    center_x = x;
    center_y = y;

    auto_compute_offsets();
}

void Projector::set_fov(double radians) {
    double w = viewport_width;
    double h = viewport_height;
    
    double z0 = w / (2.0 * tan(radians * 0.5));
    z_viewport = z0;
    i_z_viewport = 1.0 / z_viewport;

    fov = radians;

    auto_compute_offsets();
}

void Projector::set_viewport(float width, float height) {
    viewport_width = width;
    viewport_height = height;

    center_x = viewport_width * 0.5;
    center_y = viewport_height * 0.5;
    set_fov(fov);
}

void Projector::set_viewport_parameters(float ox, float oy,
                                        float wx, float wy) {
    center_x = ox;
    center_y = oy;

    viewport_width = wx;
    viewport_height = wy;

    set_fov(fov);
}


Frustum::Frustum() {
    clip_signal_flags[LEFT] = AGAINST_LEFT;
    clip_signal_flags[RIGHT] = AGAINST_RIGHT;
    clip_signal_flags[FLOOR] = AGAINST_FLOOR;
    clip_signal_flags[CEILING] = AGAINST_CEILING;
    clip_signal_flags[HITHER] = AGAINST_HITHER;
    clip_signal_flags[YON] = AGAINST_YON;

    num_planes = 5;
    most_planes = 5;

    weird = false;
    have_constraint = false;
    calibration_number = 0;

    cx0 = cy0 = cx1 = cy1 = cx2 = cy2 = cx3 = cy3 = 0;
}

void Frustum::recalibrate(Projector *projector) {
    calibration_number++;
    if (have_constraint) {
        recalibrate(projector, cx0, cy0, cx1, cy1, cx2, cy2, cx3, cy3);
    } else {
        float w = projector->viewport_width;
        float h = projector->viewport_height;

        recalibrate(projector, 0, 0, 0, h, w, h, w, 0);
    }
}

inline void Clamp2(float *val, float min, float max) {
    if (*val < min) *val = min;
    if (*val > max) *val = max;
}

void Frustum::recalibrate(Projector *projector,
                          float x0, float y0, float x1, float y1,
                          float x2, float y2, float x3, float y3) {

    float border = 0.1;
    float fudge = 0.6;

    Clamp2(&x0, 0, projector->viewport_width);
    Clamp2(&x1, 0, projector->viewport_width);
    Clamp2(&x2, 0, projector->viewport_width);
    Clamp2(&x3, 0, projector->viewport_width);

    Clamp2(&y0, 0, projector->viewport_height);
    Clamp2(&y1, 0, projector->viewport_height);
    Clamp2(&y2, 0, projector->viewport_height);
    Clamp2(&y3, 0, projector->viewport_height);

    float left, right, bottom, top;
    left = x0;
    right = x2;
    bottom = y0;
    top = y2;

    float lambda_numerator = projector->z_viewport;
    float offset_x = projector->offset_x;
    float offset_y = projector->offset_y;

    plane[Frustum::LEFT] = Plane3(lambda_numerator,
                                  offset_x - left - border + fudge, 0, 0);

    plane[Frustum::RIGHT] =
        Plane3(-lambda_numerator, -(offset_x - right + border), 0, 0);

    plane[Frustum::FLOOR] =
        Plane3(0, -(bottom + border) + offset_y, lambda_numerator, 0);

    plane[Frustum::CEILING] =
        Plane3(0, (top - offset_y - border + fudge), -lambda_numerator, 0);

    plane[Frustum::HITHER] = Plane3(0, 1, 0, -0.01);

    plane[Frustum::YON] = Plane3(0, -1, 0, YON_RANGE);

    int i;
    for (i = 0; i < num_planes; i++) {
        plane[i].normalize();
    }
}

