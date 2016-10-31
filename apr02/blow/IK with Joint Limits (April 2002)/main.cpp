#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"
#include "jpeg_load.h"
#include "geometry.h"

#include "vector3.h"
#include "quaternion.h"
#include "simple_rotation.h"

#include <gl/gl.h>
#include <gl/glu.h>

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;

extern bool use_fast_normalize;
extern bool want_smoothness = true;

enum Change_Axis {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2
};

char *axis_names[] = {
    "X", "Y", "Z"
};

Change_Axis change_axis;
Vector3 object_pos;
Vector3 vec_spot;

bool moving_forward, moving_backward, moving_left, moving_right;
bool target_forward, target_backward;

bool should_draw_bone_states;
bool should_draw_joint_limits = false;
bool should_impose_joint_limits = false;

const float FORWARD_SPEED = 3.0f;
const float SIDEWAYS_SPEED = M_PI * 0.9f;
const float TARGET_SPEED = 3.0f;


const double VIEW_DISTANCE_MIN = 4.0;
const double VIEW_DISTANCE_MAX = 20.0;

const float BONE_THIN_RADIUS = 0.02f;

double view_theta;
double view_distance;

GLuint font_handle = 0;
extern int g_ScreenWidth;
extern int g_ScreenHeight;

bool graph_mode_active = false;
int starting_pose = 0;

void begin_line_mode(float r, float g, float b);
void end_line_mode();
void begin_text_mode();
void end_text_mode();

struct Limiting_Polygon;

bool limit_rotation_cone_style(Quaternion *attempt, double theta, 
                               Quaternion *result);
Quaternion limit_rotation(Quaternion *attempt, Limiting_Polygon *polygon,
                          float max_axial_angle);



const int MAX_LIMITING_POLYGON_VERTICES = 16;
struct Limiting_Polygon {
    Vector3 vertices[MAX_LIMITING_POLYGON_VERTICES];
    int num_vertices;

    void init();
    void add_vertex(float y, float z);
    void finalize();
};


struct Loaded_Texture_Info {
    GLuint texture_handle;
    int width, height;
    bool loaded_successfully;
};

Loaded_Texture_Info spot_texture;
Loaded_Texture_Info target_texture;
Loaded_Texture_Info anchor_texture;
Loaded_Texture_Info twist_stripes;
Loaded_Texture_Info shoulder_texture;
Loaded_Texture_Info elbow_texture;


void begin_bitmap_mode(GLuint texture_handle);
void end_bitmap_mode();

const int MAX_BONES = 16;
struct Bone {
    void reset();

    int parent_index;
    Quaternion orientation_local;

    float bone_length;
    float near_radius_a, near_radius_b;
    float mid_radius_a, mid_radius_b;
    float far_radius_a, far_radius_b;
    float bulge_midpoint;

    float color_r, color_g, color_b;

    Vector3 limit_direction;
    float limit_angle;

    // Workspace stuff that you don't initialize:
    Quaternion orientation_global;
    Vector3 position_global;

    Limiting_Polygon limiting_polygon;
};

struct Bone_Hierarchy {
    Bone bones[MAX_BONES];
    int num_bones;

    void do_bone_ik(int end_index, Vector3 *target, float step_size);
    void update_transforms();
    Vector3 get_endpoint_of_chain(int index);
};


void Limiting_Polygon::init() {
    num_vertices = 0;
}

void Limiting_Polygon::add_vertex(float y, float z) {
    assert(num_vertices < MAX_LIMITING_POLYGON_VERTICES);
    Vector3 *vertex = &vertices[num_vertices++];

    vertex->x = 1;
    vertex->y = y;
    vertex->z = z;
}

void Limiting_Polygon::finalize() {
}

void compute_xy(Vector *result, int ix, int iy, int screen_i, int screen_j) {
    float fx = ix;
    float fy = iy;

    float Z_PLANE = -1;
    *result = Vector(fx, fy, Z_PLANE);
}

void add_letter_quad(char *s, Vector *uv, Vector *xy, Vector *rgb,
		     int i, int xs, int ys, 
		     int letter_width, int letter_height,
		     float cr, float cg, float cb,
		     int screen_i, int screen_j) {

    unsigned int c = s[i];
    unsigned int ci = (c % 16);
    unsigned int cj = (c / 16);

    int w = letter_width;
    int h = letter_height;

    int n0 = i * 4 + 0;
    int n1 = i * 4 + 1;
    int n2 = i * 4 + 2;
    int n3 = i * 4 + 3;

    uv[n0].x = 0.0;
    uv[n0].y = 1.0;
    uv[n1].x = 0.0;
    uv[n1].y = 0.0;
    uv[n2].x = 1.0;
    uv[n2].y = 0.0;
    uv[n3].x = 1.0;
    uv[n3].y = 1.0;

    compute_xy(&xy[n0], xs+0, ys+0, screen_i, screen_j);
    compute_xy(&xy[n1], xs+0, ys+h, screen_i, screen_j);
    compute_xy(&xy[n2], xs+w, ys+h, screen_i, screen_j);
    compute_xy(&xy[n3], xs+w, ys+0, screen_i, screen_j);

    rgb[n0] = Vector(cr, cg, cb);
    rgb[n1] = Vector(cr, cg, cb);
    rgb[n2] = Vector(cr, cg, cb);
    rgb[n3] = Vector(cr, cg, cb);


    glTexCoord2f(ci, cj);
    glVertex2f(xy[n0].x, xy[n0].y);

    glTexCoord2f(ci, cj + 1);
    glVertex2f(xy[n1].x, xy[n1].y);

    glTexCoord2f(ci + 1, cj + 1);
    glVertex2f(xy[n2].x, xy[n2].y);

    glTexCoord2f(ci + 1, cj);
    glVertex2f(xy[n3].x, xy[n3].y);
}

const int LETTER_WIDTH = 11;
const int LETTER_HEIGHT = 13;

