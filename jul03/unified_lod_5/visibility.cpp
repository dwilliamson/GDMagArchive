#include "framework.h"
#include "mesh.h"

#include <math.h>
#include <string.h>
#include <float.h>   // For FLT_MAX
#include <stdlib.h>  // For qsort

#include "make_world.h"
#include "viewpoint.h"
#include "visibility.h"

/*
    This file tests visibility of the blocks to see whether
    they are in the frustum (and should be rendered), or not.
*/


// Give us an array of points representing the extrema
// of the view frustum.
void get_frustum_points(Vector3 *points) {
    Projector *projector = client_globals.view_projector;
    Frustum *frustum = &projector->my_frustum;

    Vector3 up_vector(0, 0, 1);
    Vector3 left_vector(0, 1, 0);
    Vector3 forward_vector(1, 0, 0);

    up_vector.rotate(client_globals.camera_orientation);
    left_vector.rotate(client_globals.camera_orientation);
    forward_vector.rotate(client_globals.camera_orientation);

    Vector3 eye = client_globals.camera_position;
    float z_viewport = 0.5f;
    Vector3 center = eye + forward_vector * z_viewport;

    float fov = projector->fov;
    float s = z_viewport * tan(fov * 0.5f);
    float t = s * (projector->viewport_height / projector->viewport_width);

    Vector3 p0 = center + left_vector * s - up_vector * t;
    Vector3 p1 = center - left_vector * s - up_vector * t;
    Vector3 p2 = center - left_vector * s + up_vector * t;
    Vector3 p3 = center + left_vector * s + up_vector * t;

    Vector3 diff = p0 - eye;
    float len = diff.length();
    float ratio = VIEWING_DISTANCE / len;

    Vector3 p4 = eye + (p0 - eye) * ratio;
    Vector3 p5 = eye + (p1 - eye) * ratio;
    Vector3 p6 = eye + (p2 - eye) * ratio;
    Vector3 p7 = eye + (p3 - eye) * ratio;

    points[0] = p0;
    points[1] = p1;
    points[2] = p2;
    points[3] = p3;
    points[4] = p4;
    points[5] = p5;
    points[6] = p6;
    points[7] = p7;
}

// Given a bounding box as 'min' and 'extents', generate the 8 points
// at the corners of the box.
void get_box_points(Vector3 min, Vector3 extents, Vector3 *points) {
    Vector3 p0, p1, p2, p3, p4, p5, p6, p7;
    p0 = p1 = p2 = p3 = p4 = p5 = p6 = p7 = min;
    p1.x += extents.x;
    p2.x += extents.x;
    
    p5.x += extents.x;
    p6.x += extents.x;

    p2.y += extents.y;
    p3.y += extents.y;

    p6.y += extents.y;
    p7.y += extents.y;

    p4.z += extents.z;
    p5.z += extents.z;
    p6.z += extents.z;
    p7.z += extents.z;

    points[0] = p0;
    points[1] = p1;
    points[2] = p2;
    points[3] = p3;
    points[4] = p4;
    points[5] = p5;
    points[6] = p6;
    points[7] = p7;
}

void plane_from_points(int index, Plane3 *planes, Vector3 *points,
                       int n0, int n1, int n2) {
    Vector3 v1 = points[n1] - points[n0];
    Vector3 v2 = points[n2] - points[n0];

    Vector3 cross = cross_product(v1, v2);
    cross.normalize();

    float d = -dot_product(cross, points[n0]);
    planes[index].a = cross.x;
    planes[index].b = cross.y;
    planes[index].c = cross.z;
    planes[index].d = d;
}

void init_planes(Vector3 *points, Plane3 *planes, int *flags) {
    int i;
    for (i = 0; i < 8; i++) flags[i] = 0;

    plane_from_points(Frustum::HITHER,
                      planes, points, 0, 1, 2);
    plane_from_points(Frustum::LEFT,
                      planes, points, 4, 0, 3);
    plane_from_points(Frustum::RIGHT,
                      planes, points, 1, 5, 6);
    plane_from_points(Frustum::FLOOR,
                      planes, points, 1, 0, 5);
    plane_from_points(Frustum::CEILING,
                      planes, points, 3, 2, 6);
    plane_from_points(Frustum::YON,
                      planes, points, 5, 4, 6);
}

// Okay, this is lame.  I didn't actually want to code up a box-frustum
// test, so I just use the box to get a bounding sphere and compare
// that against the frustum.  By doing the real test, youc an probably
// reject more triangles and get slightly better frame rates.  Go check
// magic-software.com or somewhere like that for a box-frustum test.
bool box_versus_frustum(Vector3 min, Vector3 extents, Vector3 *frustum_points, Plane3 *frustum_planes) {
    Vector3 box_points[8];
    get_box_points(min, extents, box_points);
    
    Plane3 box_planes[6];
    int box_flags[8];

    float radius = extents.length() * 0.5f;
    Vector3 center = min + extents * 0.5f;

    int i;
    for (i = 0; i < 6; i++) {
        Plane3 *plane = &frustum_planes[i];
        float dot = center.x*plane->a + center.y*plane->b + center.z*plane->c + plane->d;
        if (dot >= radius) {
            return false;
        }
    }

    return true;
}


void add_appropriate_children(Auto_Array <World_Block *> *array,
                              World_Block *block) {
    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        World_Block *child = block->children[i];
        if (!child) continue;

        child->use_zfunc_lessequal = false;
        array->add(child);
    }
}

// Recursive helper function
void collect_world(World_Block *block, 
                     Segregated_World_Pieces *pieces,
                     Vector3 *frustum_points, Plane3 *frustum_planes) {

    if (block == NULL) return;

    bool visible = box_versus_frustum(block->bounding_box_corner + block->position, block->bounding_box_extents, frustum_points, frustum_planes);
    if (!visible) return;

    int i;
    switch (block->lod_instance_state) {
    case I_AM_NOT_INVOLVED:
        for (i = 0; i < MAX_CHILDREN; i++) {
            collect_world(block->children[i], pieces,
                            frustum_points, frustum_planes);
        }
        break;
    case I_AM_SINGLE:
        pieces->solid.add(block);
        break;
    case I_AM_TRANSITIONING:
        if (block->opacity == 1) {
            pieces->solid.add(block);
            add_appropriate_children(&pieces->fading, block);
        } else {
            block->use_zfunc_lessequal = true;
            pieces->fading.add(block);
            add_appropriate_children(&pieces->solid, block);
        }
        break;
    default:
        assert(0);  // Shouldn't get here!
        break;
    }
}


// Entry point
void collect_visible_world(World_Block *block, 
                             Segregated_World_Pieces *pieces) {
    if (block == NULL) return;

    Vector3 frustum_points[8];
    get_frustum_points(frustum_points);

    Plane3 frustum_planes[6];
    int frustum_flags[8];
    init_planes(frustum_points, frustum_planes, frustum_flags);

    collect_world(block, pieces,
                    frustum_points, frustum_planes);
}
