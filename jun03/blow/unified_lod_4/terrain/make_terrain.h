struct Triangle_List_Mesh;
struct Fast_Rendering_Vertex_Data;
struct Mesh_Reducer;
struct Static_Terrain_Block;
struct Seam_Database;

typedef unsigned long Block_Identifier;
const Block_Identifier NO_BLOCK = 0xffffffff;

enum Lod_Instance_State {
    I_AM_NOT_INVOLVED,
    I_AM_SINGLE,
    I_AM_TRANSITIONING
};


struct Seam_Index {
    short which_mesh;             // This will be 0 or 1
    unsigned short vertex_index;  // On that mesh
};

struct Mesh_Seam {
    Mesh_Seam(int num_faces);
    ~Mesh_Seam();

    void remove_degenerate_faces();
    void sort_block_membership();
    void simplify_block_membership();

    Static_Terrain_Block *block_membership[3];

    int num_faces;
    Seam_Index *indices;  // num_faces * 3 of these
    bool is_high_res_seam;
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


const int MAX_CHILDREN = 4;
struct Static_Terrain_Block {
    Static_Terrain_Block();
    ~Static_Terrain_Block();

    Vector3 position;

    Vector3 bounding_box_corner;
    Vector3 bounding_box_extents;

    float edge_length;

    Triangle_List_Mesh *mesh;
    Block_Identifier block_id;


    int *index_map;  // Used only during preprocess time, NULL at runtime
    int *index_map_into_parent;  // Used only during preprocess time, NULL at runtime



    int temporary_integer_storage;  // XXX needed?
    int block_search_marker;

    Static_Terrain_Block *parent;  // XXX needed?
    Static_Terrain_Block *children[MAX_CHILDREN];
    int num_children;

    int my_child_index;    // XXX needed?
    int leaf_distance;

    // Stuff used only at runtime:
    Lod_Instance_State lod_instance_state;

    float existence_parameter;
    float opacity;
    float distance_from_viewpoint;

    int frame_index;
    bool use_zfunc_lessequal;
};


inline int Elevation_Map::get_index(int x, int y) {
    return y * num_samples_x + x;
}


void save_terrain(Static_Terrain_Block *tree, Seam_Database *database, File_Handle *f);
Static_Terrain_Block *load_terrain(File_Handle *f, Seam_Database **database_return);

Static_Terrain_Block *make_static_terrain_block(Elevation_Map *map,
                                                int block_samples);
Elevation_Map *load_elevation_map(char *filename);
Elevation_Map *make_dummy_elevation_map(float units_per_sample, int elevation_map_samples,
                                        float bump_f1, float bump_f2, float bump_height);


void init_rendering_data(Static_Terrain_Block *block);
void make_terrain_block(Elevation_Map *map, Static_Terrain_Block *block,
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
                   
