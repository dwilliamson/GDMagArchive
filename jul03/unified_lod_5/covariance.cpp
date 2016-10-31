#include "framework.h"
#include "covariance.h"
#include "robust_math.h"
#include <math.h>

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

void Covariance3::scale(float factor) {
    a *= factor;
    b *= factor;
    c *= factor;
    d *= factor;
    e *= factor;
    f *= factor;
}

Covariance3 Covariance3::add(const Covariance3 &other) {
    Covariance3 result;
    result.a = a + other.a;
    result.b = b + other.b;
    result.c = c + other.c;
    result.d = d + other.d;
    result.e = e + other.e;
    result.f = f + other.f;

    return result;
}

void Covariance3::accumulate(const Vector3 &v) {
    float x = v.x;
    float y = v.y;
    float z = v.z;

    a += x*x;
    b += x*y;
    c += x*z;
    d += y*y;
    e += y*z;
    f += z*z;
}

void Covariance3::reset() {
    a = b = c = d = e = f = 0;
}

void copy_matrix(double *dest, double *src, int ni, int nj) {
    int i, j;
    for (j = 0; j < nj; j++) {
        for (i = 0; i < ni; i++) {
            int index = j * ni + i;
            dest[index] = src[index];
        }
    }
}

void copy_matrix(double dest[3][3], double src[3][3], int ni, int nj) {
    int i, j;
    for (j = 0; j < nj; j++) {
        for (i = 0; i < ni; i++) {
            dest[i][j] = src[i][j];
        }
    }
}

const int ARRAY_SIZE = 3;
void swap_rows(double A[3][3], int row_pivot[3], int column_pivot[3], 
               int i0, int i1) {
/*
    int j;
    for (j = 0; j < ARRAY_SIZE; j++) {
        double tmp = A[row_pivot[i0]][column_pivot[j]];
        A[row_pivot[i0]][column_pivot[j]] = A[row_pivot[i1]][column_pivot[j]];
        A[row_pivot[i1]][column_pivot[j]] = tmp;
    }
*/
    int tmp = row_pivot[i0];
    row_pivot[i0] = row_pivot[i1];
    row_pivot[i1] = tmp;
}

void swap_columns(double A[3][3], int row_pivot[3], int column_pivot[3], 
                  int j0, int j1) {
/*
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) {
        double tmp = A[row_pivot[i]][column_pivot[j0]];
        A[row_pivot[i]][column_pivot[j0]] = A[row_pivot[i]][column_pivot[j1]];
        A[row_pivot[i]][column_pivot[j1]] = tmp;
    }
*/

    int tmp = column_pivot[j0];
    column_pivot[j0] = column_pivot[j1];
    column_pivot[j1] = tmp;
}

void add_rows(double A[3][3], int row_pivot[3], int column_pivot[3], 
              int dest_row, int src_row,
              double factor) {
    int j;
    for (j = 0; j < ARRAY_SIZE; j++) {
        A[row_pivot[dest_row]][column_pivot[j]] += factor * A[row_pivot[src_row]][column_pivot[j]];
    }
}

void simple_find_nullspace33(double A[3][3], double nullspace[3]) {
    int row_pivot[ARRAY_SIZE];
    int column_pivot[ARRAY_SIZE];
    int i, j;
    for (i = 0; i < ARRAY_SIZE; i++) {
        row_pivot[i] = i;
        column_pivot[i] = i;
    }

    // Do Gauss-Jordan thingy.  First step: make it upper-triangular.
    for (j = 0; j < ARRAY_SIZE - 1; j++) {
        // Find the highest coefficient...
        double best_magnitude = 0;
        int best_i = -1;
        int best_j = -1;

        int ii, jj;
        for (ii = j; ii < ARRAY_SIZE; ii++) {
            for (jj = j; jj < ARRAY_SIZE; jj++) {

                float magnitude = fabs(A[row_pivot[ii]][column_pivot[jj]]);
                if (magnitude > best_magnitude) {
                    best_i = ii;
                    best_j = jj;
                    best_magnitude = magnitude;
                }
            }
        }

        assert(best_i != -1);
        assert(best_j != -1);

        // If it's not what we want to work on, swap the rows.
        if (best_i != j) swap_rows(A, row_pivot, column_pivot, j, best_i);
        if (best_j != j) swap_columns(A, row_pivot, column_pivot, j, best_j);

        // Reduce the current row.
        double element = A[row_pivot[j]][column_pivot[j]];
        assert(element != 0);
        double factor = 1.0 / element;
        A[row_pivot[j]][column_pivot[j]] = 1.0;

        for (jj = j + 1; jj < ARRAY_SIZE; jj++) A[row_pivot[j]][column_pivot[jj]] *= factor;

        // Now we go downward and do row subtractions.
        // Technically we don't need to subtract the whole row,
        // since the beginning is zeroes... but for now we'll just do it.
        for (i = j + 1; i < ARRAY_SIZE; i++) {
            add_rows(A, row_pivot, column_pivot,
                     i, j, -A[row_pivot[i]][column_pivot[j]]);
            A[row_pivot[i]][column_pivot[j]] = 0;
        }
    }

    // XXX Check here to make sure we have epsilon in the corner.

    // Right now there is only one up-subtract, so don't even bother
    // to make a loop.

    double factor = -A[row_pivot[0]][column_pivot[1]];
    add_rows(A, row_pivot, column_pivot, 0, 1, factor);

    nullspace[column_pivot[0]] = A[row_pivot[0]][column_pivot[2]];
    nullspace[column_pivot[1]] = A[row_pivot[1]][column_pivot[2]];
    nullspace[column_pivot[2]] = -1;
}

