#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "app_shell/app_shell.h"
#include "covariance.h"
#include "robust_math.h"

void Covariance2::reset() {
	a = b = c = 0;
}

Covariance2 Covariance2::invert() {
    double det = a*c - b*b;
    double factor = 1.0 / det;

    Covariance2 result;
    result.a = c * factor;
    result.b = -b * factor;
    result.c = a * factor;

    return result;
}

Covariance2 Covariance2::add(const Covariance2 &other) {
    Covariance2 result;
    result.a = a + other.a;
    result.b = b + other.b;
    result.c = c + other.c;

    return result;
}

void Covariance2::scale(float factor) {
	a *= factor;
	b *= factor;
	c *= factor;
}

void Covariance2::rotate(float theta) {
	double s = sin(theta);
	double t = cos(theta);

	float a_prime = a*t*t + b*2*s*t + c*s*s;
	float b_prime = -a*s*t + b*(t*t - s*s) + c*s*t;
	float c_prime = a*s*s - b*2*s*t + c*t*t;

	a = a_prime;
	b = b_prime;
	c = c_prime;
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

// The Covariance2 eigenvector path is completely separate (I wrote
// it first, and it was simpler to just crank out).  

int Covariance2::find_eigenvalues(float eigenvalues[2]) {
    double qa, qb, qc;
    qa = 1;
    qb = -(a + c);
    qc = a * c - b * b;

    Solution_Set solution;
    solve_quadratic_where_discriminant_is_known_to_be_nonnegative(qa, qb, qc, &solution);

    // If there's only one solution, explicitly state it as a
    // double eigenvalue.
    if (solution.num_solutions == 1) {
        solution.solutions[1] = solution.solutions[0];
        solution.num_solutions = 2;
    }

    eigenvalues[0] = solution.solutions[0];
    eigenvalues[1] = solution.solutions[1];

    if (eigenvalues[0] < 0) eigenvalues[0] = 0;
    if (eigenvalues[1] < 0) eigenvalues[1] = 0;

    if (eigenvalues[0] < eigenvalues[1]) {
        float tmp = eigenvalues[1];
        eigenvalues[1] = eigenvalues[0];
        eigenvalues[0] = tmp;
    }

    assert(eigenvalues[0] >= eigenvalues[1]);
    return solution.num_solutions;
}

int Covariance2::find_eigenvectors(float eigenvalues[2], Vector3 eigenvectors[2]) {
    int num_eigenvalues = find_eigenvalues(eigenvalues);
    assert(num_eigenvalues == 2);

    // Now that we have the quadratic coefficients, find the eigenvectors.

    const double VANISHING_EPSILON = 1.0e-5;
    const double SAMENESS_LOW = 0.9999;
    const double SAMENESS_HIGH = 1.0001;

    bool punt = false;
    const double A_EPSILON = 0.0000001;
    if (a < A_EPSILON) {
        punt = true;
    } else {
        double ratio = fabs(eigenvalues[1] / eigenvalues[0]);
        if ((ratio > SAMENESS_LOW) && (ratio < SAMENESS_HIGH)) punt = true;
    }

    if (punt) {
        if (a < 0) a = 0;
        eigenvalues[0] = a;
        eigenvalues[1] = a;

        eigenvectors[0].set(1, 0, 0);
        eigenvectors[1].set(0, 1, 0);
        num_eigenvalues = 2;
        return num_eigenvalues;
    }

    int j;
    for (j = 0; j < num_eigenvalues; j++) {
        double lambda = eigenvalues[j];

        Vector3 result1, result2;
        result1.set(-b, a - lambda, 0);
        result2.set(-(c - lambda), b, 0);

        Vector3 result;
        if (result1.length_squared() > result2.length_squared()) {
            result = result1;
        } else {
            result = result2;
        }

        result.normalize();
        eigenvectors[j] = result;
    }

    assert(num_eigenvalues == 2);
    assert(eigenvalues[0] >= 0);
    assert(eigenvalues[1] >= 0);

    return num_eigenvalues;
}

void Covariance2::translate(Covariance2 *dest, float kx, float ky) {
    dest->a = a - kx*kx;
    dest->b = b - kx*ky;
    dest->c = c - ky*ky;
}

void Covariance2::move_to_global_coordinates(Covariance2 *dest, float x, float y) {
    dest->a = a + x*x;
    dest->b = b + x*y;
    dest->c = c + y*y;
}

void Covariance2::move_to_local_coordinates(Covariance2 *dest, float x, float y) {
    dest->a = a - x*x;
    dest->b = b - x*y;
    dest->c = c - y*y;
}

