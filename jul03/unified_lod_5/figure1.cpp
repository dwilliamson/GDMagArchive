#include "framework.h"
#include "app_shell/os_specific_opengl_headers.h"
#include "covariance.h"

#include <math.h>

char *app_name = "Figure 1";

const int MAX_POINTS = 100;
Vector2 barycenters[MAX_POINTS];
int num_barycenters = 0;

struct Covariance_Body2 {
    Covariance2 covariance;
    Vector2 mean;
    float mass;

    void reset();
    void accumulate(float x, float y, float mass);
    void normalize();
};

const float NUM_STANDARD_DEVIATIONS = 1.7f;

// Everything this function does is 2-dimensional; I am just using
// a Vector3 for the vector operations to avoid having to do all kinds
// of code-writing for the Vector2 class.  The 'z' coordinate of these
// 3-vectors is always 0.
void draw_one_ellipse(Covariance_Body2 *body) {
    Vector2 axis[2];
    float lambda[2];

    body->covariance.find_eigenvectors(lambda, axis);
    
    float len0 = sqrt(lambda[0]) * NUM_STANDARD_DEVIATIONS;
    float len1 = sqrt(lambda[1]) * NUM_STANDARD_DEVIATIONS;

    Vector2 axis0 = axis[0] * len0;
    Vector2 axis1 = axis[1] * len1;

    app_shell->triangle_mode_begin();

    const int NUM_VERTICES = 300;
    static Vector2 vertices[NUM_VERTICES];

    // Generate the vertex coordinates for the ellipse.

    int j;
    for (j = 0; j < NUM_VERTICES; j++) {
        double theta = 2 * M_PI * (j / (double)NUM_VERTICES);
        double ct = cos(theta);
        double st = sin(theta);

        Vector2 a0 = axis0 * ct;
        Vector2 a1 = axis1 * st;

        Vector2 pos = a0 + a1 + body->mean;
        
        vertices[j] = pos;
    }

    // Draw the ellipse.

    glBegin(GL_TRIANGLES);
    for (j = 0; j < NUM_VERTICES; j++) {
        int n0 = j;
        int n1 = (j + 1) % NUM_VERTICES;
        glVertex2f(body->mean.x, body->mean.y);
        glVertex2f(vertices[n0].x, vertices[n0].y);
        glVertex2f(vertices[n1].x, vertices[n1].y);
    }
    glEnd();

    app_shell->triangle_mode_end();
}


void Covariance_Body2::reset() {
    mass = 0;
    mean.x = 0;
    mean.y = 0;
    covariance.reset();
}

void Covariance_Body2::accumulate(float x, float y, float point_mass) {
    mass += point_mass;

    float cx = x * point_mass;
    float cy = y * point_mass;
    covariance.a += cx * cx;
    covariance.b += cx * cy;
    covariance.c += cy * cy;
}

void Covariance_Body2::normalize() {
    if (mass == 0) return;

    float imass = 1.0f / mass;
    covariance.scale(imass);
    mass = 1.0f;
}

void compute_covariance_body(Covariance_Body2 *body) {
    // First start with an empty body.
    body->reset();

    if (num_barycenters == 0) return;

    // Now add all the input points.  You can actually compute the covariance
    // matrix in one pass (you don't need to compute the mean separately),
    // but we do it in two passes here for clarity.

    float sum_x = 0;
    float sum_y = 0;
    int i;
    for (i = 0; i < num_barycenters; i++) {
        sum_x += barycenters[i].x;
        sum_y += barycenters[i].y;
    }

    float mean_x = sum_x / num_barycenters;
    float mean_y = sum_y / num_barycenters;

    body->mean.x = mean_x;
    body->mean.y = mean_y;

    for (i = 0; i < num_barycenters; i++) {
        float x = barycenters[i].x - mean_x;
        float y = barycenters[i].y - mean_y;
        float mass = 1;  // For this example, all bodies have the same mass.

        body->accumulate(x, y, mass);
    }

    body->normalize();
}