void test(double *values, int n0, int n1) {
    if (values[n1] > values[n0]) {
        double tmp = values[n0];
        values[n0] = values[n1];
        values[n1] = tmp;
    }
}

void sort_eigenvalues(double *values, int num_values) {
    assert(num_values == 3);

    test(values, 0, 1);
    test(values, 0, 2);
    test(values, 1, 2);
}

int Covariance3::find_eigenvalues(float eigenvalues[3]) {
    double c3 = -1;
    double c2 = a + d + f;
    double c1 = -(d*f + a*f + a*d) + (b*b + c*c + e*e);
    double c0 = a*d*f - a*e*e - b*b*f + 2*b*c*e - c*c*d;

    Solution_Set solution_set;
    solve_cubic(c3, c2, c1, c0, &solution_set);

    sort_eigenvalues(solution_set.solutions, 3);

    eigenvalues[0] = solution_set.solutions[0];
    eigenvalues[1] = solution_set.solutions[1];
    eigenvalues[2] = solution_set.solutions[2];

    // XXX Verify that solution_set.solutions[n] won't have garbage.

    return solution_set.num_solutions;
}

void init_Ax(double A[3][3], Vector3 x, double result[3]) {
    result[0] = A[0][0] * x.x + A[0][1] * x.y + A[0][2] * x.z;
    result[1] = A[1][0] * x.x + A[1][1] * x.y + A[1][2] * x.z;
    result[2] = A[2][0] * x.x + A[2][1] * x.y + A[2][2] * x.z;
}

void init_Lx(double eigenvalue, Vector3 x, double result[3]) {
    result[0] = x.x * eigenvalue;
    result[1] = x.y * eigenvalue;
    result[2] = x.z * eigenvalue;
}

void test_eigenvectors(double A[3][3], Vector3 eigenvectors[3], float eigenvalues[3]) {
    double Ax0[3];
    double Ax1[3];
    double Ax2[3];
    double Lx0[3];
    double Lx1[3];
    double Lx2[3];
    
    init_Lx(eigenvalues[0], eigenvectors[0], Lx0);
    init_Lx(eigenvalues[1], eigenvectors[1], Lx1);
    init_Lx(eigenvalues[2], eigenvectors[2], Lx2);

    init_Ax(A, eigenvectors[0], Ax0);
    init_Ax(A, eigenvectors[1], Ax1);
    init_Ax(A, eigenvectors[2], Ax2);
}
    
