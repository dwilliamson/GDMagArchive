#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"
#include <gl/glu.h>
#include <math.h>

#include "profiler/profiler_lowlevel.h"
#include "main.h"
#include "entity.h"

// Definitions for the program zones we will use in
// this file (there are also some zones in the
// 'profiler' subdirectory and in main.cpp)

Define_Zone(draw_entities);
Define_Zone(draw_ground);

int num_entities_drawn;
int num_entities;
Entity *entities[MAX_ENTITIES];


// Entity maintenance stuff.
Entity::Entity() {
    float s = .3;

    position = Vector3(0, 0, 0);
    extents = Vector3(s, s, s);
    theta = 0;
    texture_handle = -1;
}

void add_entity(Entity *e) {
    assert(num_entities < MAX_ENTITIES);
    entities[num_entities++] = e;
}

Vector3 rotate(Vector3 v, float theta) {
    float ct = cos(theta);
    float st = sin(theta);

    float xprime = v.x * ct + v.y * st;
    float yprime = -v.y * ct + v.x * st;

    return Vector3(xprime, yprime, v.z);
}

void get_box_coordinates(Entity *entity, Vector3 results[8]) {
    Vector3 p = entity->position;
    float x = entity->extents.x;
    float y = entity->extents.y;
    float z = entity->extents.z;

    Vector3 s0(x, 0, 0);
    Vector3 s1(0, y, 0);

    s0 = rotate(s0, entity->theta);
    s1 = rotate(s1, entity->theta);

    Vector3 m0 = p.subtract(s0).subtract(s1);
    Vector3 m1 = p.add(s0).subtract(s1);
    Vector3 m2 = p.add(s0).add(s1);
    Vector3 m3 = p.subtract(s0).add(s1);

    results[0] = m0.add(Vector3(0, 0, -z));
    results[1] = m1.add(Vector3(0, 0, -z));
    results[2] = m2.add(Vector3(0, 0, -z));
    results[3] = m3.add(Vector3(0, 0, -z));
    results[4] = m0.add(Vector3(0, 0, +z));
    results[5] = m1.add(Vector3(0, 0, +z));
    results[6] = m2.add(Vector3(0, 0, +z));
    results[7] = m3.add(Vector3(0, 0, +z));
}

Vector3 get_face_normal(Vector3 *points, int n0, int n1, int n2) {
    Vector3 diff_a = points[n1].subtract(points[n0]);
    Vector3 diff_b = points[n2].subtract(points[n0]);

    Vector3 result = cross_product(diff_a, diff_b);
    result.normalize();
    
    return result;
}


// Render one face of one entity.

void draw_entity_face(Vector3 *points, int n0, int n1, int n2, int n3) {
    Vector3 normal_a = get_face_normal(points, n0, n1, n2);

    // Dot with light source to get intensity
    const float AMBIENT_STRENGTH = 0.38f;
    const float LIGHT_STRENGTH = 1.0f - AMBIENT_STRENGTH;

    float n;
    n = dot_product(light_direction, normal_a);
    if (n < 0) n = 0;
    n = n * LIGHT_STRENGTH + AMBIENT_STRENGTH;

    glTexCoord2f(0, 0);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n0]);
    glTexCoord2f(1, 0);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n1]);
    glTexCoord2f(1, 1);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n2]);

    glTexCoord2f(0, 0);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n0]);
    glTexCoord2f(1, 1);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n2]);
    glTexCoord2f(0, 1);
    glColor3f(n, n, n);
    glVertex3fv((float *)&points[n3]);

}


// Render one entire entity... since this is a tremendously
// sophisticated rendering engine, the entity is a 6-sided box
// with a texture map.
void draw_entity(Entity *entity) {
    if (entity->texture_handle) glBindTexture(GL_TEXTURE_2D, entity->texture_handle);

    Vector3 points[8];
    get_box_coordinates(entity, points);

    glBegin(GL_TRIANGLES);
    draw_entity_face(points, 0, 1, 2, 3);
    draw_entity_face(points, 1, 5, 6, 2);
    draw_entity_face(points, 5, 4, 7, 6);
    draw_entity_face(points, 3, 2, 6, 7);
    draw_entity_face(points, 4, 0, 3, 7);
    draw_entity_face(points, 4, 5, 1, 0);
    glEnd();
}

