struct Face_Data {
    int    vertex_indices[3];
    Vector texture_vertices[3];

    Vector normals[3];
    Vector tangents[3];
    Vector cross_tangents[3];
    int material_index;
};

const int MAX_MATERIALS = 16;
struct Brdf_Map_Info;

struct Triangle_Model {
    Triangle_Model(int nvertices, int nfaces, int nmaterials);
    ~Triangle_Model();

    Triangle_Model *copy();

    Vector position;
    Quaternion orientation;

    int nvertices;
    int nfaces;
    int nmaterials;

    Vector *vertices;
    Vector *vertex_normals;
    Face_Data *faces;

    Vector *face_vertices;

    void condense();
    void recompute_normals_and_frames();

    Vector *condensed_vertices;
    Vector *condensed_normals;
    Vector *condensed_tangents;
    Vector *condensed_cross_tangents;
    Vector *condensed_texture_vertices;
    int    *condensed_indices;

    Vector *condensed_vertex_colors;

    Vector *face_brdf_channel_0;
    Vector *face_brdf_channel_1;
    Vector *face_brdf_channel_ambient;

    int    num_condensed_items;

    int material_indices[MAX_MATERIALS];

  private:
    void condense_face_vertex(Face_Data *face, int face_index, 
			      int vertex_index);
    int find_a_match(Vector *position, Vector *normal, Vector *tangent,
		     Vector *texture_coordinates);
};
