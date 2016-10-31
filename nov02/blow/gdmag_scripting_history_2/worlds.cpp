#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "scripting.h"
#include "worlds.h"

// Crap for figures
#include <math.h>
#include <gl/glu.h>
#include <float.h>

#include "command_execution.h"
#include "standard_commands.h"


void world_draw_item(Loaded_Texture_Info *texture, Vector2 *position,
                     float radius) {
    float w = radius * 0.5f;
    float x = position->x;
    float y = position->y;

    float ix0 = (x - w) * app_shell->screen_width;
    float ix1 = (x + w) * app_shell->screen_width;
    float iy0 = (y - w) * app_shell->screen_width;
    float iy1 = (y + w) * app_shell->screen_width;

    glColor3f(1, 1, 1);
    app_shell->bitmap_mode_begin(texture->texture_handle);
    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(ix0, iy0);

    glTexCoord2f(1, 0);
    glVertex2f(ix1, iy0);

    glTexCoord2f(1, 1);
    glVertex2f(ix1, iy1);

    glTexCoord2f(0, 1);
    glVertex2f(ix0, iy1);
    glEnd();

    app_shell->bitmap_mode_end();
}


//
// World3 stuff below.
// World3 is the Thief tailing game.
//


Loaded_Texture_Info thief_texture;
Loaded_Texture_Info messenger_texture;

enum World3_Movement_Mode {
    MOVE_STOPPED,
    MOVE_WALKING
};

World3_Movement_Mode messenger_movement_mode;
Vector2 messenger_direction = { 1, 0 };
double messenger_time_for_next_mode_switch = 0;
const float MESSENGER_STOP_PROBABILITY = 0.1f;
const float MESSENGER_PAUSE_TIME = 2.5f;
const float MESSENGER_WALK_TIME = 0.9f;
const float MESSENGER_SPEED = 0.03f;
const float MESSENGER_PERTURB_SCALE = 0.4f;

void world3_init() {
    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
    app_shell->load_texture(&messenger_texture, "data\\messenger.jpg");
    app_shell->load_texture(&thief_texture, "data\\thief.jpg");

    // Provide initial values for messenger and thief positions.

	Variable_Binding *messenger = bindings->lookup("messenger_position");
	if (messenger && (messenger->value.type == VARTYPE_VECTOR2)) {
        Vector2 pos;
        pos.set(0.1f, 0.3f);
        messenger->value.vector2.set_for_all_history(pos);
    }

	Variable_Binding *thief = bindings->lookup("thief_position");
	if (thief && (thief->value.type == VARTYPE_VECTOR2)) {
        Vector2 pos;
        pos.set(0.7f, 0.7f);
        thief->value.vector2.set_for_all_history(pos);
    }
}

inline float random_0_to_1() {
    const int NUM_QUANTA = 1000;
    int value = rand() % NUM_QUANTA;
    float float_value = (float)(value / (float)(NUM_QUANTA - 1));
    return float_value;
}

void world3_update_messenger(double now, double dt) {
    if (now >= messenger_time_for_next_mode_switch) {
        float float_value = random_0_to_1();

        float x = random_0_to_1();
        float y = random_0_to_1();
        x *= 2;
        x -= 1;
        y *= 2;
        y -= 1;

        if (float_value <= MESSENGER_STOP_PROBABILITY) {
            messenger_direction.set(0, 0);
            messenger_movement_mode = MOVE_STOPPED;
            messenger_time_for_next_mode_switch = now + MESSENGER_PAUSE_TIME;
        } else if (messenger_movement_mode == MOVE_STOPPED) {
            messenger_movement_mode = MOVE_WALKING;
            messenger_direction.set(x, y);
            messenger_time_for_next_mode_switch = now + MESSENGER_WALK_TIME;
        } else {
            x *= MESSENGER_PERTURB_SCALE;
            y *= MESSENGER_PERTURB_SCALE;
            messenger_direction.x += x;
            messenger_direction.y += y;
            messenger_time_for_next_mode_switch = now + MESSENGER_WALK_TIME;
        }
    }

    messenger_direction.normalize();
    float dx = messenger_direction.x * MESSENGER_SPEED * dt;
    float dy = messenger_direction.y * MESSENGER_SPEED * dt;

    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
	Variable_Binding *messenger = bindings->lookup("messenger_position");
    if (!messenger) return;
    if (messenger->value.type != VARTYPE_VECTOR2) return;

    float x = messenger->value.vector2.history_values[0].x;
    float y = messenger->value.vector2.history_values[0].y;

    float new_x = x + dx;
    float new_y = y + dy;
    bool bounce = false;
    if (new_x < 0) {
        bounce = true;
        new_x = 0;
    }
    if (new_y < 0) {
        bounce = true;
        new_y = 0;
    }

    if (new_x > 1) {
        bounce = true;
        new_x = 1;
    }

    float ymax = app_shell->screen_height / (float)app_shell->screen_width;
    if (new_y > ymax) {
        bounce = true;
        new_y = ymax;
    }

    if (bounce) {
        messenger_direction.x = -messenger_direction.x;
        messenger_direction.y = -messenger_direction.y;
    }


    Vector2 vec;
    vec.set(new_x, new_y);

    messenger->value.vector2.set(vec);
}

