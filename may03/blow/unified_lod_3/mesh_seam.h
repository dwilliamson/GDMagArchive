struct Seam_Index {
    short which_mesh;             // This will be 0 or 1
    unsigned short vertex_index;  // On that mesh
    Vector2 uv;
};

struct Mesh_Seam {
    Mesh_Seam(int num_faces);
    ~Mesh_Seam();

    void remove_degenerate_faces();
    void compute_uv_coordinates(Triangle_List_Mesh *);

    int num_faces;
    Seam_Index *indices;  // num_faces * 3 of these
};

