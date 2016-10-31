#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <float.h>

struct Vector3 {
	Vector3(float x, float y, float z);
	Vector3();
	
	float x, y, z;

	float length();
	void normalize();
	void set(float x, float y, float z);
};



inline Vector3::Vector3() {
}

inline Vector3::Vector3(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline void Vector3::set(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline float dot_product(const Vector3 &a, const Vector3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline void Vector3::normalize() {
	float ilen = 1.0 / sqrt(x*x + y*y + z*z);
	x *= ilen;
	y *= ilen;
	z *= ilen;
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
	// evenly-distributed unit vectors.

	float x = random_scalar_from_minus_one_to_one();
	float y = random_scalar_from_minus_one_to_one();
	float z = random_scalar_from_minus_one_to_one();

	Vector3 result(x, y, z);
	result.normalize();
	return result;
}

bool load_vector_file(char *filename, Vector3 **vectors_return, int *num_vectors_return) {
    const int BUFFER_SIZE = 2048;
    char buf[BUFFER_SIZE];

    FILE *f = fopen(filename, "rt");
    if (f == NULL) return false;

    char *s;
    s = fgets(buf, BUFFER_SIZE, f);
    if (s == NULL) {
        fclose(f);
        return false;
    }

    int num_vectors = atoi(s);
    assert(num_vectors >= 0);

    Vector3 *vectors = new Vector3[num_vectors];

    int i;
    for (i = 0; i < num_vectors; i++) {
        s = fgets(buf, BUFFER_SIZE, f);
        if (s == NULL) {
            fclose(f);
            assert(0);
            return false;
        }

        float x, y, z;
        int success = sscanf(s, "%f %f %f", &x, &y, &z);
        assert(success == 3);

        vectors[i].set(x, y, z);
    }

    fclose(f);

    *num_vectors_return = num_vectors;
    *vectors_return = vectors;

    return true;
}

int find_best_vector(Vector3 input, Vector3 *vectors, int num_vectors) {
    float best_dot = -FLT_MAX;
    int best_index = -1;

    int i;
    for (i = 0; i < num_vectors; i++) {
        float dot = dot_product(input, vectors[i]);
        if (dot > best_dot) {
            best_dot = dot;
            best_index = i;
        }
    }

    return best_index;
}
        
void main(void) {
    // This test is kind of flawed... since it doesn't actually generate
    // unit vectors that are uniformly distributed, our error measure
    // isn't uniformly distributed either, which is bad.  But I am
    // just throwing this together as a fast cheap hack to make sure
    // that the vector maker thingy is doing remotely the right thing.

    Vector3 *vectors;
    int num_vectors;
    char *filename = "results_vector3_unit.txt";
    bool success = load_vector_file(filename, &vectors, &num_vectors);

    if (!success) {
        printf("Unable to load vector file '%s'!\n", filename);
        exit(1);
    }


    int NUM_VECTORS_TO_TEST = 80000;
    float theta_max = 0;
    float theta_sum = 0;

    int i;
    for (i = 0; i < NUM_VECTORS_TO_TEST; i++) {
        Vector3 test_vector = random_unit_vector();
        int encoded = find_best_vector(test_vector, vectors, num_vectors);
        
        float dot = dot_product(vectors[encoded], test_vector);
        if (dot < -1) dot = -1;
        if (dot > 1) dot = 1;
        double theta = acos(dot);

        if (theta > theta_max) theta_max = theta;
        theta_sum += theta;
    }

    float theta_average = theta_sum / NUM_VECTORS_TO_TEST;

    printf("Angle of error MAX: %f, AVERAGE: %f\n",
           theta_max, theta_average);
    exit(0);
}
