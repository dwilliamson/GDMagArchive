#include "app_shell.h"
#include "profiler_lowlevel.h"
#include "profiler_highlevel.h"

#include "feature_map.h"
#include "statistics.h"

#include <math.h>

const float VARIANCE_TOLERANCE_FACTOR = 0.5f;
const int FEATURE_MAP_WARMUP_CYCLES = 10;
const int MIN_HIT_COUNTS = 10;

const double SPEEDSTEP_DETECTION_RATIO = 0.08;

Define_Zone(profile_tracker_update);
Define_Zone(profile_tracker_draw);
Define_Zone(padding2);

const double FRAME_TIME_INITIAL = 0.001;

const int IMPORTANT_SLOT = 1;
const int SLOT_TO_CLASSIFY = 0;

const int KOHONEN_MAP_SIZE = 8;
double my_busy_double = 1;

extern Font *small_font; // XXXXX

void draw_column_stripes(int num_active_zones, float sx, float sy, float hh,
                         bool do_third_column,
                         float *name_x1_ret, float *num1_x1_ret, 
                         float *num2_x1_ret);


static int sort_records(const void *a, const void *b) {
    Profile_Tracker_Data_Record *record_a = *(Profile_Tracker_Data_Record **)a;
    Profile_Tracker_Data_Record *record_b = *(Profile_Tracker_Data_Record **)b;

    float time_a = record_a->displayed_quantity;
    float time_b = record_b->displayed_quantity;

    if (time_a < time_b) return +1;
    if (time_a > time_b) return -1;
    return 0;
}

#ifdef SQRT_VECTOR
void timings_to_unit_vector(Feature_Map_Vector *vector) {
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        float mag = vector->coordinates[i].magnitude;
        vector->coordinates[i].magnitude = sqrt(fabs(mag));
    }
}

void unit_vector_to_timings(Feature_Map_Vector *vector) {
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        float mag = vector->coordinates[i].magnitude;
        vector->coordinates[i].magnitude = mag * mag;
    }
}
#endif

void timings_to_unit_vector(Feature_Map_Vector *vector) {
    vector->normalize();
}

void unit_vector_to_timings(Feature_Map_Vector *vector) {
    float sum = 0;
    int i;
    for (i = 0; i < vector->num_dimensions; i++) {
        float mag = vector->coordinates[i].magnitude;
        sum += fabs(mag);
    }

    assert(sum != 0);

    for (i = 0; i < vector->num_dimensions; i++) {
        vector->coordinates[i].magnitude /= sum;
    }    
}

Profile_Tracker::Profile_Tracker() {
    update_index = 0;
    last_update_time = 0;

    times_to_reach_90_percent[0] = 0.1f;
    times_to_reach_90_percent[1] = 0.8f;
    times_to_reach_90_percent[2] = 2.5f;

    update_feature_map = false;
    draw_feature_map_report = false;

    selected_map_index = -1;
    displayed_quantity = SELF_TIME;

    int i;
    for (i = 0; i < MAX_PROFILING_ZONES; i++) {
        Profile_Tracker_Data_Record *record = &data_records[i];
        record->index = -1;

        record->self_time.clear();
        record->hierarchical_time.clear();
        record->entry_count.clear();
    }

    frame_time.clear();

    int j;
    for (j = 0; j < NUM_PROFILE_TRACKER_HISTORY_SLOTS; j++) {
        frame_time.values[j] = FRAME_TIME_INITIAL;
    }

    int num_vector_dimensions = Profiling::num_zones;
    feature_map = new Feature_Map(num_vector_dimensions, KOHONEN_MAP_SIZE, FEATURE_MAP_WARMUP_CYCLES);
    input_vector = new Feature_Map_Vector(num_vector_dimensions);

    int num_vectors = KOHONEN_MAP_SIZE * KOHONEN_MAP_SIZE;
    scratch_colors = new Vector3[num_vectors];

    map_selection_array = new Map_Selection[num_vector_dimensions];
    memset(map_selection_array, 0, sizeof(*map_selection_array) * num_vector_dimensions);

    mouse_x = mouse_y = 0;

    draw_color_r = 1;
    draw_color_g = 1;
    draw_color_b = 1;
    draw_color_a = 1;
}

