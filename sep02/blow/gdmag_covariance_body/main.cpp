#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"
#include "covariance.h"

#include <math.h>

char *app_name = "Covariance Body";

int figure_to_draw = 1;

struct Vector2 {
    float x, y;
};

struct Covariance_Body2 {
    Covariance_Matrix2 covariance;
    Vector2 mean;
    float mass;

    void reset();
    void accumulate(float x, float y, float mass);
    void normalize();
};

const int MAX_INPUT_POINTS = 200;
Vector2 input_points[MAX_INPUT_POINTS];

const int DISTRIBUTION_RESOLUTION = 500;
float distribution_values[DISTRIBUTION_RESOLUTION];

int num_input_points = 0;

void add_input_point(float x, float y) {
    if (num_input_points == MAX_INPUT_POINTS) return;
    input_points[num_input_points].x = x;
    input_points[num_input_points].y = y;
    num_input_points++;
}

void remove_input_point() {
    if (num_input_points == 0) return;
    num_input_points--;
}


double sinc(double x) {
    if (x == 0.0) return 1.0;
    return sin(M_PI * x) / (M_PI * x);
}

float lanczos_filtered_sinc(float x) {
    const float k = 2;
    double sinc_value = sinc(k*x);
    double window_value = sinc(x);
    return sinc_value * window_value;
}

float find_distribution_value(float x, float kernel_halfwidth) {
    float sum = 0;

    int i;
    for (i = 0; i < num_input_points; i++) {
        float projected_distance = fabs(input_points[i].x - x);
        float parameter = (projected_distance / kernel_halfwidth);
        if (parameter < 0) parameter = 0;
        if (parameter > 1) parameter = 1;
        
        float membership = lanczos_filtered_sinc(parameter);

        sum += membership;
    }

    return sum;
}

void draw_vertical_projection_lines() {
    app_shell->line_mode_begin();
    glEnable(GL_BLEND);
    glLineWidth(1.0f);
    glColor4f(1, 0.1f, 0.15f, 0.6f);

    glBegin(GL_LINES);
    int i;
    for (i = 0; i < num_input_points; i++) {
        float x, y;
        x = input_points[i].x;
        y = input_points[i].y;

        glVertex2f(x, y);
        glVertex2f(x, 0);
    }
    glEnd();

    app_shell->line_mode_end();
}

void draw_distribution() {
    float kernel_halfwidth = app_shell->screen_width * 0.15f;
    
    float highest_value = 0;

    float dx = app_shell->screen_width / (float)(DISTRIBUTION_RESOLUTION - 1);

    int i;
    for (i = 0; i < DISTRIBUTION_RESOLUTION; i++) {
        float x = i * dx;
        float value = find_distribution_value(x, kernel_halfwidth);
        distribution_values[i] = value;
        if (value > highest_value) highest_value = value;
    }

    if (highest_value > 0) {
        for (i = 0; i < DISTRIBUTION_RESOLUTION; i++) {
            distribution_values[i] /= highest_value;
        }
    }

    // Actually do the drawing.

    app_shell->triangle_mode_begin();
    glColor4f(0.05f, 0.7f, 0.05f, 0.55f);
    const float Y_SCALE = 0.3f * app_shell->screen_height;

    glBegin(GL_QUADS);
    for (i = 0; i < DISTRIBUTION_RESOLUTION - 1; i++) {
        float x0 = i * dx;
        float x1 = (i + 1) * dx;
        float y0 = distribution_values[i] * Y_SCALE;
        float y1 = distribution_values[i+1] * Y_SCALE;

        glVertex2f(x0, y0);
        glVertex2f(x0, 0);
        glVertex2f(x1, 0);
        glVertex2f(x1, y1);
    }
    glEnd();

    app_shell->triangle_mode_end();
}

const float NUM_STANDARD_DEVIATIONS = 1.7f;

