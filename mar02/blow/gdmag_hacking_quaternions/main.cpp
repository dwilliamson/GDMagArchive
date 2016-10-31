/*
This file contains all the stuff that I used to find the fastest
quaternion slerp approximation that I could given the time available.
There are two components to the approximation; one involves linear
interpolation to find a vector in approximately the right direction,
and the other involves normalizing that vector.  

Since linear interpolation induces distortion compared to spherical
interpolation, we warp the interpolation "time" parameter to cancel
as much of this distortion as we can.  See the article.  
("Hacking Quaternions", The Inner Product, by Jonathan Blow,
Game Developer Magazine, March 2002).  

So this file contains some ad hoc numerical optimizers, and some
stuff to graph the response functions as parameters are being tweaked 
with.  If you're the kind of person who doesn't want to know how the
sausage was made, and you just want to taste the results, I'd 
recommend looking at quasi_slerp.h and quasi_slerp.cpp.

    -Jonathan Blow   jon@bolt-action.com   February 10, 2002.
 */




// This code is Copyright (c) Jonathan Blow, 2002.  All rights reserved.
// See the full copyright notice in the accompanying file "Copyright.txt".

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"
#include "geometry.h"
#include "quaternion.h"
#include "utility.h"

const int DO_CONTINUOUS_SPLINE = 0;

enum App_Mode {
    APP_MODE_ANGLE_OPTIMIZER = 0,
    APP_MODE_LENGTH_OPTIMIZER,
    APP_MODE_QUASI_SLERP_TEST,
    APP_MODE_FIGURE_1,
    APP_MODE_FIGURE_2,
    APP_MODE_FIGURE_3,
    APP_MODE_FIGURE_4,
    APP_MODE_FIGURE_5
};

App_Mode app_mode;

// This Parameter struct is just a framework for holding
// a floating-point value that our numerical optimizer can tweak.
// It sets up the initial value and the valid range over which
// we want to search.
struct Parameter {
    double value;
    double last_value;
    double initial_value;
    double step;

    void init(double value, double step_fraction);
};

// Optimization_Parameters is the struct that holds the variables
// that control our interpolation error measurements.  'alpha' is
// the angle between the two quaternions.  'worst_case_slope' is
// the slope of our t-compensating spline when the angle between
// the quaternions is 90 degrees.  'slope_attenuation' is a scalar
// that controls how the slope of the spline is tweaked as we go
// from 0 degrees to 90 degrees.

// This struct is passed to procedures that iterate over the range
// of 'alpha' and over many values of the interpolation 
// time parameter 't' to compute the amount of error induced by
// the interpolation approximation, compared to a full slerp.
// The summed errors are stored back in the struct.
struct Optimization_Parameters {
    Parameter alpha;
    Parameter worst_case_slope;
    Parameter slope_attenuation;

    double total_squared_error;
    double max_squared_error;

    int total_error_counter;
    int max_error_counter;
};

// Length_Optimization_Parameters is much like Optimization_Parameters,
// but it is for routines that normalize vectors, whereas the previous
// struct was for routines that interpolate them.  The normalization is
// simpler, so we have less stuff going on.
struct Length_Optimization_Parameters {
    Parameter scale;
    Parameter x_offset;

    double total_squared_error;
    int total_error_counter;
};

// Miscellaneous variables that do random junk we need to do.
// Someday I will make a wrapper class to hold this stuff since
// it's pretty much the same from month to month.
GLuint font_handle = 0;

char *spot_texture_name = "white_dot.jpg";
GLuint spot_texture_handle = -1;

void draw_scene_angle_optimizer();
void draw_scene_length_optimizer();
void draw_scene_quasi_slerp_test();
void init_angle_optimizer();
void init_length_optimizer();
void init_quasi_slerp_test();

void reroll_quasi_slerp_test();

// These are some old t-compensation functions that I was 
// screwing with.  I left them in because, hey, why not.
/*
float correction(float t, float alpha) {
    float k = 0.45;

    float a = 2 * k;
    float b = -3 * k;
    float c = 1 + k;
    double y_fraction = t * (a*t*t + b*t + c);

    return y_fraction;
}
*/

/*
float correction(float t, float alpha) {
    float m0 = 1;
    float m1 = 1;

    float a = m0 + m1;
    float b = -2*m0 - m1;
    float c = m0;
    float d = 1;

    double y_fraction = a*t*t*t + b*t*t + c*t + d;

    return y_fraction;
}
*/

// This function passes the interpolation time parameter 't' through
// a spline in order to warp it, in such a way as to roughly compensate
// for the distortion induced by linear interpolation.  The particular
// shape of the spline is controlled by the Optimization_Parameters
// argument.

// At first glance this function looks expensive because of things 
// like the cos(alpha) in there, but that's just because this is a 
// quickly-put-together version of the function that I used to control 
// the numerical optimizer.  The final version of the function, with
// the hardcoded parameters that resulted from the numerical optimizer,
// is quite fast.
float correction(float t, Optimization_Parameters *parameters) {
    double alpha = parameters->alpha.value;
    double cos_alpha = cos(alpha);
    double factor = 1 - parameters->slope_attenuation.value * cos_alpha;
    factor *= factor;
    
    float k = parameters->worst_case_slope.value * factor;

    float b = 2 * k;
    float c = -3 * k;
    float d = 1 + k;

    double y_fraction = t*(b*t + c) + d;

    return y_fraction;
}

// theta_distortion(.) measures the difference in angles produced
// by lerp versus slerp.  Used to help us draw graphs and compute
// error.  The return value represents how far, angle-wise,
// you are from the start to the finish for a given 't'.
// If there were no distortion, this function would be the identity --
// you would just get 't' back as the return value.  Instead we
// get a value that moves from 0 to 1 with wavering speed.

