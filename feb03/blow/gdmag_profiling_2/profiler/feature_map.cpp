#include "app_shell.h"
#include "feature_map.h"

#include <math.h>
#include <float.h>

const float KERNEL_RADIUS = 4;

const int RADIUS_DWINDLES_BY_HARDNESS = 0;
const int RADIUS_DWINDLES_BY_TIME = 0;
const int RADIUS_DWINDLES_BY_PSEUDOTIME = 1;
const int LIMIT_CONVERGENCE_BY_RADIUS = 0;

const float RADIUS_TIME_DWINDLE_MAX_TIME = 90.0f;
const float RADIUS_TIME_DWINDLE_MIN_FACTOR = 0.25f;

const float MINIMUM_SUBSPACE_VECTOR_LENGTH = 0.00005;  // Measured in seconds

// XXXX The precomputed_factors used to update 'hit_counts'
// and 'total_pulls' should not depend on frame time, probably;
// they should be a different independent set of stuff.

// XXXXX Need to handle the case where the guy who best matches
// the input vector is frozen, so he won't move any closer,
// but he's not "good enough"... if there is unfrozen space
// out in the feature map, maybe we want to start using that
// or something.  Aww hell I don't know, maybe not.

const float VECTOR_PULL_FACTOR_MAX = 0.08f;
const float VECTOR_PULL_FACTOR_MIN = 0.0f;

const float VECTOR_PULL_DOT_MIN = 0.90f;
const float VECTOR_PULL_DOT_MAX = 1.0f;

const float PERTURB_DOT_THRESHOLD = 0.95f;
const float PULLS_FOR_HARDENING = 500.0f;
const float PULLS_FOR_PSEUDOTIME_SETTLING = 30;

const int WRAP_FEATURE_MAP = 0;

struct Kohonen_Iterator_Slot {
    float magnitude;
    int basis_vector;
    bool valid;
};

void break_here() {
}

Feature_Map_Vector::Feature_Map_Vector(int _max_dimensions) {
    num_dimensions = 0;
    max_dimensions = _max_dimensions;
    coordinates = new Feature_Map_Vector_Coordinate[max_dimensions];
}

Feature_Map_Vector::~Feature_Map_Vector() {
    delete [] coordinates;
}

float Feature_Map_Vector::length_squared() {
    float sum = 0;

    int i;
    for (i = 0; i < num_dimensions; i++) {
        float mag = coordinates[i].magnitude;
        sum += mag*mag;
    }

    return sum;
}

float Feature_Map_Vector::length() {
    return sqrt(length_squared());
}

void Feature_Map_Vector::normalize() {
    float len2 = length_squared();
    if (!len2) return;

    len2 = fabs(len2);
    float ilen2 = 1.0f  / sqrt(len2);  // XXX still numerically brittle

    int i;
    for (i = 0; i < num_dimensions; i++) {
        coordinates[i].magnitude *= ilen2;
    }
}

void Feature_Map_Vector::scale(float factor) {
    int i;
    for (i = 0; i < num_dimensions; i++) {
        coordinates[i].magnitude *= factor;
    }
}

void Feature_Map_Vector::copy_from(Feature_Map_Vector *other) {
    assert(other->max_dimensions == max_dimensions);

    num_dimensions = other->num_dimensions;

    int i;
    for (i = 0; i < num_dimensions; i++) {
        coordinates[i] = other->coordinates[i];
    }
}

void vector_add(Feature_Map_Vector *a, Feature_Map_Vector *b,
                Feature_Map_Vector *dest, float factor) {
    assert(a->num_dimensions == b->num_dimensions);
    dest->num_dimensions = a->num_dimensions;
    int i;
    for (i = 0; i < a->num_dimensions; i++) {
        assert(a->coordinates[i].basis_vector == i);
        assert(b->coordinates[i].basis_vector == i);
        dest->coordinates[i].basis_vector = i;
        dest->coordinates[i].magnitude = a->coordinates[i].magnitude + b->coordinates[i].magnitude * factor;
    }
}