void draw_line(int *x, int *y, char *s) {
    const int LETTER_PAD = 2;

    const int MAXLEN = 1024;
    Vector uv[MAXLEN * 4];
    Vector xy[MAXLEN * 4];
    Vector rgb[MAXLEN * 4];

    int len = strlen(s);
    if (len > MAXLEN) len = MAXLEN;

    float cr = 1.0f;
    float cg = 1.0f;
    float cb = 1.0f;

    int xs = 0;
    int ys = *y;

    glBegin(GL_QUADS);

    int i;
    for (i = 0; i < len; i++) {
        add_letter_quad(s, uv, xy, rgb, i, xs, ys, LETTER_WIDTH, LETTER_HEIGHT,
                        cr, cg, cb, g_ScreenWidth, g_ScreenHeight);

        xs += LETTER_WIDTH;
    }

    glEnd();

    *y += LETTER_HEIGHT + LETTER_PAD;
}


void draw_texture_quad(GLuint texture_handle, float x, float y, 
                       float width, float height) {

    begin_bitmap_mode(texture_handle);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f(1, 0);
    glVertex2f(x + width, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + width, y + height);

    glTexCoord2f(0, 1);
    glVertex2f(x, y + height);

    glEnd();
    end_bitmap_mode();
}

void draw_texture_quad2(float x, float y, 
                        float width, float height) {

    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f(1, 0);
    glVertex2f(x + width, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + width, y + height);

    glTexCoord2f(0, 1);
    glVertex2f(x, y + height);
}

const float FR_WIDTH = 800;
const float FR_X0 = 100;

void draw_spot_at(float x, float y, float radius) {
    float x0 = x - radius;
    float y0 = y - radius;
    draw_texture_quad(spot_texture.texture_handle, x0, y0, 2 * radius, 2 * radius);
}

void graph_function_axes() {
    const float PAD = 10;
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = (768 / 2 - 50) * 0.6;

    glBegin(GL_LINE_STRIP);
    glVertex2f(x0 - PAD, y0);
    glVertex2f(x0 + width + 2 * PAD, y0);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0 - height/2 - 2 * PAD);
    glVertex2f(x0, y0 + height/2 + 2 * PAD);
    glEnd();
}

GLuint gl_texture_from_bitmap(char *bits, int width, int height) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_BGRA_EXT,
                      GL_UNSIGNED_BYTE, (void *)bits);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return texture_id;
}

unsigned char *do_dumb_alpha_thing(unsigned char *src, int width, int height) {
    int npixels = width * height;
    unsigned char *dest = (unsigned char *)malloc(npixels * 4);

    int i;
    for (i = 0; i < npixels; i++) {
        int r = src[i * 3 + 0];
        int g = src[i * 3 + 1];
        int b = src[i * 3 + 2];

        int sum = r + g + b;
        int a;
        if (sum) {
            a = 255;
        } else {
            a = 0;
        }

        dest[i * 4 + 0] = r;
        dest[i * 4 + 1] = g;
        dest[i * 4 + 2] = b;
        dest[i * 4 + 3] = a;
    }

    free(src);
    return dest;
}

bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result) {
    unsigned char *bitmap;
    int width, height;
    bool success = load_jpeg_file(filename, &bitmap, &width, &height);
    if (success == false) return false;

    bitmap = do_dumb_alpha_thing(bitmap, width, height);

    GLuint texture = gl_texture_from_bitmap((char *)bitmap, width, height);

    // Give the appropriate data values back to the caller.

    *result = texture;
    *width_result = width;
    *height_result = height;

    // By the way... since we aren't remembering the pointer to 'bitmap'
    // anywhere, we can never deallocate it, so it gets leaked.  Not a big
    // deal in this demo, but for real software, you want to change that.

    return true;
}

void load_texture(Loaded_Texture_Info *info, char *name) {
    bool success = texture_from_file(name, &info->texture_handle,
                                     &info->width, &info->height);
    info->loaded_successfully = success;
}

void init_textures() {
    load_texture(&spot_texture, "white_dot.jpg");
    load_texture(&target_texture, "target.jpg");
    load_texture(&anchor_texture, "anchor.jpg");
    load_texture(&twist_stripes, "twist_stripes.jpg");
}


// The test code below is probably not relevant to anything you
// want to do; was just using it to verify that my functions
// were kosher.
void do_euler_test() {
    Vector3 x(1, 0, 0);
    Vector3 y(0, 1, 0);

    float alpha = M_PI * 0.3;
    float beta = M_PI * 0.2;

    Quaternion qx;
    qx.set_from_axis_and_angle(x.x, x.y, x.z, alpha);

    Vector3 yp = qx.rotate(y);
    Quaternion qy;
    qy.set_from_axis_and_angle(yp.x, yp.y, yp.z, beta);

    Quaternion rot = qy.multiply(qx);

    Vector3 xprime = rot.rotate_x_axis();
    Quaternion S, T;
    S = simple_rotation(x, xprime);

    T = rot.multiply(S.conjugate());
}

void test_stuff() {
    Vector3 v2(-1, 3, -6);
    v2 = v2.normalize();
    Vector3 v3(4, -0.5, -3);
    v3 = v3.normalize();
    Quaternion q1(1, 2, 3, 4);
    q1.normalize();
    Quaternion q2, q3;
    q2.set_from_axis_and_angle(v2.x, v2.y, v2.z, M_PI * 0.333);
    q3.set_from_axis_and_angle(v3.x, v3.y, v3.z, M_PI * 0.7);

    Vector3 x(1, 0, 0);

    Vector3 a1 = q1.rotate(x);
    Vector3 a2 = q2.rotate(x);
    Vector3 a3 = q3.rotate(x);

    Vector3 b1 = q1.rotate_x_axis();
    Vector3 b2 = q2.rotate_x_axis();
    Vector3 b3 = q3.rotate_x_axis();

    do_euler_test();
}



// A bunch of random opengl rendering setup stuff.

