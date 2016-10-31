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
#include "fourier.h"

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;

GLuint font_handle = 0;
extern int g_ScreenWidth;
extern int g_ScreenHeight;

const double GAMMA = 2.0;
bool graph_mode_active = false;
bool fillscreen_mode_active = false;
int fillscreen_filter = 0;
int fillscreen_miplevel = 3;

const float KAISER_ALPHA = 4.0;
const int LEVELS_TO_REDUCE = 3;

enum Filter_Types {
    FILTER_INTEGER_BOX,
    FILTER_STARTING_FROM_TOP,
    FILTER_DFT,
    FILTER_LIGHT_LINEAR,
    NUM_MIPMAP_FILTER_TYPES
};

const int GOOD_FILTER_WIDTH = 10;

char *the_filter_names[NUM_MIPMAP_FILTER_TYPES] = {
    "integer box",
    "kaiser",
    "full DFT",
    "linear-light kaiser"
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

char *spot_texture_name = "white_dot.jpg";
GLuint spot_texture_handle = -1;

char *texture_names[] = {
    "ChoirComm01_512x256.jpg",
    "dtr3_ss_phoenix1mile.jpg",
    "bright.jpg",
    "trukblower2.jpg",
    "CBBB_New04_128x256.jpg",
    "Hotel_SuomiVodka02_256x256.jpg",
    "CitySewers01ab.jpg"
    "Van02_512x128.jpg"
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


const float FR_WIDTH = 800;
const float FR_X0 = 100;

void draw_spot_at(float x, float y, float radius) {
    float x0 = x - radius;
    float y0 = y - radius;
    draw_texture_quad(spot_texture_handle, x0, y0, 2 * radius, 2 * radius);
}

void graph_point_sampled_texture() {
    const int DIMENSIONS = 8;

    float x0 = FR_X0;
    float y0 = FR_X0;
    float height = 768 / 2 - 50;
    float width = height;

    float x1 = x0 + width;
    float y1 = y0 + height;

    glColor3f(.35, .35, .35);

    int i, j;
    for (j = 0; j < DIMENSIONS; j++) {
        float y = y0 + (j / (double)(DIMENSIONS - 1)) * (y1 - y0);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y);
        glVertex2f(x1, y);
        glEnd();
    }

    for (i = 0; i < DIMENSIONS; i++) {
        float x = x0 + (i / (double)(DIMENSIONS - 1)) * (x1 - x0);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y0);
        glVertex2f(x, y1);
        glEnd();
    }

    for (j = 0; j < DIMENSIONS; j++) {
        for (i = 0; i < DIMENSIONS; i++) {
            if ((i == DIMENSIONS - 3) && (j == 4)) {
                glColor3f(1, 1, 1);
            } else {
                glColor3f(3 / 255.0, 97 / 255.0, 15 / 255.0);
            }

            float x = x0 + (i / (double)(DIMENSIONS - 1)) * (x1 - x0);
            float y = y0 + (j / (double)(DIMENSIONS - 1)) * (y1 - y0);

            double radius = (width / DIMENSIONS) * 0.3;
            draw_spot_at(x, y, radius);
        }
    }
}

void fft_upsample(float *src, float *dest, int width, int index) {
    int fft_width = width * 2;
    int offset = 0;
    if (index != 0) offset = 1;

    float *real_in = new float[fft_width];
    float *imag_in = new float[fft_width];
    float *real_out = new float[fft_width];
    float *imag_out = new float[fft_width];

    int i;
    for (i = 0; i < fft_width; i++) {
        real_in[i] = imag_in[i] = real_out[i] = imag_out[i] = 0;
    }

    for (i = 0; i < width; i++) {
        real_in[i * 2 + offset] = src[i] * 2;
    }

    fft_float(fft_width, 0, real_in, imag_in, real_out, imag_out);
    
    // Zero out some coefficinets of this sucker to do the appropriate
    // low-passing.

    int BAND_SIZE = fft_width / 4;
    for (i = 0; i < fft_width; i++) {
        if (i < BAND_SIZE) continue;
        if (i >= fft_width - BAND_SIZE) continue;
        real_out[i] = 0;
        imag_out[i] = 0;
    }

    fft_float(fft_width, 1, real_out, imag_out, dest, imag_in);

    delete [] real_in;
    delete [] real_out;
    delete [] imag_in;
    delete [] imag_out;
}

