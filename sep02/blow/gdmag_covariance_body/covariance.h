struct Covariance_Matrix2 {
    float a, b, c;

    void reset();

    Covariance_Matrix2 add(const Covariance_Matrix2 &other);
    Covariance_Matrix2 invert();

	void rotate(float theta);
    void scale(float factor);
	
    void move_to_global_coordinates(Covariance_Matrix2 *dest, float x, float y);
    void move_to_local_coordinates(Covariance_Matrix2 *dest, float x, float y);

    int find_eigenvectors(float eigenvalues[2], Vector3 eigenvectors[2]);
    int find_eigenvalues(float eigenvalues[2]);
};


