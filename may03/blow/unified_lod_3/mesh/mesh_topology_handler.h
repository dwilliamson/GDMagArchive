struct Triangle_List_Mesh;

enum Topology_Flags {
    VERTEX_IS_ON_LONELY_EDGE = 0x1,
    VERTEX_IS_LIVE = 0x2,
    VERTEX_IS_MARKED = 0x4
};

struct Mesh_Topology_Handler {
    void init(Triangle_List_Mesh *mesh);
    ~Mesh_Topology_Handler();

    Triangle_List_Mesh *mesh;

    int num_faces_remaining;
    int num_vertices_remaining;
    int num_vertices_originally;

    Reducer_Face *faces;
    Reducer_Face_Membership *face_membership;
    int *material_touched_by_vertex;
    int max_vertices;

    unsigned char *vertex_flags;
    int *vertex_coincidence_chain;
    

    int *new_index_to_old_index;   // XXXX currently not usable (does not get remapped from smodel to result)
    int *old_index_to_new_index;
    int *vertex_collapse_destination;

    Vector3 *output_vertices;
    Vector2 *output_uvs;
    Vector3 *output_normals;
    // @Cleanup: Normals are kinda redundant with tangent frames,
    // but we're not really using the tangent frames yet so we use
    // the normals.
    Quaternion *output_tangent_frames;


    void remap_vertex_face_indices(int old_index, int new_index);
    void eliminate_vertex_from_mesh(int dead_vertex);
    int simple_find_alias_to_map_to(int from_index, int to_index);

    void clear_faces_to_check();
    void add_face_to_check(int face_index);
    void check_degenerate_faces();

    void compact_vertices();

    bool collapse_moves_material_boundary(int from_index, int to_index);
    bool collapse_changes_topology(int from_index, int to_index);
    bool collapse_creates_icky_face(int from_index, int to_index);

    void sanity_membership();
    void sanity_membership(int orig_vertex_index);
    void assert_face_in_membership(Reducer_Face_Membership *membership, Reducer_Face *face);

  private:
    Auto_Array <int> *faces_to_check;

    void add_face_membership_to_vertex(int face_index, int vertex_index);
    void init_face_membership_info();

    bool face_would_be_icky(int face_index,
                            int old_vertex_index,
                            int new_vertex_index);
    void remap_face_membership_face_indices(int old_index, int new_index);
    void remap_face_membership_face_indices_helper(int vertex_index,
                                                   int old_face_index,
                                                   int new_face_index);
    void remove_face_membership_from_vertex(int vertex_index,
                                            int face_index);
    void build_vertex_coincidence_chain();

    void get_listed_faces(Triangle_List_Mesh *lmesh);
};

struct Reducer_Face_Membership {
    Auto_Array <int> faces;
};

