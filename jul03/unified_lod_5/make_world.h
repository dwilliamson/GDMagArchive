struct Triangle_List_Mesh;
struct Fast_Rendering_Vertex_Data;
struct Mesh_Reducer;
struct World_Block;
struct Seam_Database;
struct Mesh_Seam;

typedef unsigned long Block_Identifier;
const Block_Identifier NO_BLOCK = 0xffffffff;

enum Lod_Instance_State {
    I_AM_NOT_INVOLVED,
    I_AM_SINGLE,
    I_AM_TRANSITIONING
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
struct World_Block {
    World_Block();
    ~World_Block();

    Vector3 position;

    Vector3 bounding_box_corner;
    Vector3 bounding_box_extents;

    float edge_length;
    float worldspace_error;

    Triangle_List_Mesh *mesh;
    Block_Identifier block_id;


    int *index_map;  // Used only during preprocess time, NULL at runtime
    int *index_map_into_parent;  // Used only during preprocess time, NULL at runtime



    int temporary_integer_storage;  // XXX needed?
    int block_search_marker;

    World_Block *parent;  // XXX needed?
    World_Block *children[MAX_CHILDREN];
    int num_children;

    int leaf_distance;


    // Stuff used only at runtime:
    Lod_Instance_State lod_instance_state;

    float existence_parameter;
    float opacity;
    float distance_from_viewpoint;

    float distance_at_which_i_begin_subdividing;
    float distance_of_lod_demand;
    float distance_at_which_i_am_gone;

    int frame_index;
    bool use_zfunc_lessequal;
};


inline int Elevation_Map::get_index(int x, int y) {
    return y * num_samples_x + x;
}


void save_world(World_Block *tree, Seam_Database *database, File_Handle *f);
World_Block *load_world(File_Handle *f, Seam_Database **database_return);

World_Block *make_static_world_block(Elevation_Map *map,
                                                int block_samples);
Elevation_Map *load_elevation_map(char *filename);
Elevation_Map *make_dummy_elevation_map(float units_per_sample, int elevation_map_samples,
                                        float bump_f1, float bump_f2, float bump_height);


void init_rendering_data(World_Block *block);
void make_world_block(Elevation_Map *map, World_Block *block,
                        int samples_i, int samples_j,
                        int offset_i, int offset_j);
void init_block_for_mesh(World_Block *block, Triangle_List_Mesh *mesh);



void build_tangent_frames(Elevation_Map *map);


Vector3 get_position(Elevation_Map *map, int index);
Vector3 get_uv(Elevation_Map *map, int index);
Vector3 get_position(Elevation_Map *map, int i, int j);
Vector3 get_uv(Elevation_Map *map, int i, int j);
                   

Triangle_List_Mesh *merge_meshes(int num_meshes, Triangle_List_Mesh **meshes, 
                                 Vector3 *offsets, int **index_maps,
                                 Auto_Array <Mesh_Seam *> *seams);
