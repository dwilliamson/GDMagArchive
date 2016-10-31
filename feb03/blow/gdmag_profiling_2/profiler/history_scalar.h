#pragma once

#ifndef __HISTORY_SCALAR_H
#define __HISTORY_SCALAR_H

const int NUM_PROFILE_TRACKER_HISTORY_SLOTS = 3;
struct History_Scalar {
    float values[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    float variances[NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    void update(float new_value, float *k_array);
    void set_for_all_past_history(double value);

    void clear();
};

#endif
