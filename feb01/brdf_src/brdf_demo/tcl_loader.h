const int MAX_VERTICES = 80000;
const int MAX_FACES = 80000;

struct Tcl_Loader {
  public:
    Tcl_Loader();
    ~Tcl_Loader();

    Triangle_Model *load(char *filename);

  private:
    FILE *current_file;
    int nfaces;
    int nvertices;

    int vertices_in_this_polygon;

    Vector current_texture_coords[3];
    Vector current_normal[3];
    Vector current_frame_vector[3];
    int    current_vector_index[3];

    char line_data[BUFSIZ];

    Face_Data faces[MAX_FACES];
    Vector vertices[MAX_VERTICES];

    char *get_command_name(char *s, char **s_ret);
    char *get_next_line();

    void finish_current_polygon();
    int find_vector_index(Vector v);
    void emit_a_triangle();
};

