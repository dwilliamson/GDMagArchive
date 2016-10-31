#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"
#include "jpeg_load.h"
#include "geometry.h"
#include "image_buffer.h"
#include "filter.h"

#define DO_WIDE_FILTERS 1

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;

GLuint font_handle = 0;
extern int g_ScreenWidth;
extern int g_ScreenHeight;

bool graph_mode_active = false;
bool fillscreen_mode_active = false;
bool fillscreen_use_box_filter = true;
int fillscreen_miplevel = 3;

const float KAISER_ALPHA = 4.0;
const int LEVELS_TO_REDUCE = 3;

enum Filter_Types {
    FILTER_POINT,
    FILTER_INTEGER_BOX,
    FILTER_BOX,
    FILTER_LANCZOS,
    FILTER_KAISER,
    FILTER_BAD_RINGING,
    FILTER_STARTING_FROM_TOP,
    NUM_MIPMAP_FILTER_TYPES
};

const int GOOD_FILTER_WIDTH = 14;
const int BAD_RINGING_FILTER_WIDTH = 64;

char *the_filter_names[NUM_MIPMAP_FILTER_TYPES] = {
    "point",
    "integer box",
    "box",
    "lanczos",
    "kaiser",
    "bad ringing",
    "starting-from-top kaiser",
};

const int MAX_MIPMAP_LEVELS = 8;
struct Texture_Mipmap_Level {
    GLuint texture_handle;    // Only used for level 0.
    int width;
    int height;

    // The following is only used for level 1+:
    GLuint mipmap_texture_handles[NUM_MIPMAP_FILTER_TYPES];
};

struct Texture_Info {
    char *name;
    int num_mipmap_levels;

    Texture_Mipmap_Level mipmap_levels[MAX_MIPMAP_LEVELS];
};

const int NUM_WIDE_FILTERS = 8;
const int MAX_TEXTURES = 20;
Texture_Info the_texture_info[MAX_TEXTURES];
int num_loaded_textures = 0;
int current_texture_index = 0;

Filter *the_mipmap_filters[NUM_MIPMAP_FILTER_TYPES];
Filter *the_wide_filters[NUM_WIDE_FILTERS];

void begin_line_mode(float r, float g, float b);
void end_line_mode();
void begin_text_mode();
void end_text_mode();

char *texture_names[] = {
    "ChoirComm01_512x256.jpg",
    "CBBB_New04_128x256.jpg",
    "Hotel_SuomiVodka02_256x256.jpg",
    "dtr3_ss_phoenix1mile.jpg",
    "CitySewers01ab.jpg",
};

void begin_bitmap_mode(GLuint texture_handle);
void end_bitmap_mode();

const int NUM_TEXTURES = sizeof(texture_names) / sizeof(char *);


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

void draw_texture_quads(int texture_index) {
    Texture_Info *texture_info = &the_texture_info[texture_index];

    int y_start = 0;

    int x, y;
    x = 0;

    int mipmap_y_stride = -1;
    int PAD = 10;

    int i;
    for (i = 0; i < texture_info->num_mipmap_levels; i++) {
        y = y_start;

        Texture_Mipmap_Level *mipmap_level = &texture_info->mipmap_levels[i];

        if (i == 0) {
            draw_texture_quad(mipmap_level->texture_handle, x, y,
                              mipmap_level->width, mipmap_level->height);
            x += mipmap_level->width + PAD;
        } else {
            if (mipmap_y_stride == -1) mipmap_y_stride = mipmap_level->height + PAD;
            int j;
            for (j = 0; j < NUM_MIPMAP_FILTER_TYPES; j++) {
                draw_texture_quad(mipmap_level->mipmap_texture_handles[j], x, y,
                                  mipmap_level->width, mipmap_level->height);
                
//                y += mipmap_y_stride;
                y += mipmap_level->height + PAD;
            }

            x += mipmap_level->width + PAD;
        }
    }

}

double scaled_sinc(double x) {
    x = 1.0 - x;
    if (x == 0.0) return 1.0;
    return sin(M_PI * x) / (M_PI * x);
}

