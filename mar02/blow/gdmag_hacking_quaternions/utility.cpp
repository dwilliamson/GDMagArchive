#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"
#include "jpeg_load.h"
#include "geometry.h"
#include "utility.h"

extern int g_ScreenWidth;
extern int g_ScreenHeight;
extern GLuint font_handle;
extern GLuint spot_texture_handle;

// Utility functions for drawing text on the screen.

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

void draw_text_line(int *x, int *y, char *s) {
    const int MAXLEN = 1024;
    Vector uv[MAXLEN * 4];
    Vector xy[MAXLEN * 4];
    Vector rgb[MAXLEN * 4];

    int len = strlen(s);
    if (len > MAXLEN) len = MAXLEN;

    float cr = 1.0f;
    float cg = 1.0f;
    float cb = 1.0f;

    int xs = *x;
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

void graph_function(float x0, float y0, float scale,
                    double (*function)(double x)) {
    glBegin(GL_LINE_STRIP);

    int i;
    for (i = 0; i < scale; i++) {
        float perc = i / scale;
        float x = x0 + i;
        float y = y0 + function(perc) * scale;
        glVertex2f(x, y);
    }
    glEnd();
}


const float FR_WIDTH = 800;
const float FR_X0 = 100;

void draw_spot_at(float x, float y, float radius) {
    float x0 = x - radius;
    float y0 = y - radius;
    draw_texture_quad(spot_texture_handle, x0, y0, 2 * radius, 2 * radius);
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 
                 width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 (void *)bits);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return texture_id;
}

bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result) {
    unsigned char *bitmap;
    int width, height;
    bool success = load_jpeg_file(filename, &bitmap, &width, &height);
    if (success == false) return false;

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

void begin_line_mode(float r, float g, float b) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_ScreenWidth, 0, g_ScreenHeight, 0, -100);

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