static int compare_basis_vectors(const void *item1, const void *item2) {
    Feature_Map_Vector_Coordinate *e1 = (Feature_Map_Vector_Coordinate *)item1;
    Feature_Map_Vector_Coordinate *e2 = (Feature_Map_Vector_Coordinate *)item2;

    return e1->basis_vector - e2->basis_vector;
}

void Feature_Map_Vector::sort_by_basis_vector() {
    qsort(coordinates, num_dimensions, sizeof(coordinates[0]), 
          compare_basis_vectors);
}

static int compare_magnitudes(const void *item1, const void *item2) {
    Feature_Map_Vector_Coordinate *e1 = (Feature_Map_Vector_Coordinate *)item1;
    Feature_Map_Vector_Coordinate *e2 = (Feature_Map_Vector_Coordinate *)item2;

    float m1 = fabs(e1->magnitude);
    float m2 = fabs(e2->magnitude);

    if (m2 > m1) return +1;
    if (m2 < m1) return -1;
    return 0;
}

void Feature_Map_Vector::sort_by_magnitude() {
    qsort(coordinates, num_dimensions, sizeof(coordinates[0]), 
          compare_magnitudes);
}

float dot_product(Feature_Map_Vector *a, Feature_Map_Vector *b) {
    float sum = 0;

    assert(a->num_dimensions == b->num_dimensions);

    int i;
    for (i = 0; i < a->num_dimensions; i++) {
        assert(a->coordinates[i].basis_vector == i);
        assert(b->coordinates[i].basis_vector == i);
        sum += a->coordinates[i].magnitude * b->coordinates[i].magnitude;
    }

    return sum;
}

float distance(Feature_Map_Vector *a, Feature_Map_Vector *b) {
    float sum = 0;

    assert(a->num_dimensions == b->num_dimensions);

    int i;
    for (i = 0; i < a->num_dimensions; i++) {
        assert(a->coordinates[i].basis_vector == i);
        assert(b->coordinates[i].basis_vector == i);
        float dx = a->coordinates[i].magnitude - b->coordinates[i].magnitude;
        sum += dx*dx;
    }

    float result = sqrt(sum);
    return result;
}


void generate_random_vector(Feature_Map_Vector *vector, Random_Generator *random_generator) {
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        vector->coordinates[i].magnitude = random_generator->get_within_range(0, 0.099);
        vector->coordinates[i].basis_vector = i;
    }
}

Feature_Map::Feature_Map(int _num_vector_dimensions,
                         int map_buckets_per_side,
                         int _max_warmup_vectors) {
    assert(map_buckets_per_side > 1);

    num_warmup_vectors = 0;
    max_warmup_vectors = _max_warmup_vectors;

    num_map_buckets_per_side = map_buckets_per_side;
    num_map_vectors_total = 0;

    num_vector_dimensions = _num_vector_dimensions;
    last_hit_vector = -1;
    total_hits = 0;

    random_generator = new Random_Generator();

    scratch_vector = new Feature_Map_Vector(num_vector_dimensions);
    scratch_vector_2 = new Feature_Map_Vector(num_vector_dimensions);
    spread_scratch_vector = new Feature_Map_Vector(num_vector_dimensions);
    mean_scratch_vector = new Feature_Map_Vector(num_vector_dimensions);

    pseudo_time.clear();

    times_to_reach_90_percent[0] = 0.1f;
    times_to_reach_90_percent[1] = 0.7f;
    times_to_reach_90_percent[2] = 2.5f;

    now = 0;
    dt = 0;

    best_dist = 0;



    // Allocate the warmup vectors.

    warmup_vectors = new Feature_Map_Vector *[max_warmup_vectors];
    int i;
    for (i = 0; i < max_warmup_vectors; i++) {
        warmup_vectors[i] = new Feature_Map_Vector(num_vector_dimensions);
    }

    if (max_warmup_vectors == 0) {
        complete_warmup();
    } else {
        state = WARMING_UP;
    }
}



Feature_Map::~Feature_Map() {
    delete [] cluster_vectors;
}


