/*
  This file provides functions that solve quadratic equations.
*/

const int SS_MAX_SOLUTIONS = 3;

struct Solution_Set {
    int num_solutions;
    double solutions[SS_MAX_SOLUTIONS];
};

// Solve the quadratic equation a*x*x + b*x + c = 0
// The results are placed in 'solution' (you must provide the storage for this)
void solve_quadratic(double a, double b, double c, Solution_Set *solution);
void solve_quadratic_where_discriminant_is_known_to_be_nonnegative(double a, double b, double c, Solution_Set *solution);
