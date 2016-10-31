#include "framework.h"

#include "mesh.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <float.h>   // For FLT_MAX
#include <stdlib.h>  // For qsort

#include "make_terrain.h"

#include "viewpoint.h"
#include "os_specific_opengl_headers.h"
#include "framework/rendering.h"
#include "visibility.h"
#include "render_state.h"
#include "seam_database.h"

char *app_name = "Unified Rendering LOD #4";

// Globals to control how we render and behave...

bool wireframe = false;
bool draw_solid_blocks = true;
bool moving = false;
bool color_code_by_lod = false;

bool draw_fading_in_blocks = true;
bool draw_fading_out_blocks = true;
bool freeze_blocks = false;
bool textures_disabled = false;

Client_Globals client_globals;

int global_current_lod_frame_index = 0;
void *global_hinstance;

const int NUM_FRAMES_WITHOUT_INPUT = 3;

Elevation_Map *the_elevation_map;
float camera_theta, camera_phi;

float viewpoint_speed = 0;
int mesh_triangles = 0;
int mesh_triangles_rendered = 0;
int seam_triangles_rendered = 0;

Static_Terrain_Block *the_terrain_tree;
char *global_terrain_file_name;


Loaded_Texture_Info white_texture;
Seam_Database *seam_database;


// Forward-declared functions...
void recursively_init_rendering_data(Static_Terrain_Block *node);


// Okay, here we go.

// Here are just a bunch of functions to draw the
// text display... they aren't related to the core
// algorithm.
void draw_mesh_info(float x, float y) {
    Font *font = client_globals.big_font;

    const float TIME_TO_CONVERGE_WITHIN_90_PERCENT = 0.5f;  // In seconds
    float dt = app_shell->get_dt();
    float factor = pow(0.1f, dt / TIME_TO_CONVERGE_WITHIN_90_PERCENT);
    static bool first_update = true;

    static float frame_dt = 0;
    if (first_update) {
        first_update = false;
        frame_dt = dt;
    } else {
        frame_dt = frame_dt * factor + dt * (1 - factor);
    }



    float frame_rate;
    if (frame_dt > 0) {
        frame_rate = 1.0f / frame_dt;
    } else {
        frame_rate = 0;
    }

    char buffer[256];
    app_shell->text_mode_begin(font);
    sprintf(buffer, "mesh triangles: %6d ; rendered (not culled): %6d", mesh_triangles, mesh_triangles_rendered);
    app_shell->draw_text(font, x, y, buffer);
    y -= font->character_height;
    sprintf(buffer, "seam triangles rendered: %4d", seam_triangles_rendered);
    app_shell->draw_text(font, x, y, buffer);
    app_shell->text_mode_end();
}

char *on_off_string(bool value) {
    if (value) return "ON";
    return "OFF";
}

void draw_terrain_view_hud() {
    Font *big_font = client_globals.big_font;
    Font *small_font = client_globals.small_font;

    const float PAD = 10;
    float x = PAD;
    float y = app_shell->screen_height - big_font->character_height - PAD;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    app_shell->text_mode_begin(big_font);
    app_shell->draw_text(big_font, x, y, "Terrain Demo");
    y -= big_font->character_height - PAD;


    char buf[512];  // Static buffers are lame and you should never use them in real code!


    if (the_terrain_tree == NULL) {
        y -= 3 * big_font->character_height;
        
        sprintf(buf, "Terrain file '%s' not found!", global_terrain_file_name);
        app_shell->draw_text(big_font, x, y, buf);
        app_shell->text_mode_end();
        return;
    }

    app_shell->text_mode_end();

    app_shell->text_mode_begin(small_font);

    app_shell->draw_text(small_font, x, y, "Press a number key (0-9) to change speed.");
    y -= small_font->character_height;

    sprintf(buf, "Current speed is: %.1f", viewpoint_speed);
    app_shell->draw_text(small_font, x, y, buf);
    y -= small_font->character_height;

    if (moving) {
        app_shell->draw_text(small_font, x, y, "Press the spacebar to STOP moving.");
    } else {
        app_shell->draw_text(small_font, x, y, "Press the spacebar to START moving.");
    }
    y -= small_font->character_height;


    char *wireframe_string = on_off_string(wireframe);
    char *solid_blocks_string = on_off_string(draw_solid_blocks);
    char *use_textures_string = on_off_string(!textures_disabled);
    sprintf(buf, "W: wireframe (%s),   S: draw solid blocks (%s),   T: use textures (%s)",
            wireframe_string, solid_blocks_string, use_textures_string);
    app_shell->draw_text(small_font, x, y, buf);
    y -= small_font->character_height;

    if (color_code_by_lod) {
        app_shell->draw_text(small_font, x, y, "Press 'C' to color blocks using texture mapping.");
    } else {
        app_shell->draw_text(small_font, x, y, "Press 'C' to color-code the blocks by LOD scale.");
    }
    y -= small_font->character_height;


    app_shell->draw_text(small_font, x, y, "Use the mouse to aim the camera.");
    y -= small_font->character_height;

    app_shell->text_mode_end();

    // Extra spacing
    y -= small_font->character_height;

    draw_mesh_info(x, y);
}