// As discussed in the article, this version of the function was
// derived by drawing a chord across a 2D circle on some graph
// paper.
float theta_distortion(float t, Optimization_Parameters *parameters) {
    double alpha = parameters->alpha.value;
    double numerator = t * sin(alpha);
    double denominator = 1 + t * (cos(alpha) - 1);
        
    double theta = atan2(numerator, denominator);

    assert(alpha != 0);
    double y_fraction = theta / alpha;

    return y_fraction;
}

// This version of theta_distortion takes two quaternions as 
// arguments.  Basically it exists to verify that the above
// formulation is correct, when you actually take quaternions
// and do the stuff in 4D.
float theta_distortion(float t, Optimization_Parameters *parameters,
                       Quaternion *q0, Quaternion *q1) {
    Quaternion q2 = lerp(*q0, *q1, t);
    q2.normalize();
    double theta = acos(dot_product(*q0, q2));
    double theta_max = acos(dot_product(*q0, *q1));
    return theta / theta_max;
}

// percentage_slerped(.) is like the above version of theta_distortion
// except that it actually uses a proper slerp; thus it should be
// the identity (within cumulative floating point roundoff error).
// It's basically here to verify that the above formulation is correct.
float percentage_slerped(float t, Optimization_Parameters *parameters,
                         Quaternion *q0, Quaternion *q1) {
    Quaternion q2 = slerp(*q0, *q1, t);
    double theta = acos(dot_product(*q0, q2));
    double theta_max = acos(dot_product(*q0, *q1));
    return theta / theta_max;
}


// draw_sqrt_function(.) graphs sqrt(x) for 0.5 <= x <= 1.5.
// Then it draws the tangent line in the neighborhood of 1.
// Used to create a figure for the article.
void draw_sqrt_function(float x0, float y0, float width, float height,
                        float pad) {
    // Draw the axes.

    width *= 2;
    glLineWidth(2);
    if (1) {
        glColor3f(1, 1, 1);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0 - pad, y0);
        glVertex2f(x0 + width + 2 * pad, y0);
        glEnd();


        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0 - pad);
        glVertex2f(x0, y0 + height + 2 * pad);
        glEnd();
    }
    
    glLineWidth(4);

    // Per pixel, compute and plot the sqrt function.
    
    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    glColor3f(0, 1, 0);
    glBegin(GL_LINE_STRIP);

    const double YSCALE = 1.0 / 3.0;

    int i;
    for (i = i0 - 20; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);

        double xmin = 0.5;
        double xmax = 1.5;
        double s = t * (xmax - xmin) + xmin;
        double y_fraction = 1.0 / sqrt(s);
        y_fraction *= YSCALE;
        float y = y0 + width * y_fraction;

        if (1) glVertex2f(i, y);
    }

    glEnd();

    // Now draw the tangent line.

    glLineWidth(4);
    glColor3f(1, 1, 0);
    glBegin(GL_LINE_STRIP);
    float fx0, fx1, fy0, fy1;
    fx0 = 0.5;
    fx1 = 1.5;
    
    fy0 = 1 - 0.5 * (-0.5);
    fy1 = 1 - 0.5 * (0.5);
    fy0 *= YSCALE;
    fy1 *= YSCALE;

    float fj0 = y0 + width * fy0;
    float fj1 = y0 + width * fy1;
    
    float fi0 = i0;
    float fi1 = i1;

    double back = 20;
    double ratio = back / (fi1 - fi0);
    fi0 -= back;
    fj0 -= ratio * (fj1 - fj0);
    glVertex2f(fi0, fj0);
    glVertex2f(fi1, fj1);
    glEnd();
}

// An approximation to 1/sqrt(x) that is adjustable; we can control
// the neighborhood where we want accuracy, and the scale by which
// we pump it up to straddle the error across both sides of the
// function.  (The scale will be a number slightly larger than 1).
double isqrt_approx_in_neighborhood(double s, double neighborhood, double scale) {
    return scale / sqrt(neighborhood) + scale * (s - neighborhood) * (-0.5 * 1.0 / (neighborhood * sqrt(neighborhood)));
}

// Graph the above approximation.  Used for making figures and
// verifying that the isqrt thing is a reasonable idea.
void draw_isqrt_function(float x0, float y0, float width, float height,
                        float pad) {
    // Draw the axes.

    glLineWidth(2);
    if (1) {
        glColor3f(1, 1, 1);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0 - pad, y0);
        glVertex2f(x0 + width + 2 * pad, y0);
        glEnd();


        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0 - pad);
        glVertex2f(x0, y0 + height + 2 * pad);
        glEnd();

        // Draw green line for f(s) = 1
        glColor3f(0, 1, 0);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x0 - pad, y0 + height);
        glVertex2f(x0 + width + pad, y0 + height);
        glEnd();
    }
    
    glLineWidth(4);

    // Per pixel, compute and plot the value.
    
    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    glColor3f(1, 1, 0);
    glBegin(GL_LINE_STRIP);

    int i;
    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double xmin = 0.5;
        double xmax = 1.0;
        double s = t * (xmax - xmin) + xmin;
        double k = isqrt_approx_in_neighborhood(s, 1, 1);
        double y_fraction = k * sqrt(s);
        float y = y0 + width * y_fraction;

        if (1) glVertex2f(i, y);
    }

    glEnd();
}