extern double get_freq();

void Profile_Tracker::set_displayed_quantity(Displayed_Quantity desired) {
    displayed_quantity = desired;
}


float get_stdev(History_Scalar *scalar, int slot) {
    float value = scalar->values[slot];
    float variance = scalar->variances[slot];
    variance = variance - value * value;
    if (variance < 0) variance = 0;
    float stdev = sqrt(variance);
    return stdev;
}

void Profile_Tracker::update() {
    Profile_Scope(profile_tracker_update);

    // Precompute the time factors

    double now = app_shell->get_time();

    if (update_index == 0) {
        dt = FRAME_TIME_INITIAL;
    } else {
        dt = now - last_update_time;
    }

//    if (dt < FRAME_TIME_INITIAL) dt = FRAME_TIME_INITIAL;

    last_update_time = now;
    
    feature_map->update_time(now, dt);

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        precomputed_factors[i] = pow(0.1f, dt / times_to_reach_90_percent[i]);
    }

    precomputed_factors[0] = 0; // For now we want this to be instantaneous.

    Profiling_Int64 timestamp_delta;
    current_integer_timestamp = os_fast_get_integer_timestamp();
    if (update_index == 0) {
        double sum = 0;
        for (i = 0; i < num_active_zones; i++) {
            sum += Profiling::zone_pointers_by_index[i]->total_self_ticks;
        }

        if (sum == 0) sum = 1;

        timestamp_delta = sum;
    } else {
        timestamp_delta = current_integer_timestamp - last_integer_timestamp;
        if (timestamp_delta == 0) timestamp_delta = 1;
    }

    last_integer_timestamp = current_integer_timestamp;



    double timestamps_per_second = (double)timestamp_delta / dt;
    if (update_index < NUM_THROWAWAY_UPDATES) {
        integer_timestamps_per_second.set_for_all_past_history(timestamps_per_second);
    } else {
        integer_timestamps_per_second.update(timestamps_per_second, precomputed_factors);
    }

    double timestamps_to_seconds;
    if (timestamps_per_second) {
        timestamps_to_seconds = 1.0 / timestamps_per_second;
    } else {
        timestamps_to_seconds = 0;
    }

    for (i = 0; i < MAX_PROFILING_ZONES; i++) {
        Program_Zone *zone = Profiling::zone_pointers_by_index[i];
        if (zone == NULL) break;

        Profile_Tracker_Data_Record *record = &data_records[i];

        record->index = zone->index;

        double self_time = zone->total_self_ticks * timestamps_to_seconds;
        double hier_time = zone->total_hier_ticks * timestamps_to_seconds;
        
        double entry_count = zone->total_entry_count;

        assert(hier_time >= 0);

        if (update_index < NUM_THROWAWAY_UPDATES) {
            record->self_time.set_for_all_past_history(self_time);
            record->hierarchical_time.set_for_all_past_history(hier_time);
            record->entry_count.set_for_all_past_history(entry_count);
        } else {
            // @Improvement:  If we want to be very special, we can verify here that
            // our self_time and hier_times roughly add up to dt.  If they don't, then
            // maybe SpeedStep kicked in, or something else happened to disrupt the
            // accuracy of our integer timestamps, and they should not be trusted
            // this frame...
            record->self_time.update(self_time, precomputed_factors);
            record->hierarchical_time.update(hier_time, precomputed_factors);
            record->entry_count.update(entry_count, precomputed_factors);
        }

        // XXX This should probably be cleared out only by a core system thing
        // that runs once per frame (i.e. we should be able to instantiate
        // multiple profilers looking at different things and they should all
        // work)
        zone->total_self_ticks = 0;
        zone->total_hier_ticks = 0;
        zone->total_entry_count = 0;
    }

    frame_time.update(dt, precomputed_factors);


    int slot = IMPORTANT_SLOT;

    int cursor = 0;
    for (i = 0; i < Profiling::num_zones; i++) {
        Profile_Tracker_Data_Record *record = &data_records[i];
        assert(record->index != -1);

        switch (displayed_quantity) {
        case SELF_TIME:
            record->displayed_quantity = record->self_time.values[slot] * 1000;
            break;
        case SELF_STDEV: {
            float stdev = get_stdev(&record->self_time, slot);
            record->displayed_quantity = stdev * 1000;
            break;
        }
        case HIERARCHICAL_TIME:
            record->displayed_quantity = record->hierarchical_time.values[slot] * 1000;
            break;
        case HIERARCHICAL_STDEV: {
            float stdev = get_stdev(&record->hierarchical_time, slot);
            record->displayed_quantity = stdev * 1000;
            break;
        }
        default:
            assert(0);
        }

        sorted_pointers[cursor++] = record;
    }

    num_active_zones = cursor;

    if (update_feature_map && (update_index >= NUM_THROWAWAY_UPDATES)) {
        // Update the Kohonen feature map

        for (i = 0; i < num_active_zones; i++) {
            Profile_Tracker_Data_Record *record = &data_records[i];
            float time = record->self_time.values[SLOT_TO_CLASSIFY];

            input_vector->coordinates[i].basis_vector = i;
            input_vector->coordinates[i].magnitude = time;
        }        

        input_vector->num_dimensions = Profiling::num_zones;

        // XXX Really stupid hack here -- if the vector has a lot of time in 
        // 'uncharted', and we haven't discarded too many, don't classify it.
        // This is a workaround for a problem I've seen where one vector gets
        // classified that has a huge amount of 'uncharted', such that the
        // initialization routine correlates one of the major axes with 'uncharted'
        // and that just ain't cool.  This could be a bug in my code that
        // produces an erroneous vector, or a weird glitch in the OS that
        // causes a spike somewhere shortly after you hit the first keyboard
        // input in the app, or something.  Not sure now, and don't
        // have time to analyze it, but this should be fixed at some point.

        // UPDATE: it seems to be this lame key-hook software on my Compaq 
        // laptop causing the debugger to load extra DLL symbols the first
        // time I hit a key.  Lame.
        {
            const int MAX_FRAMES_TO_DITCH_DUE_TO_WEIRDNESS = 20;
            const float WEIRDNESS_THRESHOLD = 0.005f;
            static int num_frames_ditched;
            if ((input_vector->coordinates[0].magnitude > WEIRDNESS_THRESHOLD) &&
                (num_frames_ditched < MAX_FRAMES_TO_DITCH_DUE_TO_WEIRDNESS)) {
                num_frames_ditched++;
                // Don't classify.
            } else {
                // Do classify.
                feature_map->classify_input_vector(input_vector);
            }
        }
    }

    // Sort.
    qsort(sorted_pointers, num_active_zones, sizeof(Profile_Tracker_Data_Record *), sort_records);

    update_index++;
}