double linear_blend(double x) {
    return x;
}

double pow_22(double x) {
    return pow(x, 1.0 / 2.2);
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

void draw_function_axes(float x0, float y0, float scale) {
    const float PAD = 10;
    glBegin(GL_LINE_STRIP);
    glVertex2f(x0 - PAD, y0);
    glVertex2f(x0 + scale, y0);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0 - PAD);
    glVertex2f(x0, y0 + scale);
    glEnd();
}

const float FR_WIDTH = 800;
const float FR_X0 = 100;

void graph_frequency_response_axes() {
    const float PAD = 10;
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    glBegin(GL_LINE_STRIP);
    glVertex2f(x0 - PAD, y0);
    glVertex2f(x0 + width + 2 * PAD, y0);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0 - height - 2 * PAD);
    glVertex2f(x0, y0 + PAD);
    glEnd();
}

void graph_frequency_response_ideal() {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, 768 - (y0 + height));
    glVertex2f(x0 + width * 0.5f, 768 - (y0 + height));
    glVertex2f(x0 + width * 0.5f, 768 - y0);
    glVertex2f(x0 + width, 768 - y0);
    glEnd();
}

void graph_frequency_response(Filter *filter) {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    glBegin(GL_LINE_STRIP);

    int iwidth = (int)width;

    int i;
    for (i = 0; i < width; i++) {
        double omega = (i / width) * M_PI;

        double sum_real = 0;
        double sum_imaginary = 0;

        int j;
        for (j = 0; j < filter->nsamples; j++) {
            double tau = omega * j;
            sum_real += filter->data[j] * cos(tau);
            sum_imaginary += filter->data[j] * sin(tau);
        }

        double magnitude = sqrt(sum_real * sum_real + sum_imaginary * sum_imaginary);

        double x = (i / width) * width + x0;
        double y = 768 - ((magnitude * height) + y0);

        glVertex2f(x, y);
    }

    glEnd();
}

void draw_graphs() {
    begin_line_mode(1, 1, 1);

/*
    // Code to draw figure 1.
    glLineWidth(3);
    glColor3f(1, 1, 1);
    graph_frequency_response_axes();

    glLineWidth(5);
    glColor3f(0.7, 0.2, 0.3);
    graph_frequency_response_ideal();
    glColor3f(1, 1, 0);
    graph_frequency_response(the_mipmap_filters[FILTER_BOX]);
    glLineWidth(1);
    glColor3f(1, 1, 1);
*/

/*
    // Code to draw figure 2.
    glLineWidth(3);
    glColor3f(1, 1, 1);
    graph_frequency_response_axes();

    glLineWidth(5);
    glColor3f(0.7, 0.2, 0.3);
    graph_frequency_response_ideal();
    glColor3f(0, .7, .7);
    graph_frequency_response(the_mipmap_filters[FILTER_POINT]);
    glColor3f(1, 1, 0);
    graph_frequency_response(the_mipmap_filters[FILTER_BOX]);
    glColor3f(0, 0, 1);
    graph_frequency_response(the_mipmap_filters[FILTER_LANCZOS]);
    glColor3f(0, 1, 0);
    graph_frequency_response(the_mipmap_filters[FILTER_KAISER]);
    glLineWidth(1);
    glColor3f(1, 1, 1);
*/

/*
 */
    // Code to draw figure 3.

    glLineWidth(3);
    glColor3f(1, 1, 1);
    graph_frequency_response_axes();
    void fill_lanczos_filter(Filter *filter, double additional_stretch = 1,
                             float EXTRA1 = 1, float EXTRA2 = 1);

    glLineWidth(5);
    glColor3f(0.7, 0.2, 0.3);
    graph_frequency_response_ideal();

    Filter *filter = new Filter(16);
    fill_lanczos_filter(filter);
    glColor3f(0, 1, 0);
    graph_frequency_response(filter);
    delete filter;

    filter = new Filter(64);
    fill_lanczos_filter(filter);
    glColor3f(1, 1, 0);
    graph_frequency_response(filter);
    delete filter;
    glLineWidth(1);
    glColor3f(1, 1, 1);

/*
    graph_frequency_response(the_mipmap_filters[FILTER_BOX], 1, 1, 0);

*/
/*
    graph_frequency_response(the_wide_filters[0], 0, 1, 1);
    graph_frequency_response(the_wide_filters[1], 0, 1, 1);
*/
//    graph_frequency_response(the_wide_filters[2], 0, 0, 1);
}

