struct Triangle_List_Mesh;

struct Reducer_Face_Membership;
struct Lightweight_Proximity_Grid;
struct Priority_Queue;
struct Mesh_Topology_Handler;
struct Reducer_Priority_Queue_Data;
struct Error_Quadric;
struct Mesh_Builder;

#include "auto_array.h"

enum Reducer_Face_Flags {
    FACE_IS_A_SEAM_FILL = 0x1
};

struct Reducer_Face {
    int indices[3];
    int material;
    int flags;
};

struct Mesh_Reducer {
    Mesh_Reducer();
    ~Mesh_Reducer();

    void init(Triangle_List_Mesh *mesh);
    void collapse_similar_vertices(float distance_threshold);
    void mark_face_as_seam(int face_index);
    void reduce(int num_target_faces);

    void get_result(Triangle_List_Mesh **result);

    Triangle_List_Mesh *mesh;

    // Configuration options below:         ---------------

    struct Tuning {
        float material_boundary_penalty;
        float topology_change_penalty;
        float icky_face_penalty;
        float texture_space_importance_factor;
        float lonely_edge_constraint_factor;
    } tuning;

    int   num_target_faces;

    // End configuration options.           ---------------

    Mesh_Topology_Handler *topology_handler;

    int   initial_num_vertices;
    Priority_Queue *priority_queue;
    Lightweight_Proximity_Grid *proximity_grid;

    float *material_area_uv;
    float *material_area_xyz;
    float *material_factor_uv_over_xyz;

    Error_Quadric *error_quadrics;

    int last_v0_examined;
    int last_v1_examined;
    float hunt_radius_expansion_factor;

    // Stat-keeping stuff:

    int num_lonely_edges_detected;

    // Methods.

    void remove_vertex_from_grid(int index);

    void perform_one_reduction();
    void perform_one_reduction(int kill_v0, int kill_v1);

    void update_best_candidates(int vertex_index,
                                float *best_error_result,
                                int *vest_v1_result);
    void update_best_candidates(int index0, int index1,
                                float *best_error_result,
                                int *best_v1_result);
    float compute_collapse_error(int index0, int index1);

    void init_priority_queue();
    void queue_single_vertex(int index);
    void update_single_vertex_without_queueing(int index,
                                               float *error_result,
                                               Reducer_Priority_Queue_Data *data_result);

    int find_alias_to_map_to(int from_index, int to_index);

    void handle_lonely_edges();
    void compensate_for_lonely_edge(Reducer_Face *face, 
                                    int vertex_index_0, int vertex_index_1);

    void count_material_areas();
    void add_face_constraint_to_quadrics(Reducer_Face *);
    void init_quadrics();

    Error_Quadric *get_quadric(int index);
    void fill_point(float *point, int vertex_index, int material_index);


    Mesh_Builder *prepare_result(int **remap = NULL);

    void validate_collapse(int v0, int v1);
    void validate_priority_queue();

    float find_hunt_radius_for_vertex(int vertex_index, Auto_Array <int> *targets);


    bool vertex_is_marked(int index);
    void mark_vertex(int index);
    void unmark_vertex(int index);
    void collect_and_mark_vertex_star(int vertex_index, Auto_Array <int> *targets);
};