/*
  Given two blocks and a seam, 'draw_terrain_view_seam'
  does the appropriate coordinate generation and all
  that stuff.
*/
void draw_terrain_view_seam(Mesh_Seam *seam) {
    seam_triangles_rendered += seam->num_faces;

    // @Incomplete: Right now we assume that the seam only uses
    // one texture, and that it's the first texture in its A mesh.
    // In other words this is a total hack just to work with 
    // simple monotextured landscape.  That's fine, because when
    // we upgrade this code to handle polygon soups, this problem
    // will be solved naturally.

    if (textures_disabled) {
        extern Loaded_Texture_Info white_texture;
        glBindTexture(GL_TEXTURE_2D, white_texture.texture_handle);
    } else {
        int tileno = seam->block_membership[0]->mesh->material_info[0].texture_index;
        glBindTexture(GL_TEXTURE_2D, tileno);
    }


    glBegin(GL_TRIANGLES);

    Vector3 block_position;

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        Static_Terrain_Block *block = seam->block_membership[index->which_mesh];

        Triangle_List_Mesh *mesh = block->mesh;

        Vector3 pos = mesh->vertices[index->vertex_index] + block->position;
        Vector3 normal(0, 0, 1);
        normal.rotate(mesh->tangent_frames[index->vertex_index]);

        glNormal3fv((float *)&normal);
        glTexCoord2fv((float *)&mesh->uvs[index->vertex_index]);
        glVertex3fv((float *)&pos);
    }

    glEnd();
}




// If we're drawing the scene with each block colored to clearly
// show its LOD, this function gives us the appropriate color.
// A leaf distance of 0 indicates a leaf block; every integer
// higher than that is one level up the tree.
void switch_lod_color(Static_Terrain_Block *tree, float alpha) {
    Clamp(alpha, 0, 1);

    switch (tree->leaf_distance) {
    case 0:
        glColor4f(1, 0, 0, alpha);
        break;
    case 1:
        glColor4f(1, 1, 0, alpha);
        break;
    case 2:
        glColor4f(0, 1, 0, alpha);
        break;
    case 3:
        glColor4f(0, 1, 1, alpha);
        break;
    case 4:
        glColor4f(0, 0, 1, alpha);
        break;
    case 5:
    default:
        glColor4f(1, 0, 1, alpha);
        break;
    }
}

void draw_single_block(Static_Terrain_Block *tree) {
    if (tree == NULL) return;
    Static_Terrain_Block *block = tree;

    mesh_triangles_rendered += block->mesh->num_faces;


    // Push the block's position onto the transform stack.
    // We store a position in worldspace, apart from the
    // block's vertex coordinates, mainly because it
    // helps us instance blocks (though we don't use
    // instancing in this demo!)
    Transformer *transformer = client_globals.transformer;
    Transformation_Matrix tm;
    tm.identity();
    tm.translate(block->position);
    transformer->push(&tm);

    // Give the new transform to OpenGL
    rendering_3d(transformer->current_transform, client_globals.view_projector);

    // If we are color-coding blocks to make LOD obvious,
    // change the color now...
    if (color_code_by_lod) switch_lod_color(tree, tree->opacity);

    // Actually output the mesh...
    emit_mesh(block->mesh);

    transformer->pop();
}

// Function called by qsort() to sort blocks by distance.
int compare_terrain_blocks(const void *b1, const void *b2) {
    Static_Terrain_Block *node1 = *(Static_Terrain_Block **)b1;
    Static_Terrain_Block *node2 = *(Static_Terrain_Block **)b2;
    if (node1->distance_from_viewpoint > node2->distance_from_viewpoint) return -1;
    if (node1->distance_from_viewpoint < node2->distance_from_viewpoint) return +1;
    return 0;
}

