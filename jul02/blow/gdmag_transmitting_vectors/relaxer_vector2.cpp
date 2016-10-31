#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"

#include "app_shell.h"
#include "relaxer.h"
#include "relaxer_vector2.h"

extern bool should_draw_cells;

Relaxer_Vector2::Relaxer_Vector2() {
	num_vectors = 0;
	vectors = NULL;
	workspace = NULL;
}

Relaxer_Vector2::~Relaxer_Vector2() {
	if (vectors) delete [] vectors;
	if (workspace) delete [] workspace;
}

float random_scalar_from_zero_to_one() {
	int val = rand() % RAND_MAX;
	float result = val / (float)RAND_MAX;

	return result;
}

Vector3 random_vector_0_to_1_xy() {
	// XXX Here is an unquarantined random number generator.
	// If this were to be put into a production game, we would
	// want to replace it with a random number generator that does
	// not pollute global state, so that this code behaves in a
	// more modular and predictable fashion.

	float x = random_scalar_from_zero_to_one();
	float y = random_scalar_from_zero_to_one();
	float z = 0;

	Vector3 result(x, y, z);
	return result;
}

void Relaxer_Vector2::init(int _num_vectors) {
    num_vectors = _num_vectors;

	assert(vectors == NULL);
	vectors = new Vector3[num_vectors];
	workspace = new Vector3[num_vectors];
	
	int i;
	for (i = 0; i < num_vectors; i++) {
		vectors[i] = random_vector_0_to_1_xy();

		bool found_duplicate = false;
		int j;
		for (j = 0; j < i; j++) {
			// This search for duplicate vectors is slow now, but
			// it can easily be optimized!

			if (vectors[i].x != vectors[j].x) continue;
			if (vectors[i].y != vectors[j].y) continue;

			found_duplicate = true;
			break;
		}

		if (found_duplicate) i--;
	}
}

void do_one_force(const Vector3 &v0, const Vector3 &v1,
				  float ox, float oy, float factor,
				  Vector3 *accumulator) {

	Vector3 nv0(v0.x + ox, v0.y + oy, 0);

	Vector3 w = v1.subtract(nv0);
	double r2 = w.length_squared();

	if (r2 == 0) return;

	double force_magnitude = factor / r2;
	w.normalize();
	w.scale(force_magnitude);

	Vector3 force = w;
	
	*accumulator = accumulator->add(force);
}

double Relaxer_Vector2::do_one_vector(int index, double factor) {
    Vector3 accumulator(0, 0, 0);

	Vector3 vector = vectors[index];
	
	const double GAMMA = 1.0 / (1.0 * M_PI);

    int i;
    for (i = 0; i < num_vectors; i++) {
        if (i == index) continue;

		int stride = 5;
		int offset = -(stride / 2);
		int j, k;
		for (j = 0; j < stride; j++) {
			for (k = 0; k < stride; k++) {
				do_one_force(vectors[i], vector,
							 j + offset, k + offset, factor, &accumulator);
			}
		}
		/*
		  do_one_force(vectors[i], vector, -1,  0, factor, &accumulator);
		do_one_force(vectors[i], vector, -1, +1, factor, &accumulator);
		do_one_force(vectors[i], vector,  0, -1, factor, &accumulator);
		do_one_force(vectors[i], vector,  0,  0, factor, &accumulator);
		do_one_force(vectors[i], vector,  0, +1, factor, &accumulator);
		do_one_force(vectors[i], vector, +1, -1, factor, &accumulator);
		do_one_force(vectors[i], vector, +1,  0, factor, &accumulator);
		do_one_force(vectors[i], vector, +1, +1, factor, &accumulator);
		*/
    }

	const float ACCUMULATOR_LEN_MAX = 0.05f;
	if (accumulator.length() > ACCUMULATOR_LEN_MAX) {
		accumulator.normalize();
		accumulator.scale(ACCUMULATOR_LEN_MAX);
	}
	
	vector = vector.add(accumulator);


	if (vector.x < 0) vector.x += 1;
	if (vector.x > 1) vector.x -= 1;
	if (vector.y < 0) vector.y += 1;
	if (vector.y > 1) vector.y -= 1;
	/*
	if (vector.x < 0) vector.x = 0;
	if (vector.x > 1) vector.x = 1;
	if (vector.y < 0) vector.y = 0;
	if (vector.y > 1) vector.y = 1;
	*/
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

double Relaxer_Vector2::do_one_timestep(double dt) {
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

void Relaxer_Vector2::relax(double dt) {
	const double THRESHOLD = 0.0001;
	while (1) {
		double max_perturbation = do_one_timestep(dt);
		if (max_perturbation < THRESHOLD) break;
	}
}

void Relaxer_Vector2::relax_and_draw(double dt) {
    do_one_timestep(dt);

    app_shell->init_modelview_transform();

    glColor3f(1, 1, 1);

    int texture = spot_texture.texture_handle;

    float s = 6;
    float w = app_shell->screen_width * 0.5;

    app_shell->bitmap_mode_begin(texture);


    int i;
    for (i = 0; i < num_vectors; i++) {
        Vector3 vector = vectors[i];

        app_shell->draw_texture_quad(texture, 
                                     vector.x * w - s*.5, vector.y * w - s*.5,
                                     s, s);
	}
	
    app_shell->bitmap_mode_end();

	if (should_draw_cells) {
		app_shell->line_mode_begin();
		glColor3f(0, 1, 0);
		
		for (i = 0; i < num_vectors; i++) {
			Vector3 vector = vectors[i];

			float distance2_min = FLT_MAX;
			int j;
			for (j = 0; j < num_vectors; j++) {
				if (j == i) continue;
				float distance2 = distance_squared(vector, vectors[j]);
				if (distance2 < distance2_min) distance2_min = distance2;
			}

			const float FACTOR = 1.5f;
			for (j = 0; j < i; j++) {
				float distance2 = distance_squared(vector, vectors[j]);
				if (distance2 > FACTOR *  distance2_min) continue;

				Vector3 toward = vectors[j].subtract(vector);
				Vector3 perp(-toward.y, toward.x, 0);

				perp.scale(0.15f);
				toward.scale(0.5f);

				Vector3 p0 = vector.add(toward).subtract(perp);
				Vector3 p1 = vector.add(toward).add(perp);
			

				glBegin(GL_LINE_STRIP);
				glVertex2f(p0.x*w, p0.y*w);
				glVertex2f(p1.x*w, p1.y*w);
				glEnd();

			}
		}

		app_shell->line_mode_end();
    }

}

int Relaxer_Vector2::encode(Vector3 input) {
	assert(0);
	return 0;
}

Vector3 Relaxer_Vector2::decode(int index) {
	return vectors[index];
}