void world3_update(double dt) {
    double now = app_shell->get_time();

    world3_update_messenger(now, dt);
}

void world3_draw() {
    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
	Variable_Binding *thief = bindings->lookup("thief_position");
	Variable_Binding *messenger = bindings->lookup("messenger_position");

    const float UNIT_SIZE = 0.1f;
	if (thief && (thief->value.type == VARTYPE_VECTOR2)) {
        world_draw_item(&thief_texture, &thief->value.vector2.history_values[0],
                        UNIT_SIZE);
    }

	if (messenger && (messenger->value.type == VARTYPE_VECTOR2)) {
        world_draw_item(&messenger_texture, &messenger->value.vector2.history_values[0],
                        UNIT_SIZE);
    }
}


//
// World2 stuff below.
// World2 is the mortar-and-target game.
//
const int WORLD2_MAX_PROJECTILES = 100;
const int WORLD2_MAX_EXPLOSIONS = 100;

int world2_num_projectiles = 0;
int world2_num_explosions = 0;

const float WORLD2_EXPLOSION_DURATION = 0.3f;
const float WORLD2_PROJECTILE_RADIUS = 0.009f;
const float WORLD2_EXPLOSION_RADIUS = 0.11f;
const float WORLD2_MUZZLE_VELOCITY = 1.0f;
const float WORLD2_ACCELERATION_DUE_TO_GRAVITY = -1.2;

struct World2_Projectile {
    Vector3 position;
    Vector3 velocity;
    float red, green, blue;
};

struct World2_Explosion {
    Vector3 position;
    float start_time;
    float brightness;
    float radius;
};

World2_Projectile world2_projectiles[WORLD2_MAX_PROJECTILES];
World2_Explosion world2_explosions[WORLD2_MAX_PROJECTILES];

Loaded_Texture_Info mortar_texture;
Loaded_Texture_Info target_texture;

void world2_init() {
    app_shell->load_texture(&target_texture, "data\\target.jpg");
    app_shell->load_texture(&mortar_texture, "data\\mortar.jpg");
}

void world2_add_explosion(Vector3 position, double now) {
    if (world2_num_explosions == WORLD2_MAX_PROJECTILES) return;
    World2_Explosion *explosion = &world2_explosions[world2_num_explosions];
    world2_num_explosions++;

    explosion->position = position;
    explosion->start_time = now;
}

void world2_update(double dt) {
    double now = app_shell->get_time();

    int i;
    for (i = 0; i < world2_num_projectiles; i++) {
        World2_Projectile *projectile = &world2_projectiles[i];
        Vector3 distance = projectile->velocity;
        distance.scale(dt);

        projectile->position = projectile->position.add(distance);
        projectile->position.z += 0.5 * dt * dt * WORLD2_ACCELERATION_DUE_TO_GRAVITY;
        projectile->velocity.z += dt * WORLD2_ACCELERATION_DUE_TO_GRAVITY;

        if (projectile->position.z < 0) {
            world2_add_explosion(projectile->position, now);
            world2_projectiles[i] = world2_projectiles[world2_num_projectiles - 1];
            world2_num_projectiles--;
            i--;
        }
    }

    for (i = 0; i < world2_num_explosions; i++) {
        World2_Explosion *explosion = &world2_explosions[i];
        double diff = now - explosion->start_time;
        if (diff > WORLD2_EXPLOSION_DURATION) {
            world2_explosions[i] = world2_explosions[world2_num_explosions - 1];
            world2_num_explosions--;
            i--;
        } else {
            float ratio = diff / WORLD2_EXPLOSION_DURATION;
            explosion->radius = WORLD2_EXPLOSION_RADIUS * ratio;

            float brightness = 1.0f - ratio;

            if (brightness < 0) brightness = 0;
            if (brightness > 1) brightness = 1;
            explosion->brightness = sqrt(brightness);
        }
    }
}

