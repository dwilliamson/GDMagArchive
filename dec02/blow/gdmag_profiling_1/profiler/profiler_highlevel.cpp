#include "app_shell.h"
#include "profiler_lowlevel.h"
#include "profiler_highlevel.h"


#include <math.h>

const float VARIANCE_TOLERANCE_FACTOR = 0.5f;

Define_Zone(profile_tracker_update);

const double FRAME_TIME_INITIAL = 0.001;

const int IMPORTANT_SLOT = 1;


const int ITERATIONS = 10000000;
double my_busy_double = 1;

static int sort_records(const void *a, const void *b) {
    Profile_Tracker_Data_Record *record_a = *(Profile_Tracker_Data_Record **)a;
    Profile_Tracker_Data_Record *record_b = *(Profile_Tracker_Data_Record **)b;

    float time_a = record_a->displayed_quantity;
    float time_b = record_b->displayed_quantity;

    if (time_a < time_b) return +1;
    if (time_a > time_b) return -1;
    return 0;
}

void History_Scalar::update(double new_value, double *k_array) {
    
    double new_variance = new_value * new_value;

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        float k = k_array[i];
        values[i] = values[i] * k + new_value * (1 - k);
        variances[i] = variances[i] * k + new_variance * (1 - k);
    }
}

void History_Scalar::eternity_set(double new_value) {
    double new_variance = new_value * new_value;

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        values[i] = new_value;
        variances[i] = new_variance;
    }
}

Profile_Tracker::Profile_Tracker() {
    update_index = 0;
    last_update_time = 0;

    times_to_reach_90_percent[0] = 0.1f;
    times_to_reach_90_percent[1] = 0.8f;
    times_to_reach_90_percent[2] = 2.5f;

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
}
/*
static LARGE_INTEGER get_time_reading() {
    LARGE_INTEGER freq;
    LARGE_INTEGER time;

    BOOL ok = QueryPerformanceFrequency(&freq);
    assert(ok == TRUE);

    freq.QuadPart = freq.QuadPart / 1000;

    ok = QueryPerformanceCounter(&time);
    assert(ok == TRUE);

    time.QuadPart = time.QuadPart / freq.QuadPart;

	return time;
}
*/

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
    

    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        precomputed_factors[i] = pow(0.1f, dt / times_to_reach_90_percent[i]);
    }

    precomputed_factors[0] = 0; // For now we want this to be instantaneous.

    Profiling_Int64 timestamp_delta;
    os_fast_get_integer_timestamp(&current_integer_timestamp);
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
        integer_timestamps_per_second.eternity_set(timestamps_per_second);
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
            int j;
            for (j = 0; j < NUM_PROFILE_TRACKER_HISTORY_SLOTS; j++) {
                record->self_time.values[j] = self_time;
                record->self_time.variances[j] = self_time;
                record->hierarchical_time.values[j] = hier_time;
                record->hierarchical_time.variances[j] = hier_time;
                record->entry_count.values[j] = entry_count;
                record->entry_count.variances[j] = entry_count;
            }
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

    update_index++;


    int slot = IMPORTANT_SLOT;

    int cursor = 0;
    for (i = 0; i < MAX_PROFILING_ZONES; i++) {
        Profile_Tracker_Data_Record *record = &data_records[i];
        if (record->index == -1) continue;

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

    qsort(sorted_pointers, num_active_zones, sizeof(Profile_Tracker_Data_Record *), sort_records);

    float total_self_time[NUM_PROFILE_TRACKER_HISTORY_SLOTS];

    int j;
    for (j = 0; j < NUM_PROFILE_TRACKER_HISTORY_SLOTS; j++) {
        float sum = 0;
        for (i = 0; i < num_active_zones; i++) {
            sum += sorted_pointers[i]->self_time.values[j];
        }

        if (sum == 0) sum = 0.01;
        total_self_time[j] = sum;
    }
}

bool get_text_color_and_glow_color(float value, float stdev,
                                   Vector3 *text_color_ret,
                                   Vector3 *glow_color_ret,
                                   float *glow_alpha_ret) {
    Vector3 hot(1, 1.0, 0.9);
    Vector3 cold(0.15, 0.9, 0.15);

    Vector3 glow_cold(0.5f, 0.5f, 0);
    Vector3 glow_hot(1.0f, 1.0f, 0);

    const float VALUE_EPSILON = 0.000001;
    float fabs_value = fabs(value);

    if (fabs_value < VALUE_EPSILON) {
        *text_color_ret = cold;
        *glow_color_ret = Vector3(0, 0, 0);
        *glow_alpha_ret = 0;
        return false;
//        if (variance < fabs_value) return cold;
//      return hot;
    }

    float factor = (stdev / fabs_value) * (1.0f / VARIANCE_TOLERANCE_FACTOR);
    if (factor < 0) factor = 0;
    if (factor > 1) factor = 1;
    

    Vector3 diff = hot.subtract(cold);
    diff = diff.scale(factor);

    Vector3 text_result = cold.add(diff);
    *text_color_ret = text_result;


    // Figure out whether to start up the glow as well.
    const float GLOW_RANGE = 0.5f;
    const float GLOW_ALPHA_MAX = 0.5f;
    float glow_alpha = (factor - GLOW_RANGE) / (1 - GLOW_RANGE);
    if (glow_alpha < 0) {
        glow_color_ret->set(0, 0, 0);
        *glow_alpha_ret = 0;
        return false;
    }

    diff = glow_hot.subtract(glow_cold);
    diff = diff.scale(glow_alpha);
    Vector3 glow_result = glow_cold.add(diff);
    *glow_color_ret = glow_result;

    *glow_alpha_ret = glow_alpha * GLOW_ALPHA_MAX;
    return true;
}



void History_Scalar::clear() {
    int i;
    for (i = 0; i < NUM_PROFILE_TRACKER_HISTORY_SLOTS; i++) {
        values[i] = 0;
        variances[i] = 0;
    }
}
