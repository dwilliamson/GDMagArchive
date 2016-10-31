struct Filter;
struct Image_Buffer;

const int MAX_IMAGE_CHANNELS = 3;

struct Image_Channel {
    float *data;
    int width, height;
    Image_Buffer *my_image_buffer;

    Image_Channel(Image_Buffer *image_buffer, int width, int height);
    ~Image_Channel();

    int get_index(int i, int j);
    int get_index_with_reflecting(int i, int j);
    void downsample_into(Image_Channel *result, Filter *filter);
    void downsample_into_fft(Image_Channel *result);
    void clamp(float low, float high);
    void exponentiate(double power);
};

struct Image_Buffer {
    Image_Buffer(int width, int height);
    ~Image_Buffer();

    Image_Buffer *copy();
    int get_index(int i, int j);
    int get_index_with_reflecting(int i, int j);
    void clamp(float low, float high);
    void downsample_into(Image_Buffer *result, Filter *filter);
    void downsample_into_fft(Image_Buffer *result);
    void exponentiate(double power);

    int width, height;

    int nchannels;
    Image_Channel *channels[MAX_IMAGE_CHANNELS];
};


inline int Image_Buffer::get_index(int i, int j) {
    assert(i >= 0);
    assert(i < width);
    assert(j >= 0);
    assert(j < height);

    return j * width + i;
}

inline int Image_Buffer::get_index_with_reflecting(int i, int j) {
    while ((i < 0) || (i > (width - 1))) {
        if (i < 0) i = -i;
        if (i >= width) i = width + width - i - 1;
    }

    while ((j < 0) || (j > (height - 1))) {
        if (j < 0) j = -j;
        if (j >= height) j = height + height - j - 1;
    }

    return get_index(i, j);
}

inline int Image_Channel::get_index(int i, int j) {
    return my_image_buffer->get_index(i, j);
}

inline int Image_Channel::get_index_with_reflecting(int i, int j) {
    return my_image_buffer->get_index_with_reflecting(i, j);
}