float Feature_Map::get_pull_factor(int index) {
    float pulls = total_pulls[index].values[0];

    float pulls_for_hardening = PULLS_FOR_HARDENING;

    float hardness = pulls / pulls_for_hardening;
    if (hardness > 1) hardness = 1;
    assert(hardness >= 0);

    if (RADIUS_DWINDLES_BY_HARDNESS) {
        hardnesses[index] = hardness;
        float pull_factor = VECTOR_PULL_FACTOR_MAX + hardness * (VECTOR_PULL_FACTOR_MIN - VECTOR_PULL_FACTOR_MAX);
        return pull_factor;
    }

    float pull_factor = VECTOR_PULL_FACTOR_MAX;
    return pull_factor;
}

void Feature_Map::initialize_filter(int base_i, int base_j, float radius) {
    int w = num_map_buckets_per_side;
    int index = base_j*w+base_i;

    filter_base_i = base_i;
    filter_base_j = base_j;
    filter_magnitude = 1.0f;

    filter_radius = radius;
    if (RADIUS_DWINDLES_BY_HARDNESS) {
        filter_radius = radius * (1.0f - hardnesses[index]);
    } else if (RADIUS_DWINDLES_BY_TIME) {
        float now = app_shell->get_time();
        float perc = now / RADIUS_TIME_DWINDLE_MAX_TIME;
        if (perc > 1) perc = 1;
        assert(perc >= 0);
        float factor = 1.0f + perc * (RADIUS_TIME_DWINDLE_MIN_FACTOR - 1.0f);
        filter_radius = radius * factor;
    } else if (RADIUS_DWINDLES_BY_PSEUDOTIME) {
        float pt = pseudo_time.values[1];
        float factor = 1.0f + pt * (RADIUS_TIME_DWINDLE_MIN_FACTOR - 1.0f);
        filter_radius = radius * factor;
    }
}

float Feature_Map::evaluate_filter(int offset_i, int offset_j) {
    float distance = sqrt(offset_i*offset_i + offset_j*offset_j);

    assert(filter_radius >= 0);
    if (filter_radius == 0) return 0;

    float factor = filter_magnitude * (1 - (distance / filter_radius));
    if (factor < 0) factor = 0;

    return factor;
}

void Feature_Map::perturb_vector(int base_i, int base_j, int oi, int oj,
                                         Feature_Map_Vector *target_vector) {
    float weight = evaluate_filter(oi, oj);
    if (weight == 0) return;


    int x = base_i + oi;
    int y = base_j + oj;

    int w = num_map_buckets_per_side;

    if (WRAP_FEATURE_MAP) {
        if (x < 0) x += w;
        if (x >= w) x -= w;
        if (y < 0) y += w;
        if (y >= w) y -= w;
    } else {
        if (x < 0) return;
        if (x >= w) return;
        if (y < 0) return;
        if (y >= w) return;
    }

    assert(x >= 0);
    assert(y >= 0);

    int index = y * w + x;
    assert(index >= 0);
    assert(index < w*w);


    // Update pseudotime
    float pullness = total_pulls[index].values[0] / PULLS_FOR_PSEUDOTIME_SETTLING;
    if (pullness > 1) pullness = 1;
    pseudo_time_count++;
    pseudo_time_accumulator += pullness;


    // Actually do some vector pulling.
    Feature_Map_Vector *map_vector = cluster_vectors[index];

    assert(mod_times[index].values[0] == 0);
    mod_times[index].update(weight, precomputed_factors);

    if (LIMIT_CONVERGENCE_BY_RADIUS) {
        assert(USE_DOT_PRODUCT_AS_METRIC);
        float dot = dot_product(map_vector, target_vector);
        float vector_pull_dot_threshold = VECTOR_PULL_DOT_MIN + weight * (VECTOR_PULL_DOT_MAX - VECTOR_PULL_DOT_MIN);
        if (dot > vector_pull_dot_threshold) return;
    }

    vector_add(target_vector, map_vector, scratch_vector, -1);

    float pull_factor = get_pull_factor(index);
    scratch_vector->scale(pull_factor * weight);

    vector_add(map_vector, scratch_vector, scratch_vector_2, 1);
    map_vector->copy_from(scratch_vector_2);

    if (USE_DOT_PRODUCT_AS_METRIC) {
        map_vector->normalize();
    }

    // Update 'total_pulls'
    if (weight == 1) {
        float pulls = total_pulls[index].values[0];
        total_pulls[index].update(pulls + weight, precomputed_factors);
    }

}

