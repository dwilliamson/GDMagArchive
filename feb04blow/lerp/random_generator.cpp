#include <stdlib.h>
#include "random_generator.h"

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

float Random_Generator::get_angle(float width) {
    int randint = get() % RANDRANGE;

    float num = randint - (RANDRANGE / 2);
    float denom = (float)(RANDRANGE / 2);
    
    return (num / denom) * width;
}

float Random_Generator::get_within_range(float min, float max) {
    int randint = get() % RANDRANGE;
    return min + ((float)randint / (float)RANDRANGE) * (max - min);
}

float Random_Generator::get_nonnegative_real(float max) {
    int randint = get() % RANDRANGE;

    float num = (float)(RANDRANGE - randint);
    float denom = (float)RANDRANGE;
    
    return (num / denom) * max;
}


