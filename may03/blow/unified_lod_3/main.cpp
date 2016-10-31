#include "framework.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "mesh.h"
#include "mesh_chopper.h"
#include "mesh_seam.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include <math.h>
#include <gl/glu.h>

#include "tangent_frames.h"

char *app_name = "Unified Rendering LOD #3";

bool mesh_a_highres = true;
bool mesh_b_highres = true;
bool draw_chopped_mesh = false;
bool draw_obvious_seams = true;
bool do_wireframe = true;

Triangle_List_Mesh *the_input_mesh;
Triangle_List_Mesh *mesh_a[2];
Triangle_List_Mesh *mesh_b[2];
Mesh_Seam *seam_a_highres_b_highres;
Mesh_Seam *seam_a_highres_b_lowres;
Mesh_Seam *seam_a_lowres_b_highres;
Mesh_Seam *seam_a_lowres_b_lowres;

const float POSITION_B_OFFSET = 0.6f;
const float BUNNY_Z = 2;

Vector3 position_a(0, 0, BUNNY_Z);
Vector3 position_b(POSITION_B_OFFSET, 0, BUNNY_Z);

const float POLYGON_OFFSET_MAGNITUDE = -1.1f;

Font *big_font;
Font *small_font;

bool mouselook;

int last_cursor_x, last_cursor_y;
int num_entities_drawn;

const float DEGREES_PER_MICKEY_X = 0.2f;
const float DEGREES_PER_MICKEY_Y = 0.2f;
const float PI = 3.141592653589;

float view_theta = -90;
float view_phi = -89.9;

Vector3 view_pos(-0.283588, 1.99059, 5);
Vector3 view_direction;
Vector3 light_direction(-0.7f, -0.3f, 0.4f);
Vector3 camera_left_vector, camera_up_vector;

bool moving_forward, moving_backward, moving_left, moving_right;

const float MOVEMENT_SPEED = 10.0f;
const float GROUND_SIZE = 25.0f;

void setup_mesh_render_state(bool wireframe);

void init_view_transform() {
    Vector3 up(0, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (view_phi > 89.9) view_phi = 89.9;
    if (view_phi < -89.9) view_phi = -89.9;
    float radians = PI / 180.0;
    float ct = cos(-view_theta * radians);
    float st = sin(-view_theta * radians);
    float cp = cos(view_phi * radians);
    float sp = sin(view_phi * radians);

    Vector3 e1(ct, st, 0);
    Vector3 e2(0, 0, 1);

    Vector3 forward = e1 * cp + e2 * sp;
    Vector3 upward = e2 * cp + e1 * -sp;

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

    e1 = e1 * dx;
    e2 = e2 * dy;

    view_pos = view_pos + (e1) + (e2);

    float s = GROUND_SIZE;
    if (view_pos.x < -s) view_pos.x = -s;
    if (view_pos.x > +s) view_pos.x = +s;
    if (view_pos.y < -s) view_pos.y = -s;
    if (view_pos.y > +s) view_pos.y = +s;
}

const int NUM_SEAM_COLORS = 6;
struct Seam_Color {
    float r, g, b;
};

static Seam_Color seam_colors[NUM_SEAM_COLORS] = {
    { 1, 0, 0 }, { 0, 1, 0 }, { 0.8, 0.8, 0 },
    { 1, 0, 1 }, { 0, 1, 1 }, { 0.4, 0.4, 1 }
};

inline void get_vertex_data(Triangle_List_Mesh *block_a, 
                            Triangle_List_Mesh *block_b, 
                            const Vector3 &position_a, const Vector3 &position_b,
                            Seam_Index *index,
                            Vector3 *position_return, Vector2 *uv_return,
                            Vector3 *normal_return) {
    Vector3 block_position;

    Triangle_List_Mesh *mesh;
    if (index->which_mesh) {
        mesh = block_b;
        block_position = position_b;
    } else {
        mesh = block_a;
        block_position = position_a;
    }

    Vector3 normal(0, 0, 1);
    normal.rotate(mesh->tangent_frames[index->vertex_index]);

    *position_return = mesh->vertices[index->vertex_index] + block_position;
    *normal_return = normal;
    *uv_return = mesh->uvs[index->vertex_index];
}
                            

void draw_obvious_seam(Triangle_List_Mesh *block_a, Triangle_List_Mesh *block_b, 
                       Vector3 position_a, Vector3 position_b,
                       Mesh_Seam *seam) {
    // Because otherwise we have goofy stuff with our
    // wireframe showing up in front of the seams
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(POLYGON_OFFSET_MAGNITUDE, 0);

    app_shell->triangle_mode_begin();

    glBegin(GL_TRIANGLES);

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];

        Vector3 position;
        Vector3 normal;
        Vector2 uv;
        get_vertex_data(block_a, block_b, position_a, position_b, index,
                        &position, &uv, &normal);

        int color_index = (i / 3) % NUM_SEAM_COLORS;
        Seam_Color color = seam_colors[color_index];

        glColor4f(color.r, color.g, color.b, 1);
        glVertex3fv((float *)&position);
    }

    glEnd();

    app_shell->triangle_mode_end();

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void draw_seam(Triangle_List_Mesh *block_a, Triangle_List_Mesh *block_b, 
               Vector3 position_a, Vector3 position_b,
               Mesh_Seam *seam, bool wireframe = false) {

    app_shell->triangle_mode_begin();
    setup_mesh_render_state(wireframe);

    if (wireframe) glPolygonOffset(2*POLYGON_OFFSET_MAGNITUDE, 0);
    glBegin(GL_TRIANGLES);

    int i;
    for (i = 0; i < seam->num_faces * 3; i++) {
        Seam_Index *index = &seam->indices[i];
        Vector3 position;
        Vector3 normal;
        Vector2 uv;

        get_vertex_data(block_a, block_b, position_a, position_b, index,
                        &position, &uv, &normal);

        glNormal3fv((float *)&normal);
        glTexCoord2fv((float *)&uv);
        glVertex3fv((float *)&position);
    }

    glEnd();

    app_shell->triangle_mode_end();

    glDisable(GL_POLYGON_OFFSET_LINE);
}

