struct Statistics {
    Statistics();
    void add(float value);
    void finish();

    float min;
    float max;
    float mean;
    float variance;
    float stdev;

    bool valid;

  protected:
    float sum_1;
    float sum_2;
    int sample_count;
};
