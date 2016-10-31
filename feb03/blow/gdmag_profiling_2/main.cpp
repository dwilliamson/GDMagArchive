#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "profiler/profiler_lowlevel.h"
#include "profiler/profiler_highlevel.h"

#include <math.h>
#include <gl/glu.h>

Define_Zone(draw_scene);
Define_Zone(draw_entities);
Define_Zone(draw_ground);
Define_Zone(ai_pathfind);
Define_Zone(stream_music);
Define_Zone(draw_lens_flare);
Define_Zone(padding);

char *app_name = "Interactive Profiling #2";

Profile_Tracker *profile_tracker;
Font *big_font;
Font *small_font;

bool mouselook;
bool smooth_ai_pathfind = false;

int last_cursor_x, last_cursor_y;

int num_entities_drawn;

const float DEGREES_PER_MICKEY_X = 0.2f;
const float DEGREES_PER_MICKEY_Y = 0.2f;
const float PI = 3.141592653589;

float view_theta;
float view_phi;
float cos_viewcone_angle;

Vector3 view_pos;
Vector3 view_direction;
Vector3 light_direction(-0.7f, -0.3f, 0.4f);
Vector3 sun_direction(-0.7f, -0.3f, 0.4f);
Vector3 camera_left_vector, camera_up_vector;

char *crate_texture_names[] = {
    "data\\Gdt001.jpg", "data\\darkrock001.jpg", "data\\target.jpg",
    "data\\rockwall001.jpg", "data\\5GraniteBrown001.jpg"
};
const int NUM_CRATE_TEXTURES = sizeof(crate_texture_names) / sizeof(char *);
Loaded_Texture_Info crate_textures[NUM_CRATE_TEXTURES];
Loaded_Texture_Info sun_texture;
Loaded_Texture_Info ground_texture;

char *lens_flare_texture_names[] = {
    "data\\flare0.jpg", "data\\flare1.jpg", "data\\flare2.jpg",
    "data\\flare3.jpg", "data\\flare4.jpg"
};
const int NUM_LENS_FLARE_TEXTURES = sizeof(lens_flare_texture_names) / sizeof(char *);
Loaded_Texture_Info lens_flare_textures[NUM_LENS_FLARE_TEXTURES];


bool moving_forward, moving_backward, moving_left, moving_right;

const float MOVEMENT_SPEED = 20.0f;
const float GROUND_SIZE = 50.0f;

const float COST_STREAM_MUSIC = 0.006;
const float COST_AI_PATHFIND = 0.024;
const float COST_RENDER_ENTITY = 0.0005;
const float COST_RENDER_GROUND = 0.012;
const float COST_LENS_FLARE = 0.0018;


struct Entity {
    Entity();

    Vector3 position;
    Vector3 extents;
    float theta;

    int texture_handle;
};

const int MAX_ENTITIES = 1000;
int num_entities;
Entity *entities[MAX_ENTITIES];


int busy_wait_iterations_per_second;
static double busy_double = 1;


static void busy_loop(int iterations) {
    busy_double = 1;

    int i;
    for (i = 0; i < iterations; i++) {
        busy_double *= .999992182341342124;
    }
}

static void busy_wait(float seconds) {
    int iterations = busy_wait_iterations_per_second * seconds;
    busy_loop(iterations);
}

void init_busy_wait() {
    const int ITERATIONS = 10000000;

    double t0 = app_shell->get_time();
    busy_loop(ITERATIONS);
    double t1 = app_shell->get_time();

    busy_wait_iterations_per_second = ITERATIONS / (t1 - t0);
}



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

void init_view_transform() {
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

    camera_up_vector = upward;
    camera_left_vector = cross_product(forward, upward);

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

}

void draw_entities() {
    Profile_Scope(draw_entities);

    int num_drawn = 0;
    int i;
    for (i = 0; i < num_entities; i++) {
        if (entity_is_visible(entities[i])) {
            draw_entity(entities[i]);
            num_drawn++;
        }
    }

    busy_wait(num_drawn * COST_RENDER_ENTITY);
    num_entities_drawn = num_drawn;
}

