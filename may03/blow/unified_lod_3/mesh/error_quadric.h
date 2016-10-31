const int ERROR_QUADRIC_DIMENSIONS = 5;
const int D = ERROR_QUADRIC_DIMENSIONS;
const int ERROR_QUADRIC_MATRIX_ENTRIES = (D * (D + 1)) / 2;

struct Error_Quadric {
    double matrix[ERROR_QUADRIC_MATRIX_ENTRIES];
    double vector[ERROR_QUADRIC_DIMENSIONS];
    double scalar;

    static int QuadricMatrixIndexTable[ERROR_QUADRIC_DIMENSIONS * ERROR_QUADRIC_DIMENSIONS];

    void clear();

    void accumulate_quadric(Error_Quadric *);
    void accumulate_plane(float *p0, float *p1,
                          float *p2, float extra_scale = 1.0f);
    double evaluate_error(float *Coordinates);

    void init_index_table();

  private:
    double get_matrix_value(int i, int j);
};