void Feature_Map::update_time(double _now, double _dt) {
    now = _now;
    dt = _dt;
}

void Feature_Map::classify_input_vector(Feature_Map_Vector *input_vector) {
    if (state == WARMING_UP) {
        do_warmup(input_vector);
        return;
    }

    // Update k factors and decay history scalars.

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        precomputed_factors[i] = pow(0.1f, dt / times_to_reach_90_percent[i]);
    }

    precomputed_factors[0] = 0; // For now we want this to be instantaneous.


    // Set all instantaneous mod_time values to 0 (they may
    // get changed by the classification step).
    for (i = 0; i < num_map_vectors_total; i++) {
        mod_times[i].values[0] = 0;
    }


    // Okay, now classify the lovely input vector.

    best_dist = FLT_MAX;
    int best_index = -1;

    for (i = 0; i < num_map_vectors_total; i++) {
        Feature_Map_Vector *class_vector = cluster_vectors[i];

        float dist = distance(input_vector, class_vector);
        if (dist < best_dist) {
            best_dist = dist;
            best_index = i;
        }
    }

    if (best_index != -1) {
        float kernel_radius = KERNEL_RADIUS;

        int w = num_map_buckets_per_side;
        int base_i = best_index % w;
        int base_j = best_index / w;

        initialize_filter(base_i, base_j, kernel_radius);

        pseudo_time_count = 0;
        pseudo_time_accumulator = 0;

        int i, j;
        for (j = -kernel_radius; j <= kernel_radius; j++) {
            for (i = -kernel_radius; i <= kernel_radius; i++) {
                perturb_vector(base_i, base_j, i, j, input_vector);
            }
        }

        float hits = hit_counts[best_index].values[0];
        hit_counts[best_index].update(hits + 1, precomputed_factors);

        float pt_increment = pseudo_time_accumulator / pseudo_time_count;
        pseudo_time.update(pt_increment, precomputed_factors);
    }

    last_hit_vector = best_index;
    total_hits += 1.0f;

    // Anyone we didn't modify should get cooled off now..
    for (i = 0; i < num_map_vectors_total; i++) {
        if (mod_times[i].values[0] == 0) {
            mod_times[i].update(0, precomputed_factors);
        }
    }
}





Random_Generator::Random_Generator() {
    state = 0xbeefface;
}

void Random_Generator::seed(unsigned long new_seed) {
    state = new_seed;
}

unsigned long Random_Generator::get() {
    long x, hi, lo, t;

    x = (long)state;
    hi = x / 127773;
    lo = x % 127773;
    t = 16807 * lo - 2836 * hi;
    if (t <= 0) t += 0x7fffffff;
    state = (unsigned long)t;
    return state;
}

const unsigned long RANDRANGE = 0x10000000UL;

float Random_Generator::get_within_range(float min, float max) {
    int randint = get() % RANDRANGE;
    return min + ((float)randint / (float)RANDRANGE) * (max - min);
}


int Feature_Map::get_index(int ix, int iy) {
    int w = num_map_buckets_per_side;

    if (ix < 0) return -1;
    if (iy < 0) return -1;
    if (ix >= w) return -1;
    if (iy >= w) return -1;

    return iy * w + ix;
}

void Feature_Map::allocate_vectors() {
    int num_vectors = num_map_buckets_per_side * num_map_buckets_per_side;
    num_map_vectors_total = num_vectors;

    cluster_vectors = new Feature_Map_Vector *[num_vectors];

    int i;
    for (i = 0; i < num_vectors; i++) {
        cluster_vectors[i] = new Feature_Map_Vector(num_vector_dimensions);
        cluster_vectors[i]->num_dimensions = num_vector_dimensions;
    }

    num_spread_scratch_vectors = 0;
    spread_scratch_vectors = NULL;


    mod_times = new History_Scalar[num_vectors];
    hit_counts = new History_Scalar[num_vectors];
    total_pulls = new History_Scalar[num_vectors];

    hardnesses = new float[num_vectors];
    for (i = 0; i < num_vectors; i++) {
        mod_times[i].clear();
        hit_counts[i].clear();
        total_pulls[i].clear();
        hardnesses[i] = 0;
    }

    for (i = 0; i < MAX_SPREAD_VECTORS; i++) {
        spread_basis_vectors[i] = new Feature_Map_Vector(num_vector_dimensions);
        spread_basis_vectors[i]->num_dimensions = num_vector_dimensions;
    }
}