int Covariance3::find_eigenvectors(float eigenvalues[3], Vector3 eigenvectors[3]) {
    int num_eigenvalues = find_eigenvalues(eigenvalues);

    if (eigenvalues[0] == 0) {
        eigenvectors[0].set(1, 0, 0);
        eigenvectors[1].set(0, 1, 0);
        eigenvectors[2].set(0, 0, 1);
        return 0;
    }

    // Now find the eigenvectors corresponding to each eigenvalue.

    // Fill in an array.
    double A[3][3];
    double A0[3][3];
    double A1[3][3];
    double A2[3][3];

    A[0][0] =  a;
    A[0][1] =  b;
    A[0][2] =  c;
    A[1][1] =  d;
    A[1][2] =  e;
    A[2][2] =  f;

    A[1][0] = A[0][1];
    A[2][0] = A[0][2];
    A[2][1] = A[1][2];

    copy_matrix(A0, A, 3, 3);
    copy_matrix(A1, A, 3, 3);
    copy_matrix(A2, A, 3, 3);

    A0[0][0] -= eigenvalues[0];
    A0[1][1] -= eigenvalues[0];
    A0[2][2] -= eigenvalues[0];

    A1[0][0] -= eigenvalues[1];
    A1[1][1] -= eigenvalues[1];
    A1[2][2] -= eigenvalues[1];

    A2[0][0] -= eigenvalues[2];
    A2[1][1] -= eigenvalues[2];
    A2[2][2] -= eigenvalues[2];

    double null0[3];
    simple_find_nullspace33(A0, null0);
    eigenvectors[0].set(null0[0], null0[1], null0[2]);
    eigenvectors[0].normalize();

    if (eigenvalues[1] == 0) {
        Vector3 vec1(1, 0, 0);
        Vector3 vec2(0, 1, 0);
        float dot1 = dot_product(eigenvectors[0], vec1);
        float dot2 = dot_product(eigenvectors[0], vec2);
        Vector3 use_me;
        if (fabs(dot1) > fabs(dot2)) {
            use_me = vec1;
        } else {
            use_me = vec2;
        }

        float dot = dot_product(use_me, eigenvectors[0]);
        Vector3 to_subtract = eigenvectors[0];
        to_subtract.scale(dot);
        use_me = use_me - to_subtract;
        use_me.normalize();
        eigenvectors[1] = use_me;
    } else {
        double null1[3];
        simple_find_nullspace33(A1, null1);
        eigenvectors[1].set(null1[0], null1[1], null1[2]);
        eigenvectors[1].normalize();
    }

    if (eigenvalues[2] == 0) {
        eigenvectors[2] = cross_product(eigenvectors[0], eigenvectors[1]);
    } else {
        double null2[3];
        simple_find_nullspace33(A2, null2);
        eigenvectors[2].set(null2[0], null2[1], null2[2]);
        eigenvectors[2].normalize();
    }

    // test_eigenvectors(A, eigenvectors, eigenvalues);

    return num_eigenvalues;
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
    return solution.num_solutions;
}

int Covariance2::find_eigenvectors(float eigenvalues[2], Vector2 eigenvectors[2]) {
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
        eigenvalues[0] = a;
        eigenvalues[1] = a;

        eigenvectors[0].set(1, 0);
        eigenvectors[1].set(0, 1);
        num_eigenvalues = 2;
        return num_eigenvalues;
    }

    int j;
    for (j = 0; j < num_eigenvalues; j++) {
        double lambda = eigenvalues[j];

        Vector2 result1, result2;
        result1.set(-b, a - lambda);
        result2.set(-(c - lambda), b);

        Vector2 result;
        if (result1.length_squared() > result2.length_squared()) {
            result = result1;
        } else {
            result = result2;
        }

        result.normalize();
        eigenvectors[j] = result;
    }

    return num_eigenvalues;
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

void Covariance3::move_to_global_coordinates(Covariance3 *dest, const Vector3 &pos) {
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    dest->a = a + x*x;
    dest->b = b + x*y;
    dest->c = c + x*z;
    dest->d = d + y*y;
    dest->e = e + y*z;
    dest->f = f + z*z;
}

void Covariance3::move_to_local_coordinates(Covariance3 *dest, const Vector3 &pos) {
    float x = pos.x;
    float y = pos.y;
    float z = pos.z;

    dest->a = a - x*x;
    dest->b = b - x*y;
    dest->c = c - x*z;
    dest->d = d - y*y;
    dest->e = e - y*z;
    dest->f = f - z*z;
}

void find_covariance_of_convex_polygon(Vector3 *vertices, int num_vertices,
									   Covariance2 *result) {
	Covariance2 sum;
	sum.reset();

	int i;
	for (i = 1; i < num_vertices - 1; i++) {
		int n0 = 0;
		int n1 = i;
		int n2 = i + 1;

		Vector3 p0 = vertices[n0];
		Vector3 p1 = vertices[n1];
		Vector3 p2 = vertices[n2];

		Covariance2 single_term;
		find_covariance_of_triangle(p0, p1, p2, &single_term);
		sum = sum.add(single_term);
	}

	*result = sum;
}

void find_covariance_of_triangle(const Vector3 &p0, const Vector3 &p1,
								 const Vector3 &p2, Covariance2 *result) {
	assert(p0.z == 0);
	assert(p1.z == 0);
	assert(p2.z == 0);
}
