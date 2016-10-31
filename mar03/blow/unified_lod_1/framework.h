struct Texture_Vertex {
    float u, v;

    void set(float u, float v);
};

#include "app_shell.h"
#include "framework/os_win32.h"
#include "framework/data_structures.h"
#include "framework/geometry.h"
#include "framework/geometry_supplement.h"
#include "framework/projector.h"
#include "framework/auto_array.h"


struct Font;
struct Projector;
struct Shader;
struct Arguments;

struct Client_Globals {
    Font *big_font;
    Font *small_font;

    Projector *view_projector;

    Vector3 camera_position;
    Quaternion camera_orientation;
};

extern Client_Globals client_globals;

struct Fast_Rendering_Vertex_Data {
    int num_vertices;
    int num_indices;
    int vertex_size_in_bytes;

    float *pointer_to_position_data(int vertex_index);
    float *pointer_to_vertex_normal_data(int vertex_index);
    unsigned long *pointer_to_diffuse_argb8888_data(int vertex_index);
    float *pointer_to_uv_data(int vertex_index);
    unsigned short *pointer_to_index_data(int vertex_index);

    int use_vertex_normals;
    int use_diffuse_argb8888;
    int num_uv_channels;

    int offset_diffuse_argb8888;
    int offset_vertex_normals;
    int offset_uvs;

    void *winsys_specific_1;
    void *winsys_specific_2;
    int winsys_specific_3;

    void *user_data;

    void *locked_data_pointer;
    void *locked_index_data_pointer;
};

void os_sleep(int milliseconds);

inline char *app_strdup(char *s) {
    int len = strlen(s);
    char *t = (char *)malloc(len + 1);
    strcpy(t, s);
    return t;
}


#define Min(x, y) (((x)>(y)) ? (y):(x))
#define Max(x, y) (((x)>(y)) ? (x):(y))

// Although Clamp is a useful one that gets used hugely.
#define Clamp(f, a, b) { if ((f) < (a)) (f) = (a); if ((f) > (b)) (f) = (b); }