/*
    float x0 = 10;
    float y0 = 100;
    float scale = 500;
    glColor3f(1, 1, 1);
    draw_function_axes(x0, y0, scale);
    glColor3f(0, 1, 0);
    graph_function(x0, y0, scale, scaled_sinc);
    glColor3f(0, 1, 1);
    graph_function(x0, y0, scale, pow_22);
    glColor3f(1, 0, 0);
    graph_function(x0, y0, scale, linear_blend);
    glColor3f(1, 1, 1);
*/


void draw_fillscreen_mode(int texture_index) {
    Texture_Info *texture_info = &the_texture_info[texture_index];

    // Choose a mipmap level of a size that is reasonably appropriate
    // for the texture we're looking at.
/*
    int mipmap_index;
    Texture_Mipmap_Level *biggest = &texture_info->mipmap_levels[0];
    int area = biggest->width * biggest->height;
    if (area > 256 * 256) {
        mipmap_index = 3;
    } else {
        mipmap_index = 2;
    }
*/
    int mipmap_index = fillscreen_miplevel;
    if (mipmap_index >= texture_info->num_mipmap_levels) {
        mipmap_index = texture_info->num_mipmap_levels - 1;
    }
    fillscreen_miplevel = mipmap_index;

    Texture_Mipmap_Level *mipmap_level = &texture_info->mipmap_levels[mipmap_index];

    int y_start = 0;
    int x_start = 0;

#if DO_WIDE_FILTERS
    int handle_index = FILTER_STARTING_FROM_TOP;
#else
    int handle_index = FILTER_KAISER;
#endif
    if (fillscreen_use_box_filter) handle_index = FILTER_INTEGER_BOX;

    int texture_handle = mipmap_level->mipmap_texture_handles[handle_index];

    begin_bitmap_mode(texture_handle);
    glBegin(GL_QUADS);

    int i;
    for (i = 0; i < texture_info->num_mipmap_levels; i++) {
        int x, y;
        y = y_start;

        while (y <= g_ScreenHeight) {
            x = x_start;
            while (x <= g_ScreenWidth) {
                draw_texture_quad2(x, y,
                                   mipmap_level->width, mipmap_level->height);
                x += mipmap_level->width;
            }

            y += mipmap_level->height;
        }
    }

    glEnd();
    end_bitmap_mode();

    begin_text_mode();
    int x, y;
    x = 100;
    y = 700;
    char buf[BUFSIZ];
    sprintf(buf, "Filter: %s", the_filter_names[handle_index]);
    draw_line(&x, &y, buf);
    end_text_mode();
    
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

void put_bytes_into_image_buffer(unsigned char *bitmap, Image_Buffer *image_buffer) {
    int i, j;
    for (j = 0; j < image_buffer->height; j++) {
        for (i = 0; i < image_buffer->width; i++) {
            int k = j * image_buffer->width + i;

            int index = image_buffer->get_index(i, j);
            image_buffer->channels[0]->data[index] = bitmap[k * 3 + 0] / 255.0f;
            image_buffer->channels[1]->data[index] = bitmap[k * 3 + 1] / 255.0f;
            image_buffer->channels[2]->data[index] = bitmap[k * 3 + 2] / 255.0f;
        }
    }
}

bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result, 
                       Image_Buffer **image_buffer_result = NULL) {
    unsigned char *bitmap;
    int width, height;
    bool success = load_jpeg_file(filename, &bitmap, &width, &height);
    if (success == false) return false;

    GLuint texture = gl_texture_from_bitmap((char *)bitmap, width, height);

    // Give the appropriate data values back to the caller.

    *result = texture;
    *width_result = width;
    *height_result = height;

    // If they requested an image buffer, let's fill it in for 'em.

    if (image_buffer_result) {
        Image_Buffer *image_buffer = new Image_Buffer(width, height);
        put_bytes_into_image_buffer(bitmap, image_buffer);
        *image_buffer_result = image_buffer;
    }

    // By the way... since we aren't remembering the pointer to 'bitmap'
    // anywhere, we can never deallocate it, so it gets leaked.  Not a big
    // deal in this demo, but for real software, you want to change that.

    return true;
}

