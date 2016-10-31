const int SS_MAX_SOLUTIONS = 3;

struct Solution_Set {
    int num_solutions;
    double solutions[SS_MAX_SOLUTIONS];
};

void solve_quadratic(double a, double b, double c, Solution_Set *solution);
void solve_cubic(double a, double b, double c, double d, Solution_Set *solution);
