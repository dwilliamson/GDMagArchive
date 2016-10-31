//#include "stdafx.h"  // STUPID MICROSOFT COCKSUCKERS

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "image_buffer.h"
#include "filter.h"

Image_Channel::Image_Channel(Image_Buffer *buffer, int _width, int _height) {
    my_image_buffer = buffer;
    width = _width;
    height = _height;
    int nsamples = width * height;
    data = new float[nsamples];
}

Image_Channel::~Image_Channel() {
    delete [] data;
}


Image_Buffer::Image_Buffer(int _width, int _height) {
    width = _width;
    height = _height;

    channels[0] = new Image_Channel(this, width, height);
    channels[1] = new Image_Channel(this, width, height);
    channels[2] = new Image_Channel(this, width, height);
    nchannels = 3;
}

Image_Buffer::~Image_Buffer() {
    int i;

    for (i = 0; i < nchannels; i++) {
        delete channels[i];
    }
}


void Image_Channel::downsample_into(Image_Channel *result, Filter *filter) {
    Image_Channel *tmp = new Image_Channel(my_image_buffer, width, height);

    assert(!(filter->nsamples & 1));
    int filter_offset = -((filter->nsamples / 2) - 1);

    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i += 2) {
            int dest_index = tmp->get_index(i >> 1, j);
            int src_index = get_index(i, j);
            float sum = 0;

            int k;
            for (k = 0; k < filter->nsamples; k++) {
                int src_i = i + k + filter_offset;
                int src_index = get_index_with_reflecting(src_i, j);
                sum += filter->data[k] * data[src_index];
            }

            tmp->data[dest_index] = sum;
        }
    }

    for (i = 0; i < (width >> 1); i++) {
        for (j = 0; j < height; j += 2) {
            int dest_index = result->get_index(i, j >> 1);
            int src_index = tmp->get_index(i, j);
            float sum = 0;

            int k;
            for (k = 0; k < filter->nsamples; k++) {
                int src_j = j + k + filter_offset;
                int src_index = tmp->get_index_with_reflecting(i, src_j);
                sum += filter->data[k] * tmp->data[src_index];
            }

            result->data[dest_index] = sum;
        }
    }

    delete tmp;
}

void Image_Channel::clamp(float low, float high) {
    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int index = get_index(i, j);
            float value = data[index];
            if (value < low) value = low;
            if (value > high) value = high;
            data[index] = value;
        }
    }
}

void Image_Channel::exponentiate(double power) {
    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int index = get_index(i, j);
            float value = data[index];
            data[index] = pow(value, power);
        }
    }
}

void Image_Buffer::downsample_into(Image_Buffer *result, Filter *filter) {
    int i;
    for (i = 0; i < nchannels; i++) {
        channels[i]->downsample_into(result->channels[i], filter);
    }
}

void Image_Buffer::clamp(float low, float high) {
    int i;
    for (i = 0; i < nchannels; i++) {
        channels[i]->clamp(low, high);
    }
}

void Image_Buffer::exponentiate(double power) {
    int i;
    for (i = 0; i < nchannels; i++) {
        channels[i]->exponentiate(power);
    }
}

Image_Buffer *Image_Buffer::copy() {
    Image_Buffer *result = new Image_Buffer(width, height);

    int nvalues = width * height;

    int i;
    for (i = 0; i < nvalues; i++) {
        result->channels[0]->data[i] = channels[0]->data[i];
        result->channels[1]->data[i] = channels[1]->data[i];
        result->channels[2]->data[i] = channels[2]->data[i];
    }

    return result;
}