void world2_draw_units() {

    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
	Variable_Binding *mortar = bindings->lookup("mortar_position");

    const float UNIT_SIZE = 0.05f;
	if (mortar && (mortar->value.type == VARTYPE_VECTOR2)) {
        world_draw_item(&mortar_texture, &mortar->value.vector2.history_values[0],
                        UNIT_SIZE * 1.5f);
    }
    
	Variable_Binding *target = bindings->lookup("target_position");
	if (target && (target->value.type == VARTYPE_VECTOR2)) {
        world_draw_item(&target_texture, &target->value.vector2.history_values[0],
                        UNIT_SIZE);
    }
}

void draw_circle_at(Vector3 position, float radius) {
    const int NUM_CIRCLE_VERTICES = 16;
    
    double dtheta = 2 * M_PI / (NUM_CIRCLE_VERTICES - 1);

    app_shell->triangle_mode_begin();

    glBegin(GL_TRIANGLE_FAN);

    float w = app_shell->screen_width;
    float h = app_shell->screen_height;

    float x = position.x;
    float y = position.y + position.z / 2.0;

    glVertex2f(x * w, y * w);
    int i;
    for (i = 0; i < NUM_CIRCLE_VERTICES; i++) {
        double theta = dtheta * i;
        double ct = cos(theta);
        double st = sin(theta);

        double px = radius * ct;
        double py = radius * st;
        
        glVertex2f((x + px) * w, (y + py) * w); 
    }

    glEnd();
    app_shell->triangle_mode_end();
}

void world2_draw_shadows() {
    glColor3f(0.1f, 0.1f, 0.1f);
    int i;
    for (i = 0; i < world2_num_projectiles; i++) {
        World2_Projectile *projectile = &world2_projectiles[i];
        Vector3 position = projectile->position;
        position.z = 0;
        draw_circle_at(position, WORLD2_PROJECTILE_RADIUS);
    }
}

void world2_draw_projectiles() {
    int i;
    for (i = 0; i < world2_num_projectiles; i++) {
        World2_Projectile *projectile = &world2_projectiles[i];
        Vector3 position = projectile->position;
        glColor3f(projectile->red, projectile->green, projectile->blue);
        draw_circle_at(position, WORLD2_PROJECTILE_RADIUS);
    }
}

void world2_draw_explosions() {
    int i;
    for (i = 0; i < world2_num_explosions; i++) {
        World2_Explosion *explosion = &world2_explosions[i];
        float f = explosion->brightness;
        glColor4f(1, 1, 0.5f, f);
        draw_circle_at(explosion->position, explosion->radius);
    }
}

void world2_draw() {
    world2_draw_shadows();
    world2_draw_explosions();
    world2_draw_units();
    world2_draw_projectiles();
}

void c_fire_mortar(Command_Calling_Data *data) {
    if (world2_num_projectiles == WORLD2_MAX_PROJECTILES) return;

    bool success;
    success = check_number_of_arguments(data, 4);
    success &= ensure_argument_is_vector2(data, 0);
    success &= ensure_argument_is_scalar(data, 1);
    success &= ensure_argument_is_scalar(data, 2);
    success &= ensure_argument_is_scalar(data, 3);

    if (!success) return;
    Vector2 target = get_vector2(data, 0)->history_values[0];

    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
	Variable_Binding *mortar = bindings->lookup("mortar_position");
    if (!mortar) return;
    if (mortar->value.type != VARTYPE_VECTOR2) return;

    Vector2 position = mortar->value.vector2.history_values[0];

    float dx, dy;
    dx = target.x - position.x;
    dy = target.y - position.y;
    float length = sqrt(dx*dx + dy*dy);
    if (length < 0.00001) return;  // Too close to fire at!

    float nx = dx / length;
    float ny = dy / length;


    double iv2 = 1.0 / (WORLD2_MUZZLE_VELOCITY * WORLD2_MUZZLE_VELOCITY);

    double sin_2theta = -length * WORLD2_ACCELERATION_DUE_TO_GRAVITY * iv2;
    if (sin_2theta < -1) sin_2theta = -1;
    if (sin_2theta > +1) sin_2theta = +1;

    double theta = 0.5 * asin(sin_2theta);
    theta = M_PI * 0.5 - theta;

    float vz = WORLD2_MUZZLE_VELOCITY * sin(theta);
    float vx = WORLD2_MUZZLE_VELOCITY * cos(theta);

    World2_Projectile *result = &world2_projectiles[world2_num_projectiles++];
    result->position.x = position.x;
    result->position.y = position.y;
    result->position.z = 0;
    result->velocity.x = nx * vx;
    result->velocity.y = ny * vx;
    result->velocity.z = vz;
    result->red = get_scalar(data, 1)->history_values[0];
    result->green = get_scalar(data, 2)->history_values[0];
    result->blue = get_scalar(data, 3)->history_values[0];
}


