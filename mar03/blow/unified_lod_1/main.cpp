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

char *app_name = "Unified Rendering LOD #1";

// Globals to control how we render and behave...
bool wireframe = false;
bool moving = false;
bool drawing_seam_demo = true;
bool drawing_obvious_seams = false;
bool color_code_by_lod = false;

Client_Globals client_globals;

int global_current_lod_frame_index = 0;
void *global_hinstance;
Transformer *global_transformer;

Elevation_Map *the_elevation_map;
float camera_theta, camera_phi;

float viewpoint_speed = 0;
int mesh_triangles = 0;
int mesh_triangles_rendered = 0;
int seam_triangles_rendered = 0;

Static_Terrain_Tree *the_terrain_tree;
char *global_terrain_file_name;

extern int seam_demo_resolution;



// Forward-declared functions...
void recursively_init_rendering_data(Static_Terrain_Tree *node);

void setup_mesh_lighting();
void setup_mesh_render_state(bool wireframe);
void setup_terrain_seam_render_state(bool wireframe);


// Okay, here we go.

void draw_frame_counter(float x, float y) {
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

    char buffer[100];
    sprintf(buffer, "frame rate: %.3f", frame_rate);

    app_shell->text_mode_begin(font);
    app_shell->draw_text(font, x, y, buffer);

    y -= font->character_height;
    sprintf(buffer, "mesh triangles: %6d ; rendered (not culled): %6d", mesh_triangles, mesh_triangles_rendered);
    app_shell->draw_text(font, x, y, buffer);
    y -= font->character_height;
    sprintf(buffer, "seam triangles rendered: %4d", seam_triangles_rendered);
    app_shell->draw_text(font, x, y, buffer);
    app_shell->text_mode_end();
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
    app_shell->draw_text(small_font, x, y, "Press 'S' to switch to seam demo mode.");
    y -= small_font->character_height;


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


    app_shell->draw_text(small_font, x, y, "Press 'W' to toggle wireframe mode.");
    y -= small_font->character_height;

    if (drawing_obvious_seams) {
        app_shell->draw_text(small_font, x, y, "Seams are OBVIOUS (press 'O' to toggle).");
    } else {
        app_shell->draw_text(small_font, x, y, "Seams are SUBTLE (press 'O' to toggle).");
    }
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

    draw_frame_counter(x, y);
}



// In order to draw the proper seam between a block and its
// neighbor, we have to figure out who that neighbor is.
// This code navigates the block tree to find the guy in
// the appropriate direction.  Most of the time we won't do
// much navigation, but note that for blocks in the middle we
// need to go to the root and back down.  We could cache this
// information with pointers on the blocks, but we don't.
// (Currently it does not impact performance).
Static_Terrain_Tree *get_neighbor(Static_Terrain_Tree *node,
                                  Neighbor_Direction direction, int level_offset,
                                  Static_Terrain_Tree **other_neighbor_ret) {
    if (node == NULL) return NULL;
    int node_pos = node->my_child_index;

    if (level_offset == -1) {
        if (direction == PLUS_X) {
            if ((node_pos & 1) == 0) return NULL;
            return get_neighbor(node->parent, PLUS_X, 0, other_neighbor_ret);
        }

        if (direction == PLUS_Y) {
            if ((node_pos & 2) == 0) return NULL;
            return get_neighbor(node->parent, PLUS_Y, 0, other_neighbor_ret);
        }        
    }

    if (level_offset == 1) {
        Static_Terrain_Tree *neighbor = get_neighbor(node, direction, 0, other_neighbor_ret);
        if (neighbor == NULL) return NULL;
        
        int other_child_index;
        int third_index;
        if (direction == PLUS_X) {
            other_child_index = 0;
            third_index = 2;
        } else {
            assert(direction == PLUS_Y);
            other_child_index = 0;
            third_index = 1;
        }

        *other_neighbor_ret = neighbor->children[third_index];
        return neighbor->children[other_child_index];
    }

    assert(level_offset == 0);
    if (node->parent == NULL) return NULL;

    int offset_flag = 0;
    if (direction == PLUS_X) offset_flag = 1;
    else offset_flag = 2;

    int other_child_index = node_pos ^ offset_flag;

    if ((node_pos & offset_flag) == 0) {
        return node->parent->children[other_child_index];
    } else {
        Static_Terrain_Tree *neighbor = get_neighbor(node->parent, direction, 0, other_neighbor_ret);
        if (neighbor == NULL) return NULL;
        return neighbor->children[other_child_index];
    }

    assert(0);
    return NULL;
}

