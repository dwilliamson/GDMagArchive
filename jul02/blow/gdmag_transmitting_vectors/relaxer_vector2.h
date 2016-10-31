// So that I don't have to bother defining a new class and stuff,
// this guy just uses a Vector3, but z is always 0.

struct Relaxer_Vector2 {
	Relaxer_Vector2();
	~Relaxer_Vector2();
	
	void init(int num_vectors);
	void relax(double timestep);
	void relax_and_draw(double timestep);

	int encode(Vector3 input);
	Vector3 decode(int index);

	Vector3 *vectors;
	int num_vectors;

  protected:
	double do_one_timestep(double dt);
	double do_one_vector(int index, double dt);
	Vector3 *workspace;
};