//
// World1 stuff below.
// World1 is the Dance Dance Revolution game.
//

enum Arrow_Direction {
    ARROW_DIR_WEST, ARROW_DIR_SOUTH, ARROW_DIR_NORTH, ARROW_DIR_EAST,

    NUM_ARROW_DIRECTIONS
};

const int MAX_ARROWS = 32;
float arrow_y_coordinates[MAX_ARROWS];
Arrow_Direction arrow_directions[MAX_ARROWS];
bool arrow_was_hit[MAX_ARROWS];

float arrow_x_coordinate_by_type[NUM_ARROW_DIRECTIONS];

const float ARROW_WIDTH = 0.2f;
const float ARROW_SIDE_PADDING = 0.1f;
const float ARROW_OUTLINE_TOP_PADDING = 0.5f * ARROW_WIDTH;
const float ARROW_MOVEMENT_RATE = 0.5f; // In screen-widths per second
const float TIME_FOR_FASTEST_NOTE = 0.4f;
const float TIME_FOR_OUTLINE_FLASH = 0.1f;

const float ARROW_TARGET_WIDTH = ARROW_WIDTH * 0.8f;

int num_arrows;
Loaded_Texture_Info arrow_texture;
Loaded_Texture_Info outline_texture;
Loaded_Texture_Info flashy_outline_texture;
double time_of_next_arrow = 0;
double time_to_turn_off_outline_flash = 0;
int outline_flash_index = -1;

char *judgement_string = NULL;
float judgement_string_x, judgement_string_y;

// Stuff about tweaking texture coords to make the arrow point the right way
int texture_index_offset_by_type[NUM_ARROW_DIRECTIONS] = { 3, 2, 0, 1 };
Vector2 arrow_texture_coords[NUM_ARROW_DIRECTIONS] = {
    { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 }
};

void world1_init() {
    num_arrows = 0;

    // Set up the x-coordinates for the centers of the 4 columns of arrows.

    float occupied_space = 1.0f - 2 * ARROW_SIDE_PADDING;
    float gap_space = occupied_space - 4 * ARROW_WIDTH;
    gap_space /= 3.0;

    float x_cursor = ARROW_SIDE_PADDING + ARROW_WIDTH * 0.5f;
    arrow_x_coordinate_by_type[ARROW_DIR_WEST] = x_cursor;
    x_cursor += ARROW_WIDTH + gap_space;
    arrow_x_coordinate_by_type[ARROW_DIR_SOUTH] = x_cursor;
    x_cursor += ARROW_WIDTH + gap_space;
    arrow_x_coordinate_by_type[ARROW_DIR_NORTH] = x_cursor;
    x_cursor += ARROW_WIDTH + gap_space;
    arrow_x_coordinate_by_type[ARROW_DIR_EAST] = x_cursor;

    app_shell->load_texture(&outline_texture, "data\\arrow_outline.jpg");
    app_shell->load_texture(&flashy_outline_texture, "data\\arrow_outline_flashy.jpg");
    app_shell->load_texture(&arrow_texture, "data\\arrow.jpg");


    // Provide an initial value for player goodness.
    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
    Variable_Binding *goodness = bindings->lookup("player_goodness");
    if (!goodness) return;
    if (goodness->value.type != VARTYPE_SCALAR) return;
    goodness->value.scalar.set_for_all_history(0.5f);
}

