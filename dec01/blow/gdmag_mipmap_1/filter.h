// Filter is a 1-dimensional filter of floating-point coefficients.
// The Image_Buffer passes this across an image.

struct Filter {
    Filter(int width);
    ~Filter();

    void normalize();

    float *data;
    int nsamples;
};