bool is_nearby_res(Static_Terrain_Tree *node1, Static_Terrain_Tree *node2) {
    if (node2 == NULL) return false;
    if (node2->frame_index != node1->frame_index) return false;
    if (node2->lod_instance_state != RENDER_ME) return false;

    return true;
}


// Draw a seam for the terrain demo (the seam demo has its own
// seam rendering function).
void draw_terrain_view_seam(Static_Terrain_Tree *tree_a, Static_Terrain_Tree *tree_b, 
                            Mesh_Seam *seam, int color_index) {

    // We should never draw a seam between two leaves, because
    // they already fit together perfectly.
    if (tree_a->leaf_distance == 0) assert(tree_b->leaf_distance != 0);

    seam_triangles_rendered += seam->num_faces;

    // @Incomplete: Right now we assume that the seam only uses
    // one texture, and that it's the first texture in its A mesh.
    // In other words this is a total hack just to work with 
    // simple monotextured landscape.  That's fine, because when
    // we upgrade this code to handle polygon soups, this problem
    // will be solved naturally.

    int tileno = tree_a->block->mesh->material_info[0].texture_index;
    glBindTexture(GL_TEXTURE_2D, tileno);


    glBegin(GL_TRIANGLES);

    Vector3 block_position;

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        Triangle_List_Mesh *mesh;
        if (index->which_mesh == 0) {
            mesh = tree_a->block->mesh;
            block_position = tree_a->block->position;
        } else {
            assert(index->which_mesh == 1);
            mesh = tree_b->block->mesh;
            block_position = tree_b->block->position;
        }

        Vector3 pos = mesh->vertices[index->vertex_index] + block_position;
        Vector3 normal(0, 0, 1);
        normal.rotate(mesh->tangent_frames[index->vertex_index]);

        glNormal3fv((float *)&normal);
        glTexCoord2fv((float *)&index->uv);
        glVertex3fv((float *)&pos);
    }

    glEnd();
}

void do_seam_stuff(Static_Terrain_Tree *tree, Seam_Set *seam_set,
                   Neighbor_Direction direction, int level_offset) {
    Mesh_Seam *seam_a = NULL;
    Mesh_Seam *seam_b = NULL;
    if (level_offset == 0) seam_a = seam_set->res_equal;
    if (level_offset == 1) {
        seam_a = seam_set->res_higher_a;
        seam_b = seam_set->res_higher_b;
    }
    
    if (level_offset == -1) seam_a = seam_set->res_lower;
    if (seam_a == NULL) return;

    Static_Terrain_Tree *other_neighbor = NULL;
    Static_Terrain_Tree *neighbor = get_neighbor(tree, direction, level_offset, &other_neighbor);
    if (is_nearby_res(tree, neighbor)) {
        draw_terrain_view_seam(tree, neighbor, seam_a, level_offset + 1);
    }

    if (other_neighbor && is_nearby_res(tree, other_neighbor)) {
        draw_terrain_view_seam(tree, other_neighbor, seam_b, level_offset + 1);
    }
}

// To draw a seam for one block, we check for potential neighbors
// at varying resolutions in the X and Y directions.  This stuff
// will probably get less spammy in a later version.
void draw_seams_for_single_block(Static_Terrain_Tree *tree) {
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_X], PLUS_X, 0);
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_Y], PLUS_Y, 0);
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_X], PLUS_X, 1);
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_Y], PLUS_Y, 1);
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_X], PLUS_X, -1);
    do_seam_stuff(tree, &tree->block->seam_sets[PLUS_Y], PLUS_Y, -1);
}


// If we're drawing the scene with each block colored to clearly
// show its LOD, this function gives us the appropriate color.
// A leaf distance of 0 indicates a leaf block; every integer
// higher than that is one level up the tree.
void switch_lod_color(Static_Terrain_Tree *tree) {
    switch (tree->leaf_distance) {
    case 0:
        glColor3f(1, 0, 0);
        break;
    case 1:
        glColor3f(1, 1, 0);
        break;
    case 2:
        glColor3f(0, 1, 0);
        break;
    case 3:
        glColor3f(0, 1, 1);
        break;
    case 4:
        glColor3f(0, 0, 1);
        break;
    case 5:
    default:
        glColor3f(1, 0, 1);
        break;
    }
}

