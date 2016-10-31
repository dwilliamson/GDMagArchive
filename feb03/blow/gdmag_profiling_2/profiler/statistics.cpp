#include "app_shell.h"
#include "statistics.h"

#include <math.h>
#include <float.h>

Statistics::Statistics() {
    min = FLT_MAX;
    max = -FLT_MAX;
    mean = variance = stdev = 0;
    valid = false;
    sample_count = 0;
    sum_1 = sum_2 = 0;
}

void Statistics::add(float value) {
    sample_count++;
    sum_1 += value;
    sum_2 += value * value;
    if (value < min) min = value;
    if (value > max) max = value;
}

void Statistics::finish() {
    if (sample_count == 0) return;
    assert(sample_count > 0);

    float factor = 1.0f / sample_count;
    mean = sum_1 * factor;

    variance = (sum_2 - sum_1 * sum_1) * factor;
    variance = fabs(variance);

    stdev = sqrt(variance);

    valid = true;
}