void Feature_Map::complete_warmup_with_zero_warmup_vectors() {
    int num_vectors = num_map_vectors_total;

    int i;
    for (i = 0; i < num_vectors; i++) {
        generate_random_vector(cluster_vectors[i], random_generator);
    }
}

void orthogonalize(Feature_Map_Vector *a, Feature_Map_Vector *b) {
    float dot = dot_product(a, b);
    vector_add(a, b, a, -dot);   // a -= (a dot b) b
}

void clear_vector(Feature_Map_Vector *vector) {
    vector->num_dimensions = vector->max_dimensions;
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        vector->coordinates[i].basis_vector = i;
        vector->coordinates[i].magnitude = 0;
    }
}

Feature_Map_Vector *Feature_Map::find_furthest_vector(Feature_Map_Vector *base,
                                                      Feature_Map_Vector **vectors, int num_vectors) {
    int best_index = -1;
    float best_dist = -FLT_MAX;

    int i;
    for (i = 0; i < num_vectors; i++) {
        float dist = distance(base, vectors[i]);
        if (dist > best_dist) {
            best_dist = dist;
            best_index = i;
        }
    }

    assert(best_index != -1);
    return vectors[best_index];
}

void Feature_Map::allocate_spread_scratch_vectors(int num_vectors) {
    if (num_spread_scratch_vectors >= num_vectors) return;
    int i;
    for (i = 0; i < num_spread_scratch_vectors; i++) {
        delete spread_scratch_vectors[i];
    }
    delete [] spread_scratch_vectors;

    spread_scratch_vectors = new Feature_Map_Vector *[num_vectors];
    for (i = 0; i < num_vectors; i++) {
        spread_scratch_vectors[i] = new Feature_Map_Vector(num_vector_dimensions);
        spread_scratch_vectors[i]->num_dimensions = num_vector_dimensions;
    }

    num_spread_scratch_vectors = num_vectors;
}

void Feature_Map::find_spread_vectors(Feature_Map_Vector *center,
                                      Feature_Map_Vector **input_vectors, int num_vectors,
                                      int max_results) {
    assert(max_results <= MAX_SPREAD_VECTORS);


    // Copy this into our own space and rename it, so we can
    // modify it and not worry.
    spread_scratch_vector->copy_from(center);
    center = spread_scratch_vector;


    num_spread_basis_vectors = 0;

    if (num_vectors == 0) return;

    allocate_spread_scratch_vectors(num_vectors);

    Feature_Map_Vector **vectors = spread_scratch_vectors;
    int i;
    // Subtract the center from all the input points, store the
    // results in a scratch area.
    for (i = 0; i < num_vectors; i++) {
        vector_add(input_vectors[i], center, vectors[i], -1);
    }

    for (i = 0; i < max_results; i++) {
        // Look for a relatively long dimension... hopefully near the longest.

        Feature_Map_Vector *next_basis = spread_basis_vectors[i];
        Feature_Map_Vector *furthest_vector = find_furthest_vector(center, vectors, num_vectors);
        vector_add(furthest_vector, center, next_basis, -1);

        // Orthogonalize this basis vector against each previous vector.
        int j;
        for (j = 0; j < i; j++) {
            orthogonalize(next_basis, spread_basis_vectors[j]);
        }

        // See if the vector is too short to constitute a substantial dimension.
        float basis_length = next_basis->length();
        if (basis_length < MINIMUM_SUBSPACE_VECTOR_LENGTH) {
            break;
        }

        next_basis->normalize();

        // Compute the extents of all the input vectors against this basis.
        // Store those extents, and remove that dimension from the vectors.

        Feature_Map_Vector *basis = next_basis;  // Just a renaming
        float dot_min = FLT_MAX;
        float dot_max = -FLT_MAX;

        for (j = 0; j < num_vectors; j++) {
            float dot = dot_product(vectors[i], basis);
            if (dot < dot_min) dot_min = dot;
            if (dot > dot_max) dot_max = dot;

            vector_add(vectors[i], basis, vectors[i], -dot);   // v_i -= (v_i dot e1) e1
        }

        // Also divide that dimension out of the center vector.
        float dot = dot_product(center, basis);
        vector_add(center, basis, center, -dot);

        spread_basis_dot_min[num_spread_basis_vectors] = dot_min;
        spread_basis_dot_max[num_spread_basis_vectors] = dot_max;
        spread_basis_dot_extents[num_spread_basis_vectors] = dot_max - dot_min;

        num_spread_basis_vectors++;
    }

    for (i = num_spread_basis_vectors; i < max_results; i++) {
        spread_basis_dot_min[i] = -1;
        spread_basis_dot_max[i] = 1;
        spread_basis_dot_extents[i] = 2;
        spread_basis_dot_min[i] = 0;
        clear_vector(spread_basis_vectors[i]);
    }
}