void change_player_goodness(float score) {
    Variable_Binding_List *bindings = scripting_system->variable_binding_list;
	Variable_Binding *goodness = bindings->lookup("player_goodness");
    if (!goodness) return;
    if (goodness->value.type != VARTYPE_SCALAR) return;
	goodness->value.scalar.set(score);
}

float get_target_y() {
    float y_aspect = app_shell->screen_height / (float)app_shell->screen_width;
    float y_center = y_aspect - ARROW_OUTLINE_TOP_PADDING - 0.5f * ARROW_WIDTH;

    return y_center;
}

void world1_update(double dt) {
    int i;
    for (i = 0; i < num_arrows; i++) {
        arrow_y_coordinates[i] += dt * ARROW_MOVEMENT_RATE;

        // Check to see if we missed an arrow.
        float target_y = get_target_y();
        if (arrow_y_coordinates[i] > target_y + ARROW_TARGET_WIDTH * 0.5f) {
            if (arrow_was_hit[i] == false) {
                arrow_was_hit[i] = true;
                judgement_string = "Missed!";
                judgement_string_x = arrow_x_coordinate_by_type[arrow_directions[i]];
                judgement_string_y = arrow_y_coordinates[i];
                change_player_goodness(0.0f);
            }
        }
    }

    
    // Eliminate any arrows that have expired.
    float y_aspect = app_shell->screen_height / (float)app_shell->screen_width;
    float y_limit = y_aspect + ARROW_WIDTH * 0.5f;
    while (num_arrows && (arrow_y_coordinates[0] > y_limit)) {
        for (i = 1; i < num_arrows; i++) {
            arrow_y_coordinates[i - 1] = arrow_y_coordinates[i];
            arrow_directions[i - 1] = arrow_directions[i];
            arrow_was_hit[i - 1] = arrow_was_hit[i];
        }

        num_arrows--;
    }

    // Add any new arrows...
    double now = app_shell->get_time();
    if ((now > time_of_next_arrow) && (num_arrows < MAX_ARROWS)) {
        arrow_directions[num_arrows] = (Arrow_Direction)(rand() % NUM_ARROW_DIRECTIONS);
        arrow_y_coordinates[num_arrows] = 0.0f - ARROW_WIDTH * 0.5f;
        arrow_was_hit[num_arrows] = false;
        num_arrows++;

        float delay;
        if (rand() % 4 == 0) {
            delay = TIME_FOR_FASTEST_NOTE;
        } else {
            delay = 2 * TIME_FOR_FASTEST_NOTE;
        }

        time_of_next_arrow = now + delay;
    }

    // Manage the outline...
    if (now > time_to_turn_off_outline_flash) {
        outline_flash_index = -1;
    }
}


void draw_arrow_bitmap(int texture_handle, Arrow_Direction direction, float x, float y) {
    if (texture_handle == -1) return;
    float w = ARROW_WIDTH * 0.5f;

    float ix0 = (x - w) * app_shell->screen_width;
    float ix1 = (x + w) * app_shell->screen_width;
    float iy0 = (y - w) * app_shell->screen_width;
    float iy1 = (y + w) * app_shell->screen_width;

    glColor3f(1, 1, 1);
    app_shell->bitmap_mode_begin(texture_handle);
    
    int uv_index = texture_index_offset_by_type[direction];
    Vector2 uv;

    glBegin(GL_QUADS);
    uv = arrow_texture_coords[(uv_index + 0) % NUM_ARROW_DIRECTIONS];
    glTexCoord2f(uv.x, uv.y);
    glVertex2f(ix0, iy0);

    uv = arrow_texture_coords[(uv_index + 1) % NUM_ARROW_DIRECTIONS];
    glTexCoord2f(uv.x, uv.y);
    glVertex2f(ix1, iy0);

    uv = arrow_texture_coords[(uv_index + 2) % NUM_ARROW_DIRECTIONS];
    glTexCoord2f(uv.x, uv.y);
    glVertex2f(ix1, iy1);

    uv = arrow_texture_coords[(uv_index + 3) % NUM_ARROW_DIRECTIONS];
    glTexCoord2f(uv.x, uv.y);
    glVertex2f(ix0, iy1);
    glEnd();

    app_shell->bitmap_mode_end();
}