void graph_cross_section() {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    float source_data[8];
    source_data[0] = 40 / 255.0f;
    source_data[1] = source_data[0];
    source_data[2] = source_data[0];
    source_data[3] = source_data[0];
    source_data[4] = source_data[0];
    source_data[5] = 1.0f;
    source_data[6] = source_data[0];
    source_data[7] = source_data[0];

    const float YSCALE = 0.5f;

    static float *upsampled_data = NULL;
    static int upsampled_len = 0;
    if (upsampled_data == NULL) {
        int width = 8;
        float *cursor = source_data;

        int i;
        for (i = 0; i < 10; i++) {
            float *next = new float[width * 2];
            fft_upsample(cursor, next, width, i);

            if (cursor != source_data) delete [] cursor;
            cursor = next;
            width *= 2;
        }

        upsampled_data = cursor;
        upsampled_len = width;
    }

    // Draw the function graph.

    glColor3f(0, 1, 1);
    int i;

    glBegin(GL_LINE_STRIP);
    for (i = 0; i < width; i++) {
        float perc = i / (double)width;
        int index = (int)(perc * upsampled_len);

        double magnitude = upsampled_data[index] * YSCALE;

        double x = (i / width) * width + x0;
        double y = 768 - ((magnitude * height) + y0);

        glVertex2f(x, y);
    }

    glEnd();

    // Draw the dots.
/*
    const int DIMENSIONS = 8;
    for (i = 0; i < DIMENSIONS; i++) {
        float perc = i / (double)(DIMENSIONS - 1);
        int index = upsampled_len * perc;
        if (index >= upsampled_len) index = upsampled_len - 1;

//        float magnitude = upsampled_data[index];
        float magnitude = source_data[i];

        float y = 768 - (y0 + YSCALE * height * magnitude);
        float x = x0 + width * (i / (double)(DIMENSIONS - 1));

        if (i == DIMENSIONS - 3) {
            glColor3f(1, 1, 1);
        } else {
            glColor3f(3 / 255.0, 97 / 255.0, 15 / 255.0);
        }

        double radius = (width / DIMENSIONS) * 0.15;
        draw_spot_at(x, y, radius);
    }
*/
}

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

double compute_content_magnitude(double omega, double alpha, double N) {
    double magnitude = 0.5f * ((sin(omega - alpha) / (omega - alpha)) + sin(omega + alpha) / (omega + alpha));
    return magnitude;
/*
    double real_part1 = 0, imag_part1 = 0;

    // First part of integrand

    double arg;
    arg = (omega - alpha) * 2 * M_PI;
    real_part1 += cos(arg);
    imag_part1 += sin(arg);

    arg = 0;
    real_part1 -= cos(arg);
    imag_part1 -= sin(arg);

    real_part1 *= 1.0 / (omega - alpha);
    imag_part1 *= 1.0 / (omega - alpha);

    double real_part2 = 0, imag_part2 = 0;

    // Second part of integrand

    arg = -(omega + alpha) * 2 * M_PI;
    real_part2 += cos(arg);
    imag_part2 += sin(arg);

    arg = 0;
    real_part2 -= cos(arg);
    imag_part2 -= sin(arg);

    real_part2 *= -1.0 / (omega + alpha);
    imag_part2 *= -1.0 / (omega + alpha);

    // Put them together.

    double real_part, imag_part;
    real_part = real_part1 + real_part2;
    imag_part = imag_part1 + imag_part2;

    double magnitude = sqrt(real_part * real_part + imag_part * imag_part);

    double factor = 1.0 / (4 * M_PI);
    magnitude *= factor;

    return magnitude;
*/
}