// This function calls the theta_distortion stuff above,
// varying 't', and graphing the results.  It graphs some
// other stuff too, in different colors, like the compensating
// spline, the product of the distortion and the spline-correction factor
// (which is hopefully distorted a lot less), and the ideal
// line [the line from (0, 0) to (1, 1), which is what we would
// like to get by multiplying the distortion with the correction factor.]
void draw_theta_distortion(float x0, float y0, float width, float height,
                           float pad, Optimization_Parameters *parameters, bool draw, bool draw_spline = true, bool draw_product = true) {
    // Draw the axes.

    glLineWidth(2);
    if (draw) {
        glColor3f(1, 1, 1);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0 - pad, y0);
        glVertex2f(x0 + width + 2 * pad, y0);
        glEnd();


        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0 - pad);
        glVertex2f(x0, y0 + height + 2 * pad);
        glEnd();


        glLineWidth(3);

        // Draw the ideal line.

        glColor3f(0, 1, 0);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0);
        glVertex2f(x0 + width, y0 + height);
        glEnd();
    }
    
    glLineWidth(4);

    const double alpha = parameters->alpha.value;


    // Per pixel, compute and plot the distorted theta.
    
    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    glBegin(GL_LINE_STRIP);
    glColor3f(1, 0, 0);

    int i;
    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double y_fraction = theta_distortion(t, parameters);
        float y = y0 + width * y_fraction;

        if (draw) glVertex2f(i, y);

        double max_error = y_fraction - t;
        double max_squared = max_error * max_error;
        parameters->max_squared_error += max_squared;
        parameters->max_error_counter++;
    }

    glEnd();




    // Spline

    if (draw_spline) {
        glBegin(GL_LINE_STRIP);
        glColor3f(0, 0, 1);

        for (i = i0; i < i1; i++) {
            float t = (i - i0) / (double)(i1 - i0);
            assert(t >= 0);
            assert(t <= 1);

            double y_fraction = correction(t, parameters);
            float y = y0 + height * y_fraction;

            if (draw) glVertex2f(i, y);
        }

        glEnd();
    }

    // Product of distortion and anti-distortion

    if (draw_product) {
        glBegin(GL_LINE_STRIP);
        glColor3f(1, 0, 0);

        for (i = i0; i < i1; i++) {
            float t = (i - i0) / (double)(i1 - i0);
            assert(t >= 0);
            assert(t <= 1);

            double y_fraction;


            if (DO_CONTINUOUS_SPLINE) {
                y_fraction = correction(t, parameters) * theta_distortion(t, parameters);
            } else {
                if (t <= 0.5) {
                    y_fraction = correction(t, parameters) * theta_distortion(t, parameters);
                } else {
                    y_fraction = 1 - correction(1 - t, parameters) * theta_distortion(1 - t, parameters);
                }
            }

            float y = y0 + height * y_fraction;

            if (draw) glVertex2f(i, y);

            double error = y_fraction - t;
            double squared = error * error;
            parameters->total_squared_error += squared;
            parameters->total_error_counter++;
        }

        glEnd();
    }

}

// This version is basically the same as above, but actually
// performs the stuff on quaternions.  Exists to verify the
// correctness of the above function.
void draw_theta_distortion_quaternion_version(float x0, float y0, float width, float height,
                                              float pad, Optimization_Parameters *parameters, bool draw, bool draw_spline = true, bool draw_product = true) {
    // Draw the axes.

    glLineWidth(2);
    if (draw) {
        glColor3f(1, 1, 1);

        glBegin(GL_LINE_STRIP);
        glVertex2f(x0 - pad, y0);
        glVertex2f(x0 + width + 2 * pad, y0);
        glEnd();


        glBegin(GL_LINE_STRIP);
        glVertex2f(x0, y0 - pad);
        glVertex2f(x0, y0 + height + 2 * pad);
        glEnd();
    }
    
    glLineWidth(4);

    const double alpha = parameters->alpha.value;

    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    Quaternion q0;
    Quaternion q1;
    q0.set_from_axis_and_angle(1, 0, 0, 0);
    q1.set_from_axis_and_angle(1, 0, 0, M_PI);
      

    // Per pixel, compute and plot the angle we get for slerp
    // (This should be a diagonal line, up to numerical error).

    glBegin(GL_LINE_STRIP);
    glColor3f(0, 1, 0);

    int i;
    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double y_fraction = percentage_slerped(t, parameters, &q0, &q1);
        float y = y0 + width * y_fraction;

        if (draw) glVertex2f(i, y);
    }

    glEnd();
    
    // Per pixel, compute and plot the distorted theta.
    

    glBegin(GL_LINE_STRIP);
    glColor3f(1, 0, 0);

    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double y_fraction = theta_distortion(t, parameters, &q0, &q1);
        float y = y0 + width * y_fraction;

        if (draw) glVertex2f(i, y);

        double max_error = y_fraction - t;
        double max_squared = max_error * max_error;
        parameters->max_squared_error += max_squared;
        parameters->max_error_counter++;
    }

    glEnd();




    // Spline

    if (draw_spline) {
        glBegin(GL_LINE_STRIP);
        glColor3f(0, 0, 1);

        for (i = i0; i < i1; i++) {
            float t = (i - i0) / (double)(i1 - i0);
            assert(t >= 0);
            assert(t <= 1);

            double y_fraction = correction(t, parameters);
            float y = y0 + height * y_fraction;

            if (draw) glVertex2f(i, y);
        }

        glEnd();
    }

    // Product of distortion and spline

    if (draw_product) {
        glBegin(GL_LINE_STRIP);
        glColor3f(1, 0, 0);

        for (i = i0; i < i1; i++) {
            float t = (i - i0) / (double)(i1 - i0);
            assert(t >= 0);
            assert(t <= 1);

            double y_fraction;


            if (DO_CONTINUOUS_SPLINE) {
                y_fraction = correction(t, parameters) * theta_distortion(t, parameters);
            } else {
                if (t <= 0.5) {
                    y_fraction = correction(t, parameters) * theta_distortion(t, parameters);
                } else {
                    y_fraction = 1 - correction(1 - t, parameters) * theta_distortion(1 - t, parameters);
                }
            }

            float y = y0 + height * y_fraction;

            if (draw) glVertex2f(i, y);

            double error = y_fraction - t;
            double squared = error * error;
            parameters->total_squared_error += squared;
            parameters->total_error_counter++;
        }

        glEnd();
    }

}

