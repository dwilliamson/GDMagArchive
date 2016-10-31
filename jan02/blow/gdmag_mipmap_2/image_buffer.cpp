//#include "stdafx.h"  // STUPID MICROSOFT COCKSUCKERS

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "image_buffer.h"
#include "filter.h"
#include "fourier.h"

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


inline int foldover(int i, int width) {
    if (i >= width / 2) {
        i = (width - 1 - i);
        assert(i >= 0);
    }

    return i;
}

void Image_Channel::downsample_into_fft(Image_Channel *result) {
    Image_Channel *tmp_real = new Image_Channel(my_image_buffer, width, height);
    Image_Channel *tmp_imag = new Image_Channel(my_image_buffer, tmp_real->width,
                                                 tmp_real->height);

    Image_Channel *freq_real = new Image_Channel(my_image_buffer, tmp_real->width,
                                                 tmp_real->height);
    Image_Channel *freq_imag = new Image_Channel(my_image_buffer, tmp_real->width,
                                                 tmp_real->height);

    int max_dimension = tmp_real->width;
    if (tmp_real->height > max_dimension) max_dimension = tmp_real->height;

    float *input_real = new float[max_dimension];
    float *output_real = new float[max_dimension];
    float *input_imag = new float[max_dimension];
    float *output_imag = new float[max_dimension];

    // Do the two-dimensional forward FFT.  To accomplish
    // this we first do a horizontal pass, then a vertical
    // pass.

    // Horizontal pass.

    int i, j;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int index = tmp_real->get_index(i, j);
            input_real[i] = data[index];
            input_imag[i] = 0;
        }

        fft_float(width, 0, input_real, input_imag, output_real, output_imag);

        for (i = 0; i < width; i++) {
            int index = tmp_real->get_index(i, j);
            tmp_real->data[index] = output_real[i];
            tmp_imag->data[index] = output_imag[i];
        }
    }

    // Vertical pass.

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            int index = tmp_real->get_index(i, j);
            input_real[j] = tmp_real->data[index];
            input_imag[j] = tmp_imag->data[index];
        }

        fft_float(height, 0, input_real, input_imag, output_real, output_imag);

        for (j = 0; j < height; j++) {
            int index = freq_real->get_index(i, j);
            freq_real->data[index] = output_real[j];
            freq_imag->data[index] = output_imag[j];
        }
    }

    // Do the zeroing here.
    
    int halfwidth = width / 2;
    int quarterwidth = width / 4;

    int halfheight = height / 2;
    int quarterheight = height / 4;

    int i0 = quarterwidth;
    int i1 = halfwidth + quarterwidth;
    int j0 = quarterheight;
    int j1 = halfheight + quarterheight;

//    j0 = 0;
//    j1 = height;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int index = freq_real->get_index(i, j);
            if (((i >= i0) && (i < i1)) || ((j >= j0) && (j < j1))) {
                freq_real->data[index] = 0;
                freq_imag->data[index] = 0;
            }
        }
    }

    // Reverse FFT.  For kicks we do vertical pass first,
    // then horizontal pass, though the order should not matter.


    // Vertical pass.

    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            int index = freq_real->get_index(i, j);
            input_real[j] = freq_real->data[index];
            input_imag[j] = freq_imag->data[index];
        }

        fft_float(height, 1, input_real, input_imag, output_real, output_imag);

        for (j = 0; j < height; j++) {
            int index = tmp_real->get_index(i, j);
            tmp_real->data[index] = output_real[j];
            tmp_imag->data[index] = output_imag[j];
        }
    }

    // Horizontal pass.

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int index = tmp_real->get_index(i, j);
            input_real[i] = tmp_real->data[index];
            input_imag[i] = tmp_imag->data[index];
        }

        fft_float(width, 1, input_real, input_imag, output_real, output_imag);

        for (i = 0; i < width; i++) {
            int index = tmp_real->get_index(i, j);
            tmp_real->data[index] = output_real[i];
            // output_imag is discarded.  Should be 0 within roundoff error.
        }
    }

    delete [] input_real;
    delete [] input_imag;
    delete [] output_real;
    delete [] output_imag;

    // Actually make the downsampled image.

    for (j = 0; j < height; j += 2) {
        for (i = 0; i < width; i += 2) {
            int src_index = tmp_real->get_index(i, j);
            int dest_index = result->get_index(i >> 1, j >> 1);

            result->data[dest_index] = tmp_real->data[src_index];
        }
    }

    delete tmp_real;
    delete tmp_imag;
    delete freq_real;
    delete freq_imag;
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

void Image_Buffer::downsample_into_fft(Image_Buffer *result) {
    int i;
    for (i = 0; i < nchannels; i++) {
        channels[i]->downsample_into_fft(result->channels[i]);
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
