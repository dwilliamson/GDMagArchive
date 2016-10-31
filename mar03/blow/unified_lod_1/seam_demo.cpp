#include "framework.h"

#include "make_terrain.h"
#include "mesh.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include "os_specific_opengl_headers.h"
#include "framework/rendering.h"

#include <math.h>

extern bool drawing_seam_demo;


// For the seam demo we just throw a bunch of stuff in global
// variables here, that we draw later.  It's pretty untidy,
// really.
const float POLYGON_OFFSET_MAGNITUDE = -1.1f;
Vector3 light_direction(-0.7f, 0.0f, 0.5f);

const int DEMO_BLOCK_SIZE = 18;
Static_Terrain_Block *demo_block_a, *demo_block_b, *demo_block_b_reduced, *demo_block_b_reduced_twice;
Mesh_Seam *seam_a_to_b_high_res;
Mesh_Seam *seam_a_to_b_low_res;
Mesh_Seam *seam_a_to_b_lowest_res;

Vector3 original_position_a, original_position_b;

bool seam_demo_button1_was_down = false;
float seam_demo_block_angle = M_PI * 0.25f;
int seam_demo_resolution;

const float DEMO_BUMP_F1 = 3;
const float DEMO_BUMP_F2 = 2;
const float DEMO_BUMP_HEIGHT = 1.0f;

extern Transformer *global_transformer;


// To start up... generate our terrain blocks and the
// seams between them.
void init_seam_demo() {
    int map_samples = 2 * DEMO_BLOCK_SIZE - 1;

    Elevation_Map *map = new Elevation_Map(map_samples, map_samples);

    float s = 1;
    map->square_size.set(s, s, 0);
    map->map_size.set(s * map->num_squares_x, s*map->num_squares_y, 0);
    map->corner.set(0, 0, 0);

    //
    // Put some sine wave stuff in the elevation map.
    //
    int i, j;
    for (j = 0; j < map->num_samples_y; j++) {
        for (i = 0; i < map->num_samples_x; i++) {
            float x = map->corner.x + map->square_size.x * i;
            float y = map->corner.y + map->square_size.y * j;

            float f1 = cos((i / (float)(map->num_samples_x)) * DEMO_BUMP_F1 * (2*M_PI));
            float f2 = cos((j / (float)(map->num_samples_y)) * DEMO_BUMP_F2 * (2*M_PI));
            float z = f1 * f2 * DEMO_BUMP_HEIGHT;

            int index = map->get_index(x, y);
            map->samples[index] = z;
        }
    }

    build_tangent_frames(map);


    // Now copy two terrain blocks out of neighboring areas of
    // this elevation map.
    Static_Terrain_Block *block_a, *block_b;
    int *index_map_a, *index_map_b;
    block_a = make_terrain_block(map, DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE, 0, 0);
    block_b = make_terrain_block(map, DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE-1, 0);

    

    // Make a seam between the high-res block_a and block_b.
    seam_a_to_b_high_res = make_high_res_seam_x(DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE, 
                                                block_a->index_map, block_b->index_map);

    // We will eventually want low-res versions of this seam.  We
    // will make those by starting with the high-res version and
    // cross-referencing it.  So here, we make two more high-res
    // versions to start with.
    seam_a_to_b_low_res = make_high_res_seam_x(DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE, 
                                               block_a->index_map, block_b->index_map);
    seam_a_to_b_lowest_res = make_high_res_seam_x(DEMO_BLOCK_SIZE, DEMO_BLOCK_SIZE, 
                                                  block_a->index_map, block_b->index_map);

    // 'validate_seam' does some assertions to make sure we 
    // are doing the right thing here.
    void validate_seam(Mesh_Seam *seam, Static_Terrain_Block *b0, Static_Terrain_Block *b1);
    validate_seam(seam_a_to_b_high_res, block_a, block_b);


    // We don't need the index maps on the blocks any more, so
    // we delete them to save memory.
    delete [] block_a->index_map;
    delete [] block_b->index_map;
    block_a->index_map = NULL;
    block_b->index_map = NULL;


    Loaded_Texture_Info info;
    app_shell->load_texture(&info, "data/bitmaps/3grass001.jpg");

    block_a->mesh->material_info[0].texture_index = info.texture_handle;
    block_b->mesh->material_info[0].texture_index = info.texture_handle;


    Triangle_List_Mesh *reduced_mesh;


    // Create a low resolution version of block_b.
    Mesh_Reducer reducer;
    reducer.init(block_b->mesh);
    reducer.reduce(block_b->mesh->num_faces / 4);
    reducer.get_result(&reduced_mesh);

    // Map the high-res seam through the resulting index map
    // to get a low-res seam.
    reduce_seam(seam_a_to_b_low_res, block_a->mesh, &reducer);

    // Finish the various bookkeeping for this low-res block...
    Static_Terrain_Block *block_b_reduced = new Static_Terrain_Block;
    for (i = 0; i < reduced_mesh->num_vertices; i++) reduced_mesh->vertices[i] += block_b->position;
    init_block_for_mesh(block_b_reduced, reduced_mesh);
    block_b_reduced->mesh->material_info[0].texture_index = info.texture_handle;


    // Make a very low resolution version of block_b, with a much
    // lower triangle count.
    Triangle_List_Mesh *reduced_mesh_2;

    Mesh_Reducer reducer2;
    reducer2.init(block_b->mesh);
    reducer2.reduce(block_b->mesh->num_faces / 16);
    reducer2.get_result(&reduced_mesh_2);

    // Cross-reference the seam.
    reduce_seam(seam_a_to_b_lowest_res, block_a->mesh, &reducer2);

    Static_Terrain_Block *block_b_reduced_twice = new Static_Terrain_Block;
    for (i = 0; i < reduced_mesh_2->num_vertices; i++) reduced_mesh_2->vertices[i] += block_b->position;
    init_block_for_mesh(block_b_reduced_twice, reduced_mesh_2);
    block_b_reduced_twice->mesh->material_info[0].texture_index = info.texture_handle;


    // Copy these locals into global variables for the rendering
    // function to see later...

    demo_block_a = block_a;
    demo_block_b = block_b;
    demo_block_b_reduced = block_b_reduced;
    demo_block_b_reduced_twice = block_b_reduced_twice;

    original_position_a = block_a->position;
    original_position_b = block_b->position;

    delete map;
}