void setup_mesh_render_state(bool wireframe) {
    if (wireframe) {
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
    glEnable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColor4f(0.3, 0.3, 0.8, 1.0f);

    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat unchoked[] = { 1, 1, 1, 1 };
    GLfloat light_pos[] = { 0, 0, 0, 0 };
    light_pos[0] = light_direction.x;
    light_pos[1] = light_direction.y;
    light_pos[2] = light_direction.z;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, unchoked);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
}

void draw_mesh(Triangle_List_Mesh *mesh, Vector3 mesh_position, bool wireframe = false) {
    const float s = 0.38f;
    GLfloat ambient_i0[] = { s, s, s, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    setup_mesh_render_state(wireframe);

    int i;
    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *list_info = &mesh->triangle_list_info[i];

        Mesh_Material_Info *material_info = &mesh->material_info[list_info->material_index];
        // @Functionality todo: Make material current.  Right now we are
        // basically just ignoring the mesh materials and rendering
        // the mesh in a shade of blue.

        glBegin(GL_TRIANGLES);

        int j;
        for (j = 0; j < list_info->num_vertices; j++) {
            int index = mesh->indices[list_info->start_of_list + j];
            Vector3 position = mesh->vertices[index] + mesh_position;

            Vector3 normal(0, 0, 1);
            normal.rotate(mesh->tangent_frames[index]);
            glNormal3fv((float *)&normal);
            glTexCoord2fv((float *)&mesh->uvs[index]);
            glVertex3fv((float *)&position);
        }

        glEnd();
    }

    if (wireframe) {  // Undo wireframe...
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
    }
}