// This just graphs the spline correction factors.
// Used to make a figure for the article.
void draw_compensating_product_spline(float x0, float y0, float width, float height,
                           float pad, Optimization_Parameters *parameters) {

    glLineWidth(4);

    const double alpha = parameters->alpha.value;

    glColor3f(1, 1, 0);

    // Spline

    int i;
    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    if (1) {
        glBegin(GL_LINE_STRIP);

        for (i = i0; i < i1; i++) {
            float t = (i - i0) / (double)(i1 - i0);
            assert(t >= 0);
            assert(t <= 1);

            double y_fraction = correction(t, parameters);
            y_fraction *= t;

            float y = y0 + height * y_fraction;

            if (1) glVertex2f(i, y);
        }

        glEnd();
    }
}

//
// Okay, that was all the angle-undistortion stuff.  Now we are
// on to the fast normalization stuff.
//



// Here are a few versions of fast_normalize that I was playing
// around with.  The final result is in quasi_slerp.h along with the
// final undistortion stuff.
double fast_normalize(double s, Length_Optimization_Parameters *parameters) {
    double result;
    result = isqrt_approx_in_neighborhood(s, 
                                          parameters->x_offset.value,
                                          parameters->scale.value);

    return result;
}

double fast_normalize1(double s) {
    double result;
    if (s >= .75) {
        result = isqrt_approx_in_neighborhood(s, 0.87, 1.005);
    } else {
        result = isqrt_approx_in_neighborhood(s, 0.63, 1.005);
    }

    return result;
}

/*
double fast_normalize2(double s) {
    double result, result2, result3;
    result = isqrt_approx_in_neighborhood(s, 1.0, 1.0);
    result2 = isqrt_approx_in_neighborhood(s * result * result, 1.0, 1.0);
    result3 = isqrt_approx_in_neighborhood(s * result2, 1.0, 1.0);

    return result * result2 * result3;
}
*/


// A semi-hardened version of the inverse square root approximation.
// Just used by the fast_normalize_hardened below... just here to
// verify that the numbers I'd come up with were working, prior
// to putting stuff in quasi_slerp.h.
double isqrt_approx_in_neighborhood2(double s) {
    const float NEIGHBORHOOD = 0.959066;
    const float SCALE = 1.000311;
    const float ADDITIVE_CONSTANT = SCALE / sqrt(NEIGHBORHOOD);
    const float FACTOR = SCALE * (-0.5 / (NEIGHBORHOOD * sqrt(NEIGHBORHOOD)));

    return ADDITIVE_CONSTANT + (s - NEIGHBORHOOD) * FACTOR;
}

// Use the above function to do a fast normalize.
inline float fast_normalize_hardened(double value) {
    float s = value;
    float k = isqrt_approx_in_neighborhood2(s);

    if (s <= 0.91521198) {
        k *= isqrt_approx_in_neighborhood2(k * k * s);

        if (s <= 0.65211970) {
            k *= isqrt_approx_in_neighborhood2(k * k * s);
        }
    }

    return k;
}


// fast_normalize4(.) is a version that keeps iterating and performing
// the tangent line approximation.  I used it to figure out how many
// tries were needed for the hardened version above.
int maxtries;
int last_ntries;
double fast_normalize4(double s, Length_Optimization_Parameters *parameters) {
    double factor = 1;
    const double LIMIT = 0.9995;
    double limit2 = LIMIT * LIMIT;
    int ntries = 0;
    while (factor * factor * s < limit2) {
//        double new_factor = isqrt_approx_in_neighborhood(factor * factor * s, .959066, 1.000311);
        double new_factor = isqrt_approx_in_neighborhood(factor * factor * s, parameters->x_offset.value, parameters->scale.value);
        factor *= new_factor;
        ntries++;
        if (ntries > 8) break;
    }

    if (ntries > maxtries) maxtries = ntries;

    last_ntries = ntries;

    return factor;
}

// fast_normalize3 is pretty much the same kind of deal as fast_normalize4,
// but is a slightly higher-level earlier version (it calls fast_normalize
// every iteration instead of going straight for the tangent line
// approximation itself).
double fast_normalize3(double s, Length_Optimization_Parameters *parameters) {
    double s_orig = s;

    double factor = 1;
    const double LIMIT = 0.9995;
    double limit2 = LIMIT * LIMIT;
    int ntries = 0;
    while (factor * factor * s < limit2) {
        double new_factor = fast_normalize(factor * factor * s, parameters);
        factor *= new_factor;
        ntries++;
        if (ntries > 8) break;
    }

    if (ntries > maxtries) maxtries = ntries;

    last_ntries = ntries;

    return factor;
}

