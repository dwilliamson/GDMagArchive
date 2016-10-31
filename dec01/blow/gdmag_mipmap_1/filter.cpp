#include <assert.h>
#include "filter.h"

Filter::Filter(int _width) {
    nsamples = _width;
    data = new float[nsamples];

    int i;
    for (i = 0; i < nsamples; i++) data[i] = 0;
}

Filter::~Filter() {
    delete [] data;
}

void Filter::normalize() {
    float sum = 0;

    int i;
    for (i = 0; i < nsamples; i++) sum += data[i];

    assert(sum != 0);
    if (sum == 0) return;

    for (i = 0; i < nsamples; i++) data[i] /= sum;
}
