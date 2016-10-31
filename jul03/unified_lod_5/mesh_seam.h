struct World_Block;

// We need Seam_Index to use an int because preprocessing uses
// real big meshes.  Probably we want to make a Runtime_Seam_Index
// and a Preprocess_Time_Seam_Index.
struct Seam_Index {
    int vertex_index;           // On that mesh
    Vector2 uv;                   // Texture coordinates
    short which_mesh;             // This will be 0 or 1
};

struct Mesh_Seam {
    Mesh_Seam(int num_faces);
    ~Mesh_Seam();

    void remove_degenerate_faces();
    void compute_uv_coordinates(Triangle_List_Mesh *);
    void sort_block_membership();
    void simplify_block_membership();

    World_Block *block_membership[3];

    int num_faces;
    Seam_Index *indices;  // num_faces * 3 of these
//    Mesh_Seam *original_seam;  // Used for database stuff.
    bool is_high_res_seam;
};