void graph_rippled_cosine_wave(double N, double OMEGA) {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    double XSCALE = 16.0;
    double YSCALE = 0.25;

    const int FFT_WIDTH = 256;
    assert(FFT_WIDTH == N);
    static float magnitudes[FFT_WIDTH];

    static bool initted = false;
    if (!initted) {
        initted = true;

        int i;
        for (i = 0; i < FFT_WIDTH; i++) {
            double t = (i / (double)FFT_WIDTH);

            double magnitude = cos(OMEGA * 2 * M_PI * t);
            magnitudes[i] = magnitude;
        }

        float imag_in[FFT_WIDTH];
        float real_out[FFT_WIDTH];
        float imag_out[FFT_WIDTH];

        for (i = 0; i < FFT_WIDTH; i++) {
            imag_in[i] = real_out[i] = imag_out[i] = 0;
        }

        fft_float(FFT_WIDTH, 0, magnitudes, imag_in, real_out, imag_out);

        // Zero out some coefficinets of this sucker to do the appropriate
        // low-passing.  Because we have oversampled our data in order
        // to graph it, we chop out more than half the frequencies.

        int BAND_SIZE = FFT_WIDTH / 4;

        for (i = 0; i < FFT_WIDTH; i++) {
            if (i < BAND_SIZE) continue;
            if (i >= FFT_WIDTH - BAND_SIZE) continue;
            real_out[i] = 0;
            imag_out[i] = 0;
        }

        fft_float(FFT_WIDTH, 1, real_out, imag_out, magnitudes, imag_in);
    }


    glBegin(GL_LINE_STRIP);
    int i;
    for (i = 0; i < width; i++) {
        double t = (i / (double)width);
        t *= (1.0 / XSCALE);

        int index = FFT_WIDTH * t;
        double magnitude = YSCALE * magnitudes[index];

        double x = ((i / width) * width + x0);
        double y = 768 - ((magnitude * height) + y0);

        glVertex2f(x, y);
    }
    glEnd();

    glColor3f(1, 1, 1);

    double delta = 2 * (1.0 / N) * XSCALE * width;
    double sample_pip_height = height * 0.1f;
    double next_sample_pos = x0;

    for (i = 0; i < width; i++) {
        double t = (i / (double)width);
        t *= (1.0 / XSCALE);

        double x = ((i / width) * width + x0);
        double y = 768 - y0;

        if (x < next_sample_pos) continue;
        next_sample_pos += delta;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y - sample_pip_height);
        glVertex2f(x, y + sample_pip_height);
        glEnd();
    }
}

void graph_cosine_wave(double N, double OMEGA) {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    double XSCALE = 16.0;
    double YSCALE = 0.25;

    glBegin(GL_LINE_STRIP);

    int i;
    for (i = 0; i < width; i++) {
        double t = (i / (double)width);
        t *= (1.0 / XSCALE);

        double magnitude = cos(OMEGA * 2 * M_PI * t) * YSCALE;

        double x = ((i / width) * width + x0);
        double y = 768 - ((magnitude * height) + y0);

        glVertex2f(x, y);
    }

    glEnd();

    glColor3f(1, 1, 1);

    double delta = 2 * (1.0 / N) * XSCALE * width;
    double sample_pip_height = height * 0.1f;
    double next_sample_pos = x0;

    for (i = 0; i < width; i++) {
        double t = (i / (double)width);
        t *= (1.0 / XSCALE);

        double x = ((i / width) * width + x0);
        double y = 768 - y0;

        if (x < next_sample_pos) continue;
        next_sample_pos += delta;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x, y - sample_pip_height);
        glVertex2f(x, y + sample_pip_height);
        glEnd();
    }
}

void graph_cosine_frequency_content(double N, double OMEGA) {
    float x0 = FR_X0;
    float y0 = 768 / 2;
    float width = FR_WIDTH - 2 * x0;
    float height = 768 / 2 - 50;

    glBegin(GL_LINE_STRIP);

    int iwidth = (int)width;

    const double YSCALE = 1;
    const double XSCALE = 1;

    int i;
    for (i = 0; i < width; i++) {
        double alpha = (i / (double)width) * (N * 0.5f);

        double magnitude = YSCALE * compute_content_magnitude(OMEGA, alpha, N);

        double x = XSCALE * ((i / width) * width + x0);
        double y = 768 - ((magnitude * height) + y0);

        glVertex2f(x, y);
    }

    glEnd();
}

