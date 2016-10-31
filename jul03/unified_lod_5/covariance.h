/*

  This file contains classes to compute the covariance of
  n-dimensional vectors.  'Covariance2' and 'Covariance3' are
  hardcoded versions to work in 2 and 3 dimensions, at times
  when efficiency is very important.  Otherwise, you can use
  the struct 'Covariance' which generalizes to any number of
  dimensions.  Right now there is no Covariance4, though I 
  can envision cases when it might be useful (a recent project
  I've been thinking about, regarding animation compression,
  might benefit from a hardcoded Covariance4).

*/

struct Covariance2 {
    float a, b, c;   // A 2x2 symmetric matrix; 3 coefficients.

    void reset();    // Set all coefficients to 0.

    void accumulate(float x, float y);
    Covariance2 add(const Covariance2 &other);

    Covariance2 invert();   // Set myself to the inverse of my 2x2 matrix.

    // Make me as though all my member points were rotated by theta...
	void rotate(float theta);

    // Scale each coefficient of my matrix by this factor.
    void scale(float factor);

	// Give me the covariance matrix that results when my mass
    // is located at (x, y) but we decide to view it from (0, 0).
    void move_to_global_coordinates(Covariance2 *dest, float x, float y);

	// Give me the covariance matrix that results when my mass
    // is located at (0, 0) but we decide to view it from (x, y).
    void move_to_local_coordinates(Covariance2 *dest, float x, float y);

    // Compute the eigenvalues of this matrix (the lengths of the axes
    // of my ellipse).
    int find_eigenvalues(float eigenvalues[2]);

    // Compute the eigenvectors and eigenvalues of this matrix (all the
    // information you need to describe / draw the ellipse).
    int find_eigenvectors(float eigenvalues[2], Vector2 eigenvectors[2]);
};


// Comments for Covariance3 and Covariance are very brief; they are like
// Covariance2.  See above.

struct Covariance3 {
    float a, b, c, d, e, f;  // 3x3 symmetric matrix; 6 coefficients
    void reset();

    void accumulate(const Vector3 &p);
    Covariance3 add(const Covariance3 &other);

    void scale(float factor);
    void move_to_global_coordinates(Covariance3 *dest, const Vector3 &pos);
    void move_to_local_coordinates(Covariance3 *dest, const Vector3 &pos);

    int find_eigenvalues(float eigenvalues[3]);
    int find_eigenvectors(float eigenvalues[3], Vector3 eigenvectors[3]);
};

/*
  One important difference between Covariance and the above classes
  is that Covariance has a memory responsibility imposed by the fact
  that it has to allocate its coefficient array.  Because of this,
  we did not take the path of providing an 'add' function as with
  the above classes; because that would involve returning a copy,
  and this struct is no longer lightweight.  So instead we provide
  an 'accumulate' that modifies the matrix in-place.
*/

struct Covariance {
    Covariance(int num_dimensions);
    ~Covariance();

    int num_dimensions;
    float *coefficients;  // n by n symmetric matrix; (1/2)n(n+1) coefficients.

    void reset();
    void accumulate(float *vector);
    // If you 'accumulate' one Covariance with another, they must
    // be of the same dimensionality.
    void accumulate(Covariance *other);  

    void scale(float factor);
    void move_to_global_coordinates(Covariance *dest, const Vector3 &pos);
    void move_to_local_coordinates(Covariance *dest, const Vector3 &pos);

    // The space for 'eigenvalues_return' and 'eigenvectors_return'
    // should be provided by the caller.
    int find_eigenvalues(float *eigenvalues_return);
    int find_eigenvectors(float *eigenvalues_return, 
                          float **eigenvectors_return);
};


void find_covariance_of_convex_polygon(Vector3 *vertices, int num_vertices,
									   Covariance2 *result);
void find_covariance_of_triangle(const Vector3 &p0, const Vector3 &p1,
								 const Vector3 &p2, Covariance2 *result);


inline void Covariance2::reset() {
    a = b = c = 0;
}

inline void Covariance2::accumulate(float x, float y) {
    a += x*x;
    b += x*y;
    c += y*y;
}

