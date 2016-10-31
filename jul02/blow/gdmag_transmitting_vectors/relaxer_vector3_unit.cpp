#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "os_specific_opengl_headers.h"

#include "app_shell.h"
#include "relaxer_vector3_unit.h"

Relaxer_Vector3_Unit::Relaxer_Vector3_Unit() {
	num_vectors = 0;
	vectors = NULL;
	workspace = NULL;
}

Relaxer_Vector3_Unit::~Relaxer_Vector3_Unit() {
	if (vectors) delete [] vectors;
	if (workspace) delete [] workspace;
}

float random_scalar_from_minus_one_to_one() {
	int val = rand() % RAND_MAX;
	float result = val / (float)RAND_MAX;
	result -= 0.5f;
	result *= 2.0f;

	return result;
}

Vector3 random_unit_vector() {
	// XXX Here is an unquarantined random number generator.
	// If this were to be put into a production game, we would
	// want to replace it with a random number generator that does
	// not pollute global state, so that this code behaves in a
	// more modular and predictable fashion.

	// Also please note that this function does not produce
	// evenly-distributed unit vectors.  That doesn't matter much
	// though, because we're just feeding them into something that
	// causes them to become evenly distributed.

	float x = random_scalar_from_minus_one_to_one();
	float y = random_scalar_from_minus_one_to_one();
	float z = random_scalar_from_minus_one_to_one();

	Vector3 result(x, y, z);
	result.normalize();
	return result;
}

void Relaxer_Vector3_Unit::init(int _num_vectors) {
    num_vectors = _num_vectors;

	assert(vectors == NULL);
	vectors = new Vector3[num_vectors];
	workspace = new Vector3[num_vectors];
	
	int i;
	for (i = 0; i < num_vectors; i++) {
		vectors[i] = random_unit_vector();

		bool found_duplicate = false;
		int j;
		for (j = 0; j < i; j++) {
			// If this vector is too close to someone we already
			// created, we need to reject it and try again.  This
			// prevents us from picking two vectors that are exactly
			// the same; such vectors would never diverge, and that
			// would be trouble!

			// This search for duplicate vectors is slow now, but
			// it can easily be optimized!
			
			const float DOT_PRODUCT_MAX = 0.999f;
			float dot = dot_product(vectors[i], vectors[j]);
			if (dot > DOT_PRODUCT_MAX) {
				found_duplicate = true;
				break;
			}
		}

		if (found_duplicate) i--;
	}
}

double Relaxer_Vector3_Unit::do_one_vector(int index, double factor) {
    Vector3 accumulator;
    accumulator.x = 0;
    accumulator.y = 0;
    accumulator.z = 0;

	Vector3 vector = vectors[index];
	
	const double GAMMA = 1.0 / (1.0 * M_PI);

    int i;
    for (i = 0; i < num_vectors; i++) {
        if (i == index) continue;

		float dot = dot_product(vectors[i], vector);

		if (dot < -1) dot = -1;
		if (dot > 1) dot = 1;
		double angle = acos(dot);
		double parameter = 1.0 / (1.0 + angle * GAMMA);
		double forcemag = parameter * parameter * factor;

		Vector3 delta = vector.subtract(vectors[i]);
		if (delta.length() > 0) delta.normalize();
		delta.scale(forcemag);

		accumulator = accumulator.add(delta);
    }

    accumulator.normalize();

	// Subtract out the parallel component of 'accumulator', to get
	// only the orthogonal component.  Add this to the input vector,
	// then renormalize it.

    float dot = dot_product(accumulator, vector);
    Vector3 parallel = vector;
    parallel.scale(dot);
    accumulator = accumulator.subtract(parallel);

	vector = vector.add(accumulator);
	vector.normalize();

	// Write the result into the workspace array.  Later, this will
	// be copied back into the vectors array.  We're doing this because
	// we don't want to be changing the vectors in the middle of the
	// timestep, as that makes things harder to reason about.  (Though
	// the system would probably converge faster, actually).

	workspace[index] = vector;

	// Return the length by which we perturbed the vector.  When this
	// length reaches some very small value for all vectors, we know
	// it's probably time to stop.

    double len = accumulator.length();
    return len;
}

double Relaxer_Vector3_Unit::do_one_timestep(double dt) {
	double max_perturbation = 0;
	
	int i;
	for (i = 0; i < num_vectors; i++) {
		double perturbation = do_one_vector(i, dt);
		if (perturbation > max_perturbation) max_perturbation = perturbation;
	}
	
	// Copy the workspace results back into our original thingy.

	for (i = 0; i < num_vectors; i++) vectors[i] = workspace[i];

	return max_perturbation;
}

void Relaxer_Vector3_Unit::relax(double dt) {
	const double THRESHOLD = 0.0001;
	while (1) {
		double max_perturbation = do_one_timestep(dt);
		if (max_perturbation < THRESHOLD) break;
	}
}

void Relaxer_Vector3_Unit::relax_and_draw(double dt) {
    do_one_timestep(dt);

    float w = app_shell->screen_width * 0.3;

    app_shell->line_mode_begin();
	//    app_shell->init_modelview_transform();

    glColor3f(1, 1, 1);

    int i;
    for (i = 0; i < num_vectors; i++) {
        Vector3 vector = vectors[i];

		float px = vector.x;
		float py = vector.y;
		
        glBegin(GL_LINE_STRIP);
        glVertex2f(w, w);
        glVertex2f(px*w + w, py*w + w);
        glEnd();
    }

    app_shell->line_mode_end();
}

int Relaxer_Vector3_Unit::encode(Vector3 input) {
	int best_index = -1;
	float best_dot = -2;

	int i;
	for (i = 0; i < num_vectors; i++) {
		float dot = dot_product(input, vectors[i]);
		if (dot > best_dot) {
			best_dot = dot;
			best_index = i;
		}
	}

	assert(best_index >= 0);
	assert(best_index < num_vectors);
	return best_index;
}

Vector3 Relaxer_Vector3_Unit::decode(int index) {
	return vectors[index];
}