void do_triangle(Vector2 p0, Vector2 p1, Vector2 p2) {
    p0.x *= app_shell->screen_width;
    p0.y *= app_shell->screen_height;
    p1.x *= app_shell->screen_width;
    p1.y *= app_shell->screen_height;
    p2.x *= app_shell->screen_width;
    p2.y *= app_shell->screen_height;

    glBegin(GL_LINES);
    glVertex2fv((float *)&p0);
    glVertex2fv((float *)&p1);

    glVertex2fv((float *)&p1);
    glVertex2fv((float *)&p2);

    glVertex2fv((float *)&p2);
    glVertex2fv((float *)&p0);

    glEnd();

    Vector2 mean = (p0 + p1 + p2) * 0.3333333f;
    if (num_barycenters < MAX_POINTS) {
        barycenters[num_barycenters++] = mean;
    }
}

void draw_scene() {
    Vector2 p0(0.06f, 0.23f);
    Vector2 p1(0.16f, 0.23f);
    Vector2 p2(0.26f, 0.165f);
    Vector2 p3(0.57f, 0.27f);
    Vector2 p4(0.6f, 0.16f);
    Vector2 p5(0.85f, 0.45f);
    Vector2 p6(0.96f, 0.80f);
    Vector2 p7(0.78f, 0.80f);
    Vector2 p8(0.58f, 0.93f);
    Vector2 p9(0.3f, 0.72f);
    Vector2 p10(0.205f, 0.685f);
    Vector2 p11(0.25f, 0.42f);
    Vector2 p12(0.65f, 0.47f);

    // Start doing a bunch of triangles...
    glColor3f(1, 1, 0);
    glLineWidth(3);
    app_shell->line_mode_begin();

    do_triangle(p0, p1, p11);
    do_triangle(p1, p2, p11);
    do_triangle(p2, p3, p11);
    do_triangle(p3, p4, p12);
    do_triangle(p4, p5, p12);
    do_triangle(p11, p3, p12);

    do_triangle(p10, p11, p9);
    do_triangle(p9, p11, p12);
    do_triangle(p9, p12, p5);
    do_triangle(p9, p5, p7);
    do_triangle(p7, p5, p6);
    do_triangle(p8, p9, p7);

    do_triangle(p10, p0, p11);

    app_shell->line_mode_end();


    app_shell->triangle_mode_begin();
    glColor3f(1, 1, 1);

    int i;
    for (i = 0; i < num_barycenters; i++) {
        float x0, x1, y0, y1;
        float s = 7;
        const int NUM_SAMPLES = 30;

        Vector2 center = barycenters[i];

        glBegin(GL_TRIANGLES);

        int j;
        for (j = 0; j < NUM_SAMPLES; j++) {
            float theta0 = 2 * M_PI * ((j+0) / (float)NUM_SAMPLES);
            float theta1 = 2 * M_PI * ((j+1) / (float)NUM_SAMPLES);
            Vector2 v0 = Vector2(cos(theta0), sin(theta0)) * s + center;
            Vector2 v1 = Vector2(cos(theta1), sin(theta1)) * s + center;

            glVertex2fv((float *)&center);
            glVertex2fv((float *)&v0);
            glVertex2fv((float *)&v1);
        }

        glEnd();
    }

    app_shell->triangle_mode_end();

    Covariance_Body2 body;
    compute_covariance_body(&body);
    glColor4f(0.1f, 0.1f, 0.9f, 0.4f);
    draw_one_ellipse(&body);


    // Draw the line down the middle of the ellipse...

    glColor3f(1, 0.1, 0.05);
    glLineWidth(4);
    float half_len = 0.6f * app_shell->screen_width;

    Vector2 axis[2];
    float lambda[2];
    body.covariance.find_eigenvectors(lambda, axis);

    Vector2 line_p0 = body.mean - axis[1] * half_len;
    Vector2 line_p1 = body.mean + axis[1] * half_len;

    app_shell->line_mode_begin();
    glBegin(GL_LINES);
    glVertex2fv((float *)&line_p0);
    glVertex2fv((float *)&line_p1);
    glEnd();
    app_shell->line_mode_end();

    // Sleepytime!
    app_shell->sleep(0.02f);
}

void handle_keydown(int key) {
}

void handle_keyup(int) {
}

void app_init(int argc, char **argv) {
}