// Everything this function does is 2-dimensional; I am just using
// a Vector3 for the vector operations to avoid having to do all kinds
// of code-writing for the Vector2 class.  The 'z' coordinate of these
// 3-vectors is always 0.
void draw_one_ellipse(Covariance_Body2 *body) {
    Vector3 axis[2];
    float lambda[2];

    body->covariance.find_eigenvectors(lambda, axis);
    
    float len0 = sqrt(lambda[0]) * NUM_STANDARD_DEVIATIONS;
    float len1 = sqrt(lambda[1]) * NUM_STANDARD_DEVIATIONS;

    Vector3 axis0 = axis[0];
    Vector3 axis1 = axis[1];

    axis0.scale(len0);
    axis1.scale(len1);

    app_shell->triangle_mode_begin();

    const int NUM_VERTICES = 300;
    static Vector3 vertices[NUM_VERTICES];

    // Generate the vertex coordinates for the ellipse.

    int j;
    for (j = 0; j < NUM_VERTICES; j++) {
        double theta = 2 * M_PI * (j / (double)NUM_VERTICES);
        double ct = cos(theta);
        double st = sin(theta);

        Vector3 a0 = axis0;
        Vector3 a1 = axis1;
        a0.scale(ct);
        a1.scale(st);

        Vector3 pos = a0.add(a1);
        pos.x += body->mean.x;
        pos.y += body->mean.y;
        
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

void draw_input_points() {
    app_shell->line_mode_begin();
    glLineWidth(1.0f);
    glColor3f(1, 1, 1);

    int i;
    for (i = 0; i < num_input_points; i++) {
        float x0, x1, y0, y1;
        float s = 4;
        x0 = input_points[i].x - s;
        x1 = input_points[i].x + s - 1;
        y0 = input_points[i].y - s;
        y1 = input_points[i].y + s - 1;

        glBegin(GL_LINES);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
        glVertex2f(x1, y0);
        glVertex2f(x0, y1);
        glEnd();
    }

    app_shell->line_mode_end();
}

void compute_covariance_body(Covariance_Body2 *body) {
    // First start with an empty body.
    body->reset();

    if (num_input_points == 0) return;

    // Now add all the input points.  You can actually compute the covariance
    // matrix in one pass (you don't need to compute the mean separately),
    // but we do it in two passes here for clarity.

    float sum_x = 0;
    float sum_y = 0;
    int i;
    for (i = 0; i < num_input_points; i++) {
        sum_x += input_points[i].x;
        sum_y += input_points[i].y;
    }

    float mean_x = sum_x / num_input_points;
    float mean_y = sum_y / num_input_points;

    body->mean.x = mean_x;
    body->mean.y = mean_y;

    for (i = 0; i < num_input_points; i++) {
        float x = input_points[i].x - mean_x;
        float y = input_points[i].y - mean_y;
        float mass = 1;  // For this example, all bodies have the same mass.

        body->accumulate(x, y, mass);
    }

    body->normalize();
}

void draw_figure1() {
    
    // Initialize a covariance body consisting of the input masses.
    Covariance_Body2 body;
    compute_covariance_body(&body);
    glColor4f(0.1f, 0.1f, 0.9f, 0.6f);
    draw_one_ellipse(&body);

    // Draw the data points over the ellipse (so they show up
    // better in the magazine!)
    draw_input_points();
}

void draw_figure2() {
    Covariance_Body2 body;
    compute_covariance_body(&body);
    glColor4f(0.1f, 0.1f, 0.9f, 0.35f);
    draw_one_ellipse(&body);


    draw_vertical_projection_lines();
    draw_distribution();
    draw_input_points();

    // Compute the mean, and draw it.
    float total_mass = 0;
    float accumulator = 0;
    float dx = app_shell->screen_width / (float)(DISTRIBUTION_RESOLUTION - 1);
    int i;
    for (i = 0; i < DISTRIBUTION_RESOLUTION; i++) {
        float mass = distribution_values[i];
        float x = i * dx;

        total_mass += mass;
        accumulator += x * mass;
    }

    float mean = accumulator / total_mass;

    // Compute the variance.  It's not necessary to do this in
    // a separate pass -- we can do it in the same pass while we
    // are computing the mean, but now we do it this way for clarity.

    accumulator = 0;
    for (i = 0; i < DISTRIBUTION_RESOLUTION; i++) {
        float mass = distribution_values[i];
        float relative_x = i * dx - mean;
        accumulator += (relative_x * relative_x) * mass;
    }
    
    float variance = accumulator / total_mass;
    assert(variance >= 0);
    float stdev = sqrt(variance);

    glColor3f(1, 1, 0);
    glLineWidth(3);
    app_shell->line_mode_begin();
    float yy = 100;
    float ys = 15;
    float y0 = yy - ys;
    float y1 = yy + ys;
    glBegin(GL_LINES);
    glVertex2f(mean, y0);
    glVertex2f(mean, y1);
    glVertex2f(mean - stdev * NUM_STANDARD_DEVIATIONS, yy);
    glVertex2f(mean + stdev * NUM_STANDARD_DEVIATIONS, yy);
    glVertex2f(mean - stdev * NUM_STANDARD_DEVIATIONS, yy-ys*0.5f);
    glVertex2f(mean - stdev * NUM_STANDARD_DEVIATIONS, yy+ys*0.5f+1);
    glVertex2f(mean + stdev * NUM_STANDARD_DEVIATIONS, yy-ys*0.5f);
    glVertex2f(mean + stdev * NUM_STANDARD_DEVIATIONS, yy+ys*0.5f+1);
    glEnd();
    
    app_shell->line_mode_end();
}

void draw_scene() {
	int x, y;
	x = app_shell->mouse_pointer_x;
	y = app_shell->mouse_pointer_y;

	// We divide y by the screen width, not the height... this is
	// so that the y axis of the coordinate system won't be
	// scaled with respect to the x.
	float fx = x / (float)app_shell->screen_width;
	float fy = y / (float)app_shell->screen_width;

	double now = app_shell->get_time();

    if (figure_to_draw == 1) draw_figure1();
    if (figure_to_draw == 2) draw_figure2();
}

void handle_keydown(int key) {
    if (key == ' ') {
        add_input_point(app_shell->mouse_pointer_x, app_shell->mouse_pointer_y);
    } else if (key == ')') {
        remove_input_point();
    } else if (key == '1') {
        figure_to_draw = 1;
    } else if (key == '2') {
        figure_to_draw = 2;
    }
}

void handle_keyup(int) {
}

void app_init() {
}