void Profile_Tracker::enable_feature_map_update(bool should_update) {
    update_feature_map = should_update;
}

void Profile_Tracker::set_feature_map_reporting(bool should_report,
                                                float _mouse_x, float _mouse_y) {
    draw_feature_map_report = should_report;
    mouse_x = _mouse_x;
    mouse_y = _mouse_y;
}

Vector3 get_color(float value, float stdev) {
    Vector3 hot(1, 1.0, 0.9);
    Vector3 cold(0.15, 0.9, 0.15);

    const float VALUE_EPSILON = 0.000001;
    float fabs_value = fabs(value);

    if (fabs_value < VALUE_EPSILON) {
        return cold;
//        if (variance < fabs_value) return cold;
//      return hot;
    }

    float factor = (stdev / fabs_value) * (1.0f / VARIANCE_TOLERANCE_FACTOR);
    if (factor < 0) factor = 0;
    if (factor > 1) factor = 1;
    

    Vector3 diff = hot.subtract(cold);
    diff = diff.scale(factor);

    Vector3 result = cold.add(diff);
    return result;
}



void do_scratch_colors_for_coordinate(Feature_Map *feature_map, Vector3 *scratch_colors, int basis_vector, float scale_r, float scale_g, float scale_b, bool modulate = false) {
    int stride = feature_map->num_map_buckets_per_side;

    // Compute the feature map intensities.
    // First loop over everything and find the statistics.
    int i, j;
    Statistics statistics;
    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;
            if (feature_map->hit_counts[index].values[0] < MIN_HIT_COUNTS) continue;
            statistics.add(feature_map->cluster_vectors[index]->coordinates[basis_vector].magnitude);
        }
    }

    statistics.finish();

    float max = statistics.max;
    if (max == 0) max = 1;
    float imax = 1.0f / fabs(max);


    for (j = 0; j < stride; j++) {
        for (i = 0; i < stride; i++) {
            int index = j * stride + i;

            float val;
            val = feature_map->cluster_vectors[index]->coordinates[basis_vector].magnitude;
            val = val * imax;
            if (modulate) {
                scratch_colors[index].x *= val * scale_r;
                scratch_colors[index].y *= val * scale_g;
                scratch_colors[index].z *= val * scale_b;
            } else {
                scratch_colors[index] = Vector3(val * scale_r, val * scale_g, val * scale_b);
            }
        }
    }
}