// Set up a single directional light to shine on our mesh.
void setup_mesh_lighting() {
    glEnable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColor4f(1, 1, 1, 1);

    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 0, 0, 0, 0 };

    if (drawing_seam_demo) {
        // Rotate the light souce into object space...

        Vector3 dir = light_direction;
        Quaternion block_spin;
        block_spin.set_from_axis_and_angle(1, 0, 0, seam_demo_block_angle);
        dir.rotate(block_spin);

        light_pos[0] = dir.x;
        light_pos[1] = dir.y; 
        light_pos[2] = dir.z;
    } else {
        // Light from some arbitrary direction in world space,
        // for the terrain demo.

        light_pos[0] = 0.1f;
        light_pos[1] = 0.7f;
        light_pos[2] = 0.3f;
    }

    // Set the various GL lighting properties.
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    GLfloat unchoked[] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, unchoked);

    const float s = 0.4f;
    GLfloat ambient_i0[] = { s, s, s, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);
}

// Set up the render state for drawing a seam...
void setup_seam_render_state(bool wireframe) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    if (wireframe) {
        glDisable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(POLYGON_OFFSET_MAGNITUDE, 0);

        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, 1);
        glEnable(GL_CULL_FACE);

        return;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setup_mesh_lighting();
}

/*
  Draw a seam.  The various colors here are for making
  the seam stand out, so's we can inspect it.
*/
struct Color4 {
    float a, r, g, b;
};

const int NUM_SEAM_COLORS = 4;
static Color4 seam_colors[NUM_SEAM_COLORS] = {
    { 0.3f, 1.0f, 0.5f, 0.15f },
    { 0.3f, 0.5f, 1.0f, 0.15f },
    { 0.3f, 1.0f, 1.0f, 0.15f },
    { 0.3f, 0.0f, 0.0f, 1.0f }
};

void draw_seam_demo_seam(Static_Terrain_Block *block_a, Static_Terrain_Block *block_b, 
                         Mesh_Seam *seam) {

    setup_seam_render_state(false);

    Transformer *transformer = global_transformer;
    rendering_3d(transformer->current_transform, client_globals.view_projector);

    glBegin(GL_TRIANGLES);

    Vector3 block_position;

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        Triangle_List_Mesh *mesh;
        if (index->which_mesh) {
            mesh = block_b->mesh;
            block_position = block_b->position;
        } else {
            mesh = block_a->mesh;
            block_position = block_a->position;
        }

        Vector3 pos = mesh->vertices[index->vertex_index] + block_position;
        Vector2 uv = mesh->uvs[index->vertex_index];

        int color_index = (i / 3) % NUM_SEAM_COLORS;
        Color4 color = seam_colors[color_index];

        glColor4f(color.r, color.g, color.b, color.a);
        glVertex3f(pos.x, pos.y, pos.z);
    }

    glEnd();
}


// Set up the render state for drawing a mesh.
void setup_mesh_render_state(bool wireframe) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);

    if (wireframe) {
        glDisable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(POLYGON_OFFSET_MAGNITUDE, 0);

        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, 1);
        glEnable(GL_CULL_FACE);

        return;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    setup_mesh_lighting();
}

void draw_seam_demo_block(Static_Terrain_Block *block) {
    extern bool wireframe;

    Transformer *transformer = global_transformer;
    Transformation_Matrix tm;
    tm.identity();
    tm.translate(block->position);
    transformer->push(&tm);

    rendering_3d(transformer->current_transform, client_globals.view_projector);
    setup_mesh_render_state(wireframe);


    emit_mesh(block->mesh);

    transformer->pop();
}


