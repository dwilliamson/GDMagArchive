struct Triangle_List_Mesh;

struct Tangent_Frame_Maker {
    Tangent_Frame_Maker();
    ~Tangent_Frame_Maker();

    // If you want to compute the frames all in one shot, use this.
    void compute_tangent_frames(Triangle_List_Mesh *mesh);

    // If you want to init/compute them incrementally, use this...
    void begin_tangent_frames(int num_vertices);
    void accumulate_triangle(int indices[3], Vector3 vertices[3], 
                             Vector3 texture_coordinates[3]);
    void complete_tangent_frames();


    int num_vertices;
    Quaternion *tangent_frames;   // You will need to delete this yourself
                                  // if you don't want it.


    Vector3 *normals, *tangents, *binormals;  // These are deleted by this class.
};

// @Robustness: Epsilons like this are really dumb since we don't
// know what scale the user's model is at.
const double LAME_ASS_EPSILON = 1.0e-12;