void draw_graphs() {
    const double N = 256;
    const double OMEGA = 57.5;


    begin_line_mode(1, 1, 1);
    
/*
    // Code to draw figure 3.
    graph_point_sampled_texture();
*/

    // Code to draw figure 4.
    glColor3f(0.5, 0.5, 0.5);
    graph_function_axes();
//    graph_cross_section();

/*
    // Code to draw figure 6.
    glLineWidth(3);
    glColor3f(1, 1, 1);
    graph_frequency_response_axes();

    glLineWidth(3);
    glColor3f(0.7, 0.2, 0.3);
    graph_frequency_response_ideal();
    glColor3f(1, 1, 0);
    graph_cosine_frequency_content(N, OMEGA);
    glLineWidth(1);
    glColor3f(1, 1, 1);
*/


    // Code to draw figure 5.
    glLineWidth(3);
    glColor3f(0.5, 0.5, 0.5);
    graph_function_axes();

    glLineWidth(3);
    glColor3f(0, 0.8, 0.3);
    graph_cosine_wave(N, OMEGA);
    glLineWidth(1);
    glColor3f(1, 1, 1);

}



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

    int handle_index = fillscreen_filter;

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

inline void rgb_to_xyz(float r, float g, float b,
                       float *x_result, float *y_result, float *z_result) {

    // These variables m?? are the coefficients of the matrix
    // that transforms RGB space into XYZ space.

    const float m00 = 0.412453;
    const float m01 = 0.357580;
    const float m02 = 0.180423;
    const float m10 = 0.212671;
    const float m11 = 0.715160;
    const float m12 = 0.072169;
    const float m20 = 0.019334;
    const float m21 = 0.119193;
    const float m22 = 0.950227;

    *x_result = m00 * r + m01 * g + m02 * b;
    *y_result = m10 * r + m11 * g + m12 * b;
    *z_result = m20 * r + m21 * g + m22 * b;
}

inline void xyz_to_rgb(float x, float y, float z,
                       float *r_result, float *g_result, float *b_result) {

    // These variables m?? are the inverse of the matrix
    // given in rgb_to_xyz.

    const float m00 = 3.240481;
    const float m01 =-1.537151;
    const float m02 =-0.498536;
    const float m10 =-0.969255;
    const float m11 = 1.875990;
    const float m12 = 0.041556;
    const float m20 = 0.055647;
    const float m21 =-0.204041;
    const float m22 = 1.053731;

    *r_result = m00 * x + m01 * y + m02 * z;
    *g_result = m10 * x + m11 * y + m12 * z;
    *b_result = m20 * x + m21 * y + m22 * z;
}

inline void xyz_to_lab(float x, float y, float z,
                       float *l_result, float *a_result, float *b_result) {
    const float x_white = (0.950456);
    const float y_white = (1.0);
    const float z_white = (1.08875);
    
    const double power = 1.0 / 3.0;

    float x_ratio = pow((x / x_white), power);
    float y_ratio = pow((y / y_white), power);
    float z_ratio = pow((z / z_white), power);

    *l_result = 116.0f * y_ratio - 16.0f;
    *a_result = 500.0f * (x_ratio - y_ratio);
    *b_result = 200.0f * (y_ratio - z_ratio);
}

inline void lab_to_xyz(float l, float a, float b,
                       float *x_result, float *y_result, float *z_result) {
    const float x_white = (0.950456);
    const float y_white = (1.0);
    const float z_white = (1.08875);

    float y_ratio = (l + 16.0f) / 116.0f;
    *y_result = y_white * y_ratio * y_ratio * y_ratio;

    float x_ratio = (1.0 / 500.0) * a + y_ratio;
    *x_result = x_white * x_ratio * x_ratio * x_ratio;

    float z_ratio = y_ratio - (1.0 / 200.0) * b;
    *z_result = z_white * z_ratio * z_ratio * z_ratio;
}

