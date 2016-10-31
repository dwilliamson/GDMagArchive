struct Mesh_Seam;
struct Triangle_List_Mesh;
struct Mesh_Material_Info;
struct Mesh_Chopper;
struct Vertex_Specifier;
struct Hash_Table;
struct Mesh_Builder;

#include "hash_table.h"  // Need to do this for Vertex_Specifier... boo!!

struct Vertex_Specifier : public Hashable {
    Vertex_Specifier(int input_n0, int input_n1, int plane_index);
    Vertex_Specifier(Vertex_Specifier *other);
    Vertex_Specifier() {};

    int compare(Hashable *other);
    int get_hash_code();

    int input_n0;
    int input_n1;
    int plane_index;
    int output_index;
};

struct Intermediate_Result {
    Intermediate_Result();
    ~Intermediate_Result();

    void init(Triangle_List_Mesh *mesh);
    void clean();

    int maybe_add_vertex(Vector3 position, Vector2 uv, 
                         Quaternion tangent_frame, Vertex_Specifier *specifier);

    Mesh_Chopper *my_chopper;
    Mesh_Builder *mesh_builder;
    Hash_Table *vertex_hash_table;

    struct Seam_Edge {
        int index_0;
        int index_1;

        Vertex_Specifier spec_0;
        Vertex_Specifier spec_1;
    };

    void register_seam_edge(int index_0, int index_1, 
                            Vertex_Specifier *spec_0,
                            Vertex_Specifier *spec_1);
    List seam_edges;

    int find_vertex(Vertex_Specifier *specifier);
  protected:

    int add_vertex(Vector3 position, Vector2 uv, Quaternion tangent_frame,
                   Vertex_Specifier *specifier);

    friend Mesh_Chopper;
};

struct Chopped_Result {
    Triangle_List_Mesh *result_mesh;
    Hash_Table *vertex_hash_table;
};

const float PLANE_EPSILON_DEFAULT = 0.0001;
struct Mesh_Chopper {
    Mesh_Chopper();
    ~Mesh_Chopper();

    void set_plane_epsilon(float new_epsilon);
    void chop_mesh(Triangle_List_Mesh *input_mesh,
                   Plane3 *plane, int plane_id,
                   Chopped_Result *result_a_return,
                   Chopped_Result *result_b_return,
                   Mesh_Seam **seam_a_to_b_return);


    int current_material_index;

    int xref_vertex(Intermediate_Result *result, int index);
    int xref_vertex_no_creation(Intermediate_Result *result, int index);

  protected:
    Mesh_Seam *build_seam();
    void register_coplanar_vertex(int input_index,
                                  Intermediate_Result *dest_result);
    void add_one_triangle(int n0, int n1, int n2);
    void add_triangle(int n0, int n1, int n2, Intermediate_Result *dest_result);
    void do_an_edge_split(int n0, int n1,
                          Vertex_Specifier *spec_return,
                          int *index_a_return, int *index_b_return);

    int classify_vertex(int index);

    void rotate_indices(int *n0, int *n1, int *n2,
                        int *c0, int *c1, int *c2);

    Vector3 plane_normal;
    float plane_d;
    int current_plane_index;

    float plane_epsilon;

    Triangle_List_Mesh *input_mesh;

    Intermediate_Result intermediate_result_a;
    Intermediate_Result intermediate_result_b;

    friend Intermediate_Result;
};

inline Vertex_Specifier::Vertex_Specifier(int _input_n0, int _input_n1, 
                                          int _plane_index) {
    input_n0 = _input_n0;
    input_n1 = _input_n1;
    plane_index = _plane_index;

    output_index = -1;
}

inline Vertex_Specifier::Vertex_Specifier(Vertex_Specifier *other) {
    input_n0 = other->input_n0;
    input_n1 = other->input_n1;
    plane_index = other->plane_index;

    output_index = other->output_index;
}