void world1_draw_outline(Arrow_Direction direction) {
    float x_center = arrow_x_coordinate_by_type[direction];
    float y_center = get_target_y();
    
    int handle = outline_texture.texture_handle;
    if (outline_flash_index == (int)direction) handle = flashy_outline_texture.texture_handle;
    draw_arrow_bitmap(handle, direction, x_center, y_center);
}

void world1_draw() {
    world1_draw_outline(ARROW_DIR_WEST);
    world1_draw_outline(ARROW_DIR_SOUTH);
    world1_draw_outline(ARROW_DIR_NORTH);
    world1_draw_outline(ARROW_DIR_EAST);

    int i;
    for (i = 0; i < num_arrows; i++) {
        draw_arrow_bitmap(arrow_texture.texture_handle, arrow_directions[i],
                          arrow_x_coordinate_by_type[arrow_directions[i]],
                          arrow_y_coordinates[i]);
    }

    if (judgement_string) {
        app_shell->text_mode_begin();
        int sx = judgement_string_x * app_shell->screen_width;
        int sy = judgement_string_y * app_shell->screen_width;
        app_shell->draw_text_line(&sx, &sy, judgement_string);
        app_shell->text_mode_end();
    }
}

void world1_handle_key_down(int key) {
    int direction = -1;
    if (key == '!') direction = ARROW_DIR_WEST;
    if (key == '$') direction = ARROW_DIR_SOUTH;
    if (key == '#') direction = ARROW_DIR_NORTH;
    if (key == '@') direction = ARROW_DIR_EAST;

    if (direction != -1) {
        outline_flash_index = (int)direction;
        time_to_turn_off_outline_flash = app_shell->get_time() + TIME_FOR_OUTLINE_FLASH;

        // Find the arrow closest to the outline.  If it hasn't been
        // scored yet, and it is the right direction, and it is close 
        // enough, judge it.

        float target_y = get_target_y();
        float best_distance = FLT_MAX;
        int best_index = -1;

        int i;
        for (i = 0; i < num_arrows; i++) {
            if (arrow_directions[i] != direction) continue;

            float distance = fabs(arrow_y_coordinates[i] - target_y);

            if (distance < best_distance) {
                best_distance = distance;
                best_index = i;
            }
        }

        if ((best_index != -1) && (best_distance <= ARROW_TARGET_WIDTH * 0.5f) && !arrow_was_hit[best_index]) {
            float score;

            arrow_was_hit[best_index] = true;

            float ratio = best_distance / (ARROW_TARGET_WIDTH * 0.5f);

            judgement_string = "Perfect!";
            score = 1.0f;

            if (ratio > 0.25f) {
                judgement_string = "Great!";
                score = 0.75f;
            }

            if (ratio > 0.5f) {
                judgement_string = "Okay...";
                score = 0.5f;
            }

            if (ratio > 0.75f) {
                judgement_string = "Boo!";
                score = 0.25f;
            }


            judgement_string_x = arrow_x_coordinate_by_type[direction];
            judgement_string_y = arrow_y_coordinates[best_index];
            change_player_goodness(score);
        }
    }
}


//
// Hyper-lame world init stuff below.
//



void world_init(int index) {
    extern Compiled_Script *compiled_scripts[];

    switch (index) {
    case 1:
        assert(compiled_scripts[1]);

        scripting_system->variable_binding_list = compiled_scripts[1]->global_variables;
        world1_init();
        break;
    case 2:
        assert(compiled_scripts[2]);
        scripting_system->variable_binding_list = compiled_scripts[2]->global_variables;
        world2_init();
        break;
    case 3:
        assert(compiled_scripts[3]);
        scripting_system->variable_binding_list = compiled_scripts[3]->global_variables;
        world3_init();
        break;
    }
}

void world_update(int index, double dt) {
    switch (index) {
    case 1:
        world1_update(dt);
        break;
    case 2:
        world2_update(dt);
        break;
    case 3:
        world3_update(dt);
        break;
    }
}

void world_draw(int index) {
    switch (index) {
    case 1:
        world1_draw();
        break;
    case 2:
        world2_draw();
        break;
    case 3:
        world3_draw();
        break;
    }
}

void world_handle_key_down(int index, int key) {
    if (index == 1) world1_handle_key_down(key);
}