// draw_length_distortion(.) graphs what we get when we try to
// normalize a vector with a fast normalizer that is tuned to
// the provided Length_Optimization_Parameters.
void draw_length_distortion(float x0, float y0, float width, float height,
                            float pad, Length_Optimization_Parameters *parameters) {
    glLineWidth(2);
    // Draw the axes.

    glColor3f(1, 1, 1);


    glBegin(GL_LINE_STRIP);
    glVertex2f(x0 - pad, y0);
    glVertex2f(x0 + width + 2 * pad, y0);
    glEnd();


    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0 - pad);
    glVertex2f(x0, y0 + height + 2 * pad);
    glEnd();

    // Draw the ideal.

    glColor3f(0, 1, 0);
    
    glBegin(GL_LINE_STRIP);
    glVertex2f(x0 - pad, y0 + height);
    glVertex2f(x0 + width + 2 * pad, y0 + height);
    glEnd();

    
    // Per pixel, compute and plot the distorted length.
    
    int i0 = (int)x0;
    int i1 = (int)(x0 + width + 1);

    double xmin = 0.5;
    double xmax = 1.0;

    int num_samples = 0;

    glBegin(GL_LINE_STRIP);
    glColor3f(1, 1, 0);

    int nsamples = 0;
    double total_squared_error = 0;

    int tx = 500;
    int ty = 0;

    int num_switchovers = 0;
    const int MAX_SWITCHOVERS = 10;
    float switchovers[MAX_SWITCHOVERS];

    maxtries = -1;
    int last_last_ntries = -1;
    last_ntries = -1;
    int i;
    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double s = t * (xmax - xmin) + xmin;
        double result1 = fast_normalize4(s, parameters);
//        double result1 = fast_normalize_hardened(s);
//        assert(result == result1);
        double result = result1;

        double y_fraction = result * sqrt(s);
        float y = y0 + width * y_fraction;
        glVertex2f(i, y);

        double error2 = y_fraction - 1.0;
        error2 *= error2;

        total_squared_error += error2;
        nsamples++;

        parameters->total_squared_error += error2;
        parameters->total_error_counter++;

        if ((i != i0) && (last_ntries != last_last_ntries)) {
            switchovers[num_switchovers++] = s;
        }

        last_last_ntries = last_ntries;
    }

    glEnd();


    glBegin(GL_LINE_STRIP);
    glColor3f(1, 0, 0);

    for (i = i0; i < i1; i++) {
        float t = (i - i0) / (double)(i1 - i0);
        assert(t >= 0);
        assert(t <= 1);

        double s = t + 0.5f;
        double y_fraction = 1.0 / sqrt(s) / (1.0 / sqrt(s));

        float y = y0 + width * y_fraction;
        glVertex2f(i, y);
    }
    glEnd();

    total_squared_error /= nsamples;
    double error = sqrt(total_squared_error);


    begin_text_mode();
 
    char buf[1000];
    
    sprintf(buf, "Last ntries: %d", last_ntries);
    draw_text_line(&tx, &ty, buf);

    for (i = 0; i < num_switchovers; i++) {
        sprintf(buf, "Switchover at %.8f", switchovers[i]);
        draw_text_line(&tx, &ty, buf);
    }

    sprintf(buf, "Error: %.8f   Tries: %d", error, maxtries);
    draw_text_line(&tx, &ty, buf);
    end_text_mode();
    
}