void Profile_Tracker::draw(float x, float y) {
    Profile_Scope(profile_tracker_draw);

    int slot = IMPORTANT_SLOT;

    float sx = x;
    float sy = y;
    float hh = small_font->character_height;


    // Draw the fps counter.

    float avg_frame_time = frame_time.values[slot];
    if (avg_frame_time == 0) avg_frame_time = 0.01f;
    float fps = 1.0f / avg_frame_time;

    char *displayed_quantity_name = "*error*";
    switch (displayed_quantity) {
    case SELF_TIME:
        displayed_quantity_name = "SELF TIME";
        break;
    case SELF_STDEV:
        displayed_quantity_name = "SELF STDEV";
        break;
    case HIERARCHICAL_TIME:
        displayed_quantity_name = "HIERARCHICAL TIME";
        break;
    case HIERARCHICAL_STDEV:
        displayed_quantity_name = "HIERARCHICAL STDEV";
        break;
    }

    const float header_pad = 4;
    float header_x0 = sx;

    set_draw_color(0.1f, 0.3f, 0, 0.85);
    static char buf[BUFSIZ];   // XXX static buffer

    sprintf(buf, "fps: %3.2f (Frame time %3.3fms) Displaying %s", fps, avg_frame_time * 1000, displayed_quantity_name);
    
    float header_x1 = header_x0 + small_font->get_string_width_in_pixels(buf) + header_pad;

    draw_colored_rectangle(header_x0, sy-1.5*hh, header_x1, sy+1.5*hh);

    draw_text(sx, sy, buf);
    sy -= hh;
    sprintf(buf, "   pseudotime %.4f best_dist %.5f", feature_map->pseudo_time.values[2], feature_map->best_dist);
    draw_text(sx, sy, buf);
    sy -= hh;

    sy -= 1.5*hh;


    // Detect SpeedStep and warn the user if it is screwing us up.
    sy -= 1.5*hh;

    int ss_slot = 1;
    double ss_val = integer_timestamps_per_second.values[ss_slot];
    double ss_variance = integer_timestamps_per_second.variances[ss_slot] - ss_val*ss_val;
    double ss_stdev = sqrt(fabs(ss_variance));
    float ss_ratio;
    if (ss_val) {
        ss_ratio = ss_stdev / fabs(ss_val);
    } else {
        ss_ratio = 0;
    }

    if (ss_ratio > SPEEDSTEP_DETECTION_RATIO) {
        set_draw_color(0.9f, 0.1f, 0.1f, 0.85f);
        draw_colored_rectangle(header_x0, sy, header_x1, sy+3*hh);

        char *warning1 = "WARNING: SpeedStep detected.  Results are unreliable!";
        char *warning2 = "(Try running on a desktop machine instead.)";

        float len1 = small_font->get_string_width_in_pixels(warning1);
        float len2 = small_font->get_string_width_in_pixels(warning2);
        float offset1 = (header_x1 - header_x0 - len1) * 0.5f;
        float offset2 = (header_x1 - header_x0 - len2) * 0.5f;

        draw_text(sx+offset1, sy +1.5*hh, warning1);
        draw_text(sx+offset2, sy +0.5*hh, warning2);

        sy -= 3*hh;
    }


    // Start drawing the actual report.

    float name_x1, num1_x1, num2_x1;
    draw_column_stripes(num_active_zones, sx, sy, hh, true,
                        &name_x1, &num1_x1, &num2_x1);


    // Draw the zone data.
    int i;
    for (i = 0; i < num_active_zones; i++) {
        Profile_Tracker_Data_Record *record = sorted_pointers[i];
        float self_percentage = 100.0 * (record->self_time.values[slot] / frame_time.values[slot]);
        float hier_percentage = 100.0 * (record->hierarchical_time.values[slot] / frame_time.values[slot]);
        assert(hier_percentage > -1.0f);

        float self_ms = 1000 * record->self_time.values[slot];
        float hier_ms = 1000 * record->hierarchical_time.values[slot];

        Program_Zone *zone = Profiling::zone_pointers_by_index[record->index];

        float name_len = small_font->get_string_width_in_pixels(zone->name);

        float value = record->self_time.values[slot];
        float variance = record->self_time.variances[slot];
        variance = variance - value * value;
        if (variance < 0) variance = 0;
        float stdev = sqrt(variance);

        Vector3 vcolor = get_color(value, stdev);
        draw_text(name_x1 - name_len, sy, zone->name, vcolor.x, vcolor.y, vcolor.z);

        sprintf(buf, "%5.2f", record->displayed_quantity);
        float num1_len = small_font->get_string_width_in_pixels(buf);
        draw_text(num1_x1 - num1_len, sy, buf, vcolor.x, vcolor.y, vcolor.z);

        sprintf(buf, "%.2f", record->entry_count.values[1]);
        float num2_len = small_font->get_string_width_in_pixels(buf);
        draw_text(num2_x1 - num2_len, sy, buf, vcolor.x, vcolor.y, vcolor.z);

        sy -= hh;
    }

    if (feature_map->state == Feature_Map::WARMING_UP) return;  // Don't do feature map stuff!

    // Index mouse into feature map...
    float mx0 = x + 430;
    float my0 = y - 20;
    float mx1 = mx0 + 100;
    float my1 = my0 + 100;

    float perc_x = (mouse_x - mx0) / (mx1 - mx0);
    float perc_y = (mouse_y - my0) / (my1 - my0);
    if (draw_feature_map_report &&
        (perc_x >= 0) && (perc_x <= 1) &&
        (perc_y >= 0) && (perc_y <= 1)) {

        int w = feature_map->num_map_buckets_per_side;
        feature_map_cursor_x = (int)(perc_x * w);
        feature_map_cursor_y = (int)(perc_y * w);

        if (feature_map_cursor_x >= w) feature_map_cursor_x = w - 1;
        if (feature_map_cursor_y >= w) feature_map_cursor_y = w - 1;
    } else {
        feature_map_cursor_x = feature_map_cursor_y = -1;
    }

    draw_feature_map(mx0, my0, mx1 - mx0, my1 - my0);
}


void History_Scalar::update(float new_value, float *k_array) {
    
    float new_variance = new_value * new_value;

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        float k = k_array[i];
        values[i] = values[i] * k + new_value * (1 - k);
        variances[i] = variances[i] * k + new_variance * (1 - k);
    }
}

void History_Scalar::clear() {
    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        values[i] = 0;
        variances[i] = 0;
    }
}

void History_Scalar::set_for_all_past_history(double new_value) {
    double new_variance = new_value * new_value;

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        values[i] = new_value;
        variances[i] = new_variance;
    }
}

