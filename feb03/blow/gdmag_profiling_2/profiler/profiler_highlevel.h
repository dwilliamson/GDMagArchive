const int NUM_THROWAWAY_UPDATES = 20;

struct Feature_Map;
struct Feature_Map_Vector;

#include "history_scalar.h"

struct Profile_Tracker_Data_Record {
    int index;
    History_Scalar self_time;
    History_Scalar hierarchical_time;
    History_Scalar entry_count;
    float displayed_quantity;
};

struct Map_Selection {
    bool mouse_button_is_down;
    bool selected;
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
    float times_to_reach_90_percent[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    float precomputed_factors[NUM_PROFILE_TRACKER_HISTORY_SLOTS];

    Profile_Tracker_Data_Record *sorted_pointers[MAX_PROFILING_ZONES];

    Feature_Map *feature_map;
    Feature_Map_Vector *input_vector;
    Vector3 *scratch_colors;

    int num_active_zones;

    Profiling_Int64 last_integer_timestamp;
    Profiling_Int64 current_integer_timestamp;

    int update_index;
    double last_update_time;
    double dt;
    double dt_per_integer_timestamp;

    float mouse_x, mouse_y;

    int selected_map_index;
    Map_Selection *map_selection_array;

    Displayed_Quantity displayed_quantity;

    void enable_feature_map_update(bool should_update);
    void set_feature_map_reporting(bool should_report,
                                   float mouse_x, float mouse_y);

  protected:
    bool update_feature_map;
    bool draw_feature_map_report;
    int feature_map_cursor_x, feature_map_cursor_y;

    float draw_color_r, draw_color_g, draw_color_b, draw_color_a;

    // Stuff defined in the render file:

    void draw_text(float x, float y, char *buf, 
                   float r = 1, float g = 1, float b = 1, float a = 1, float max_width = 0);

    void draw_feature_map(float x, float y, float w, float h);
    void draw_basis_vector_report(Feature_Map_Vector *vector,
                                  float x, float y, Vector3 *color);
    void set_draw_color(float r, float g, float b, float a);
    void draw_colored_rectangle(float x0, float y0, float x1, float y1);
};