GLuint texture_from_image_buffer(Image_Buffer *buffer) {
    unsigned char *bitmap;

    int width = buffer->width;
    int height = buffer->height;
    int nchannels = buffer->nchannels;

    bitmap = new unsigned char[width * height * nchannels];

    int i, j, k;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            for (k = 0; k < nchannels; k++) {
                int index = buffer->get_index(i, j);
                float channel_value = buffer->channels[k]->data[index];

                if (channel_value < 0) channel_value = 0;
                if (channel_value > 1) channel_value = 1;

                unsigned char pixel_value = (unsigned char)(channel_value * 255.0f + 0.5f);
                bitmap[(j * width + i) * nchannels + k] = pixel_value;
            }
        }
    }

    // By the way... since we aren't remembering the pointer to 'bitmap'
    // anywhere, we can never deallocate it, so it gets leaked.  Not a big
    // deal in this demo, but for real software, you want to change that.

    GLuint texture = gl_texture_from_bitmap((char *)bitmap, width, height);
    return texture;
}

inline int get_integer(Image_Buffer *image, int channel_index, int pixel_index) {
    return (int)(image->channels[channel_index]->data[pixel_index] * 255.0f);
}

void integer_box_downsample(Image_Buffer *source, Image_Buffer *dest) {
    int j;
    for (j = 0; j < dest->height; j++) {
        int i;
        for (i = 0; i < dest->width; i++) {
            int s0 = source->get_index(i*2, j*2);
            int s1 = source->get_index(i*2+1, j*2);
            int s2 = source->get_index(i*2, j*2+1);
            int s3 = source->get_index(i*2+1, j*2+1);

            int red = (get_integer(source, 0, s0) + get_integer(source, 0, s1) 
                       + get_integer(source, 0, s2) + get_integer(source, 0, s3)) / 4;
            int green = (get_integer(source, 1, s0) + get_integer(source, 1, s1) 
                       + get_integer(source, 1, s2) + get_integer(source, 1, s3)) / 4;
            int blue = (get_integer(source, 2, s0) + get_integer(source, 2, s1) 
                       + get_integer(source, 2, s2) + get_integer(source, 2, s3)) / 4;

            int d0 = dest->get_index(i, j);
            dest->channels[0]->data[d0] = (red / 255.0f);
            dest->channels[1]->data[d0] = (green / 255.0f);
            dest->channels[2]->data[d0] = (blue / 255.0f);
        }
    }
}

void build_mipmaps_integer_box(Image_Buffer *source_buffer,
                               Texture_Info *texture_info,
                               int filter_index,
                               int levels_to_reduce) {
    texture_info->num_mipmap_levels = levels_to_reduce + 1;
    
    Image_Buffer *previous = source_buffer;

    int i;
    for (i = 0; i < levels_to_reduce; i++) {
        Image_Buffer *reduced = new Image_Buffer(previous->width / 2, 
                                                 previous->height / 2);

        Texture_Mipmap_Level *mipmap_level = &texture_info->mipmap_levels[i + 1];
        mipmap_level->width = reduced->width;
        mipmap_level->height = reduced->height;

        integer_box_downsample(previous, reduced);

        if (previous != source_buffer) delete previous;
        previous = reduced;

        GLuint texture_handle;
        texture_handle = texture_from_image_buffer(previous);

        mipmap_level->mipmap_texture_handles[filter_index] = texture_handle;
    }

    if (previous != source_buffer) delete previous;
}

