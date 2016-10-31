const int NUM_PROFILE_TRACKER_HISTORY_SLOTS = 3;
const int NUM_THROWAWAY_UPDATES = 10;

struct History_Scalar {
    double values[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    double variances[NUM_PROFILE_TRACKER_HISTORY_SLOTS];

    void update(double new_value, double *k_array);
    void eternity_set(double value);

    void clear();
};

struct Profile_Tracker_Data_Record {
    int index;
    History_Scalar self_time;
    History_Scalar hierarchical_time;
    History_Scalar entry_count;
    float displayed_quantity;
};

struct Profile_Tracker {
    Profile_Tracker();
    ~Profile_Tracker();

    void update();
    void draw(float x, float y);

    enum Displayed_Quantity {
        SELF_TIME = 0,
        HIERARCHICAL_TIME,
        SELF_STDEV,
        HIERARCHICAL_STDEV
    };

    void set_displayed_quantity(Displayed_Quantity);


    History_Scalar frame_time;
    History_Scalar integer_timestamps_per_second;

    Profile_Tracker_Data_Record data_records[MAX_PROFILING_ZONES];
    double times_to_reach_90_percent[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    double precomputed_factors[NUM_PROFILE_TRACKER_HISTORY_SLOTS];

    Profile_Tracker_Data_Record *sorted_pointers[MAX_PROFILING_ZONES];

    int num_active_zones;

    Profiling_Int64 last_integer_timestamp;
    Profiling_Int64 current_integer_timestamp;

    int update_index;
    double last_update_time;
    double dt;
    double dt_per_integer_timestamp;

    Displayed_Quantity displayed_quantity;
};

