struct Quaternion;

struct Mesh_Material_Info {
    char *name;    // Used for export only?
    int texture_index;
};

struct Triangle_List_Info {
    int material_index;  // Which material do we use to render these?
    int num_vertices;    // How many vertices are in the list?  (must be a multiple of 3)
    int start_of_list;   // What is the index of the beginning of this list?
};

struct Triangle_List_Mesh {
    Triangle_List_Mesh();
    ~Triangle_List_Mesh();

    // Things that all mesh types contain.

    void allocate_materials(int n);  // Used once at init only!
    void allocate_geometry(int num_vertices, int num_faces);

    int num_vertices;    // How many vertices are in the mesh
    int num_faces;       // How many faces are in the mesh

    Vector3 *vertices;          // Vertex data (1 vector per vertex)
    Vector2 *uvs;            // Texture coordinates (1 per vertex)

    int *indices;
    int num_indices;                // Total length of array 'indices'

    int *canonical_vertex_map;

    Quaternion *tangent_frames;  // One per vertex

    int num_triangle_lists;
    Triangle_List_Info *triangle_list_info; // There are 'num_triangle_lists' of these

	int num_materials;         // How many materials are used in this mesh.
    Mesh_Material_Info *material_info;   // Materials used.

    char *name;

    void *user_data;   // The app can set this to whatever it wants.
                       // user_data does not get saved or loaded; 
                       // it is initialized to NULL.

    void update_normals_slow(float *vertices, float *vertex_normals_result,
                             int source_stride_in_bytes = -1, int dest_stride_in_bytes = -1);

};

void copy_material_info(Mesh_Material_Info *dest, Mesh_Material_Info *src);

Triangle_List_Mesh *load_triangle_list_mesh(FILE *f);
void save_triangle_list_mesh(Triangle_List_Mesh *mesh, FILE *f);