// XXX At some point replace all these scratch vectors
// with a templated delete_when_done class... it really is
// the right thing here, sad as that may be.
void Feature_Map::complete_warmup() {
    state = TRAINING;
    allocate_vectors();

    if (num_warmup_vectors == 0) {
        assert(max_warmup_vectors == 0);
        complete_warmup_with_zero_warmup_vectors();
        return;
    }

    Feature_Map_Vector *mean = mean_scratch_vector;


    // Compute the mean, then subtract it from all the vectors.

    clear_vector(mean);
   
    int num_vectors = num_warmup_vectors;
    float mean_scale = 1.0f / num_vectors;

    int i;
    for (i = 0; i < num_vectors; i++) {
        int j;
        for (j = 0; j < num_vector_dimensions; j++) {
            mean->coordinates[j].magnitude += warmup_vectors[i]->coordinates[j].magnitude * mean_scale;
        }
    }

    find_spread_vectors(mean, warmup_vectors, num_warmup_vectors, 2);

    if (num_spread_basis_vectors < 2) {
        complete_warmup_with_zero_warmup_vectors();  // XXX do what I said
        return;
    }

    Feature_Map_Vector *e1 = spread_basis_vectors[0];
    Feature_Map_Vector *e2 = spread_basis_vectors[1];

    for (i = 0; i < num_vectors; i++) {
        vector_add(warmup_vectors[i], mean, warmup_vectors[i], -1);
    }

    float basis_1_dot_min = spread_basis_dot_min[0];
    float basis_1_dot_extents = spread_basis_dot_extents[0];
    float basis_2_dot_min = spread_basis_dot_min[1];
    float basis_2_dot_extents = spread_basis_dot_extents[1];

    // Okay, now we actually fricking generate the initial vectors.
    // Took us long enough.

    float grid = 1.0f / (num_map_buckets_per_side - 1);
/*
    float e1_step = basis_length_1 * grid;
    float e2_step = basis_length_2 * grid;
*/

    float e1_step = basis_1_dot_extents * grid;
    float e2_step = basis_2_dot_extents * grid;


    break_here();

    int j;
    for (i = 0; i < num_map_buckets_per_side; i++) {
        float e1_pos = (0.5f + i) * e1_step + basis_1_dot_min;
        for (j = 0; j < num_map_buckets_per_side; j++) {
            float e2_pos = (0.5f + j) * e2_step + basis_2_dot_min;

            int index = j * num_map_buckets_per_side + i;
            Feature_Map_Vector *dest = cluster_vectors[index];
            clear_vector(dest);
            vector_add(mean, e1, dest, e1_pos);
            vector_add(dest, e2, dest, e2_pos);

//            dest->copy_from(mean);
        }
    }
}


void Feature_Map::do_warmup(Feature_Map_Vector *vector) {
    assert(num_warmup_vectors < max_warmup_vectors);

    warmup_vectors[num_warmup_vectors++]->copy_from(vector);

    if (num_warmup_vectors == max_warmup_vectors) {
        complete_warmup();
    }
}