// Draw a block!
void draw_single_block(Static_Terrain_Tree *tree) {
    if (tree == NULL) return;
    Static_Terrain_Block *block = tree->block;

    mesh_triangles_rendered += block->mesh->num_faces;


    // Push the block's position onto the transform stack.
    // We store a position in worldspace, apart from the
    // block's vertex coordinates, mainly because it
    // helps us instance blocks (though we don't use
    // instancing in this demo!)
    Transformer *transformer = global_transformer;
    Transformation_Matrix tm;
    tm.identity();
    tm.translate(block->position);
    transformer->push(&tm);

    // Give the new transform to OpenGL
    rendering_3d(transformer->current_transform, client_globals.view_projector);

    // If we are color-coding blocks to make LOD obvious,
    // change the color now...
    if (color_code_by_lod) switch_lod_color(tree);

    // Actually output the mesh...
    emit_mesh(block->mesh);

    transformer->pop();
}


// Function called by qsort() to sort blocks by distance.
int compare_terrain_blocks(const void *b1, const void *b2) {
    Static_Terrain_Tree *node1 = *(Static_Terrain_Tree **)b1;
    Static_Terrain_Tree *node2 = *(Static_Terrain_Tree **)b2;
    if (node1->distance_from_viewpoint > node2->distance_from_viewpoint) return -1;
    if (node1->distance_from_viewpoint < node2->distance_from_viewpoint) return +1;
    return 0;
}
    
void sort_terrain(Auto_Array <Static_Terrain_Tree *> *array) {
    Static_Terrain_Tree *node;
    Array_Foreach(array, node) {
        node->distance_from_viewpoint = distance(node->block->position, client_globals.camera_position);
    } Endeach;

    qsort(array->data, array->live_items, sizeof(array->data[0]), compare_terrain_blocks);
}


const float SQRT_10 = 3.17;

void find_terrain_to_draw(Static_Terrain_Tree *tree) {
    // Here we select which LOD to use for each block.
    // This is somewhat ad-hoc since we don't really
    // relate the choice function to pixel error, or
    // even to block size; so if you radically change
    // the size of the blocks during the preprocess
    // phase, you will get different terrain quality
    // levels here.  We'll fix this in a future version.

    if (tree == NULL) return;

    int frame_index = global_current_lod_frame_index;
    tree->frame_index = frame_index;


    // Set z values to 0, so we are doing a 2D distance check.
    Vector3 block_pos = tree->block->position;
    Vector3 camera_pos = client_globals.camera_position;
    block_pos.z = camera_pos.z = 0;

    float dist = distance(block_pos, camera_pos);
    tree->debug_camera_dist = dist;
    float extent = Max(tree->block->bounding_box_extents.x, tree->block->bounding_box_extents.y);
    float r = extent * 0.5f;
    float k = SQRT_10 * 1.2f;

    float range_for_children = k*r;
    tree->debug_r = r;
    tree->debug_iso_radius = range_for_children;

    // If we're far enough away or have no children, just render this block.
    if ((dist > range_for_children) || (tree->leaf_distance == 0)) {
        tree->lod_instance_state = RENDER_ME;
        mesh_triangles += tree->block->mesh->num_faces;
        return;
    }
    
    // Otherwise render this block.
    tree->lod_instance_state = RENDER_MY_CHILDREN;

    find_terrain_to_draw(tree->children[0]);
    find_terrain_to_draw(tree->children[1]);
    find_terrain_to_draw(tree->children[2]);
    find_terrain_to_draw(tree->children[3]);
}


// Rotate the camera based on mouse input.
void update_camera_orientation() {
    int mouse_x = app_shell->mouse_pointer_delta_x;
    int mouse_y = app_shell->mouse_pointer_delta_y;

    const float RADIANS_PER_MOUSE_TICK = .01;
    camera_theta -= mouse_x * RADIANS_PER_MOUSE_TICK;
    camera_phi -= mouse_y * RADIANS_PER_MOUSE_TICK;

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


// Set up render state for a terrain mesh; mainly just calls
// out to the version in seam_demo.cpp.
void setup_terrain_mesh_render_state(bool wireframe) {
    setup_mesh_render_state(wireframe);
    if (color_code_by_lod) glDisable(GL_TEXTURE_2D);
}

// Set up render state for a terrain seam; mainly just calls
// out to the version in seam_demo.cpp.
void setup_terrain_seam_render_state(bool wireframe) {
    if (!drawing_obvious_seams) {
        setup_mesh_render_state(wireframe);
        return;
    }

    // Obvious seams...

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_TEXTURE_2D);

    setup_mesh_lighting();
}


