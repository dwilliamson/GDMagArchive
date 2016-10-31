#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "robust_math.h"

void solve_quadratic(double a, double b, double c, 
                     Solution_Set *result) {

    if (a == 0.0) {  // Then bx + c = 0; thus, x = -c / b
        if (b == 0.0) {
            result->num_solutions = 0;
            return;
        }

        result->solutions[0] = -c / b;
        result->num_solutions = 1;
        return;
    }

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0) {
        result->num_solutions = 0;
        return;
    }

    double sign_b = 1.0;
    if (b < 0.0) sign_b = -1.0;

    int nroots = 0;
    double q = -0.5 * (b + sign_b * sqrt(discriminant));
    
    nroots++;
    result->solutions[0] = q / a;

    if (q != 0.0) {
        double solution = c / q;
        if (solution != result->solutions[0]) {
            nroots++;
            if (solution > result->solutions[0]) {
                result->solutions[1] = solution;
            } else {
                result->solutions[1] = result->solutions[0];
                result->solutions[0] = solution;
            }
        }
    }

    result->num_solutions = nroots;
}

void solve_quadratic_where_discriminant_is_known_to_be_nonnegative(double a, double b, double c, 
                     Solution_Set *result) {

    if (a == 0.0) {  // Then bx + c = 0; thus, x = -c / b
        if (b == 0.0) {
            result->num_solutions = 0;
            return;
        }

        result->solutions[0] = -c / b;
        result->num_solutions = 1;
        return;
    }

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0) discriminant = 0.0;

    double sign_b = 1.0;
    if (b < 0.0) sign_b = -1.0;

    int nroots = 0;
    double q = -0.5 * (b + sign_b * sqrt(discriminant));
    
    nroots++;
    result->solutions[0] = q / a;

    if (q != 0.0) {
        double solution = c / q;
        if (solution != result->solutions[0]) {
            nroots++;
            result->solutions[1] = solution;
        }
    }

    result->num_solutions = nroots;
}