void rgb_to_lab(Image_Buffer *buffer) {
    float r, g, b;
    float x, y, z;
    float _l, _a, _b;

    int i, j;
    for (j = 0; j < buffer->height; j++) {
        for (i = 0; i < buffer->width; i++) {
            int index = buffer->get_index(i, j);

            r = buffer->channels[0]->data[index];
            g = buffer->channels[1]->data[index];
            b = buffer->channels[2]->data[index];

            rgb_to_xyz(r, g, b, &x, &y, &z);
            xyz_to_lab(x, y, z, &_l, &_a, &_b);

/*
            _l = (rand() % 255) / 255.0;
            _a = (rand() % 255) / 255.0;
            _b = (rand() % 255) / 255.0;
*/

            buffer->channels[0]->data[index] = _l;
            buffer->channels[1]->data[index] = _a;
            buffer->channels[2]->data[index] = _b;
        }
    }

}

void lab_to_rgb(Image_Buffer *buffer) {
    float _l, _a, _b;
    float x, y, z;
    float r, g, b;

    int i, j;
    for (j = 0; j < buffer->height; j++) {
        for (i = 0; i < buffer->width; i++) {
            int index = buffer->get_index(i, j);

            _l = buffer->channels[0]->data[index];
            _a = buffer->channels[1]->data[index];
            _b = buffer->channels[2]->data[index];

            lab_to_xyz(_l, _a, _b, &x, &y, &z);
            xyz_to_rgb(x, y, z, &r, &g, &b);
//            xyz_to_rgb(_l, _a, _b, &r, &g, &b);

            buffer->channels[0]->data[index] = r;
            buffer->channels[1]->data[index] = g;
            buffer->channels[2]->data[index] = b;
        }
    }

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

void build_mipmaps_full_dft(Image_Buffer *source_buffer,
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

        previous->downsample_into_fft(reduced);

        if (previous != source_buffer) delete previous;
        previous = reduced;

        GLuint texture_handle;
        texture_handle = texture_from_image_buffer(previous);

        mipmap_level->mipmap_texture_handles[filter_index] = texture_handle;
    }

    if (previous != source_buffer) delete previous;
}

void build_mipmaps(Image_Buffer *source_buffer, Texture_Info *texture_info,
                   int filter_index, int levels_to_reduce,
                   bool do_gamma = false, bool do_cie = false) {

    Image_Buffer *previous = source_buffer;

    if (do_gamma) {
        previous = source_buffer->copy();
        previous->exponentiate(GAMMA);
    }

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
        if (do_gamma) {
            Image_Buffer *tmp = reduced->copy();
            tmp->clamp(0, 100);
            tmp->exponentiate(1.0 / GAMMA);

            texture_handle = texture_from_image_buffer(tmp);
            delete tmp;
        } else {
            texture_handle = texture_from_image_buffer(previous);
        }

        mipmap_level->mipmap_texture_handles[filter_index] = texture_handle;
    }

    if (previous != source_buffer) delete previous;
}

