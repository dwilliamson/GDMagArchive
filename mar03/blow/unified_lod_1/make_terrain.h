struct Triangle_List_Mesh;
struct Fast_Rendering_Vertex_Data;
struct Mesh_Reducer;
struct Triangle_List_Mesh;

enum Neighbor_Direction {
    PLUS_X,
    PLUS_Y,

    NUM_NEIGHBOR_DIRECTIONS
};

enum Lod_Instance_State {
    RENDER_ME,
    RENDER_MY_CHILDREN
};


struct Seam_Index {
    short which_mesh;             // This will be 0 or 1
    unsigned short vertex_index;  // On that mesh
    Vector2 uv;                   // Texture coordinates
};

struct Mesh_Seam {
    Mesh_Seam(int num_faces);
    ~Mesh_Seam();

    void remove_degenerate_faces();
    void compute_uv_coordinates(Triangle_List_Mesh *);

    int num_faces;
    Seam_Index *indices;  // num_faces * 3 of these
};

struct Seam_Set {
    Seam_Set();

    Mesh_Seam *res_lower;
    Mesh_Seam *res_equal;
    Mesh_Seam *res_higher_a;
    Mesh_Seam *res_higher_b;
};

struct Elevation_Map {
    Elevation_Map(int num_samples_x, int num_samples_y);
    ~Elevation_Map();

    int num_samples_x, num_samples_y;
    int num_squares_x, num_squares_y;

    Vector3 square_size;
    Vector3 map_size;

    Vector3 corner;

    int get_index(int x, int y);
    float *samples;
    Quaternion *tangent_frames;
};


struct Static_Terrain_Block {
    Vector3 position;

    Vector3 bounding_box_corner;
    Vector3 bounding_box_extents;

    Triangle_List_Mesh *mesh;
    Seam_Set seam_sets[NUM_NEIGHBOR_DIRECTIONS];


    int *index_map;  // Used only during preprocess time, NULL at runtime
    int *index_map_into_parent;  // Used only during preprocess time, NULL at runtime


    int temporary_integer_storage;
};


struct Static_Terrain_Tree {
    Static_Terrain_Tree();

    Static_Terrain_Block *block;
    Static_Terrain_Tree *parent;
    Static_Terrain_Tree *children[4];

    int my_child_index;
    int leaf_distance;

    // Stuff used only at runtime:
    Lod_Instance_State lod_instance_state;
    float distance_from_viewpoint;

    int frame_index;

    float debug_camera_dist;
    float debug_r;
    float debug_iso_radius;
};


inline int Elevation_Map::get_index(int x, int y) {
    return y * num_samples_x + x;
}


void save_terrain(Static_Terrain_Tree *tree, File_Handle *f);
Static_Terrain_Tree *load_terrain(File_Handle *f);

Static_Terrain_Tree *make_static_terrain_tree(Elevation_Map *map,
                                              int block_samples);
Elevation_Map *load_elevation_map(char *filename);
Elevation_Map *make_dummy_elevation_map(float units_per_sample, int elevation_map_samples,
                                        float bump_f1, float bump_f2, float bump_height);


void init_rendering_data(Static_Terrain_Block *block);
Static_Terrain_Block *make_terrain_block(Elevation_Map *map,
                                         int samples_i, int samples_j,
                                         int offset_i, int offset_j);
void init_block_for_mesh(Static_Terrain_Block *block, Triangle_List_Mesh *mesh);



void reduce_seam(Mesh_Seam *seam, Triangle_List_Mesh *mesh_0, Mesh_Reducer *reducer);
Mesh_Seam *make_high_res_seam_x(int num_samples_x, int num_samples_y, int *index_map_a, int *index_map_b);
Mesh_Seam *make_high_res_seam_y(int num_samples_x, int num_samples_y, int *index_map_a, int *index_map_b);


void build_tangent_frames(Elevation_Map *map);


Vector3 get_position(Elevation_Map *map, int index);
Vector3 get_uv(Elevation_Map *map, int index);
Vector3 get_position(Elevation_Map *map, int i, int j);
Vector3 get_uv(Elevation_Map *map, int i, int j);
                   
