struct Covariance2 {
    float a, b, c;

    void reset();

    Covariance2 add(const Covariance2 &other);
    Covariance2 invert();

	void rotate(float theta);
    void scale(float factor);
	
    void move_to_global_coordinates(Covariance2 *dest, float x, float y);
    void move_to_local_coordinates(Covariance2 *dest, float x, float y);
    void translate(Covariance2 *dest, float x, float y);

    int find_eigenvectors(float eigenvalues[2], Vector3 eigenvectors[2]);
    int find_eigenvalues(float eigenvalues[2]);
};


