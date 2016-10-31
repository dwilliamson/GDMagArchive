struct Mesh_Builder {
    struct Face {
        int n0, n1, n2;
        int material_index;
    };

    Mesh_Builder(int max_vertices, int max_faces);
    ~Mesh_Builder();

    Triangle_List_Mesh *build_mesh();

    int num_vertices;
    Vector3 *vertex_positions;
    Vector2 *vertex_uvs;
    Quaternion *tangent_frames;

    int num_faces;
    Face *faces;

    int max_vertices;
    int max_faces;

    List materials;

    
    void add_triangle(int n0, int n1, int n2, int material_index);
    void add_material(Mesh_Material_Info *info);
    int add_vertex(Vector3 position, Vector2 uv, Quaternion frame);

  protected:
    int find_end_of_matching_materials(int cursor);
    int count_triangle_lists();
    int do_one_triangle_list(int cursor, int list_index,
                             Triangle_List_Mesh *result);
};