void build_mipmaps(Image_Buffer *source_buffer, Texture_Info *texture_info,
                   int filter_index, int levels_to_reduce) {

    Image_Buffer *previous = source_buffer;
    texture_info->num_mipmap_levels = levels_to_reduce + 1;

    Filter *filter = the_mipmap_filters[filter_index];
    assert(filter != NULL);

    int i;
    for (i = 0; i < levels_to_reduce; i++) {
        Image_Buffer *reduced = new Image_Buffer(previous->width / 2, 
                                                 previous->height / 2);

        Texture_Mipmap_Level *mipmap_level = &texture_info->mipmap_levels[i + 1];
        mipmap_level->width = reduced->width;
        mipmap_level->height = reduced->height;

        previous->downsample_into(reduced, filter);
        if (previous != source_buffer) delete previous;

        previous = reduced;

        GLuint texture_handle;
        texture_handle = texture_from_image_buffer(previous);

        mipmap_level->mipmap_texture_handles[filter_index] = texture_handle;
    }

    if (previous != source_buffer) delete previous;
}

void random_fill(Image_Buffer *buffer) {
    int i, j;
    for (j = 0; j < buffer->height; j++) {
        for (i = 0; i < buffer->width; i++) {
            int index = buffer->get_index(i, j);
            float val = (rand() % 255) / 255.0f;
            buffer->channels[0]->data[index] = val;
            buffer->channels[1]->data[index] = val;
            buffer->channels[2]->data[index] = val;
        }
    }
}

void build_mipmaps_wide(Image_Buffer *source_buffer, Texture_Info *texture_info,
                        int filter_index, int levels_to_reduce) {

    Filter other_filter(2);
    other_filter.data[0] = 0;
    other_filter.data[1] = 1;

    int i;
    for (i = 1; i <= levels_to_reduce; i++) {
        assert(i < NUM_WIDE_FILTERS);
        Filter *filter = the_wide_filters[i-1];
        
        Image_Buffer *previous = new Image_Buffer(source_buffer->width / 2,
                                                  source_buffer->height / 2);
        source_buffer->downsample_into(previous, filter);

        int j;
        for (j = 1; j < i; j++) {
            Image_Buffer *reduced = new Image_Buffer(previous->width / 2, 
                                                     previous->height / 2);
            
            previous->downsample_into(reduced, &other_filter);
            delete previous;
            previous = reduced;
        }

        Texture_Mipmap_Level *mipmap_level = &texture_info->mipmap_levels[i];

        // Width and height should have been set by prior calls
        // to build_mipmaps().  This isn't the sort of assert you
        // leave in if you extract "build_mipmaps_wide()" for a 
        // real app.  It's just here to help me know I haven't
        // screwed things up, for the current app.
        assert(mipmap_level->width == previous->width);
        assert(mipmap_level->height == previous->height);

        int texture_handle = texture_from_image_buffer(previous);
        mipmap_level->mipmap_texture_handles[filter_index] = texture_handle;
        delete previous;
    }
}

inline double sinc(double x) {
    if (x == 0.0) return 1.0;
    return sin(M_PI * x) / (M_PI * x);
}

void graph_filter(Filter *filter, float r, float g, float b) {
    float x0 = 10;
    float y0 = 768 / 2;
    float width = 1024 - 2 * x0;
    float height = 768 / 2 - 50;

    begin_line_mode(r, g, b);

    glBegin(GL_LINE_STRIP);

    float offset = -filter->nsamples / 2;

    float multiple = filter->nsamples / 64.0;

    int i;
    for (i = 0; i < filter->nsamples; i++) {
        float y = filter->data[i] * height + y0;

        float x = (i + offset + 0.5f); 
        x *= multiple;
        x = x * (width / 256.0) + x0 + width * 0.5;

        glVertex2f(x, y);
    } 

    glEnd();

    end_line_mode();
}