void ai_pathfind() {
    Profile_Scope(ai_pathfind);

    const int QUANTA = 10000;
    float randomness = (rand() % QUANTA) / (float)QUANTA;
    if (smooth_ai_pathfind) randomness = 0.5f;

    busy_wait(COST_AI_PATHFIND * randomness);
}

void stream_music() {
    Profile_Scope(stream_music);
    busy_wait(COST_STREAM_MUSIC);
}

void turn_cursor_off() {
    while (1) {
        int count = ShowCursor(FALSE);
        if (count < 0) break;
    }
}

void turn_cursor_on() {
    while (1) {
        int count = ShowCursor(TRUE);
        if (count >= 0) break;
    }
}

void reset_cursor_to_center() {
    int center_x = app_shell->screen_width * 0.5f;
    int center_y = app_shell->screen_height * 0.5f;

    SetCursorPos(center_x, center_y);

    POINT cursor_point;
    BOOL success = GetCursorPos(&cursor_point);
    if (success) {
        last_cursor_x = cursor_point.x;
        last_cursor_y = cursor_point.y;
    }
}

void handle_mouse(double dt) {
    if (mouselook) {
        POINT cursor_point;
        BOOL success = GetCursorPos(&cursor_point);
        if (success) {
            float dx = cursor_point.x - last_cursor_x;
            float dy = cursor_point.y - last_cursor_y;

            view_theta += dx * DEGREES_PER_MICKEY_X;
            view_phi -= dy * DEGREES_PER_MICKEY_Y;
        }

        reset_cursor_to_center();
    }

    // Handle motion

    float dx = 0, dy = 0;
    if (moving_forward) dy += 1.0f;
    if (moving_backward) dy -= 1.0f;
    if (moving_left) dx += 1.0f;
    if (moving_right) dx -= 1.0f;

    dx *= MOVEMENT_SPEED * dt;
    dy *= MOVEMENT_SPEED * dt;

    Vector3 e1(-view_direction.y, view_direction.x, 0);
    Vector3 e2(view_direction.x, view_direction.y, 0);

    if (e1.length_squared() == 0) return;
    if (e2.length_squared() == 0) return;

    e1.normalize();
    e2.normalize();

    e1 = e1.scale(dx);
    e2 = e2.scale(dy);

    view_pos = view_pos.add(e1).add(e2);

    float s = GROUND_SIZE;
    if (view_pos.x < -s) view_pos.x = -s;
    if (view_pos.x > +s) view_pos.x = +s;
    if (view_pos.y < -s) view_pos.y = -s;
    if (view_pos.y > +s) view_pos.y = +s;
}

void draw_billboard_quad(Vector3 position, float radius, int texture_handle,
                         float alpha = 1.0f) {
    Vector3 left = camera_left_vector.scale(radius);
    Vector3 up = camera_up_vector.scale(radius);
    

    Vector3 p0, p1, p2, p3;
    p0 = position.subtract(left).subtract(up);
    p1 = position.add(left).subtract(up);
    p2 = position.add(left).add(up);
    p3 = position.subtract(left).add(up);

    glColor4f(1, 1, 1, alpha);

    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDisable(GL_DEPTH_TEST);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3fv((float *)&p0);

    glTexCoord2f(1, 0);
    glVertex3fv((float *)&p1);

    glTexCoord2f(1, 1);
    glVertex3fv((float *)&p2);

    glTexCoord2f(0, 1);
    glVertex3fv((float *)&p3);
    glEnd();

    glDisable(GL_BLEND);
}

void draw_lens_flare(Vector3 sun_pos, float sun_radius, float lens_severity) {
    Profile_Scope(draw_lens_flare);
    
    const float MAX_FLARES = 14.0f;
    float f_num_flares = lens_severity * MAX_FLARES;
    int num_flares = (int)floor(f_num_flares);
    float residue = f_num_flares - num_flares;
    if (residue < 0) residue = 0;
    if (residue > 1) residue = 1;

    busy_wait(COST_LENS_FLARE * num_flares);

    Vector3 delta = view_pos.subtract(sun_pos);

    int i;
    for (i = 0; i < num_flares; i++) {
        float t = i / MAX_FLARES;
        Vector3 flare_vector = delta.scale(t);
        Vector3 flare_pos = sun_pos.add(flare_vector);

        int handle_index = i % NUM_LENS_FLARE_TEXTURES;

        float alpha = 1;
        if (i == num_flares - 1) alpha = residue;
        draw_billboard_quad(flare_pos, sun_radius * 1.1,
                            lens_flare_textures[handle_index].texture_handle, 
                            alpha);
    }
}

