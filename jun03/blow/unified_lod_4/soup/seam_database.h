struct Mesh_Seam;
struct World_Block;

struct Rewrite_Rule {
    World_Block *source_block;
    World_Block *dest_block;
    int *index_map;
};

struct Seam_Database {
    Seam_Database();
    ~Seam_Database();

    void add(Mesh_Seam *seam);
    void delete_high_resolution_seams();
    void perform_rewrite_rule(Rewrite_Rule *rule,
                              int max_hierarchy_distance);
    void remove(Mesh_Seam *seam);
//    void find_single_block_seams(World_Block *block, List *results);
    void find_seams_containing_these_blocks(World_Block **blocks, int num_blocks, Auto_Array <Mesh_Seam *> *results);
    void find_seams_containing_at_least_this_block(World_Block *block, Auto_Array <Mesh_Seam *> *results);
    void collect_fully_marked_seams(Auto_Array <Mesh_Seam *> *seams);
    void delete_seams_that_touch_marked_blocks();

    Auto_Array <Mesh_Seam *> seams;

    int num_seams_added_directly;
    int num_seams_created_by_rewrites;
    int num_seams_merged;

  protected:
    void collect_seams(World_Block *block, int n0, int n1, List *results);
    int merge_seams();

    Mesh_Seam *rewrite_seam(Mesh_Seam *old_seam,
                            World_Block *source_block,
                            World_Block *dest_block,
                            int *index_map,
                            int max_hierarchy_distance);
};

extern Seam_Database *seam_database;  // XXX Do this better
