/*
  The Projector is a class for describing the projection from 3D viewspace
  to 2D screen-space.  
 */
#ifndef _PROJECTOR_H
#define _PROJECTOR_H

#include "geometry_supplement.h"

struct Projector;

const int MAX_FRUSTUM_PLANES = 10;

/*
  'Frustum' represents the view frustum planes in world space.
*/
struct Frustum {
    Frustum();

    // How many planes are in this frustum?
    int num_planes;
    // The array of planes (only the first 'num_planes' of these are active,
    // and only the first 'most_planes' of these are valid).
    Plane3 plane[MAX_FRUSTUM_PLANES];
    // 'most_planes' is the number of initialized planes.  'num_planes' may
    // be temporarily set to be less than this, if for example we call
    // yon_clip_immune() to turn off the far plane.
    int most_planes;

    // 'recalibrate': Compute new frustum planes based on this projector.
    void recalibrate(Projector *projector);
    // Compute new frustum planes based on this projector and some
    // coordinates of the viewport in screen space.
    void recalibrate(Projector *projector,
                     float x0, float y0, float x1, float y1,
                     float x2, float y2, float x3, float y3);

    // If 'is_immune' is true, things won't be clipped against the
    // far clipping plane.
    void yon_clip_immune(bool is_immune);

    // Some constants useflu for clipping and the like.
    enum clip_flag {
        AGAINST_HITHER = 0x1,
        AGAINST_LEFT = 0x2,
        AGAINST_RIGHT = 0x4,
        AGAINST_FLOOR = 0x8,
        AGAINST_CEILING = 0x10,
        AGAINST_YON = 0x20,

        AGAINST_ALL = 0x3f
    };

    enum plane_index {
        HITHER = 0,
        LEFT = 1,
        RIGHT = 2,
        FLOOR = 3,
        CEILING = 4,
        YON = 5
    };

    //
    // Data members that help internal things function:
    //
    unsigned int clip_signal_flags[MAX_FRUSTUM_PLANES];

    bool weird;
    bool have_constraint;
    float cx0, cy0, cx1, cy1, cx2, cy2, cx3, cy3;
    int calibration_number;
};

struct Projector {
    Projector();
    ~Projector();

    // Set the field of view, in radians (default is M_PI * 0.5)
    void set_fov(double radians);
    // Set the viewport width and height
    void set_viewport(float width, float height);
    // Set the viewport width and height, and the offset in x and y
    // from the corner of the frame buffer where the viewport begins.
    void set_viewport_parameters(float offset_x, float offset_y,
                                 float width, float height);
    // Set the center of the viewport [(0, 0) is the corner of the frame
    // buffer]; also set the offset from the corner where the viewport begins.
    void set_offsets(double center_x, double center_y, 
                     double offset_x, double offset_y);

    // Set the screen-space center of projection
    void set_cop(float center_x, float center_y);
    // Get the screen-space center of projection
    void get_cop(float *center_x, float *center_y);

    //
    // Other stuff...
    //
    void auto_compute_offsets();

    Frustum my_frustum;

    double fov;

    double z_viewport;
    double i_z_viewport;
    double offset_x, offset_y;
    double center_x, center_y;
    
    double viewport_width, viewport_height;

    double pixels_per_unit;
};

#endif _PROJECTOR_H