void draw_terrain_demo() {
    if (the_terrain_tree == NULL) return;

    global_current_lod_frame_index++;
    
    mesh_triangles = 0;  // This gets incremented by find_terrain_to_draw
    find_terrain_to_draw(the_terrain_tree);


    // Find visible terrain blocks...
    Auto_Array <Static_Terrain_Tree *> terrain_array;
    collect_terrain(the_terrain_tree, &terrain_array);
    sort_terrain(&terrain_array);



    push_viewpoint_transform(global_transformer, client_globals.camera_position, client_globals.camera_orientation);

    // Send this transform to the graphics library so that the lighting
    // gets set up properly.
    rendering_3d(global_transformer->current_transform, client_globals.view_projector);

    // Draw the terrain blocks...

    setup_terrain_mesh_render_state(wireframe);

    Static_Terrain_Tree *node;
    Array_Foreach(&terrain_array, node) {
        draw_single_block(node);
    } Endeach;


    // Draw the seams...

    // Reset the transform just in case.
    rendering_3d(global_transformer->current_transform, client_globals.view_projector);
    setup_terrain_seam_render_state(wireframe);

    Transformer *transformer = global_transformer;
    rendering_3d(transformer->current_transform, client_globals.view_projector);

    Array_Foreach(&terrain_array, node) {
        draw_seams_for_single_block(node);
    } Endeach;


    // Done!

    pop_viewpoint_transform(global_transformer);
}


// Draw the scene!
void draw_scene() {
    Transformer *tr = global_transformer;
    Projector *pr = client_globals.view_projector;

    // Reset some stat counters...
    mesh_triangles_rendered = 0;
    mesh_triangles = 0;
    seam_triangles_rendered = 0;

    // Set up the view projection
    pr->set_viewport(app_shell->screen_width, app_shell->screen_height);
    pr->set_fov(M_PI * 0.4);


    // Draw the seam demo or the terrain demo
    if (drawing_seam_demo) {
        extern void draw_seam_demo();
        draw_seam_demo();
    } else {
        update_camera_orientation();
        simulate_viewpoint_motion();
        draw_terrain_demo();
        draw_terrain_view_hud();
    }

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
    case 'T':
        drawing_seam_demo = false;
        break;
    case 'S':
        drawing_seam_demo = true;
        break;
    case 'O':
        drawing_obvious_seams = !drawing_obvious_seams;
        break;
    case 'C':
        color_code_by_lod = !color_code_by_lod;
        break;
    case 'R':
        seam_demo_resolution++;
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
        adjust_view_speed(-90);
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

void app_init(int argc, char **argv) {
	// So that current_client_time is initialized so that
	// our startup code in various modules can refer to it.

    global_transformer = new Transformer();
    global_transformer->push_identity();

    client_globals.view_projector = new Projector();
    client_globals.view_projector->set_viewport(app_shell->screen_width, app_shell->screen_height);
    client_globals.view_projector->set_fov(M_PI * 0.5);

    client_globals.big_font = app_shell->load_font("data/Century Big");
    assert(client_globals.big_font != NULL);

    client_globals.small_font = app_shell->load_font("data/Century Small");
    assert(client_globals.small_font != NULL);

    // Load mesh, init vertex data

    char *name;
    if (argc > 0) {
        name = argv[0];
    } else {
        name = "data/crater_0513.terrain_tree";
    }

    global_terrain_file_name = name;
    File_Handle *f = fopen(name, "rb");
    if (f == NULL) {
        name = "data/test_bumps_small.terrain_tree";
        f = fopen(name, "rb");
    }

    if (f) {
        the_terrain_tree = load_terrain(f);
        fclose(f);
        assert(the_terrain_tree);
        recursively_init_rendering_data(the_terrain_tree);
    } else {
        the_terrain_tree = NULL;
    }

    client_globals.camera_position = Vector3(20, 20, 16);
    camera_theta = M_PI * 0.25;

    // Initialize the seam demo...
    extern void init_seam_demo();
    init_seam_demo();
}

void init_rendering_data(Static_Terrain_Block *block) {
    int i;
    for (i = 0; i < block->mesh->num_materials; i++) {
        int handle = app_shell->find_or_load_texture(block->mesh->material_info[i].name, "");
        block->mesh->material_info[i].texture_index = handle;
    }
}


void recursively_init_rendering_data(Static_Terrain_Tree *node) {
    if (node == NULL) return;

    if (node->children[0]) {
        recursively_init_rendering_data(node->children[0]);
        recursively_init_rendering_data(node->children[1]);
        recursively_init_rendering_data(node->children[2]);
        recursively_init_rendering_data(node->children[3]);
    }

    init_rendering_data(node->block);
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