// draw_graph_vector(.) just draws an arrow with a big arrowhead
// on the screen.  Used to make a figure for the article.
void draw_graph_vector(float x, float y, float w, float h, double theta) {
    float xmid = x + 0.5 * w;
    float ymid = y + 0.5 * h;

    float radius = w * 0.5 * 0.9;
    float head_forward = radius * 0.11f;
    float head_sideways = head_forward * 0.4f;
    
    double ct = cos(theta);
    double st = sin(theta);

    float end_x = xmid + ct * radius;
    float end_y = ymid + st * radius;

    glBegin(GL_LINE_STRIP);
    glVertex2f(xmid, ymid);
    glVertex2f(end_x, end_y);
    glEnd();


    float head_placement_radius = radius * 0.96f;
    float head_x = xmid + ct * head_placement_radius;
    float head_y = ymid + st * head_placement_radius;

    float x0 = head_x + (-st) * head_sideways;
    float y0 = head_y + ct * head_sideways;

    float x1 = head_x - (-st) * head_sideways;
    float y1 = head_y - ct * head_sideways;

    float x2 = head_x + ct * head_forward;
    float y2 = head_y + st * head_forward;

    glBegin(GL_TRIANGLES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

// This draws the figure showing the cross-section of quaternion space
// such that we get 2 quaternions on the unit circle... and the chord
// between them that we get by lerping.
void draw_2d_lerp_graph(float x, float y, float w, float h) {
    float xmid = x + 0.5 * w;
    float ymid = y + 0.5 * h;

    // Draw the axes.

    float g = 0.5f;
    glLineWidth(2);
    glColor3f(g, g, g);

    glBegin(GL_LINE_STRIP);
    glVertex2f(x, ymid);
    glVertex2f(x + w, ymid);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex2f(xmid, y);
    glVertex2f(xmid, y + h);
    glEnd();


    // Draw the circle.
    const int NSEGMENTS = 1024;

    glLineWidth(4);
    glColor3f(0, 0, 1);

    glBegin(GL_LINE_STRIP);

    float radius = w * 0.5 * 0.9;
    int i;
    for (i = 0; i < NSEGMENTS; i++) {
        float perc = i / (double)(NSEGMENTS - 1);
        double ct = cos(perc * 2 * M_PI);
        double st = sin(perc * 2 * M_PI);

        glVertex2f(xmid + ct * radius, ymid + st * radius);
    }

    glEnd();

    // Draw the slerp arc.

    double theta0 = 0;
    double theta1 = M_PI * 0.42;
    
    const int N_ARC_SEGMENTS = 100;

    glColor3f(1, 0, 0);
    glLineWidth(6);
    glBegin(GL_LINE_STRIP);

    for (i = 0; i < N_ARC_SEGMENTS; i++) {
        float perc = i / (double)(N_ARC_SEGMENTS - 1);
        double theta = perc * theta1;
        double ct = cos(theta);
        double st = sin(theta);

        glVertex2f(xmid + ct * radius, ymid + st * radius);
    }

    glEnd();

    // Draw the interpolation line between the vectors.

    float x0 = cos(theta0) * radius + xmid;
    float y0 = sin(theta0) * radius + ymid;

    float x1 = cos(theta1) * radius + xmid;
    float y1 = sin(theta1) * radius + ymid;

    glColor3f(0, 1, 0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();

    // Draw the 2 vectors.

    glColor3f(1, 1, 0);
    glLineWidth(5);
    draw_graph_vector(x, y, w, h, theta0);
    draw_graph_vector(x, y, w, h, theta1);

}

// Draw whichever graph it's time to draw.
void draw_figure(int graph_number) {
    const double N = 256;
    const double OMEGA = 57.5;


    begin_line_mode(1, 1, 1);
    
    glColor3f(0.5, 0.5, 0.5);

    float x = 120;
    float y = 220;
    float w = 400;
    float h = 400;

    assert(graph_number >= 1);
    assert(graph_number <= 5);

    switch (graph_number) {
        case 1: {
            draw_2d_lerp_graph(x, y, w, h);
            break;
        }

        case 2: {
            Optimization_Parameters parameters;
            parameters.alpha.init(M_PI * 0.5, 0.1);
            parameters.worst_case_slope.init(0.45, 1.0);
            parameters.slope_attenuation.init(0.9, 1.0);

            draw_theta_distortion(x, y, w, h, 10, &parameters, true, false, false);
            break;
        }
        case 3: {
            Optimization_Parameters parameters;
            parameters.alpha.init(M_PI * 0.5, 0.1);
            parameters.worst_case_slope.init(0.45, 1.0);
            parameters.slope_attenuation.init(0.9, 1.0);

            draw_theta_distortion(x, y, w, h, 10, &parameters, true, false, false);
            draw_compensating_product_spline(x, y, w, h, 10, &parameters);
            break;
        }
        case 4: {
            draw_sqrt_function(x, y, w, h, 10);
            break;
        }
        case 5: {
            draw_isqrt_function(x, y, w, h, 10);
            break;
        }
        default:
            graph_function_axes();
            break;
    }
}

// Load whatever textures we need.
void init_textures() {
    char *name = spot_texture_name;
    int width, height;
    bool success = texture_from_file(name, &spot_texture_handle,
                                     &width, &height);
    assert(success);
}

// Load the font, then init the textures and whatever variables we need.
void init_scene() {
    int width, height;
    bool success = texture_from_file("font.jpg", &font_handle, &width, &height);
    assert(success);

    init_textures();

    init_angle_optimizer();
    app_mode = APP_MODE_ANGLE_OPTIMIZER;
}

// Just initialize the values of a Parameter struct.
void Parameter::init(double _value, double _step_fraction) {
    value = _value;
    step = value * _step_fraction;
    initial_value = _value;
    last_value = _value;
}


// Okay, below is a bunch of stuff for my ad hoc numerical optimizer.
// It works by varying the values in this global variable 'current_parameters'
// and calling the appropriate error measurement function.  When the
// resulting error is lower than anything we have seen so far, the
// 'current_parameters' gets copied into 'best_parameters'.

// Actually there are 2 optimizers here.  The variables I just 
// mentioned are for the angle-distortion optimizer.  The vector-length
// optimizer uses the similarly-named variables 'best_length_parameters'
// and 'current_length_parameters'.
Optimization_Parameters best_parameters;
Optimization_Parameters current_parameters;
int opt_step_number = 0;
int opt_phase_index = 0;

Length_Optimization_Parameters best_length_parameters;
Length_Optimization_Parameters current_length_parameters;

// Set up the global variables for the angle optimizer.  Called once
// when we first start running it.
void init_angle_optimizer() {
    Optimization_Parameters parameters;
    parameters.alpha.init(M_PI * 0.5, 0.1);
    parameters.worst_case_slope.init(0.45, 1.0);
    parameters.slope_attenuation.init(0.9, 1.0);

    current_parameters = parameters;
    best_parameters = parameters;
    opt_step_number = 0;
    opt_phase_index = 0;
}

// Set up the parameters for the angle optimizer.  (Obvious comment alert!)
void setup_parameters_for_angle_optimizer() {
    const int STEPS_PER_ITERATION = 200;
    const int MAJOR_PHASE_STEPS = 2 * STEPS_PER_ITERATION;

    int old_opt_phase_index = opt_phase_index;
    opt_phase_index = opt_step_number / MAJOR_PHASE_STEPS;

    if (opt_phase_index != old_opt_phase_index) {
        best_parameters.worst_case_slope.initial_value = best_parameters.worst_case_slope.value;
        best_parameters.slope_attenuation.initial_value = best_parameters.slope_attenuation.value;

        best_parameters.worst_case_slope.step *= 0.5;
        best_parameters.slope_attenuation.step *= 0.5;
    }

    current_parameters = best_parameters;

    int modded_step_number = opt_step_number % MAJOR_PHASE_STEPS;
    if (modded_step_number < STEPS_PER_ITERATION) {
        int step_number = modded_step_number;
        double perc = step_number / (double)STEPS_PER_ITERATION;

        double slope_range = current_parameters.worst_case_slope.step;
        current_parameters.worst_case_slope.value = current_parameters.worst_case_slope.initial_value + (perc - 0.5) * slope_range;
    } else {
        int step_number = modded_step_number - STEPS_PER_ITERATION;
        double perc = step_number / (double)STEPS_PER_ITERATION;

        double slope_range = current_parameters.slope_attenuation.step;
        current_parameters.slope_attenuation.value = current_parameters.slope_attenuation.initial_value + (perc - 0.5) * slope_range;
    }


    
    current_parameters.total_squared_error = 0;
    current_parameters.max_squared_error = 0;

    current_parameters.total_error_counter = 0;
    current_parameters.max_error_counter = 0;

}

// Draw the parameters for the angle optimizer, so's we can 
// watch what is happening while it works.
void draw_parms(char *title, Optimization_Parameters *parameters,
                int *x, int *y) {
    char buf[1000];
    sprintf(buf, "%s", title);
    draw_text_line(x, y, buf);

    double error = parameters->total_squared_error;
    error /= (double)parameters->total_error_counter;
    error = sqrt(error);
    
    double slope = parameters->worst_case_slope.value;
    double attenuation = parameters->slope_attenuation.value;
    sprintf(buf, "Error: %.8f  Slope: %.8f  Atten: %.8f", 
            error, slope, attenuation);
    draw_text_line(x, y, buf);
    
}

// Draw the parameters for the length optimizer, so we can
// watch that one, too.
void draw_length_parms(char *title, Length_Optimization_Parameters *parameters, int *x, int *y) {
    double squared_error = parameters->total_squared_error;
    int num_samples = parameters->total_error_counter;
    squared_error /= num_samples;

    begin_text_mode();

    char buf[1000];
    sprintf(buf, "%s", title);
    draw_text_line(x, y, buf);

    sprintf(buf, "Error: %.6f", sqrt(squared_error));
    draw_text_line(x, y, buf);
    sprintf(buf, "Scale: %1.6f  Offset: %.6f", parameters->scale.value,
            parameters->x_offset.value);
    draw_text_line(x, y, buf);
    end_text_mode();
}

// Set up the global variables for the length optimizer.  Called once
// when we first start running it.
void init_length_optimizer() {
    Length_Optimization_Parameters parameters;
    parameters.scale.init(1.005, 0.1);
    parameters.x_offset.init(0.87, 0.3);

    parameters.total_squared_error = 100;
    parameters.total_error_counter = 10;
    current_length_parameters = parameters;
    best_length_parameters = parameters;
    opt_step_number = 0;
    opt_phase_index = 0;
}

// Set up the parameters for the length optimizer.
void setup_length_parameters() {
    const int STEPS_PER_ITERATION = 200;
    const int MAJOR_PHASE_STEPS = 2 * STEPS_PER_ITERATION;

    int old_opt_phase_index = opt_phase_index;
    opt_phase_index = opt_step_number / MAJOR_PHASE_STEPS;

    if (opt_phase_index != old_opt_phase_index) {
        best_length_parameters.scale.initial_value = best_length_parameters.scale.value;
        best_length_parameters.x_offset.initial_value = best_length_parameters.x_offset.value;

        best_length_parameters.scale.step *= 0.5;
        best_length_parameters.x_offset.step *= 0.5;
    }

    current_length_parameters = best_length_parameters;

    int modded_step_number = opt_step_number % MAJOR_PHASE_STEPS;
    if (modded_step_number < STEPS_PER_ITERATION) {
        int step_number = modded_step_number;
        double perc = step_number / (double)STEPS_PER_ITERATION;

        double slope_range = current_length_parameters.scale.step;
        current_length_parameters.scale.value = current_length_parameters.scale.initial_value + (perc - 0.5) * slope_range;
    } else {
        int step_number = modded_step_number - STEPS_PER_ITERATION;
        double perc = step_number / (double)STEPS_PER_ITERATION;

        double slope_range = current_length_parameters.x_offset.step;
        current_length_parameters.x_offset.value = current_length_parameters.x_offset.initial_value + (perc - 0.5) * slope_range;
    }

    current_length_parameters.total_squared_error = 0;
    current_length_parameters.total_error_counter = 0;
}

// Run one frame of the length optimizer process, and display
// the results.
void draw_scene_length_optimizer() {
    setup_length_parameters();

    begin_line_mode(1, 1, 1);

    draw_length_distortion(120, 220, 400, 400, 10, &current_length_parameters);

    if ((opt_step_number == 0) && (opt_phase_index == 0)) best_length_parameters = current_length_parameters;

    double best_error = best_length_parameters.total_squared_error;
    if (current_length_parameters.total_squared_error < best_error) {
        best_length_parameters = current_length_parameters;
    }

    int tx = 100;
    int ty = 4;
    draw_length_parms("Current", &current_length_parameters, &tx, &ty);
    draw_length_parms("Best", &best_length_parameters, &tx, &ty);

    opt_step_number++;
}

// Run one frame of the angle optimizer process, and display
// the results.
void draw_scene_angle_optimizer() {
    setup_parameters_for_angle_optimizer();

    begin_line_mode(1, 1, 1);

    const float ALPHA_RESOLUTION = 20;
    assert(ALPHA_RESOLUTION > 1);

    int k;
    for (k = 0; k < ALPHA_RESOLUTION; k++) {
        float perc = (k + 1) / (double)(ALPHA_RESOLUTION);
        current_parameters.alpha.value = perc * 0.5 * M_PI;
        draw_theta_distortion(120, 220, 400, 400, 10, &current_parameters, false);
    }
 
   draw_theta_distortion(120, 220, 400, 400, 10, &current_parameters, true);

//    draw_length_distortion(120, 220, 400, 400, 10);
    end_line_mode();

    double error = current_parameters.total_squared_error;
    error /= (double)current_parameters.total_error_counter;
    error = sqrt(error);

    double max_error = current_parameters.max_squared_error;
    max_error /= (double)current_parameters.max_error_counter;
    max_error = sqrt(max_error);

    char buf[1000];
    int x = 100;
    int y = 0;
    begin_text_mode();
    sprintf(buf, "Error: %.7f", error);
    draw_text_line(&x, &y, buf);
    sprintf(buf, "Max Error: %.7f", max_error);
    draw_text_line(&x, &y, buf);

    if ((opt_step_number == 0) && (opt_phase_index == 0)) best_parameters = current_parameters;

    double best_error = best_parameters.total_squared_error;
    best_error /= (double)best_parameters.total_error_counter;
    best_error = sqrt(best_error);
    sprintf(buf, "Best Error: %.7f", best_error);
    draw_text_line(&x, &y, buf);

    draw_text_line(&x, &y, "");

    if (error < best_error) {
        best_parameters = current_parameters;
    }

    draw_parms("Current:", &current_parameters, &x, &y);
    draw_parms("Best:", &best_parameters, &x, &y);

    end_text_mode();


    opt_step_number++;
}

// The point of draw_scene3 is just to compute the RMS error of
// the spline approximation, for which I acquired the "best" parameters.
// It only computes errors for the worst case (quaternions with
// an angle of pi/2 between them.)

void draw_scene3() {
    Optimization_Parameters parameters;
    parameters.alpha.init(M_PI * 0.5, 0.1);
    parameters.worst_case_slope.init(0.5855064, 1.0);
    parameters.slope_attenuation.init(0.8228677, 1.0);

    begin_line_mode(1, 1, 1);

    const float ALPHA_RESOLUTION = 20;
    assert(ALPHA_RESOLUTION > 1);

    draw_theta_distortion(120, 220, 400, 400, 10, &parameters, true);

    double error = parameters.total_squared_error;
    error /= (double)parameters.total_error_counter;
    error = sqrt(error);

    double max_error = parameters.max_squared_error;
    max_error /= (double)parameters.max_error_counter;
    max_error = sqrt(max_error);

    char buf[1000];
    int x = 100;
    int y = 0;
    begin_text_mode();
    sprintf(buf, "Error: %.7f", error);
    draw_text_line(&x, &y, buf);
    sprintf(buf, "Max Error: %.7f", max_error);
    draw_text_line(&x, &y, buf);

    end_text_mode();
}

// draw_scene_angle_optimizer_quaternion_version(.) is the
// same thing as draw_scene_angle_optimizer(.) except that
// it actually uses quaternions instead of the scalar metrics.
// The outcome is the same.
void draw_scene_angle_optimizer_quaternion_version() {
    begin_line_mode(1, 1, 1);

    const float ALPHA_RESOLUTION = 20;
    assert(ALPHA_RESOLUTION > 1);
 
    draw_theta_distortion_quaternion_version(120, 220, 400, 400, 10, &current_parameters, true);

    end_line_mode();

    double error = current_parameters.total_squared_error;
    error /= (double)current_parameters.total_error_counter;
    error = sqrt(error);

    double max_error = current_parameters.max_squared_error;
    max_error /= (double)current_parameters.max_error_counter;
    max_error = sqrt(max_error);

    char buf[1000];
    int x = 100;
    int y = 0;
    begin_text_mode();
    sprintf(buf, "Error: %.7f", error);
    draw_text_line(&x, &y, buf);
    sprintf(buf, "Max Error: %.7f", max_error);
    draw_text_line(&x, &y, buf);

    if ((opt_step_number == 0) && (opt_phase_index == 0)) best_parameters = current_parameters;

    double best_error = best_parameters.total_squared_error;
    best_error /= (double)best_parameters.total_error_counter;
    best_error = sqrt(best_error);
    sprintf(buf, "Best Error: %.7f", best_error);
    draw_text_line(&x, &y, buf);

    draw_text_line(&x, &y, "");

    if (error < best_error) {
        best_parameters = current_parameters;
    }

    draw_parms("Current:", &current_parameters, &x, &y);
    draw_parms("Best:", &best_parameters, &x, &y);

    end_text_mode();


    opt_step_number++;
}

// Figure out which scene to draw!
void draw_scene() {
    switch (app_mode) {
    case APP_MODE_ANGLE_OPTIMIZER:
    default:
        draw_scene_angle_optimizer();
        break;
    case APP_MODE_LENGTH_OPTIMIZER:
        draw_scene_length_optimizer();
        break;
    case APP_MODE_QUASI_SLERP_TEST:
        draw_scene_quasi_slerp_test();
        break;
    case APP_MODE_FIGURE_1:
    case APP_MODE_FIGURE_2:
    case APP_MODE_FIGURE_3:
    case APP_MODE_FIGURE_4:
    case APP_MODE_FIGURE_5:
        draw_figure(app_mode - APP_MODE_FIGURE_1 + 1);
        break;
    }
}

// Do stuff based on key presses.
void handle_keydown(int key) {
    if (key == 'A') {
        app_mode = APP_MODE_ANGLE_OPTIMIZER;
        init_angle_optimizer();
    }

    if (key == 'L') {
        app_mode = APP_MODE_LENGTH_OPTIMIZER;
        init_length_optimizer();
    }

    if (key == 'Q') {
        app_mode = APP_MODE_QUASI_SLERP_TEST;
        init_quasi_slerp_test();
    }

    if (key == 'R') {
        if (app_mode == APP_MODE_QUASI_SLERP_TEST) {
            reroll_quasi_slerp_test();
        }
    }

    if (key == '1') {
        app_mode = APP_MODE_FIGURE_1;
    }

    if (key == '2') {
        app_mode = APP_MODE_FIGURE_2;
    }

    if (key == '3') {
        app_mode = APP_MODE_FIGURE_3;
    }

    if (key == '4') {
        app_mode = APP_MODE_FIGURE_4;
    }

    if (key == '5') {
        app_mode = APP_MODE_FIGURE_5;
    }
}