// If the user is rotating the block, this function updates
// its orientation.
void update_block_rotation() {
    int ix, iy;
    bool button1_down = app_shell->mouse_button_1_is_down;

    float dx = app_shell->mouse_pointer_delta_x;
    float dy = app_shell->mouse_pointer_delta_y;

    if (button1_down) {
        if (seam_demo_button1_was_down) {
            float ratio = dy / app_shell->screen_height;
            float dtheta = ratio * 2 * M_PI;
            seam_demo_block_angle += dtheta;

            Clamp(seam_demo_block_angle, 0, 0.5f * M_PI);
        }
    }

    seam_demo_button1_was_down = button1_down;
}


// I don't want to use gluLookAt because I like working in native
// data structures and minimizing my reliance on the underlying
// platform.  This function computes a look-at transform.
void make_look_at_transform(Transformation_Matrix *tm,
                            Vector3 pos, Vector3 look_at, Vector3 right_point) {
    // Make e1, e2, e3   a basis where e1 is the x axis and points
    // right at the object; e2 points left, e3 points up.
    Vector3 e1 = look_at - pos;
    e1.normalize();

    Vector3 e2 = -1 * (right_point - look_at);
    e2.normalize();

    Vector3 e3 = cross_product(e1, e2);

    tm->identity();
    tm->coef[0][0] = e1.x;
    tm->coef[0][1] = e1.y;
    tm->coef[0][2] = e1.z;

    tm->coef[1][0] = e2.x;
    tm->coef[1][1] = e2.y;
    tm->coef[1][2] = e2.z;

    tm->coef[2][0] = e3.x;
    tm->coef[2][1] = e3.y;
    tm->coef[2][2] = e3.z;

    Vector3 p_prime = tm->transform(pos);
    tm->coef[0][3] = -p_prime.x;
    tm->coef[1][3] = -p_prime.y;
    tm->coef[2][3] = -p_prime.z;
}



// Draw all the text on the screen...
void draw_seam_demo_hud() {
    Font *big_font = client_globals.big_font;
    Font *small_font = client_globals.small_font;

    const float PAD = 10;
    float x = PAD;
    float y = app_shell->screen_height - big_font->character_height - PAD;

    app_shell->text_mode_begin(big_font);
    app_shell->draw_text(big_font, x, y, "Seam Demo");
    y -= big_font->character_height - PAD;
    app_shell->text_mode_end();


    app_shell->text_mode_begin(small_font);
    app_shell->draw_text(small_font, x, y, "Press 'T' to switch to terrain demo mode.");
    y -= small_font->character_height;

    char buf[512];  // Static buffers are lame and you should never use them in real code!
    sprintf(buf, "Current resolution: %d (press 'R' to change this)", seam_demo_resolution % 3);
    app_shell->draw_text(small_font, x, y, buf);
    y -= small_font->character_height;

    app_shell->draw_text(small_font, x, y, "Press 'W' to toggle wireframe mode.");
    y -= small_font->character_height;

    app_shell->draw_text(small_font, x, y, "To rotate the terrain, hold the left mouse button and move the mouse.");
    y -= small_font->character_height;

    sprintf(buf, "button %d, angle %.3f", (int)app_shell->mouse_button_1_is_down, seam_demo_block_angle);
    app_shell->draw_text(small_font, x, y, buf);
    y -= small_font->character_height;
    

    app_shell->text_mode_end();
}



// Draw the demo!
void draw_seam_demo() {
    update_block_rotation();

    Vector3 offset(4, 0, 0);
    Vector3 mid = (original_position_a + original_position_b) * 0.5f + offset * 0.5f;
    demo_block_b->position = original_position_b + offset;
    demo_block_b_reduced->position = original_position_b + offset;
    demo_block_b_reduced_twice->position = original_position_b + offset;

    Vector3 camera_position(mid.x, mid.y, 30);


    Quaternion block_spin;
    block_spin.set_from_axis_and_angle(1, 0, 0, seam_demo_block_angle);
    Vector3 handle = camera_position - mid;
    handle.rotate(block_spin);
    camera_position = mid + handle;


    Transformation_Matrix tm;
    make_look_at_transform(&tm, camera_position, mid, demo_block_b->position);
    global_transformer->push(&tm);

    draw_seam_demo_block(demo_block_a);

    int res = seam_demo_resolution % 3;
    switch (res) {
    case 0:
        draw_seam_demo_block(demo_block_b);
        draw_seam_demo_seam(demo_block_a, demo_block_b, seam_a_to_b_high_res);
        break;
    case 1:
        draw_seam_demo_block(demo_block_b_reduced);
        draw_seam_demo_seam(demo_block_a, demo_block_b_reduced, seam_a_to_b_low_res);
        break;
    case 2:
        draw_seam_demo_block(demo_block_b_reduced_twice);
        draw_seam_demo_seam(demo_block_a, demo_block_b_reduced_twice, seam_a_to_b_lowest_res);
        break;
    }

    global_transformer->pop();

    draw_seam_demo_hud();
}