void draw_meshes() {
    if (!draw_chopped_mesh) {
        draw_mesh(the_input_mesh, Vector3(0, 0, BUNNY_Z));
        if (do_wireframe) draw_mesh(the_input_mesh, Vector3(0, 0, BUNNY_Z), true);
        return;
    }

    int index_a = mesh_a_highres ? 0 : 1;
    int index_b = mesh_b_highres ? 0 : 1;

    Triangle_List_Mesh *draw_mesh_a = mesh_a[index_a];
    Triangle_List_Mesh *draw_mesh_b = mesh_b[index_b];

    // Draw the solid versions of the meshes...
    draw_mesh(draw_mesh_a, position_a);
    draw_mesh(draw_mesh_b, position_b);

    if (do_wireframe) {
        // Now draw the wireframes over the solid versions...

        draw_mesh(draw_mesh_a, position_a, true);
        draw_mesh(draw_mesh_b, position_b, true);
    }


    // Done with the main meshes, time to draw the seams now.

    Mesh_Seam *seam = NULL;

    if (index_a == 0) {
        if (index_b == 0) seam = seam_a_highres_b_highres;
        else seam = seam_a_highres_b_lowres;
    } else {
        if (index_b == 0) seam = seam_a_lowres_b_highres;
        else seam = seam_a_lowres_b_lowres;
    }        

    if (seam) {
        if (draw_obvious_seams) {
            draw_obvious_seam(draw_mesh_a, draw_mesh_b,
                              position_a, position_b, seam);
        } else {
            draw_seam(draw_mesh_a, draw_mesh_b,
                      position_a, position_b, seam);
            if (do_wireframe) {
                draw_seam(draw_mesh_a, draw_mesh_b,
                          position_a, position_b, seam, true);
            }
        }
    }
}

// Helper function for draw_scene():
void do_mesh_text(char *buf, char *mesh_name, bool high_res, char *key_to_press) {
    char *resolution = "LOW-RES";
    if (high_res) resolution = "HIGH-RES";

    sprintf(buf, "Mesh %s is %s ('%s' to toggle)",
            mesh_name, resolution, key_to_press);

}

