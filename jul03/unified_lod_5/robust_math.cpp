#include "framework.h"

#include <complex>
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
            result->solutions[1] = solution;
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


void solve_cubic(double c3, double c2, double c1, double c0,
                 Solution_Set *result) {
    assert(c3 != 0);
    double ic3 = 1.0f / c3;
    c2 *= ic3;
    c1 *= ic3;
    c0 *= ic3;

    c3 = 1;

    // Technique as described in Numerical Recipes.
/*
    double Q = (c2*c2 - 3*c1) / 9.0;
    double R = (2*c2*c2*c2 - 9*c2*c1 + 27*c0) / 54.0;

    // For now assume 3 roots.

    assert(R*R < Q*Q*Q);

    double ratio = R / sqrt(Q*Q*Q);
    double theta = acos
*/

    double r = c2;
    double s = c1;
    double t = c0;

    double p = s - r * r / 3.0;
    double q = 2.0 * r * r * r / 27.0 - r * s / 3.0 + t;
    
    std::complex <double> D = p * p * p / 27.0 + q * q / 4.0;
    std::complex <double> sqrtD = std::sqrt(D);

    double mq2 = -q / 2.0;
    
    std::complex<double> high = mq2 + sqrtD;
    std::complex<double> low = mq2 - sqrtD;

    // Unfortunately, the std::complex that comes with gcc does not
    // let us just set the real part to 0.  So I do this tedious
    // thing wherein I construct a new complex number with the desired
    // imaginary part.  This is the kind of thing that makes me
    // want to slap sense into C++ dogmatists.
    if (low.real() < 0) low = std::complex <double> (0.0, low.imag());
    if (high.real() < 0) high = std::complex <double> (0.0, high.imag());

    std::complex<double> u = std::pow(high, 1/3.0);
    std::complex<double> v = std::pow(low, 1/3.0);

    const double SQRT3 = sqrt(3.0);
    std::complex<double> epsilon1(-0.5, SQRT3 * 0.5);
    std::complex<double> epsilon2 = std::conj(epsilon1);

    result->solutions[0] = (u + v).real();
    result->solutions[1] = (epsilon1 * u + epsilon2 * v).real();
    result->solutions[2] = (epsilon2 * u + epsilon1 * v).real();

    result->solutions[0] -= r/3.0;
    result->solutions[1] -= r/3.0;
    result->solutions[2] -= r/3.0;

    result->num_solutions = 3;
}
