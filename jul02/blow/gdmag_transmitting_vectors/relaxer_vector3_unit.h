struct Relaxer_Vector3_Unit {
	Relaxer_Vector3_Unit();
	~Relaxer_Vector3_Unit();
	
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