void draw_scene() {
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


    if (the_input_mesh == NULL) {
        char *text = "Error: Could not load the mesh!";
        Font *font = big_font;
        float text_width = font->get_string_width_in_pixels(text);

        float text_y = app_shell->screen_height * 0.7f;
        float text_x = 0.5f * (app_shell->screen_width - text_width);

        app_shell->text_mode_begin(font);
        app_shell->draw_text(font, text_x, text_y, text, 1, 0, 0, 1);
        app_shell->text_mode_end();

        app_shell->sleep(0.02f);

        return;
    }

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

    glEnable(GL_DEPTH_TEST);


    draw_meshes();

    app_shell->triangle_mode_end();


    // In case we were wireframe rendering, reset the fill state.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    int header_y = app_shell->screen_height - big_font->character_height;
    app_shell->text_mode_begin(big_font);
    char *mouselook_string = "OFF";
    if (mouselook) mouselook_string = "ON";

    char buf[256];
    sprintf(buf, "Mouse Look is %s (press spacebar to toggle).", mouselook_string);
    app_shell->draw_text(big_font, 8, header_y, buf);
    app_shell->text_mode_end();


    app_shell->text_mode_end();

    header_y -= big_font->character_height;
    header_y += 11;  // Crunch it a little bit, we need space here.

    app_shell->text_mode_begin(small_font);
    float h = small_font->character_height;

    if (do_wireframe) {
        sprintf(buf, "Drawing wireframe ('i' to toggle)");
    } else {
        sprintf(buf, "Not drawing wireframe ('i' to toggle)");
    }

    if (draw_chopped_mesh) {
        sprintf(buf, "Drawing chopped mesh ('c' to toggle)");
    } else {
        sprintf(buf, "Drawing original mesh ('c' to toggle)");
    }
    app_shell->draw_text(small_font, 8, header_y, buf);
    header_y -= h;
    app_shell->text_mode_end();


    if (draw_chopped_mesh) {
        float header_x = 24;

        do_mesh_text(buf, "A", mesh_a_highres, "1");
        app_shell->draw_text(small_font, header_x, header_y, buf);
        header_y -= h;

        do_mesh_text(buf, "B", mesh_b_highres, "2");
        app_shell->draw_text(small_font, header_x, header_y, buf);
        header_y -= h;


        if (position_b.x) {
            sprintf(buf, "Artificial extrusion ON ('f' to toggle)");
        } else {
            sprintf(buf, "Artificial extrusion OFF ('f' to toggle)");
        }
        app_shell->draw_text(small_font, header_x, header_y, buf);
        header_y -= h;

        if (draw_obvious_seams) {
            sprintf(buf, "Drawing OBVIOUS seams ('o' to toggle)");
        } else {
            sprintf(buf, "Drawing SUBTLE seams ('o' to toggle)");
        }
        app_shell->draw_text(small_font, header_x, header_y, buf);
        header_y -= h;

        if (do_wireframe) {
            sprintf(buf, "WIREFRAME rendering ('i' to toggle)");
        } else {
            sprintf(buf, "GOURAUD rendering ('i' to toggle)");
        }
        app_shell->draw_text(small_font, header_x, header_y, buf);
        header_y -= h;
    }
    
    app_shell->sleep(0.02f);
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
    case '1':
        mesh_a_highres = !mesh_a_highres;
        break;
    case '2':
        mesh_b_highres = !mesh_b_highres;
        break;
    case 'C':
        draw_chopped_mesh = !draw_chopped_mesh;
        break;
    case 'I':
        do_wireframe = !do_wireframe;
        break;
    case 'F':
        if (position_b.x) {
            position_b.x = 0;
        } else {
            position_b.x = POSITION_B_OFFSET;
        }
        break;
    case 'O':
        draw_obvious_seams = !draw_obvious_seams;
        break;
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

Vector3 make_3d(const Vector2 &input) {
    Vector3 result(input.x, input.y, 0);
    return result;
}

void do_face(Tangent_Frame_Maker *maker, Triangle_List_Mesh *mesh,
             int face_index) {
    int n0 = mesh->indices[face_index*3+0];
    int n1 = mesh->indices[face_index*3+1];
    int n2 = mesh->indices[face_index*3+2];


    Vector3 pos[3];
    Vector3 uv[3];

    pos[0] = mesh->vertices[n0];
    pos[1] = mesh->vertices[n1];
    pos[2] = mesh->vertices[n2];

    uv[0] = make_3d(mesh->uvs[n0]);
    uv[1] = make_3d(mesh->uvs[n1]);
    uv[2] = make_3d(mesh->uvs[n2]);
    
    maker->accumulate_triangle(mesh->indices + face_index * 3, pos, uv);
}

void postprocess_mesh(Triangle_List_Mesh *mesh, bool build_frames = false) {
    // XXXXX Fix this (why is it necessary, these should be loading fine from the file, are they corruped in the file?
    int i;
    for (i = 0; i < mesh->num_vertices; i++) mesh->canonical_vertex_map[i] = i;

    if (build_frames) {
        Tangent_Frame_Maker maker;
        maker.compute_tangent_frames(mesh);
        delete [] mesh->tangent_frames;
        mesh->tangent_frames = maker.tangent_frames;
    }
        
}

Triangle_List_Mesh *reduce_mesh(Triangle_List_Mesh *original_mesh, int **xref_return) {
    Mesh_Reducer reducer;
    reducer.init(original_mesh);
    reducer.reduce(original_mesh->num_faces / 4);
    
    Triangle_List_Mesh *result;
    reducer.get_result(&result);

    if (xref_return) {
        *xref_return = reducer.topology_handler->old_index_to_new_index;
        reducer.topology_handler->old_index_to_new_index = NULL;
    }

    return result;
}

Mesh_Seam *xref_seam(Mesh_Seam *source, int *xref, int mesh_index, 
                     Triangle_List_Mesh *mesh) {
    Mesh_Seam *dest = new Mesh_Seam(source->num_faces);

    int i;
    for (i = 0; i < source->num_faces * 3; i++) {
        dest->indices[i].which_mesh = source->indices[i].which_mesh;
        if (dest->indices[i].which_mesh == mesh_index) {
            dest->indices[i].vertex_index = xref[source->indices[i].vertex_index];
        } else {
            dest->indices[i].vertex_index = source->indices[i].vertex_index;
        }
    }

    dest->remove_degenerate_faces();
    dest->compute_uv_coordinates(mesh);

    return dest;
}


// Code that I used to load the bunny and make a Triangle_List_Mesh
// out of it.  Nto at all central to the algorithm!
Triangle_List_Mesh *load_the_bunny() {
    float scale = 20.0f;

    FILE *f = fopen("data\\bunny16000.txt", "rt");
    assert(f);

    int nvertices, nfaces;
    int success;

    success = fscanf(f, "%d\n", &nvertices);
    assert(success);
    fscanf(f, "%d\n", &nfaces);
    assert(success);

    Triangle_List_Mesh *mesh = new Triangle_List_Mesh();
    mesh->allocate_materials(1);
    Mesh_Material_Info *material_info = &mesh->material_info[0];
    material_info->name = app_strdup("bunny_texture");


    mesh->allocate_geometry(nvertices, nfaces);

    int i;
    for (i = 0; i < nvertices; i++) {
        float x, y, z, junk;
        success = fscanf(f, "%f %f %f %f %f", &x, &y, &z, &junk, &junk);
        mesh->vertices[i] = Vector3(x, y, z) * scale;
        mesh->uvs[i].x = 0;
        mesh->uvs[i].y = 0;

        mesh->canonical_vertex_map[i] = i;

        assert(success == 5);
    }

    mesh->num_triangle_lists = 1;
    mesh->triangle_list_info = new Triangle_List_Info[1];

    Triangle_List_Info *info = &mesh->triangle_list_info[0];
    info->material_index = 0;
    info->num_vertices = nfaces * 3;
    info->start_of_list = 0;

    for (i = 0; i < nfaces; i++) {
        int junk, n0, n1, n2;
        success = fscanf(f, "%d %d %d %d", &junk, &n0, &n1, &n2);
        assert(success == 4);

        mesh->indices[i*3+0] = n0;
        mesh->indices[i*3+1] = n1;
        mesh->indices[i*3+2] = n2;
    }

    return mesh;
}

void preprocess_the_bunny() {
    Triangle_List_Mesh *mesh = load_the_bunny();

    const int NUM_TARGET_FACES = 10000;
//    const int NUM_TARGET_FACES = 4000;

    Mesh_Reducer reducer;
    reducer.init(mesh);
    reducer.reduce(NUM_TARGET_FACES);

    Triangle_List_Mesh *reduced_mesh;
    reducer.get_result(&reduced_mesh);

    char buf[256];  // Static buffers are lame and you should never use them.
    sprintf(buf, "bunny%d.triangle_list_mesh", NUM_TARGET_FACES);
    FILE *f = fopen(buf, "wb");
    assert(f);
    save_triangle_list_mesh(reduced_mesh, f);
    fclose(f);
}

void app_init(int argc, char **argv) {
    /*
      I was too lazy to put this into a separate program
      so it went here.  Uncomment it if you want to
      preprocess things.

      preprocess_the_bunny();
    */

    light_direction.normalize();

    big_font = app_shell->load_font("data\\Century Big");
    small_font = app_shell->load_font("data\\Century Small");

    glClearColor(0, 0, 0, 0.0);

    FILE *f = fopen("data\\bunny10000.triangle_list_mesh", "rb");
    if (f == NULL) return;

    Triangle_List_Mesh *input_mesh = load_triangle_list_mesh(f);
    if (!input_mesh) return;

    postprocess_mesh(input_mesh, true);
    the_input_mesh = input_mesh;

    Mesh_Chopper *chopper = new Mesh_Chopper();

    Chopped_Result result_a, result_b;
    Mesh_Seam *seam;
    chopper->chop_mesh(input_mesh, NULL, CHOP_X,
                       &result_a, &result_b, &seam);
    
    seam_a_highres_b_highres = seam;

    mesh_a[0] = result_a.result_mesh;
    mesh_b[0] = result_b.result_mesh;

    postprocess_mesh(mesh_a[0]);
    postprocess_mesh(mesh_b[0]);

    int *xref_a, *xref_b;
    mesh_a[1] = reduce_mesh(mesh_a[0], &xref_a);
    mesh_b[1] = reduce_mesh(mesh_b[0], &xref_b);

    postprocess_mesh(mesh_a[1]);
    postprocess_mesh(mesh_b[1]);

    seam_a_highres_b_lowres = xref_seam(seam, xref_b, 1, mesh_b[1]);
    seam_a_lowres_b_highres = xref_seam(seam, xref_a, 0, mesh_a[1]);

    seam_a_lowres_b_lowres = xref_seam(seam_a_lowres_b_highres, xref_b, 1, mesh_b[1]);
}



