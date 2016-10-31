struct Mesh_Seam;
struct Static_Terrain_Block;

struct Rewrite_Rule {
    Static_Terrain_Block *source_block;
    Static_Terrain_Block *dest_block;
    int *index_map;
};

struct Seam_Database {
    Seam_Database();
    ~Seam_Database();

    void add(Mesh_Seam *seam);
    void delete_high_resolution_seams();
    void perform_rewrite_rules(Auto_Array <Rewrite_Rule *> *rules,
                               int max_hierarchy_distance,
                               int starting_index = 0);  // Umm... always call with starting_index = 0, please..  XXX exposure of this is messy, fix it.
    void remove(Mesh_Seam *seam);
//    void find_single_block_seams(Static_Terrain_Block *block, List *results);
    void find_seams_containing_these_blocks(Static_Terrain_Block **blocks, int num_blocks, Auto_Array <Mesh_Seam *> *results);
    void collect_fully_marked_seams(Auto_Array <Mesh_Seam *> *seams);

    Auto_Array <Mesh_Seam *> seams;

    int num_seams_added_directly;
    int num_seams_created_by_rewrites;
    int num_seams_merged;

  protected:
    void collect_seams(Static_Terrain_Block *block, int n0, int n1, List *results);
    int merge_seams();

    Mesh_Seam *rewrite_seam(Mesh_Seam *old_seam,
                            Static_Terrain_Block *source_block,
                            Static_Terrain_Block *dest_block,
                            int *index_map,
                            int max_hierarchy_distance);
};

extern Seam_Database *seam_database;  // XXX Do this better
