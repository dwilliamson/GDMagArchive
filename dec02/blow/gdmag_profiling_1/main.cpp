#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "profiler/profiler_lowlevel.h"
#include "profiler/profiler_highlevel.h"

#include <math.h>

#include "main.h"
#include "entity.h"


// Definitions for the program zones we will use in
// this file (there are also some zones in the
// 'profiler' subdirectory and in entity.cpp)

Define_Zone(draw_scene);
Define_Zone(ai_pathfind);
Define_Zone(stream_music);


// Various globals that we use to control the program.

char *app_name = "Interactive Profiling #1";

Profile_Tracker *profile_tracker;
Font *big_font;
Font *small_font;

bool mouselook;
bool smooth_ai_pathfind = false;

int last_cursor_x, last_cursor_y;

float view_theta;
float view_phi;
float cos_viewcone_angle;

Vector3 view_pos;
Vector3 view_direction;
Vector3 light_direction(-0.7f, -0.3f, 0.4f);
bool moving_forward, moving_backward, moving_left, moving_right;


// Texture maps to use on the crates scattered through
// the world.
char *crate_texture_names[] = {
    "data\\Gdt001.jpg", "data\\darkrock001.jpg", "data\\target.jpg",
    "data\\rockwall001.jpg", "data\\5GraniteBrown001.jpg"
};
const int NUM_CRATE_TEXTURES = sizeof(crate_texture_names) / sizeof(char *);
Loaded_Texture_Info crate_textures[NUM_CRATE_TEXTURES];
Loaded_Texture_Info ground_texture;

void handle_mouse(double dt);


// Utility variables used for busy-waiting.
int busy_wait_iterations_per_second;
static double busy_double = 1;

// Busy-waiting code.  I used to chew up time by calling
// sleep(), but this does not work properly on SpeedStep laptops
// and my main development machine is a laptop.  So instead,
// when the app starts up, I chew on a floating point number for
// a while in a tight loop and time that.  This tells me
// (approximately) how many loop iterations I need to eprform
// on this CPU to eat up one second.  I can use that, later, to
// occupy the program for various amounts of time.  If SpeedStep
// is on at the start of the program and off later, then these
// values may be off by a factor of 2 or so, but that is not a
// big deal.  I mostly just care that the CPU times, used by each
// fake-stub routine like AI and sound, do not vanish; and also
// that they are interesting quantities relative to each other.


// Loop for some number of iterations, eating time.
static void busy_loop(int iterations) {
    busy_double = 1;

    int i;
    for (i = 0; i < iterations; i++) {
        busy_double *= .999992182341342124;
    }
}

// Loop for some number of seconds, eating time.
void busy_wait(float seconds) {
/*
    app_shell->sleep(seconds);
    return;
*/
    int iterations = busy_wait_iterations_per_second * seconds;
    busy_loop(iterations);
}

// Initialize busy wait system; figure out how many
// seconds we require per iteration.
void init_busy_wait() {
    const int ITERATIONS = 10000000;

    double t0 = app_shell->get_time();
    busy_loop(ITERATIONS);
    double t1 = app_shell->get_time();

    busy_wait_iterations_per_second = ITERATIONS / (t1 - t0);
}


// Dummy routine that represents the AI or pathfinding code
// that might exist in a game.  This function intentionally
// fluctuates its CPU usage, to provide an example of an
// unstable routine for the profiler to look at.
void ai_pathfind() {
    Profile_Scope(ai_pathfind);

    const int QUANTA = 10000;
    float randomness = (rand() % QUANTA) / (float)QUANTA;
    if (smooth_ai_pathfind) randomness = 0.5f;

    busy_wait(COST_AI_PATHFIND * randomness);
}

// Dummy routine representing the streaming of soundtrack music.
// Has a fixed CPU cost.
void stream_music() {
    Profile_Scope(stream_music);
    busy_wait(COST_STREAM_MUSIC);
}





// 'draw_scene' handles the updating and drawing for the
// game systems.  It calls all the subsystems, calls the world
// rendering routine, then tells the profiler to update, and
// draws the profiler.
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

    draw_entities();

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

    profile_tracker->update();
    profile_tracker->draw(0, 300);
}


// Utility functions for handling the mouse, mouselook, etc.

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

// Handle key events...

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


// Utility functions we use to initialize the world.
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

    app_shell->load_texture(&ground_texture, "data\\ground.jpg");



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