double bessel0(double x) {
    const double EPSILON_RATIO = 1E-16;
    double xh, sum, pow, ds;

    xh = 0.5 * x;
    sum = 1.0;
    pow = 1.0;
    ds = 1.0;

    int k = 0;
    while (ds > sum * EPSILON_RATIO) {
        k++;
        pow *= (xh / k);
        ds = pow * pow;
        sum = sum + ds;
    }

    return sum;
}

inline double kaiser(double alpha, double half_width, double x) {
    double ratio = (x / half_width);
    return bessel0(alpha * sqrt(1 - ratio * ratio)) / bessel0(alpha);
}

void fill_kaiser_filter(Filter *filter, double alpha, double additional_stretch = 1) {
    int width = filter->nsamples;
    assert(!(width & 1));

    float half_width = (float)(width / 2);
    float offset = -half_width;
    float nudge = 0.5f;

    float STRETCH = additional_stretch;

    int i;
    for (i = 0; i < width; i++) {
	    float x = (i + offset) + nudge;

	    double sinc_value = sinc(x * STRETCH);
        double window_value = kaiser(alpha, half_width, x * STRETCH);

	    filter->data[i] = sinc_value * window_value;
	} 

    filter->normalize();
}

void fill_lanczos_filter(Filter *filter, double additional_stretch = 1,
                         float EXTRA1 = 1, float EXTRA2 = 1) {
    int width = filter->nsamples;
    assert(!(width & 1));

    float half_width = (float)(width / 2);
    float offset = -half_width;
    float nudge = 0.5f;

    float STRETCH = additional_stretch * 0.5;

    int i;
    for (i = 0; i < width; i++) {
	    float x = (i + offset) + nudge;

	    double sinc_value = sinc(x * STRETCH * EXTRA1);
        double window_value = sinc((x / half_width) * STRETCH * EXTRA2);

	    filter->data[i] = sinc_value * window_value;
	} 

    filter->normalize();
}

void init_filters() {
    Filter *point_filter = new Filter(2);
    point_filter->data[0] = 1;
    point_filter->data[1] = 0;
    point_filter->normalize();

    Filter *box_filter = new Filter(2);
    box_filter->data[0] = 1;
    box_filter->data[1] = 1;
    box_filter->normalize();

    Filter *gaussian_121_filter = new Filter(3);
    gaussian_121_filter->data[0] = 1;
    gaussian_121_filter->data[1] = 2;
    gaussian_121_filter->data[2] = 1;
    gaussian_121_filter->normalize();

    Filter *lanczos_filter = new Filter(GOOD_FILTER_WIDTH);
    fill_lanczos_filter(lanczos_filter);

    Filter *kaiser_filter = new Filter(GOOD_FILTER_WIDTH);
    fill_kaiser_filter(kaiser_filter, KAISER_ALPHA, 0.5);

    Filter *bad_ringing_filter = new Filter(BAD_RINGING_FILTER_WIDTH);
    fill_kaiser_filter(kaiser_filter, KAISER_ALPHA, 0.5);

    fill_kaiser_filter(bad_ringing_filter, KAISER_ALPHA, 0.5);

    the_mipmap_filters[FILTER_POINT] = point_filter;
    the_mipmap_filters[FILTER_INTEGER_BOX] = NULL;
    the_mipmap_filters[FILTER_BOX] = box_filter;
    the_mipmap_filters[FILTER_LANCZOS] = lanczos_filter;
    the_mipmap_filters[FILTER_KAISER] = kaiser_filter;
    the_mipmap_filters[FILTER_BAD_RINGING] = bad_ringing_filter;

    // We'd better not try to use this one...
    the_mipmap_filters[FILTER_STARTING_FROM_TOP] = NULL;
    
    // Initialize the wide filters.
    int i;
    for (i = 0; i < NUM_WIDE_FILTERS; i++) {
        int levels_to_reduce = i + 1;
        int filter_width = GOOD_FILTER_WIDTH * (1 << (levels_to_reduce - 1));

        if (GOOD_FILTER_WIDTH & 1) {
            if (!(filter_width & 1)) filter_width++;
        }

        Filter *filter = new Filter(filter_width);

//        float E = 1.03;
        float E = 1.00;
//        float extra_scale = pow(0.5, levels_to_reduce - 1);
//        fill_lanczos_filter(filter, extra_scale, E, 1);
        float extra_scale = pow(0.5, levels_to_reduce);
        fill_kaiser_filter(filter, KAISER_ALPHA, extra_scale);
        the_wide_filters[i] = filter;
    }
}

