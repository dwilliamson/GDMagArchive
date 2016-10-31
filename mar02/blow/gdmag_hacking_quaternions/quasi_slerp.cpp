#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"
#include "geometry.h"
#include "quaternion.h"
#include "utility.h"
#include "quasi_slerp.h"

// test_q0 and test_q1 are the quaternions that we interpolate between,
// trying out different interpolators, and displaying the results.
Quaternion test_q0;
Quaternion test_q1;
void init_quasi_slerp_test() {
    test_q0.set(0, 0, 0, 1);
    test_q1.set(0, 1, 0, 0);
}


// Here are some functions for picking random quaternions to stick
// into test_q0 and test_q1.
float random_coordinate() {
    int n = rand() % RAND_MAX;
    float f = n / (double)(RAND_MAX - 1);
    f *= 2.0f;
    f -= 1.0f;

    return f;
}

void generate_random_quaternion(Quaternion *result) {
    result->x = random_coordinate();
    result->y = random_coordinate();
    result->z = random_coordinate();
    result->w = random_coordinate();
    result->normalize();
}

void reroll_quasi_slerp_test() {
    generate_random_quaternion(&test_q0);
    generate_random_quaternion(&test_q1);

    if (dot_product(test_q0, test_q1) < 0) {
        test_q1 = test_q1.scale(-1);
    }
}

// These are wrapper functions around the analogous functions in quaternion.h.
// The only point of these is to provide for us a function pointer that takes
// a 'float' as the last argument (the guys in quaternion.h take a 'double').
Quaternion slerp_wrapper(const Quaternion &q0, const Quaternion &q1, float t) {
    return slerp(q0, q1, t);
}

Quaternion lerp_wrapper(const Quaternion &q0, const Quaternion &q1, float t) {
    return lerp(q0, q1, t);
}

// Draw a quaternion's coordinate values on the screen as text.
void list_quaternion(char *heading, const Quaternion &q, int *x, int *y) {
    char buf[1000];
    sprintf(buf, "%s: %.7f %.7f %.7f %.7f", heading, q.x, q.y, q.z, q.w);
    draw_text_line(x, y, buf);
}

// Try out an interpolator, measure the results against the right answer,
// and draw the results. 
void list_interpolator(char *name, Quaternion (*interpolator)(const Quaternion &q0, const Quaternion &q1, float t), 
                       float t, Quaternion *correct_answer, int *x, int *y) {
    int sx = *x;

    glColor3f(0.8f, 0.8f, 0.0f);

    char buf[1000];
    sprintf(buf, "%s ", name);
    draw_text_line(&sx, y, buf);
    *y -= LETTER_HEIGHT + LETTER_PAD;
    sx += LETTER_WIDTH * strlen(buf);

    Quaternion result = interpolator(test_q0, test_q1, t);
    list_quaternion("", result, &sx, y);

    glColor3f(0.94f, 0.6f, 0.65f);

    sx += 20;
    if (correct_answer) {
        float angular_error, length_error;
        Quaternion normalized_result = result;
        normalized_result.normalize();
        float cos_beta = dot_product(*correct_answer, normalized_result);
        if (cos_beta < 0) cos_beta = 0;
        if (cos_beta > 1) cos_beta = 1;

        angular_error = acos(cos_beta) * (180 / M_PI);
        length_error = fabs(sqrt(result.length_squared()) - 1.0f);

        sprintf(buf, "angular error: %.5f degrees    length error: %.5f",
                angular_error, length_error);
        draw_text_line(&sx, y, buf);
    } else {
        draw_text_line(&sx, y, "(correct answer)");
    }
}


// For a given value of 't', try all the interpolators on the
// current test_q0 and test_q1.
void do_interpolation_set(float t, int *x, int *y) {
    *y += LETTER_HEIGHT + LETTER_PAD;

    float s = 0.75f;
    glColor3f(s, s, s);

    char buf[1000];
    sprintf(buf, "For t = %.5f:", t);
    draw_text_line(x, y, buf);

    int tabbed_x = *x + 20;

    Quaternion q2 = slerp(test_q0, test_q1, t);
    list_interpolator("slerp: ", slerp_wrapper, t, NULL, &tabbed_x, y);
    list_interpolator("lerp:  ", lerp_wrapper, t, &q2, &tabbed_x, y);
    list_interpolator("nlerp: ", normalized_lerp, t, &q2, &tabbed_x, y);
    list_interpolator("qslerp:", quasi_slerp, t, &q2, &tabbed_x, y);
}


// Draw the scene: list the two current test quaternions, then
// try all the interpolators for three different values of 't'
// (currently hardcoded to 0.5, 0.25, and 0.7).
void draw_scene_quasi_slerp_test() {
    int x = 10;
    int y = 10;

    begin_text_mode();

    glColor3f(0.0f, 1.0f, 0.0f);
    draw_text_line(&x, &y, "Press 'R' to choose new random quaternions.");
    draw_text_line(&x, &y, "");

    glColor3f(1.0f, .3f, .3f);

    list_quaternion("q0: ", test_q0, &x, &y);
    list_quaternion("q1: ", test_q1, &x, &y);

    float dot = dot_product(test_q0, test_q1);
    if (dot < 0) dot = 0;
    if (dot > 1) dot = 1;
    float beta = acos(dot) * (180 / M_PI);
    
    char buf[1000];
    glColor3f(0.7f, 0.5f, 0.5f);
    sprintf(buf, "    (%.3f degrees apart)", beta);
    draw_text_line(&x, &y, buf);


    do_interpolation_set(0.5f, &x, &y);
    do_interpolation_set(0.25f, &x, &y);
    do_interpolation_set(0.7f, &x, &y);

    end_text_mode();
}