void begin_line_mode(float r, float g, float b) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_ScreenWidth, g_ScreenHeight, 0, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    ambient[1] = r;
    ambient[2] = g;
    ambient[3] = b;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
}

void end_line_mode() {
}

void begin_bitmap_mode(GLuint texture_handle) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_ScreenWidth, g_ScreenHeight, 0, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    glEnable(GL_BLEND);  // XXX Clean up earlier disables in this func
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void end_bitmap_mode() {
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

void begin_text_mode() {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(LETTER_WIDTH / 256.0, LETTER_HEIGHT / 256.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_ScreenWidth, g_ScreenHeight, 0, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, font_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
}

void end_text_mode() {
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

void set_modelview_transform(const Vector3 &up, const Vector3 &pos) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(pos.x, pos.y, pos.z, 0, 0, 0, up.x, up.y, up.z);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, 1024 / 768.0, 0.1f, 1000.0f);
}



//
// Here is where I do things to draw the bones and whatnot.
//

void draw_bone_strip(Quaternion ori_near, 
                     Vector3 axis1_near, Vector3 axis2_near,
                     Quaternion ori_far, 
                     Vector3 axis1_far, Vector3 axis2_far,
                     float len0, float len1, Vector3 position) {

    Vector3 arm(1, 0, 0);
    Vector3 arm_world = ori_far.rotate(arm);

    Vector3 p0 = position.add(arm_world.scale(len0));
    Vector3 p1 = position.add(arm_world.scale(len1));


    Vector3 handle1_near = ori_near.rotate(axis1_near);
    Vector3 handle2_near = ori_near.rotate(axis2_near);

    Vector3 handle1_far = ori_far.rotate(axis1_far);
    Vector3 handle2_far = ori_far.rotate(axis2_far);


    const int NUM_PANELS = 12;
    double dtheta = (2 * M_PI) / (double)NUM_PANELS;
    
    glBegin(GL_TRIANGLES);

    int i;
    for (i = 0; i < NUM_PANELS; i++) {
        double theta0 = i * dtheta;
        double theta1 = (i + 1) * dtheta;

        double ct0 = cos(theta0);
        double st0 = sin(theta0);
        double ct1 = cos(theta1);
        double st1 = sin(theta1);

        float u0 = i / (double)NUM_PANELS;
        float u1 = (i + 1) / (double)NUM_PANELS;

        Vector3 v0, v1, v2, v3;
        
        Vector3 pos_a_near = handle1_near.scale(ct0).add(handle2_near.scale(st0));
        Vector3 pos_b_near = handle1_near.scale(ct1).add(handle2_near.scale(st1));
        Vector3 pos_a_far = handle1_far.scale(ct0).add(handle2_far.scale(st0));
        Vector3 pos_b_far = handle1_far.scale(ct1).add(handle2_far.scale(st1));
        
        Vector3 normal_a_near = pos_a_near.normalize();
        Vector3 normal_b_near = pos_b_near.normalize();

        Vector3 normal_a_far = pos_a_far.normalize();
        Vector3 normal_b_far = pos_b_far.normalize();


        v0 = p0.add(pos_a_near);
        v1 = p1.add(pos_a_far);
        v2 = p1.add(pos_b_far);
        v3 = p0.add(pos_b_near);

        glTexCoord2f(u0, 0);
        glNormal3fv((float *)&normal_a_near);
        glVertex3f(v0.x, v0.y, v0.z);

        glTexCoord2f(u0, 1);
        glNormal3fv((float *)&normal_a_far);
        glVertex3f(v1.x, v1.y, v1.z);

        glTexCoord2f(u1, 1);
        glNormal3fv((float *)&normal_b_far);
        glVertex3f(v2.x, v2.y, v2.z);


        glTexCoord2f(u0, 0);
        glNormal3fv((float *)&normal_a_near);
        glVertex3f(v0.x, v0.y, v0.z);

        glTexCoord2f(u1, 1);
        glNormal3fv((float *)&normal_b_far);
        glVertex3f(v2.x, v2.y, v2.z);

        glTexCoord2f(u1, 0);
        glNormal3fv((float *)&normal_b_near);
        glVertex3f(v3.x, v3.y, v3.z);
    }    

    glEnd();
}

void draw_bone(Bone *bone, Bone *bone_array) {
    Vector3 pos = bone->position_global;
    float length = bone->bone_length;

    glColor3f(bone->color_r, bone->color_g, bone->color_b);

    // Figure out the orientations.

    Quaternion parent_orientation(0, 0, 0, 1);
    if (bone->parent_index != -1) parent_orientation = bone_array[bone->parent_index].orientation_global;

    Quaternion ori = bone->orientation_local;
    Vector3 rx = ori.rotate_x_axis();
    Quaternion simple = fast_simple_rotation_from_x_axis(rx);
    Quaternion orientation_near = parent_orientation.multiply(simple);
    Quaternion orientation_far = bone->orientation_global;
    Quaternion orientation_mid = slerp(orientation_near, orientation_far, 0.5f);

    float near_radius_a = bone->near_radius_a;
    float near_radius_b = bone->near_radius_b;
    float mid_radius_a = bone->mid_radius_a;
    float mid_radius_b = bone->mid_radius_b;
    float far_radius_a = bone->far_radius_a;
    float far_radius_b = bone->far_radius_b;

    if (should_draw_joint_limits) {
        near_radius_a = BONE_THIN_RADIUS;
        near_radius_b = BONE_THIN_RADIUS;
        mid_radius_a = BONE_THIN_RADIUS;
        mid_radius_b = BONE_THIN_RADIUS;
        far_radius_a = BONE_THIN_RADIUS;
        far_radius_b = BONE_THIN_RADIUS;
    }

    Vector3 axis1_near(0, near_radius_a, 0);
    Vector3 axis2_near(0, 0, near_radius_b);

    Vector3 axis1_mid(0, mid_radius_a, 0);
    Vector3 axis2_mid(0, 0, mid_radius_b);

    Vector3 axis1_far(0, far_radius_a, 0);
    Vector3 axis2_far(0, 0, far_radius_b);

    float midpoint = bone->bulge_midpoint * length;
    draw_bone_strip(orientation_near, axis1_near, axis2_near,
                    orientation_mid, axis1_mid, axis2_mid, 0, midpoint, pos);
    draw_bone_strip(orientation_mid, axis1_mid, axis2_mid,
                    orientation_far, axis1_far, axis2_far, midpoint, length, pos);
}

Vector3 get_reach_window_point(Bone *bone, Quaternion orientation, int index) {
    assert(index >= 0);
    assert(index < bone->limiting_polygon.num_vertices);

    Vector3 pos = bone->limiting_polygon.vertices[index];

    // Scale the reach window so that it's sized to be drawn
    // halfway along the bone.  Since the reach window is a 2D
    // cross-section of a cone, we can just pick the appropriate
    // length along the cone at which we want to take a cross
    // section, for most convenient rendering.

    pos = pos.scale(bone->bone_length * 0.2f);

    pos = orientation.rotate(pos);
    pos = bone->position_global.add(pos);

    return pos;
}

void draw_joint_limit(Bone *bone, Bone *bones) {
    Quaternion parent_orientation(0, 0, 0, 1);
    if (bone->parent_index != -1) {
        parent_orientation = bones[bone->parent_index].orientation_global;
    }
    
    Limiting_Polygon *polygon = &bone->limiting_polygon;

    Vector3 normal(1, 0, 0);
    normal = parent_orientation.rotate(normal);

    glEnable(GL_BLEND);  // XXX Clean up earlier disables in this func
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(bone->color_r, bone->color_g, bone->color_b, 0.6f);

    glBegin(GL_TRIANGLES);

    int i;
    for (i = 1; i < polygon->num_vertices; i++) {
        int n0 = 0;
        int n1 = i;
        int n2 = (i + 1) % polygon->num_vertices;

        Vector3 p0 = get_reach_window_point(bone, parent_orientation, n0);
        Vector3 p1 = get_reach_window_point(bone, parent_orientation, n1);
        Vector3 p2 = get_reach_window_point(bone, parent_orientation, n2);

        glTexCoord2f(0, 0);
        glNormal3fv((float *)&normal);
        glVertex3f(p0.x, p0.y, p0.z);

        glTexCoord2f(0, 0);
        glNormal3fv((float *)&normal);
        glVertex3f(p1.x, p1.y, p1.z);

        glTexCoord2f(0, 0);
        glNormal3fv((float *)&normal);
        glVertex3f(p2.x, p2.y, p2.z);
    }

    glEnd();

    glDisable(GL_BLEND);
}

void draw_target_quad(Vector3 p0, Vector3 p1, 
                      Vector3 p2, Vector3 p3) {

    Vector3 v1 = p1.subtract(p0);
    Vector3 v2 = p2.subtract(p0);

    Vector3 normal = cross_product(v1, v2).normalize();

    glNormal3fv((float *)&normal);
    glTexCoord2f(0, 0);
    glVertex3fv((float *)&p0);

    glNormal3fv((float *)&normal);
    glTexCoord2f(1, 0);
    glVertex3fv((float *)&p1);

    glNormal3fv((float *)&normal);
    glTexCoord2f(1, 1);
    glVertex3fv((float *)&p2);

    glNormal3fv((float *)&normal);
    glTexCoord2f(0, 1);
    glVertex3fv((float *)&p3);
}

void draw_box(Vector3 pos, float s, Quaternion ori) {
    float dx, dy, dz;
    dx = dy = dz = s;

    Vector3 p0 = pos.add(Vector3(-dx, -dy, -dz));
    Vector3 p1 = pos.add(Vector3(+dx, -dy, -dz));
    Vector3 p2 = pos.add(Vector3(+dx, +dy, -dz));
    Vector3 p3 = pos.add(Vector3(-dx, +dy, -dz));
    Vector3 p4 = pos.add(Vector3(-dx, -dy, +dz));
    Vector3 p5 = pos.add(Vector3(+dx, -dy, +dz));
    Vector3 p6 = pos.add(Vector3(+dx, +dy, +dz));
    Vector3 p7 = pos.add(Vector3(-dx, +dy, +dz));

    p0 = ori.rotate(p0);
    p1 = ori.rotate(p1);
    p2 = ori.rotate(p2);
    p3 = ori.rotate(p3);
    p4 = ori.rotate(p4);
    p5 = ori.rotate(p5);
    p6 = ori.rotate(p6);
    p7 = ori.rotate(p7);

    glBegin(GL_QUADS);

    draw_target_quad(p0, p1, p2, p3);
    draw_target_quad(p1, p5, p6, p2);
    draw_target_quad(p5, p4, p7, p6);
    draw_target_quad(p4, p0, p3, p7);
    draw_target_quad(p3, p2, p6, p7);
    draw_target_quad(p4, p5, p1, p0);

    glEnd();
}

void draw_target(Vector3 *pos, Quaternion *ori, float bone_length,
                 int texture_handle) {
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    const float s = 0.5f;
    GLfloat ambient_i0[] = { s, s, s, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColor4ub(255, 255, 255, 255);

    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    Vector3 arm(bone_length, 0, 0);

    Vector3 arm_world = ori->rotate(arm);

    draw_box(*pos, bone_length, *ori);
}


// update_motion() looks at the keyboard input and moves around
// the IK target, or the target spot in graph mode.

void update_motion() {
    float forward_delta = 0, sideways_delta = 0;

    if (moving_forward) forward_delta -= 1.0f;
    if (moving_backward) forward_delta += 1.0f;

    if (moving_left) sideways_delta -= 1.0f;
    if (moving_right) sideways_delta += 1.0f;

    const double dt = 0.02f;  // XXX

    float forward_dist = forward_delta * dt * FORWARD_SPEED;
    float sideways_dist = sideways_delta * dt * SIDEWAYS_SPEED;

    view_distance += forward_dist;
    if (view_distance < VIEW_DISTANCE_MIN) view_distance = VIEW_DISTANCE_MIN;
    if (view_distance > VIEW_DISTANCE_MAX) view_distance = VIEW_DISTANCE_MAX;

    view_theta += sideways_dist;
    if (view_theta < 0) view_theta += 2 * M_PI;
    if (view_theta > 2 * M_PI) view_theta -= 2 * M_PI;

    float S = 1.5;
    vec_spot.y += sideways_delta * dt * S;
    vec_spot.z += forward_delta * dt * S;

    Vector3 axis(0, 0, 0);
    switch (change_axis) {
    case AXIS_X:
    default:
        axis.x = 1;
        break;
    case AXIS_Y:
        axis.y = 1;
        break;
    case AXIS_Z:
        axis.z = 1;
        break;
    }

    float target_delta = 0;
    if (target_forward) target_delta += 1.0f;
    if (target_backward) target_delta -= 1.0f;

    float target_dist = target_delta * dt * TARGET_SPEED;
    axis = axis.scale(target_dist);

    object_pos = object_pos.add(axis);
}




// Now we're getting to the code that actually does stuff.
// I should have broken this app up into separate files.
// Too late.

void Bone::reset() {
    parent_index = -1;

    near_radius_a = near_radius_b = 0.1f;
    mid_radius_a = mid_radius_b = 0.1f;
    far_radius_a = far_radius_b = 0.08f;
    bulge_midpoint = 0.5f;

    bone_length = 1.6f;
    orientation_local = Quaternion(0, 0, 0, 1);
    limit_direction = Vector3(1, 0, 0);
    limit_angle = M_PI;

    limiting_polygon.num_vertices = 0;

    color_r = 1;
    color_g = 1;
    color_b = 1;
}

void init_limiting_polygon(Limiting_Polygon *polygon, int num_vertices, float radius) {
    assert(num_vertices > 2);
    float dtheta = (1.0f / num_vertices) * 2 * M_PI;

    polygon->init();

    int i;
    for (i = 0; i < num_vertices; i++) {
        float theta = i * dtheta;

        float ct = cos(theta);
        float st = sin(theta);
        
        polygon->add_vertex(ct * radius, st * radius);
    }

    polygon->finalize();
}

void init_bone_hierarchy(Bone_Hierarchy *hierarchy) {
    hierarchy->num_bones = 3;

    int i;
    for (i = 0; i < hierarchy->num_bones; i++) {
        hierarchy->bones[i].reset();
    }

    Bone *bone;
    bone = &hierarchy->bones[0];
    float s = 0.8f;
    bone->near_radius_a *= 3.4 * s;
    bone->near_radius_b *= 3 * s;
    bone->mid_radius_a *= 3.4 * s;
    bone->mid_radius_b *= 4 * s;
    bone->far_radius_a *= 2.1;
    bone->far_radius_b *= 2;
    bone->bone_length = 1.7f;
    bone->color_r = 1;
    bone->color_g = 0;
    bone->color_b = 0;

    init_limiting_polygon(&bone->limiting_polygon, 
                          8, 4.0f);  // Octagon, radius 4, around center.


    bone = &hierarchy->bones[1];
    bone->parent_index = 0;
    bone->bone_length = 1.4f;
    bone->near_radius_a *= 2.1;
    bone->near_radius_b *= 1.5;
    bone->mid_radius_a *= 1.5;
    bone->mid_radius_a *= 1.5;
    bone->far_radius_a *= 1;
    bone->far_radius_b *= 1.6;
    bone->color_r = 0.3f;
    bone->color_g = 0.3f;
    bone->color_b = 1.0f;

    // Set up a thin wedge kind of shape for the elbow joint.
    Limiting_Polygon *polygon = &bone->limiting_polygon;
    polygon->init();
    polygon->add_vertex(0, -.1);
    polygon->add_vertex(.2, 1);
    polygon->add_vertex(0, 5);
    polygon->add_vertex(-.2, 1);
    polygon->finalize();



    bone = &hierarchy->bones[2];
    bone->parent_index = 1;
    bone->limit_angle = M_PI * 0.3;
    bone->bone_length = 0.6f;
    bone->mid_radius_a *= 3;
    bone->far_radius_a *= 2.4;
    bone->far_radius_b *= 0.2;
    bone->bulge_midpoint = 0.3f;
    bone->color_r = 0;
    bone->color_g = 1.0f;
    bone->color_b = 0;

    init_limiting_polygon(&bone->limiting_polygon, 
                          6, 2.0f);  // Hexagon, radius 2, around center.


    // A bunch of hardcoded poses for us to try starting at.

    if (starting_pose == 1) {
        hierarchy->bones[0].orientation_local.set_from_axis_and_angle(0.621554, 0.745512, -0.240588, 1.2);
        hierarchy->bones[1].orientation_local.set_from_axis_and_angle(0, -1, 0, 1.775162);
        hierarchy->bones[2].orientation_local.set_from_axis_and_angle(0, 0, -1, 0.931422);
    }
    if (starting_pose == 2) {
        hierarchy->bones[0].orientation_local.set_from_axis_and_angle(0.233329, -0.893333, -0.384074, 1.755125);
        hierarchy->bones[1].orientation_local.set_from_axis_and_angle(0, -1, 0, 2.216264);
        hierarchy->bones[2].orientation_local.set_from_axis_and_angle(0, 0, -1, 0.898411);
    }
    if (starting_pose == 3) {
        hierarchy->bones[0].orientation_local.set_from_axis_and_angle(-0.809216, 0.582796, -0.074284, 0.943016);
        hierarchy->bones[1].orientation_local.set_from_axis_and_angle(0, -1, 0, 2.452539);
        hierarchy->bones[2].orientation_local.set_from_axis_and_angle(0, 0, 1, 0.942478);
    }
}

void Bone_Hierarchy::update_transforms() {
    int i;
    for (i = 0; i < num_bones; i++) {
        Bone *bone = &bones[i];
        if ((bone->parent_index == -1) || (i == 0)) {
            bone->position_global = Vector3(0, 0, 0);
            bone->orientation_global = bone->orientation_local;
            continue;
        }

        assert(bone->parent_index >= 0);
        assert(bone->parent_index < i);

        Bone *parent = &bones[bone->parent_index];
        bone->orientation_global = parent->orientation_global.multiply(bone->orientation_local);
        Vector3 position_local(parent->bone_length, 0, 0);
        bone->position_global = parent->orientation_global.rotate(position_local).add(parent->position_global);
    }
}

Vector3 Bone_Hierarchy::get_endpoint_of_chain(int index) {
    Bone *bone = &bones[index];
    Vector3 arm(bone->bone_length, 0, 0);
    Vector3 rotated = bone->orientation_global.rotate(arm);
    Vector3 result = rotated.add(bone->position_global);

    return result;
}

void Bone_Hierarchy::do_bone_ik(int end_index, Vector3 *target, float step_size) {
    update_transforms();

    int bone_index = end_index;
    while (bone_index != -1) {
        Bone *bone = &bones[bone_index];
        int parent_index = bone->parent_index;

        Bone *parent;
        Quaternion parent_orientation;
        if (parent_index == -1) {
            parent = NULL;
            parent_orientation.set(0, 0, 0, 1);
        } else {
            parent = &bones[parent_index];
            parent_orientation = parent->orientation_global;
        }
        
        Vector3 bone_pos = bone->position_global;

        // Transform the target into local space.

        Vector3 target_local = target->subtract(bone_pos);
        target_local = parent_orientation.conjugate().rotate(target_local);

        Vector3 endpoint = get_endpoint_of_chain(end_index);

        /*
              Figure out the transform that gets the bone to the object.
            */

        Vector3 orig_dir = endpoint.subtract(bone_pos).normalize();
        orig_dir = bone->orientation_global.conjugate().rotate(orig_dir);

        Vector3 dest_dir = target_local.normalize();

        Quaternion r0 = fast_simple_rotation(orig_dir, dest_dir);
        Quaternion r1 = slerp(bone->orientation_local, r0, step_size);

        Quaternion r2;

        /* Before I put in the polygonal joint limits, I wrote this
           circular cone-style joint limiting thing.  I left it in,
           just in case you might want to use it.

          bool limited = limit_rotation_cone_style(&r1, bone->limit_angle, &r2);
          limited = false;
          if (!limited) r2 = r1;
        */

        if (should_impose_joint_limits) {
            r2 = limit_rotation(&r1, &bone->limiting_polygon, bone->limit_angle);
        } else {
            r2 = r1;
        }

        bone->orientation_local = r2;

        update_transforms();  // Very slow... change this!
                              // A real fast IK scheme would keep some extra 
                              // bookkeeping data to allow incremental updates.
        // But the point of this app is to show that it is
        // possible to build a very fast IK system using the
        // quaternion math.  Not to actually build such a
        // production system for you (too much work for a
        // single magazine article!)

        // Continue with the loop structure.
        bone_index = bone->parent_index;
    }
}

void draw_bone_hierarchy(Bone_Hierarchy *hierarchy) {
    int i;
    for (i = 0; i < hierarchy->num_bones; i++) {
        Bone *bone = &hierarchy->bones[i];
        draw_bone(bone, hierarchy->bones);
    }
}

void draw_joint_limits_for_bone_hierarchy(Bone_Hierarchy *hierarchy) {
    int i;
    for (i = 0; i < hierarchy->num_bones; i++) {
        Bone *bone = &hierarchy->bones[i];
        draw_joint_limit(bone, hierarchy->bones);
    }
}

void draw_bone_states(Bone_Hierarchy *hierarchy) {
    int sx = 10, sy = 10;
    char buf[1000];

    begin_text_mode();

    int i;
    for (i = 0; i < hierarchy->num_bones; i++) {
        Bone *bone = &hierarchy->bones[i];
        double ax, ay, az, atheta;
        bone->orientation_local.get_axis_and_angle(&ax, &ay, &az, &atheta);
        sprintf(buf, "%d:  Axis (%.6f, %.6f, %.6f), Angle %.6f",
                i, ax, ay, az, atheta);
        draw_line(&sx, &sy, buf);
    }

    end_text_mode();
}

void ensure_vector_is_within_limiting_polygon(Limiting_Polygon *polygon,
                                              Vector3 *vector);

// do_test_polygon() does the hexagon that you see in graph mode.

void do_test_polygon() {
    Limiting_Polygon polygon;
    polygon.init();
    float px = 1.5;
    float py = 1.6;
    polygon.add_vertex(3, 0);
    polygon.add_vertex(px, py);
    polygon.add_vertex(-px, py);
    polygon.add_vertex(-3, 0);
    polygon.add_vertex(-px, -py);
    polygon.add_vertex(px, -py);
    polygon.finalize();

    const float SCALE = 80;
    const float xoff = 512;
    const float yoff = 384;

    GLuint handle = spot_texture.texture_handle;
    begin_bitmap_mode(handle);

    begin_line_mode(0, 1, 0);
    glColor3f(0, 0, 1);
    glLineWidth(7.0f);
    int i;
    for (i = 0; i < polygon.num_vertices; i++) {
        float x0 = polygon.vertices[i].y;
        float y0 = polygon.vertices[i].z;
        float x1 = polygon.vertices[(i+1)%polygon.num_vertices].y;
        float y1 = polygon.vertices[(i+1)%polygon.num_vertices].z;

        x0  = (x0 * SCALE) + xoff;
        y0  = (y0 * SCALE) + yoff;
        x1  = (x1 * SCALE) + xoff;
        y1  = (y1 * SCALE) + yoff;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
        glEnd();
    }

    Vector3 vec = vec_spot;
    vec.x = +1;
    {
        float x1 = vec.y;
        float y1 = vec.z;
        
        float x0 = 0;
        float y0 = 0;

        x0  = (x0 * SCALE) + xoff;
        y0  = (y0 * SCALE) + yoff;
        x1  = (x1 * SCALE) + xoff;
        y1  = (y1 * SCALE) + yoff;

        glColor3f(.7f, .4f, .3f);
        glLineWidth(5.0f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
        glEnd();

        glColor3f(1, 0, 0);
        float w = 30;
        draw_texture_quad(handle, x1-w*.5, y1-w*.5, w, w);

        glColor3f(1, 1, 1);
        w = 20;
        draw_texture_quad(handle, x0-w*.5, y0-w*.5, w, w);
    }


    ensure_vector_is_within_limiting_polygon(&polygon, &vec);

    glColor3f(0, 1, 0);
    {
        float x = vec.y;
        float y = vec.z;
        x  = (x * SCALE) + xoff;
        y  = (y * SCALE) + yoff;
        
        float w = 20;
        draw_texture_quad(handle, x-w*.5, y-w*.5, w, w);
    }

    end_bitmap_mode();
}


void draw_scene_ik() {
    const int DRAW_BONES = 1;

    static Bone_Hierarchy hierarchy;
    init_bone_hierarchy(&hierarchy);

    update_motion();

    Vector3 forward(1, 0, 0);
    Vector3 left(0, 1, 0);
    Vector3 up(0, 0, 1);

    Vector3 pos;
    pos.z = 0;
    pos.x = view_distance * cos(view_theta);
    pos.y = view_distance * sin(view_theta);

    
    set_modelview_transform(up, pos);

    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat unchoked[] = { 1, 1, 1, 1 };
    GLfloat light_pos[] = { -5, 1, 1, 0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, unchoked);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor4f(0.7f, 0.4f, 0.2f, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, twist_stripes.texture_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(255, 100, 50, 50);
    
    const float CONVERGENCE_DISTANCE = 0.03f;
    const float CONVERGENCE_D2 = CONVERGENCE_DISTANCE * CONVERGENCE_DISTANCE;

    int converged_after = -1;
    int num_iterations = 0;

    const int DO_BLUR = 0;

    const int MAX_STEPS = 20;
    int i;
    for (i = 0; i < MAX_STEPS; i++) {
        num_iterations++;

        float perc = (i + 1) / (double)MAX_STEPS;

        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);

        // Make sure we draw the initial state before we tweak anything.
        // XXX This doesn't seem to be working??
        
        if (DO_BLUR) {
            if (i == 0) draw_bone_hierarchy(&hierarchy);
        }

        hierarchy.do_bone_ik(hierarchy.num_bones - 1, &object_pos, 1.0f);

        if (DO_BLUR) draw_bone_hierarchy(&hierarchy);

        Vector3 endpoint;
        endpoint = hierarchy.get_endpoint_of_chain(hierarchy.num_bones - 1);
        Vector3 diff = endpoint.subtract(object_pos);
        float len2 = diff.length_squared();
        if (len2 < CONVERGENCE_D2) {
            if (converged_after == -1) converged_after = num_iterations;
            if (!want_smoothness) break;
        }
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    if (DRAW_BONES) draw_bone_hierarchy(&hierarchy);

    Quaternion arm_ori = hierarchy.bones[0].orientation_global;
    Vector3 arm_start(-0.32f, 0, 0);
    if (DRAW_BONES) draw_target(&arm_start, &arm_ori,
                                0.33f, anchor_texture.texture_handle);

    Quaternion r0(0, 0, 0, 1);
    if (DRAW_BONES) draw_target(&object_pos, &r0,
                                0.15f, target_texture.texture_handle);

    // Draw the joint limits last since they are transparent things
    // and because 3D hardware rocks, we have to draw transparent stuff
    // last.  I am not going to bother to sort, though.  Deal.
    if (DRAW_BONES && should_draw_joint_limits) draw_joint_limits_for_bone_hierarchy(&hierarchy);

    glDisable(GL_TEXTURE_2D);

    if (should_draw_bone_states) draw_bone_states(&hierarchy);

    begin_text_mode();

    int sx = 10, sy = 10;
    char buf[1000];
    sprintf(buf, "Starting pose number %d.\n", starting_pose);
    draw_line(&sx, &sy, buf);
    sprintf(buf, "Joint limit enforcement: %s.\n", should_impose_joint_limits ? "ON" : "OFF");
    draw_line(&sx, &sy, buf);
    sprintf(buf, "Axis of target control: %s.\n", axis_names[change_axis]);
    draw_line(&sx, &sy, buf);

    if (converged_after != -1) {
        sprintf(buf, "Converged after %d iterations.", converged_after);
        if (want_smoothness) {
            char buf2[1000];
            sprintf(buf2, "  (Ran %d anyway for smoothness.)", MAX_STEPS);
            strcat(buf, buf2);
        }
    } else {
        sprintf(buf, "Failed to converge after %d iterations.", num_iterations);
    }

    draw_line(&sx, &sy, buf);

    end_text_mode();
}

void init_scene_ik() {
    object_pos = Vector3(4, 1, 1);
    change_axis = AXIS_X;
    view_theta = 0;
    view_distance = 5.0;
    starting_pose = 0;
}

void draw_scene_polygon_test() {
    update_motion();
    do_test_polygon();
}

void init_scene_polygon_test() {
    vec_spot = Vector3(0, 0, 0);
}

void draw_scene() {
    static bool first = true;
    if (first) {
        first = false;
        graph_mode_active = false;
        init_scene_ik();

        int width, height;
        bool success = texture_from_file("font.jpg", &font_handle, &width, &height);
        assert(success);

        init_textures();
    }

    if (graph_mode_active) {
        draw_scene_polygon_test();
    } else {
        draw_scene_ik();
    }
}

void handle_keydown(int key) {
    switch (key) {
    case 'G':
        graph_mode_active = !graph_mode_active;
        if (!graph_mode_active) {
            init_scene_ik();
        } else {
            init_scene_polygon_test();
        }
        break;
    case 'W':
        moving_forward = 1;
        break;
    case 'S':
        moving_backward = 1;
        break;
    case 'A':
        moving_left = 1;
        break;
    case 'D':
        moving_right = 1;
        break;
    case 'E':
        target_forward = 1;
        break;
    case 'R':
        target_backward = 1;
        break;
    case 'X':
        change_axis = AXIS_X;
        break;
    case 'Y':
        change_axis = AXIS_Y;
        break;
    case 'Z':
        change_axis = AXIS_Z;
        break;
    case 'L':
        should_impose_joint_limits = !should_impose_joint_limits;
        break;
    case 'J':
        should_draw_joint_limits = !should_draw_joint_limits;
        break;
    case 'T':
        should_draw_bone_states = !should_draw_bone_states;
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
        starting_pose = key - '0';
        break;
    case 'F':
        use_fast_normalize = !use_fast_normalize;
    case 'I':
        want_smoothness = !want_smoothness;
    }
}

void handle_keyup(int key) {
    switch (key) {
    case 'W':
        moving_forward = 0;
        break;
    case 'S':
        moving_backward = 0;
        break;
    case 'A':
        moving_left = 0;
        break;
    case 'D':
        moving_right = 0;
        break;
    case 'E':
        target_forward = 0;
        break;
    case 'R':
        target_backward = 0;
        break;
    }
}




bool limit_rotation_cone_style(Quaternion *attempt, double theta, 
                               Quaternion *result) {

    Vector3 x(1, 0, 0);
    Vector3 xprime = attempt->rotate(x);

    double ct = cos(theta);
    if (dot_product(xprime, x) >= ct) return false;

    double ax, ay, az, atheta;
    attempt->get_axis_and_angle(&ax, &ay, &az, &atheta);
    if (atheta < 0) atheta += 2 * M_PI;
    if (atheta > M_PI) {
        ax = -ax;
        ay = -ay;
        ax = -az;
        atheta = -atheta + 2 * M_PI;
    }

    result->set_from_axis_and_angle(ax, ay, az, theta);
    return true;
}


// Stuff below forces an IK joint to stay within the limiting
// polygon.  It assumes the polygon has the x axis as its surface
// normal.  To handle multiple polygons that are not coplanar,
// (so that you can have joint limits that span more than one
// hemisphere), this code will have to be generalized a little.
// It is not hard to do though.  I was going to do it but ran
// out of time.
Vector3 intersect_with_plane(Vector3 *vector) {
    Vector3 result = *vector;

    const float YZ_FAR = 1000.0f;
    const float X_MIN = 0.01;
    if (result.x < X_MIN) {
        result.x = 1;
        result.y *= YZ_FAR;
        result.z *= YZ_FAR;
    }

    float factor = 1.0f / result.x;
    result = result.scale(factor);

    return result;
}

// This does a linear search to clamp the destination point to
// be within the polygon.  If you really wanted to be fast, and
// have really nice round polygons, you'd probably BSP them
// or something.
void ensure_vector_is_within_limiting_polygon(Limiting_Polygon *polygon,
                                              Vector3 *vector) {
    if (polygon->num_vertices == 0) return;

    float best_distance2 = FLT_MAX;
    Vector3 target = intersect_with_plane(vector);

    int i;
    for (i = 0; i < polygon->num_vertices; i++) {
        Vector3 *v0 = &polygon->vertices[i];
        Vector3 *v1 = &polygon->vertices[(i + 1) % polygon->num_vertices];

        float dx = v1->y - v0->y;
        float dy = v1->z - v0->z;

        float rx = target.y - v0->y;
        float ry = target.z - v0->z;

        float dot = -dy * rx + dx * ry;
        if (dot >= 0) continue;        

        // Find the projection of the target point onto this line
        // (clamping at the enpoint of the line segments).

        float l_dot_l = dx*dx + dy*dy;

        float proj_x, proj_y;
        float factor = (rx * dx + ry * dy) / l_dot_l;
        proj_x = factor * dx;
        proj_y = factor * dy;

        float j_dot_j = proj_x*proj_x + proj_y*proj_y;
        float j_dot_l = proj_x*dx + proj_y*dy;

        if (j_dot_j > l_dot_l) {
            proj_x = v1->y - v0->y;
            proj_y = v1->z - v0->z;
        } else if (j_dot_l < 0) {
            proj_x = 0;
            proj_y = 0;
        }

        float dist_dx, dist_dy;
        dist_dx = rx - proj_x;
        dist_dy = ry - proj_y;

        float dist2 = dist_dx*dist_dx + dist_dy*dist_dy;
        if (dist2 < best_distance2) {
            best_distance2 = dist2;
            vector->x = target.x;
            vector->y = proj_x + v0->y;
            vector->z = proj_y + v0->z;

            *vector = intersect_with_plane(vector);
        }
    }
}

Quaternion limit_rotation(Quaternion *attempt, Limiting_Polygon *polygon,
                          float max_axial_angle) {
    Vector3 xprime = attempt->rotate_x_axis();

    // Limit the reach.

    Quaternion simple = fast_simple_rotation_from_x_axis(xprime);
    Quaternion twist = attempt->multiply(simple.conjugate());

    ensure_vector_is_within_limiting_polygon(polygon, &xprime);
    xprime = xprime.normalize(); // XXX
    simple = fast_simple_rotation_from_x_axis(xprime);

    // Limit the twist.
    // For clarity's sake I didn't do the fast twist limiting
    // stuff talked about at the end of the article; this is
    // where that would go.

    double x, y, z, theta;
    twist.get_axis_and_angle(&x, &y, &z, &theta);
    if (theta > max_axial_angle) theta = max_axial_angle;
    if (theta < -max_axial_angle) theta = -max_axial_angle;

    twist.set_from_axis_and_angle(x, y, z, theta);

    Quaternion result = twist.multiply(simple);
    return result;
}
