#include "../framework.h"
#include "error_quadric.h"
#include <math.h>

int Error_Quadric::QuadricMatrixIndexTable[ERROR_QUADRIC_DIMENSIONS * ERROR_QUADRIC_DIMENSIONS];

void Error_Quadric::clear() {
    int i;
    for (i = 0; i < ERROR_QUADRIC_MATRIX_ENTRIES; i++) matrix[i] = 0;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) vector[i] = 0;
    scalar = 0;
}

void Error_Quadric::accumulate_quadric(Error_Quadric *other) {
    int i;
    for (i = 0; i < ERROR_QUADRIC_MATRIX_ENTRIES; i++) {
        matrix[i] += other->matrix[i];
    }

    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        vector[i] += other->vector[i];
    }

    scalar += other->scalar;
}

double outer_product_coef(double *v0, double *v1,
                          int i, int j) {
    return v0[i] * v1[j];
}

inline double kroenecker(int i, int j) {
    if (i == j) return 1.0;
    return 0.0;
}

void Error_Quadric::accumulate_plane(float *p0, float *p1,
                                     float *p2, float extra_scale) {

    // e1 and e2 will be orthogonal basis vectors that define
    // a 2D plane in n-space.

    // At first e2 is not orthogonal; we build it preliminarily
    // and then use the dot product with e1 to make it orthogonal.

    double e1[ERROR_QUADRIC_DIMENSIONS];
    double e2[ERROR_QUADRIC_DIMENSIONS];
    
    double e1_len_squared = 0;
    double e2_len_squared = 0;

    int i;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        e1[i] = p1[i] - p0[i];
        e2[i] = p2[i] - p0[i];

        e1_len_squared += e1[i] * e1[i];
    }

    // If this is degenerate, just bail.
    if (e1_len_squared == 0) return;

    // Compute the dot product so we can orthonormalize e2.
    double e1_dot_e2 = 0;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        e1[i] /= sqrt(e1_len_squared);
        e1_dot_e2 += e1[i] * e2[i];
    }

    // Make e2 orthogonal to e1.
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        e2[i] -= e1[i] * e1_dot_e2;
        e2_len_squared += e2[i] * e2[i];
    }

    if (e2_len_squared == 0) return;

    // Normalize e2.

    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        e2[i] /= sqrt(e2_len_squared);
    }

    // Compute p0_dot_e1 and p0_dot_e2

    double p0_dot_e1 = 0;
    double p0_dot_e2 = 0;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        p0_dot_e1 += p0[i] * e1[i];
        p0_dot_e2 += p0[i] * e2[i];
    }

    // Compute pperp, and add it to the vector part.
    // Compute the squared length of pperp and add it to the scalar part.
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        double pperp_i = -p0[i] + (p0_dot_e1 * e1[i]) + (p0_dot_e2 * e2[i]);

        vector[i] += pperp_i * extra_scale;

//        scalar += pperp_i * pperp_i;
        scalar += (p0[i] * p0[i]) * extra_scale;
        scalar -= (p0_dot_e1 * p0_dot_e1 * e1[i] * e1[i]) * extra_scale;
        scalar -= (p0_dot_e2 * p0_dot_e2 * e2[i] * e2[i]) * extra_scale;
    }

    // Compute the matrix part.
    int j;
    int k = 0;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        for (j = i; j < ERROR_QUADRIC_DIMENSIONS; j++) {
            double coef1 = outer_product_coef(e1, e1, i, j);
            double coef2 = outer_product_coef(e2, e2, i, j);
            double I = kroenecker(i, j);
            matrix[k] += (I - coef1 - coef2) * extra_scale;

            assert(k == QuadricMatrixIndexTable[j * ERROR_QUADRIC_DIMENSIONS + i]);
            k++;
        }
    }

    assert(k == ERROR_QUADRIC_MATRIX_ENTRIES);
}

inline double Error_Quadric::get_matrix_value(int i, int j) {
    int index = QuadricMatrixIndexTable[j * ERROR_QUADRIC_DIMENSIONS + i];
    return matrix[index];
}

void Error_Quadric::init_index_table() {
    int i, j, k;

    k = 0;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        for (j = i; j < ERROR_QUADRIC_DIMENSIONS; j++) {
            QuadricMatrixIndexTable[j * ERROR_QUADRIC_DIMENSIONS + i] = k;
            QuadricMatrixIndexTable[i * ERROR_QUADRIC_DIMENSIONS + j] = k;

            k++;
        }
    }
}

double Error_Quadric::evaluate_error(float *v) {
    // Initialize result with scalar;
    double result = scalar;

    // Add 2 * b^t * v;

    int i;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        result += 2 * vector[i] * v[i];
    }

    // Add v^t * A * v
    
    // Let w = Av
    double w[ERROR_QUADRIC_DIMENSIONS];
    int j;
    for (i = 0; i < ERROR_QUADRIC_DIMENSIONS; i++) {
        double sum = 0;

        for (j = 0; j < ERROR_QUADRIC_DIMENSIONS; j++) {
            double coef = get_matrix_value(i, j);
            sum += coef * v[j];
        }

        w[i] = sum;
    }

    // Now add v dot w to result.

    for (j = 0; j < ERROR_QUADRIC_DIMENSIONS; j++) {
        result += v[j] * w[j];
    }

    return result;
}