void attempt_texture_load(char *name) {
    GLuint texture_handle;
    int width, height;
    Image_Buffer *image_buffer_main;

    bool success = texture_from_file(name, &texture_handle, &width, &height,
                               &image_buffer_main);

    if (success && (num_loaded_textures < MAX_TEXTURES)) {
        Texture_Info *texture_info = &the_texture_info[num_loaded_textures];
        texture_info->name = name;
        texture_info->mipmap_levels[0].texture_handle = texture_handle;
        texture_info->mipmap_levels[0].width = width;
        texture_info->mipmap_levels[0].height = height;

        build_mipmaps_integer_box(image_buffer_main, texture_info,
                                  FILTER_INTEGER_BOX, LEVELS_TO_REDUCE);

        build_mipmaps(image_buffer_main, texture_info, 
                      FILTER_POINT, LEVELS_TO_REDUCE);
        
        build_mipmaps(image_buffer_main, texture_info,
                      FILTER_BOX, LEVELS_TO_REDUCE);

        build_mipmaps(image_buffer_main, texture_info,
                      FILTER_LANCZOS, LEVELS_TO_REDUCE);

        build_mipmaps(image_buffer_main, texture_info,
                      FILTER_KAISER, LEVELS_TO_REDUCE);

        build_mipmaps(image_buffer_main, texture_info,
                      FILTER_BAD_RINGING, LEVELS_TO_REDUCE);

        build_mipmaps_wide(image_buffer_main, texture_info,
                           FILTER_STARTING_FROM_TOP, LEVELS_TO_REDUCE);

        num_loaded_textures++;
    }
}

void init_textures() {
    int i;
    for (i = 0; i < NUM_TEXTURES; i++) {
        char *name = texture_names[i];
        attempt_texture_load(name);
    }

    assert(num_loaded_textures > 0);
}

void init_scene() {
    int width, height;
    bool success = texture_from_file("font.jpg", &font_handle, &width, &height);
    assert(success);

    init_filters();
    init_textures();
}

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

void draw_scene() {
    static bool initted = false;
    if (!initted) init_scene();
    initted = true;

    if (graph_mode_active) {
        draw_graphs();
    } else if (fillscreen_mode_active) {
        draw_fillscreen_mode(current_texture_index);
    } else {
        draw_texture_quads(current_texture_index);
    }

//    graph_frequency_response(the_mipmap_filters[0], 1, 0, 0);

/*
    graph_filter(the_wide_filters[0], 1, 0, 0);
    graph_filter(the_wide_filters[1], 0, 1, 0);
    graph_filter(the_wide_filters[2], 0, 0, 1);
    graph_filter(the_wide_filters[3], 0, 1, 1);
*/
}

void handle_keydown(int key) {
    if (key == '@') {
        current_texture_index++;
        if (current_texture_index >= num_loaded_textures) current_texture_index = 0;
    } else if (key == '!') {
        current_texture_index--;
        if (current_texture_index < 0) current_texture_index = num_loaded_textures - 1;
    } else if (key == '#') {
        fillscreen_miplevel--;
        if (fillscreen_miplevel < 1) fillscreen_miplevel = 1;
    } else if (key == '$') {
        fillscreen_miplevel++;
        if (fillscreen_miplevel > 9) fillscreen_miplevel = 9;
    } else if (key == 'F') {
        fillscreen_mode_active = !fillscreen_mode_active;
        fillscreen_use_box_filter = true;
    } else if (key == ' ') {
        fillscreen_use_box_filter = !fillscreen_use_box_filter;
    } else if (key == 'G') {
        graph_mode_active = !graph_mode_active;
    }
}