void build_mipmaps_wide(Image_Buffer *source_buffer, Texture_Info *texture_info,
                        int filter_index, int levels_to_reduce, Filter **filters, bool do_gamma = false) {

    Filter other_filter(2);
    other_filter.data[0] = 0;
    other_filter.data[1] = 1;

    double ramped_to_linear_light = GAMMA;
    double linear_light_to_ramped = 1.0 / GAMMA;

    if (do_gamma) {
        source_buffer = source_buffer->copy();
        source_buffer->exponentiate(ramped_to_linear_light);
    }

    int i;
    for (i = 1; i <= levels_to_reduce; i++) {
        assert(i < NUM_WIDE_FILTERS);
        Filter *filter = the_wide_filters[i-1];
        
        Image_Buffer *smaller_buffer = new Image_Buffer(source_buffer->width / 2,
                                                        source_buffer->height / 2);
        source_buffer->downsample_into(smaller_buffer, filter);

        if (do_gamma) {
            smaller_buffer->clamp(0, 100);
            smaller_buffer->exponentiate(linear_light_to_ramped);
        }

        Image_Buffer *previous = smaller_buffer;

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

    if (do_gamma) delete source_buffer;
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



inline float
sincf(float x)
{
    x *= F_PI;
    if (x < 0.01f && x > -0.01f)
        return 1.0f + x*x*(-1.0f/6.0f + x*x*1.0f/120.0f);
    else
        return sinf(x)/x;
}

inline float
Lanczos4SincFilter(float x)
{
    if (x <= -4.0f || x >= 4.0f)    // half-width of 4
        return 0.0;
    else
        return sincf(0.875f * x) * sincf(0.25f * x);
}

inline float
Lanczos8SincFilter(float x)
{
    if (x <= -8.0f || x >= 8.0f)    // half-width of 8
        return 0.0;
    else
        return sincf(0.5 * 0.9475f * x) * sincf(0.5 * 0.125f * x);
}

double
bessel0(double x) {
  const double EPSILON_RATIO = 1E-16;
  double xh, sum, pow, ds;
  int k;

  xh = 0.5 * x;
  sum = 1.0;
  pow = 1.0;
  k = 0;
  ds = 1.0;
  while (ds > sum * EPSILON_RATIO) {
    ++k;
    pow = pow * (xh / k);
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
    the_mipmap_filters[FILTER_INTEGER_BOX] = NULL;

    // We'd better not try to use this one...
    the_mipmap_filters[FILTER_STARTING_FROM_TOP] = NULL;
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
        Filter *light_linear_filter = new Filter(filter_width);

//        float E = 1.03;
        float E = 1.00;
//        float extra_scale = pow(0.5, levels_to_reduce - 1);
//        fill_lanczos_filter(filter, extra_scale, E, 1);
        float extra_scale = pow(0.5, levels_to_reduce);
        fill_kaiser_filter(filter, KAISER_ALPHA, extra_scale);
        fill_kaiser_filter(light_linear_filter, KAISER_ALPHA, extra_scale);

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

        build_mipmaps_full_dft(image_buffer_main, texture_info,
                                  FILTER_DFT, LEVELS_TO_REDUCE);

        build_mipmaps_wide(image_buffer_main, texture_info,
                           FILTER_STARTING_FROM_TOP, LEVELS_TO_REDUCE, the_wide_filters);
        build_mipmaps_wide(image_buffer_main, texture_info,
                           FILTER_LIGHT_LINEAR, LEVELS_TO_REDUCE, the_wide_filters, true);

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

    char *name = spot_texture_name;
    int width, height;
    bool success = texture_from_file(name, &spot_texture_handle,
                                     &width, &height,
                                     NULL);
    assert(success);
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


*/
/*
    graph_filter(the_wide_filters[0], 1, 0, 0);
    graph_filter(the_wide_filters[1], 0, 1, 0);
    graph_filter(the_wide_filters[2], 0, 0, 1);
    graph_filter(the_wide_filters[3], 0, 1, 1);
*/
    begin_text_mode();
    int x, y;
    x = 100;
    y = 100;
//    draw_line(&x, &y, "Hello, Sailor!");
    end_text_mode();

    
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
        fillscreen_filter = FILTER_INTEGER_BOX;
    } else if (key == '1') {
        fillscreen_filter = FILTER_INTEGER_BOX;
    } else if (key == '2') {
        fillscreen_filter = FILTER_STARTING_FROM_TOP;
    } else if (key == '3') {
        fillscreen_filter = FILTER_DFT;
    } else if (key == '4') {
        fillscreen_filter = FILTER_LIGHT_LINEAR;
    } else if (key == ' ') {
        if (fillscreen_filter == FILTER_INTEGER_BOX) {
            fillscreen_filter = FILTER_LIGHT_LINEAR;
        } else {
            fillscreen_filter = FILTER_INTEGER_BOX;
        }
    } else if (key == 'G') {
        graph_mode_active = !graph_mode_active;
    }
}