// Compute distance to each block, then sort the blocks by that value.    
void sort_terrain(Auto_Array <Static_Terrain_Block *> *array) {
    Static_Terrain_Block *node;
    Array_Foreach(array, node) {
        node->distance_from_viewpoint = distance(node->position, client_globals.camera_position);
    } Endeach;

    qsort(array->data, array->live_items, sizeof(array->data[0]), compare_terrain_blocks);
}


const float SQRT_10 = 3.17;

// If we are fading some child blocks in, we want those
// blocks to all appear with synchronized amounts of
// translucency.  This just sets that value on each block.
void set_child_properties(Static_Terrain_Block *tree,
                          float opacity, int frame_index,
                          Lod_Instance_State state) {
    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        Static_Terrain_Block *child = tree->children[i];
        if (!child) continue;

        child->opacity = opacity;
        child->frame_index = frame_index;
        child->lod_instance_state = state;
    }
}

// count_child_triangles can be precomputed if you care!
int count_child_triangles(Static_Terrain_Block *tree) {
    int sum = 0;

    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {
        Static_Terrain_Block *child = tree->children[i];
        if (!child) continue;

        sum += child->mesh->num_faces;
    }

    return sum;
}

/*
  Okay.  So to draw the terrain, first we start at the root
  and traverse the tree recursively; we figure out which LOD 
  we want for each block, and mark the blocks accordingly.  
  Then later on, we start at the root again and do another
  recursive traversal to compute visibility; if a block is
  outside the frustum we decide not to draw it, and stop
  recursing.

  Now it would appear that this is a little bit redundant
  and we should do both at the same time.  For a simple
  demo like this one, with no higher ambitions, that might
  be true.  However, for a real game we might want to do
  something more advanced, like stream environment 
  geometry from disk.  In that case, we want to be able
  to load the appropriate LODs of various blocks even
  if they are outside the frustum (because the user might
  turn his head very quickly, so we need the appropriate
  geometry to appear almost instantly).  In such a system
  it makes a lot more sense to compute LOD and perform
  streaming requests in a pre-pass, as I have it structured
  here.  
*/
void select_terrain_lods(Static_Terrain_Block *tree) {
    // Here we select which LOD to use for each block.
    // This is somewhat ad-hoc since we don't really
    // relate the choice function to pixel error, or
    // even to block size; so if you radically change
    // the size of the blocks during the preprocess
    // phase, you will get different terrain quality
    // levels here.  We'll fix this in a future version.

    if (tree == NULL) return;
    if (freeze_blocks && (global_current_lod_frame_index > 1)) return;

    int frame_index = global_current_lod_frame_index;
    tree->frame_index = frame_index;


    Vector3 block_pos = tree->position;
    Vector3 camera_pos = client_globals.camera_position;
    block_pos.z = camera_pos.z = 0;

    float dist = distance(block_pos, camera_pos);


    float r = tree->edge_length * 0.5f;
    float k = SQRT_10 * 1.2f;

    float range_mid = k*r;
    float diff_stride = range_mid*0.2f;
    float range_for_children = range_mid - diff_stride;
    float range_for_me = range_mid + diff_stride;
    float range_diff = range_for_me - range_for_children;


    float t = (dist - range_for_children) / range_diff;
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    if (tree->children[0] == NULL) t = 1;

    tree->existence_parameter = t;

    if (t == 1) {
        tree->lod_instance_state = I_AM_SINGLE;
        tree->opacity = 1;
        mesh_triangles += tree->mesh->num_faces;
        return;
    }

    if (t == 0) {
        tree->lod_instance_state = I_AM_NOT_INVOLVED;
        tree->opacity = 0;

        select_terrain_lods(tree->children[0]);
        select_terrain_lods(tree->children[1]);
        select_terrain_lods(tree->children[2]);
        select_terrain_lods(tree->children[3]);

        return;
    }

    float my_opacity, child_opacity;

    if (t >= 0.5f) {
        my_opacity = 1;
        child_opacity = 1.0f - (t - 0.5f) * 2;
        Clamp(child_opacity, 0, 1);
    } else {
        my_opacity = t * 2;
        Clamp(my_opacity, 0, 1);
        child_opacity = 1;
    }

    tree->opacity = my_opacity;
    tree->lod_instance_state = I_AM_TRANSITIONING;

    set_child_properties(tree, child_opacity, frame_index, I_AM_TRANSITIONING);

    mesh_triangles += tree->mesh->num_faces;
    mesh_triangles += count_child_triangles(tree);
}