// Perform culling of the entity with the view frustum.  Actually
// I didn't want to write the whole frustum plane thing, so I am
// just detecting the entity against a cone that is more or less
// the size of the frustum.
bool entity_is_visible(Entity *e) {
    // This function is not really correct -- PLEASE don't copy it
    // if you are learning to write your own 3d engine.

    Vector3 to_vector = e->position.subtract(view_pos);
    float dist = to_vector.length();
    to_vector.normalize();

    if (dot_product(to_vector, view_direction) >= cos_viewcone_angle) return true;

    // Check all 8 points; if any of them are within the
    // view cone, it's visible (icky!)
    Vector3 points[8];
    get_box_coordinates(e, points);
    
    int i;
    for (i = 0; i < 8; i++) {
        Vector3 to_vector = points[i].subtract(view_pos);
        float dist = to_vector.length();
        to_vector.normalize();

        if (dot_product(to_vector, view_direction) >= cos_viewcone_angle) return true;
    }

    return false;
}

// Render the ground beneath our feet.
void draw_ground() {
    Profile_Scope(draw_ground);

    glBindTexture(GL_TEXTURE_2D, ground_texture.texture_handle);
    float s = GROUND_SIZE;
    float u = 12.0f;

    glBegin(GL_TRIANGLES);
    glTexCoord2f(0, 0);
    glVertex3f(-s, -s, 0);
    glTexCoord2f(u, 0);
    glVertex3f(+s, -s, 0);
    glTexCoord2f(u, u);
    glVertex3f(+s, +s, 0);

    glTexCoord2f(0, 0);
    glVertex3f(-s, -s, 0);
    glTexCoord2f(u, u);
    glVertex3f(+s, +s, 0);
    glTexCoord2f(0, u);
    glVertex3f(-s, +s, 0);
    glEnd();
}

// Set up the world viewing transform, call render_ground,
// and render all the entities.
void draw_entities() {
    Profile_Scope(draw_entities);

    glColor4f(1, 1, 1, 1);
    app_shell->triangle_mode_begin();


    Vector3 up(0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (view_phi > 90) view_phi = 90;
    if (view_phi < -90) view_phi = -90;
    float radians = PI / 180.0;
    float ct = cos(-view_theta * radians);
    float st = sin(-view_theta * radians);
    float cp = cos(view_phi * radians);
    float sp = sin(view_phi * radians);

    Vector3 e1(ct, st, 0);
    Vector3 e2(0, 0, 1);

    Vector3 forward = e1.scale(cp).add(e2.scale(sp));
    Vector3 upward = e2.scale(cp).add(e1.scale(-sp));

    view_direction = forward;

    gluLookAt(0, 0, 0, forward.x, forward.y, forward.z, upward.x, upward.y, upward.z);
    glTranslatef(-view_pos.x, -view_pos.y, -view_pos.z);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float view_angle = 75; // degrees
    float w = app_shell->screen_width;
    float h = app_shell->screen_height;
    float viewport_ratio = w / h;

    gluPerspective(view_angle, viewport_ratio, 0.1f, 1000.0f);

    float viewcone_angle = view_angle * 0.5f;  // Half-angle of the full viewing angle
    // Adjust the viewcone angle to point all the way out to the corner of the viewport,
    // instead of just to the nearest side.
    float len = sqrt(w*w+h*h);
    viewcone_angle *= (len / w);
    cos_viewcone_angle = cos(viewcone_angle * (PI / 180.0f));

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor4f(0.7f, 0.4f, 0.2f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    draw_ground();

    int num_drawn = 0;
    int i;
    for (i = 0; i < num_entities; i++) {
        if (entity_is_visible(entities[i])) {
            draw_entity(entities[i]);
            num_drawn++;
        }
    }

    app_shell->triangle_mode_end();

    busy_wait(num_drawn * COST_RENDER_ENTITY);
    num_entities_drawn = num_drawn;
}