void draw_sun() {
    const float SUN_RADIUS = 10.0f;
    const float SUN_DISTANCE = 100.0f;

    // Compute a sun position that is relative to the viewpoint
    Vector3 sun_pos = sun_direction.scale(SUN_DISTANCE).add(view_pos);

    draw_billboard_quad(sun_pos, SUN_RADIUS, sun_texture.texture_handle);

    Vector3 v1 = sun_direction;
    v1.normalize();
    Vector3 v2 = view_direction;
    v2.normalize();
    float dot = dot_product(v1, v2);
    
    float LENS_DOT_MIN = 0.71f;
    float LENS_DOT_MAX = 0.99998f;

    float lens_severity = (dot - LENS_DOT_MIN) / (LENS_DOT_MAX - LENS_DOT_MIN);
    if (lens_severity > 0) {
        if (lens_severity > 1) lens_severity = 1;
        draw_lens_flare(sun_pos, SUN_RADIUS, lens_severity);
    }
}

void draw_scene() {
    Profile_Scope(draw_scene);

    double now = app_shell->get_time();
    double dt;
    static double last_time = 0;
    if (last_time == 0) {
        last_time = now;
    }

    dt = now - last_time;
    last_time = now;

    handle_mouse(dt);

    int x, y;
    x = app_shell->mouse_pointer_x;
    y = app_shell->mouse_pointer_y;

	    // We divide y by the screen width, not the height... this is
        // so that the y axis of the coordinate system won't be
        // scaled with respect to the x.

    float fx = x / (float)app_shell->screen_width;
    float fy = y / (float)app_shell->screen_width;

    glColor4f(1, 1, 1, 1);
    app_shell->triangle_mode_begin();

    init_view_transform();


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

    draw_sun();

    glEnable(GL_DEPTH_TEST);

    draw_ground();
    draw_entities();

    app_shell->triangle_mode_end();


    ai_pathfind();
    stream_music();


    int header_y = app_shell->screen_height - 8 - big_font->character_height;
    app_shell->text_mode_begin(big_font);
    char *mouselook_string = "OFF";
    if (mouselook) mouselook_string = "ON";

    char buf[256];
    sprintf(buf, "Mouse Look is %s (press spacebar to toggle).", mouselook_string);
    app_shell->draw_text(big_font, 8, header_y, buf);
    app_shell->text_mode_end();

    profile_tracker->enable_feature_map_update(mouselook);
    profile_tracker->set_feature_map_reporting(!mouselook,
                                               app_shell->mouse_pointer_x,
                                               app_shell->mouse_pointer_y);
    profile_tracker->update();
    profile_tracker->draw(0, 300);
}

void handle_keydown(int key) {
    if (key == ' ') {
        mouselook = !mouselook;
        if (mouselook) {
            turn_cursor_off();
            reset_cursor_to_center();
        } else {
            turn_cursor_on();
        }
    }

    switch (key) {
    case 'W':
    case App_Shell::ARROW_UP:
        moving_forward = true;
        break;
    case 'A':
    case App_Shell::ARROW_LEFT:
        moving_left = true;
        break;
    case 'S':
    case App_Shell::ARROW_DOWN:
        moving_backward = true;
        break;
    case 'D':
    case App_Shell::ARROW_RIGHT:
        moving_right = true;
        break;
    case 'C':
        profile_tracker->selected_map_index = -1;
        break;

    case '1':
        profile_tracker->set_displayed_quantity(Profile_Tracker::SELF_TIME);
        break;
    case '2':
        profile_tracker->set_displayed_quantity(Profile_Tracker::SELF_STDEV);
        break;
    case '3':
        profile_tracker->set_displayed_quantity(Profile_Tracker::HIERARCHICAL_TIME);
        break;
    case '4':
        profile_tracker->set_displayed_quantity(Profile_Tracker::HIERARCHICAL_STDEV);
        break;
    case '0':
        smooth_ai_pathfind = !smooth_ai_pathfind;
    };
}