// Rotate the camera based on mouse input.
void update_camera_orientation() {
    int mouse_x = app_shell->mouse_pointer_delta_x;
    int mouse_y = app_shell->mouse_pointer_delta_y;

    // Don't allow motion for the first several frames,
    // because we have Windows mouse weirdness problems.
    if (global_current_lod_frame_index > NUM_FRAMES_WITHOUT_INPUT) {
        const float RADIANS_PER_MOUSE_TICK = .01;
        camera_theta -= mouse_x * RADIANS_PER_MOUSE_TICK;
        camera_phi -= mouse_y * RADIANS_PER_MOUSE_TICK;
    }

    Clamp(camera_phi, -M_PI * 0.5f, M_PI * 0.5f);

    float theta = camera_theta;
    float phi = camera_phi;

    Quaternion rotation_theta;
    Quaternion rotation_phi;

    rotation_theta.set_from_axis_and_angle(0, 0, 1, theta);

    Vector3 new_y(0, 1, 0);
    new_y.rotate(rotation_theta);
    rotation_phi.set_from_axis_and_angle(new_y.x, new_y.y, new_y.z, phi);
    Quaternion rotation = rotation_phi * rotation_theta;

    client_globals.camera_orientation = rotation;
}


// Move the camera through the world based on our current
// velocity and direction.
void simulate_viewpoint_motion() {
    if (!moving) return;

    // Okay, let's simulate the view position along...
    Vector3 view_vector(1, 0, 0);
    view_vector.rotate(client_globals.camera_orientation);

    float dt = app_shell->get_dt();
    Vector3 delta = view_vector * viewpoint_speed * dt;
    client_globals.camera_position = client_globals.camera_position + delta;
}


void draw_terrain_demo() {
    if (the_terrain_tree == NULL) return;

    global_current_lod_frame_index++;
    
    mesh_triangles = 0;  // This gets incremented by select_terrain_lods

    // Figure out the LOD for each part of the terrain.
    select_terrain_lods(the_terrain_tree);

    // Find visible terrain blocks...
    Segregated_Terrain_Pieces pieces;
    collect_visible_terrain(the_terrain_tree, &pieces);


    // Push the camera transform onto the transform stack
    // (this computes the product of the transform matrix with
    // whatever was already on the stack).
    push_viewpoint_transform(client_globals.transformer, client_globals.camera_position, client_globals.camera_orientation);

    // Send this transform to the graphics library so that the lighting
    // gets set up properly.
    rendering_3d(client_globals.transformer->current_transform, client_globals.view_projector);

    // Draw the terrain blocks...


    // SOLID blocks

    setup_mesh_render_state(RS_SOLID, wireframe);

    Static_Terrain_Block *node;
    if (draw_solid_blocks) {
        Array_Foreach(&pieces.solid, node) {
            draw_single_block(node);
        } Endeach;
    }

    
    // Now we draw the seams... fun fun!
    
    Auto_Array <Mesh_Seam *> seams;

    // First mark the solid blocks.
    Array_Foreach(&pieces.solid, node) {
        node->block_search_marker = 1;
    } Endeach;

    // Now search...
    seam_database->collect_fully_marked_seams(&seams);

    // Now unmark the solid blocks!
    Array_Foreach(&pieces.solid, node) {
        node->block_search_marker = 0;
    } Endeach;

    // Set up the transform...
    rendering_3d(client_globals.transformer->current_transform, client_globals.view_projector);

    // And kick 'em out, homey!
    glColor4f(1, 1, 1, 1);
    Mesh_Seam *seam;
    Array_Foreach(&seams, seam) {
        draw_terrain_view_seam(seam);
    } Endeach;


    // All done with the seams!
    // Now time to do fading blocks.


    setup_mesh_render_state(RS_FADING, wireframe);

    // FADING blocks, and right now we toggle the z-state appropriately...
    sort_terrain(&pieces.fading);
    Array_Foreach(&pieces.fading, node) {
        setup_mesh_render_opacity(node->opacity, node->use_zfunc_lessequal);
        draw_single_block(node);
    } Endeach;


    // Done!

    pop_viewpoint_transform(client_globals.transformer);
}


