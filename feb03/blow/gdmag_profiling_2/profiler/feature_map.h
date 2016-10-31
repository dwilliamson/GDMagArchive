struct Random_Generator;
struct History_Scalar;

const int KOHONEN_MAX_DIMENSIONS = 16;
const int USE_DOT_PRODUCT_AS_METRIC = 0;  // I think this is broken, don't turn it on!

const int MAX_SPREAD_VECTORS = 5;

#include "history_scalar.h"

struct Feature_Map_Vector_Coordinate {
    float magnitude;
    int basis_vector;
};

struct Feature_Map_Vector {
    Feature_Map_Vector(int max_dimensions);
    ~Feature_Map_Vector();

    int num_dimensions;
    int max_dimensions;
    
    Feature_Map_Vector_Coordinate *coordinates;

    float length_squared();
    float length();
    void scale(float factor);
    void normalize();
    void sort_by_magnitude();
    void sort_by_basis_vector();
    void copy_from(Feature_Map_Vector *other);
};

float distance(Feature_Map_Vector *a, Feature_Map_Vector *b);
float dot_product(Feature_Map_Vector *a, Feature_Map_Vector *b);
void vector_add(Feature_Map_Vector *a, Feature_Map_Vector *b, Feature_Map_Vector *result, float factor);
void lerp(Feature_Map_Vector *a, Feature_Map_Vector *b, Feature_Map_Vector *result, float factor);

// For now, all feature maps are 2-dimensional.
struct Feature_Map {
    Feature_Map(int num_vector_dimensions,
                int map_buckets_per_side,
                int num_warmup_cycles);
    ~Feature_Map();

    enum State {
        WARMING_UP,
        TRAINING,
        CLASSIFYING_ONLY
    };

    void classify_input_vector(Feature_Map_Vector *input_vector);
    void update_time(double now, double dt);

    State state;
    int warmup_cycles_remaining;

    int num_vector_dimensions;
    int num_map_buckets_per_side;
    int num_map_vectors_total;

    int last_hit_vector;
    float total_hits;

    Feature_Map_Vector **warmup_vectors;
    int num_warmup_vectors;
    int max_warmup_vectors;

    History_Scalar pseudo_time;

    Feature_Map_Vector **cluster_vectors;
    History_Scalar *mod_times;

    // I have made 'hit_counts' and 'total_pulls' into History_Scalars,
    // when really I don't need any history to implement the immediate
    // algorithm (I can just use floats and it's fine).  However, this
    // is sort of an experiment in programming -- I'm going to see if
    // having the history around inspires me to do anything interesting.
    // At the very least I can output some debugging/diagnostic stuff
    // that shows how each area is being hit.

    History_Scalar *hit_counts;
    History_Scalar *total_pulls;

    float *hardnesses;
    Random_Generator *random_generator;

    Feature_Map_Vector *scratch_vector;
    Feature_Map_Vector *scratch_vector_2;
    Feature_Map_Vector *spread_scratch_vector;
    Feature_Map_Vector *mean_scratch_vector;

    Feature_Map_Vector **spread_scratch_vectors;
    Feature_Map_Vector *spread_basis_vectors[MAX_SPREAD_VECTORS];
    float spread_basis_dot_min[MAX_SPREAD_VECTORS];
    float spread_basis_dot_max[MAX_SPREAD_VECTORS];
    float spread_basis_dot_extents[MAX_SPREAD_VECTORS];
    int num_spread_basis_vectors;
    int num_spread_scratch_vectors;

    double now, dt;
    double best_dist;

    float times_to_reach_90_percent[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    float precomputed_factors[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    
    int get_index(int index_x, int index_y);

    void find_spread_vectors(Feature_Map_Vector *center,
                             Feature_Map_Vector **input_vectors, int num_vectors,
                             int max_results);
  private:

    int pseudo_time_count;
    float pseudo_time_accumulator;

    int filter_base_i, filter_base_j;
    float filter_magnitude;
    float filter_radius;

    void do_warmup(Feature_Map_Vector *vector);
    void allocate_vectors();
    void complete_warmup();
    void complete_warmup_with_zero_warmup_vectors();

    void perturb_vector(int index, int ox, int oy, 
                        Feature_Map_Vector *target_vector, float weight);

    float get_pull_factor(int index);
    void initialize_filter(int base_i, int base_j, float radius);
    float evaluate_filter(int offset_i, int offset_j);
    void perturb_vector(int base_i, int base_j, int oi, int oj,
                        Feature_Map_Vector *target_vector);
    
    Feature_Map_Vector *find_furthest_vector(Feature_Map_Vector *base,
                                             Feature_Map_Vector **vectors, int num_vectors);
    void allocate_spread_scratch_vectors(int num_vectors);
};


struct Random_Generator {
    Random_Generator();

    void seed(unsigned long);
    unsigned long get();
    float get_within_range(float min, float max);

  private:
    unsigned long state;
};