void handle_keyup(int key) {

    switch (key) {
    case 'W':
    case App_Shell::ARROW_UP:
        moving_forward = false;
        break;
    case 'A':
    case App_Shell::ARROW_LEFT:
        moving_left = false;
        break;
    case 'S':
    case App_Shell::ARROW_DOWN:
        moving_backward = false;
        break;
    case 'D':
    case App_Shell::ARROW_RIGHT:
        moving_right = false;
        break;
    };
}

float random_positive_scalar(float range) {
    const int QUANTA = 10000;
    float value = (rand() % QUANTA) / (float)(QUANTA);
    value *= range;

    return value;
}

float random_symmetric_scalar(float range) {
    const int QUANTA = 10000;
    float value = ((rand() % QUANTA) - QUANTA / 2) / (float)(QUANTA/2);
    value *= range;

    return value;
}

int get_crate_texture() {
    int index = rand() % NUM_CRATE_TEXTURES;
    return crate_textures[index].texture_handle;
}

void app_init() {
    init_busy_wait();

    view_pos = Vector3(-5, 0, 5);
    light_direction.normalize();

    profile_tracker = new Profile_Tracker();

    big_font = app_shell->load_font("data\\Century Big");
    small_font = app_shell->load_font("data\\Century Small");

    int i;
    for (i = 0; i < NUM_CRATE_TEXTURES; i++) {
        app_shell->load_texture(&crate_textures[i], crate_texture_names[i]);
    }

    for (i = 0; i < NUM_LENS_FLARE_TEXTURES; i++) {
        app_shell->load_texture(&lens_flare_textures[i], lens_flare_texture_names[i]);
    }

    app_shell->load_texture(&ground_texture, "data\\ground.jpg");
    app_shell->load_texture(&sun_texture, "data\\sun.jpg");



    // Stack O' Crates
    const int CRATES_IN_STACK = 7;
    const float CRATE_STACK_RADIUS = 0.6f;
    for (i = 0; i < CRATES_IN_STACK; i++) {
        Entity *e = new Entity();
        e->texture_handle = get_crate_texture();
        float z = (2*i + 1) * CRATE_STACK_RADIUS;
        e->position = Vector3(7, 2, z);

        float r = CRATE_STACK_RADIUS;
        e->extents = Vector3(r, r, r);

        float theta = random_symmetric_scalar(0.44f);  // Small angular variation

        e->theta = theta;
        add_entity(e);
    }


    // Random crates on the ground at random orientations
    const int NUM_MISC_CRATES = 12;
    for (i = 0; i < NUM_MISC_CRATES; i++) {
        Entity *e = new Entity();

        float theta = random_positive_scalar(2*PI);
        float r = random_positive_scalar(3.5f);

        Vector3 crate_pos;
        while (1) {  // Don't make any crates too close to the starting point!
            float s = GROUND_SIZE;
            float x = random_symmetric_scalar(s);
            float y = random_symmetric_scalar(s);
            crate_pos.set(x, y, r);
            Vector3 delta = crate_pos.subtract(view_pos);
            if (delta.length_squared() > 25) break;
        }

        e->texture_handle = get_crate_texture();
        e->position = crate_pos;
        e->extents = Vector3(r, r, r);

        e->theta = theta;
        add_entity(e);
    }

    // Random crates on the ground tucked in one corner
    for (i = 0; i < 43; i++) {
        Entity *e = new Entity();

        float theta = random_positive_scalar(2*PI);
        float r = random_positive_scalar(1.5f);

        float s = GROUND_SIZE * 0.1f;
        float x = random_symmetric_scalar(s) - GROUND_SIZE * 0.85f;
        float y = random_symmetric_scalar(s) - GROUND_SIZE * 0.85f;

        e->texture_handle = get_crate_texture();
        e->position = Vector3(x, y, r);
        e->extents = Vector3(r, r, r);

        e->theta = theta;
        add_entity(e);
    }

    
    glClearColor(0.5, 0.5, 0.8, 0.0);
}