// Draw the scene!
void draw_scene() {
    Transformer *tr = client_globals.transformer;
    Projector *pr = client_globals.view_projector;

    // Reset some stat counters...
    mesh_triangles_rendered = 0;
    mesh_triangles = 0;
    seam_triangles_rendered = 0;

    // Set up the view projection
    pr->set_viewport(app_shell->screen_width, app_shell->screen_height);
    pr->set_fov(M_PI * 0.4);


    update_camera_orientation();
    simulate_viewpoint_motion();
    draw_terrain_demo();
    draw_terrain_view_hud();

    // Sleep here so that the OS doesn't spaz out and get starved
    // (this is especially a problem with Windows XP; XP really sucks).
    // We're not really going for speed right now so this is okay.
    app_shell->sleep(0.01f);
}


// Handle keyboard input...
void adjust_view_speed(int factor) {
    const float BASE_UNITS_PER_SECOND = 0.8f;
    float scalar = factor * BASE_UNITS_PER_SECOND;
    viewpoint_speed = scalar;
}

void handle_keydown(int key) {
    switch (key) {
    case 'W':
        wireframe = !wireframe;
        break;
    case 'S':
        draw_solid_blocks = !draw_solid_blocks;
        break;
    case 'C':
        color_code_by_lod = !color_code_by_lod;
        break;
    case 'T':
        textures_disabled = !textures_disabled;
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8': {
        int factor = key - '0';
        adjust_view_speed(factor*factor);
        break;
    }
    case '9':
        adjust_view_speed(-9);
        break;
    case '-':
        adjust_view_speed(-1);
        break;
    case ' ':
        moving = !moving;
        break;
    }
}

void handle_keyup(int key) {
}

/*
  app_init does a bunch of unexciting stuff to load data
  and generally get us ready to run.
*/
void app_init(int argc, char **argv) {
    client_globals.transformer = new Transformer();
    client_globals.transformer->push_identity();

    client_globals.view_projector = new Projector();
    client_globals.view_projector->set_viewport(app_shell->screen_width, app_shell->screen_height);
    client_globals.view_projector->set_fov(M_PI * 0.5);


    // Fonts we use to draw HUD text.
    client_globals.big_font = app_shell->load_font("data/Century Big");
    assert(client_globals.big_font != NULL);

    client_globals.small_font = app_shell->load_font("data/Century Small");
    assert(client_globals.small_font != NULL);

    // The 'white_texture' is used when you turn off the terrain
    // texture from the HUD...
    app_shell->load_texture(&white_texture, "data/bitmaps/white.jpg");
    assert(white_texture.loaded_successfully);

    // Load mesh, init vertex data

    char *name;
    if (argc > 0) {
        name = argv[0];
    } else {
        name = "data/crater.terrain_tree";
    }

    global_terrain_file_name = name;
    File_Handle *f = fopen(name, "rb");

    if (f) {
        the_terrain_tree = load_terrain(f, &seam_database);
        fclose(f);
        assert(the_terrain_tree);
        recursively_init_rendering_data(the_terrain_tree);
    } else {
        the_terrain_tree = NULL;
    }

    client_globals.camera_position = Vector3(20, 20, 16);
    camera_theta = M_PI * 0.25;
}


void init_rendering_data(Static_Terrain_Block *block) {
    int i;
    for (i = 0; i < block->mesh->num_materials; i++) {
        int handle = app_shell->find_or_load_texture(block->mesh->material_info[i].name, "data/bitmaps/");
        block->mesh->material_info[i].texture_index = handle;
    }
}


void recursively_init_rendering_data(Static_Terrain_Block *node) {
    if (node == NULL) return;

    if (node->children[0]) {
        recursively_init_rendering_data(node->children[0]);
        recursively_init_rendering_data(node->children[1]);
        recursively_init_rendering_data(node->children[2]);
        recursively_init_rendering_data(node->children[3]);
    }

    init_rendering_data(node);
}




void push_viewpoint_transform(Transformer *tr, Vector3 pos, Quaternion ori) {
    Transformation_Matrix matrix;
    matrix.identity();
    matrix.translate(-1 * pos);
    matrix.rotate(ori.conjugate());

    tr->push(&matrix);
}

void pop_viewpoint_transform(Transformer *tr) {
    tr->pop();
}


Quaternion get_viewpoint_orientation() {
    return client_globals.camera_orientation;
}
