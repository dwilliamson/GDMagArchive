#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#pragma warning (disable:4710) // function not inlined...W4 on ostream.h

#include <stdio.h>
#include "geometry.h"
#include "Brdf_Map.h"

#include <assert.h>
#include <ddraw.h>
#include <fstream.h>
#include <strstrea.h>
#include <iomanip.h>
#include <math.h>

#include <windows.h>
#include <windowsx.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include "glrender.h"
#include "glext.h"

#include "triangle_model.h"
#include "tcl_loader.h"
#include "bmp_load.h"


//
// I leave ALLOW_MIPMAPPING off right now because for some reason,
// if I turn it on, the color quality of the app goes to hell.
// I think it has something to do with gluBuild2DMipmaps sucking,
// but I am not sure.
//

const int ALLOW_MIPMAPPING = 0;


enum { 
   DRAWING_SINGLE_LARGE_TEAPOT = 0,
   DRAWING_MULTIPLE_TEAPOTS = 1,
   DRAWING_ANISOTROPIC_DEMO = 2,
   NUM_OBJECT_VIEWING_MODES
};

int update_brdf_coordinates = 1;
int do_channel_0 = 1;
int do_channel_1 = 1;
int do_overbright = 0;
int ambient_light = 0;
int intensity_choke = 0;
int show_fps_only = 0;
int use_light_source = 1;

int screen_i = 0;
int screen_j = 0;


GLuint font_handle = 0;

float last_object_theta = 0.0f;
float last_light_theta = 0.0f;

double last_scale_down;
double last_delta;
double last_wanted_delta;
int last_mag_factor;
int mag_workspace;
double scale_down_workspace;

int animate_orientation = 1;
int animate_light = 0;
int use_mipmaps = 0;
int object_viewing_mode = 0;

bool app_is_active = true;

void (__stdcall *glActiveTextureARB)(GLenum);
void (__stdcall *glClientActiveTextureARB)(GLenum);

unsigned int GetCommandLineArgInt( char const *pArgument,
                                   char const *pCommandLine,
                                   int unsigned DefaultValue );
bool GetCommandLineArgString( char const *pArgument,
                              char const *pCommandLine,
                              char *pStringBuffer );
int AppPaint(HWND, HDC);

struct Triangle_Model;
struct Brdf_Map_Info;
class Brdf_Renderer;

const int MAX_EXTENSIONS = 1024;
const int MAX_BRDF_MAPS = 256;
const int MAX_RENDER_PROCS = 16;

const float AMBIENT_INTENSITY = 0.3f;


typedef void (Render_Proc)(Brdf_Renderer *, Triangle_Model *,
			   Brdf_Map_Info *);

void render_vanilla_proc(Brdf_Renderer *ren, Triangle_Model *model,
			 Brdf_Map_Info *info);
void render_gouraud_vanilla_proc(Brdf_Renderer *ren, Triangle_Model *model,
				 Brdf_Map_Info *info);
void render_extensions_proc(Brdf_Renderer *ren, Triangle_Model *model,
			    Brdf_Map_Info *info);
void render_nv4_proc(Brdf_Renderer *ren, Triangle_Model *model,
		     Brdf_Map_Info *info);
void render_gouraud_extensions_proc(Brdf_Renderer *ren, Triangle_Model *model,
				    Brdf_Map_Info *info);


class Brdf_Renderer {
  public:
    void init_rendering_state();
    void draw_model(Triangle_Model *);

    int triangles_drawn;
    int frames_computed;
    int passes_rendered;

    int num_brdf_maps;
  
    Brdf_Map_Info *load_brdf_map(char *filename, char *displayable_name,
				 double target_lambda = -1.0,
				 double ratio_bias = 0.5);

    Brdf_Map_Info *map_info[MAX_BRDF_MAPS];

    Render_Proc *render_procs[MAX_RENDER_PROCS];
    char *render_proc_names[MAX_RENDER_PROCS];
    double render_proc_deltas[MAX_RENDER_PROCS];
    int num_render_procs;
    int current_render_proc;

    GLuint default_texture_handle;

  protected:
    void init_gl_extensions();
    bool lookup_extension_name(char *);
    void add_render_proc(Render_Proc *, char *name, double max_delta = 4.0);
  

    char *extension_list[MAX_EXTENSIONS];
    int num_extensions;

    enum Extension_Flag {
        EXTENSION_TEXTURE_ENV_COMBINE  = 1,
        EXTENSION_TEXTURE_ENV_COMBINE4 = 2
    };

    unsigned long extensions_found;
};

enum {
    CHANNEL_0, CHANNEL_1 
};

struct Decaying_Average {
    Decaying_Average();

    void set_decay_constant(float);
    void set_value(float);

    float get_value();
    void accumulate(float);

    float decay_constant;
    float value;
};

inline float Decaying_Average::get_value() {
    return value;
}

const char *AppName = "BRDF Renderer";

int unsigned FullscreenWidth = 1152, FullscreenHeight = 864;
char unsigned FullscreenColorBits = 24, FullscreenZBits = 16;

struct Brdf_Map_Info {
    char *name;
    Brdf_Map *map;

    char *uploaded_bitmap0;
    char *uploaded_bitmap1;

    double ambient_light_scale;
    GLuint texture_id_diffuse;

    GLuint texture_id_brdf0;
    GLuint texture_id_brdf1;
    GLuint texture_id_brdf_ambient;
};

Triangle_Model *big_test_model;
Triangle_Model *anisotropy_test_model;
Triangle_Model *test_models[4];
Brdf_Renderer  *the_brdf_renderer = NULL;
Vector          the_light_pos;
GLuint reject_this_texture = 0;

Decaying_Average frame_time;


void DrawTimingBars( void );

inline unsigned int double_to_byte(double f) {
    f *= 255.0;
    f += 0.5;
    return (unsigned int)f;
}

void set_scaled_down_material_color(GLenum which) {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, which);

    unsigned int byte_value = double_to_byte(scale_down_workspace);
    assert(byte_value >= 0);
    assert(byte_value <= 256);
    glColor3ub(byte_value, byte_value, byte_value);
}

void set_full_material_color(GLenum which) {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, which);

    unsigned int byte_value = 255;
    glColor3ub(byte_value, byte_value, byte_value);
}

Triangle_Model::Triangle_Model(int _nvertices, int _nfaces,
			       int _nmaterials) {
    nvertices = _nvertices;
    nfaces = _nfaces;
    nmaterials = _nmaterials;

    position = Vector(0, 0, 0);
    orientation = Quaternion(1, 0, 0, 0);

    vertices = new Vector[nvertices];
    vertex_normals = new Vector[nvertices];
    //    brdf_channel_0_texture_vertices = new Vector[nvertices];
    //    brdf_channel_1_texture_vertices = new Vector[nvertices];
    faces = new Face_Data[nfaces];

    face_vertices = new Vector[nfaces * 3];

    face_brdf_channel_0 = NULL;
    face_brdf_channel_1 = NULL;
    face_brdf_channel_ambient = NULL;
    
    condensed_vertices = NULL;
    condensed_normals = NULL;
    condensed_tangents = NULL;
    condensed_cross_tangents = NULL;
    condensed_texture_vertices = NULL;
    condensed_indices = NULL;

    condensed_vertex_colors = NULL;
}

Triangle_Model::~Triangle_Model() {
    delete [] vertices;
    delete [] vertex_normals;
    delete [] faces;

    delete [] face_vertices;

    if (condensed_vertices) {
        delete [] condensed_vertices;
        delete [] condensed_normals;
        delete [] condensed_tangents;
        delete [] condensed_cross_tangents;
        delete [] condensed_texture_vertices;
        delete [] condensed_indices;

	delete [] condensed_vertex_colors;

	delete [] face_brdf_channel_0;
	delete [] face_brdf_channel_1;
	delete [] face_brdf_channel_ambient;
    }
}

inline bool scalars_match(double s1, double s2) {
    const double TOLERANCE1 = 0.9;
    const double TOLERANCE2 = 1.1;

    if (s1 == s2) return true;
    return false;

    if (s2 < 0.0) {
        if (s1 > s2 * TOLERANCE1) return false;
	if (s1 < s2 * TOLERANCE2) return false;
    } else {
        if (s1 < s2 * TOLERANCE1) return false;
	if (s1 > s2 * TOLERANCE2) return false;
    }

    return true;
}

inline bool vectors_match(const Vector &v1, const Vector &v2) {
    float length = ((Vector &)v1).length();
    float epsilon = 0.001;
    float dist = distance(v1, v2);
    if (dist < length * epsilon) return true;
    return false;
  /*
    if (!scalars_match(v1.x, v2.x)) return false;
    if (!scalars_match(v1.y, v2.y)) return false;
    if (!scalars_match(v1.z, v2.z)) return false;
    */
    return true;
}

inline float length_2d(const Vector &v1) {
    return sqrt(v1.x * v1.x + v1.y * v1.y);
}

inline float distance_2d(const Vector &v1, const Vector &v2) {
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    return sqrt(dx * dx + dy * dy);
}

inline bool vectors_match_2d(const Vector &v1, const Vector &v2) {
    float length = length_2d(v1);
    float epsilon = 0.001;
    float dist = distance_2d(v1, v2);
    if (dist < length * epsilon) return true;
    return false;
    /*
    if (!scalars_match(v1.x, v2.x)) return false;
    if (!scalars_match(v1.y, v2.y)) return false;
    return true;
    */
}

int Triangle_Model::find_a_match(Vector *position, Vector *normal,
				 Vector *tangent, 
				 Vector *texture_vertices) {
  //  return -1;

    const double GOAL = 0.98;

    int i;
    for (i = 0; i < num_condensed_items; i++) {
        if (!vectors_match(*position, condensed_vertices[i])) continue; 
	if (!vectors_match(*normal, condensed_normals[i])) continue;
	if (!vectors_match(*tangent, condensed_tangents[i])) continue;
	if (!vectors_match_2d(*texture_vertices, 
			      condensed_texture_vertices[i])) continue;

	return i;
    }

    return -1;
}

void Triangle_Model::condense_face_vertex(Face_Data *face,
					  int face_index, int vertex_index) {
    int j = face_index * 3 + vertex_index;

    Vector *position = &face_vertices[j];
    Vector *normal   = &face->normals[vertex_index];
    Vector *tangent  = &face->tangents[vertex_index];
    Vector *texture_vertices = &face->texture_vertices[vertex_index];

    int ci = find_a_match(position, normal, tangent, texture_vertices);
    if (ci == -1) {
        ci = num_condensed_items++;
	assert(ci < nfaces * 3);

	condensed_vertices[ci] = *position;
	condensed_normals[ci] = *normal;
	condensed_tangents[ci] = *tangent;
	condensed_texture_vertices[ci] = *texture_vertices;

	condensed_cross_tangents[ci] = cross_product(*normal, *tangent);
    }

    condensed_indices[j] = ci;
}

void Triangle_Model::condense() {
    assert(condensed_vertices == NULL);

    num_condensed_items = 0;
    int allocated = nfaces * 3;

    condensed_indices = new int[allocated];

    // XXX We are allocating too much memory for these guys
    // because we are too wussed out to do a better job.

    condensed_vertices = new Vector[allocated];
    condensed_normals = new Vector[allocated];
    condensed_tangents = new Vector[allocated];
    condensed_cross_tangents = new Vector[allocated];
    condensed_texture_vertices = new Vector[allocated];

    condensed_vertex_colors = new Vector[allocated];

    int i;
    for (i = 0; i < nfaces; i++) {
        Face_Data *face = &faces[i];
	condense_face_vertex(face, i, 0);
	condense_face_vertex(face, i, 1);
	condense_face_vertex(face, i, 2);
    }
	
    face_brdf_channel_0 = new Vector[num_condensed_items];
    face_brdf_channel_1 = new Vector[num_condensed_items];
    face_brdf_channel_ambient = new Vector[num_condensed_items];

    // XXX Leak here

    vertices = NULL;
    vertex_normals = NULL;
    faces = NULL;

    face_vertices = NULL;
}

Triangle_Model *Triangle_Model::copy() {
    Triangle_Model *other = new Triangle_Model(nvertices, nfaces,
					       nmaterials);

    int allocated = num_condensed_items;
    other->num_condensed_items = allocated;

    other->condensed_indices = new int[nfaces * 3];

    other->condensed_vertices = new Vector[allocated];
    other->condensed_normals = new Vector[allocated];
    other->condensed_tangents = new Vector[allocated];
    other->condensed_cross_tangents = new Vector[allocated];
    other->condensed_texture_vertices = new Vector[allocated];

    other->condensed_vertex_colors = new Vector[allocated];

    other->face_brdf_channel_0 = new Vector[allocated];
    other->face_brdf_channel_1 = new Vector[allocated];
    other->face_brdf_channel_ambient = new Vector[allocated];

    int i;
    for (i = 0; i < allocated; i++) {
        other->condensed_vertices[i] = condensed_vertices[i];
        other->condensed_normals[i] = condensed_normals[i];
        other->condensed_tangents[i] = condensed_tangents[i];
        other->condensed_cross_tangents[i] = condensed_cross_tangents[i];
        other->condensed_texture_vertices[i] = condensed_texture_vertices[i];
    }

    for (i = 0; i < nfaces * 3; i++) {
        other->condensed_indices[i] = condensed_indices[i];
    }

    return other;
}


Vector realign_tangent(Vector *normal, Vector *tangent) {
    Vector ct = cross_product(*normal, *tangent);
    ct.normalize();
    Vector new_tangent = cross_product(ct, *normal);

    return new_tangent;
}

void Triangle_Model::recompute_normals_and_frames() {
    int allocated = num_condensed_items;

    Vector *accum_normals = new Vector[allocated];
    int *faces_contributing = new int[allocated];
    int i;
    for (i = 0; i < allocated; i++) {
        accum_normals[i].set(0, 0, 0);
	faces_contributing[i] = 0;
    }

    for (i = 0; i < nfaces; i++) {
        int n0 = condensed_indices[i * 3 + 0];
        int n1 = condensed_indices[i * 3 + 1];
        int n2 = condensed_indices[i * 3 + 2];

	assert(n0 < allocated);
	assert(n1 < allocated);
	assert(n2 < allocated);

	Vector s1 = condensed_vertices[n1].subtract(condensed_vertices[n0]);
	Vector s2 = condensed_vertices[n2].subtract(condensed_vertices[n0]);

	Vector cross = cross_product(s2, s1);

	accum_normals[n0] = accum_normals[n0].add(cross);
	accum_normals[n1] = accum_normals[n1].add(cross);
	accum_normals[n2] = accum_normals[n2].add(cross);
	faces_contributing[n0]++;
	faces_contributing[n1]++;
	faces_contributing[n2]++;
    }

    for (i = 0; i < allocated; i++) {
        accum_normals[i].normalize();

	if (faces_contributing[i] > 5) condensed_normals[i] = accum_normals[i];
    }

    for (i = 0; i < nfaces; i++) {
        int n0 = condensed_indices[i * 3 + 0];
        int n1 = condensed_indices[i * 3 + 1];
        int n2 = condensed_indices[i * 3 + 2];

	assert(n0 < allocated);
	assert(n1 < allocated);
	assert(n2 < allocated);

        Face_Data *face = &faces[i];

	face->normals[0] = condensed_normals[n0];
	face->normals[1] = condensed_normals[n1];
	face->normals[2] = condensed_normals[n2];

	face->tangents[0] = realign_tangent(&face->normals[0], 
					    &condensed_tangents[n0]);
	face->tangents[1] = realign_tangent(&face->normals[1], 
					    &condensed_tangents[n1]);
	face->tangents[2] = realign_tangent(&face->normals[2],
					    &condensed_tangents[n2]);

	face->cross_tangents[0] = cross_product(face->normals[0], 
						face->tangents[0]);
	face->cross_tangents[1] = cross_product(face->normals[1],
						face->tangents[1]);
	face->cross_tangents[2] = cross_product(face->normals[2],
						face->tangents[2]);

    }

    delete [] accum_normals;
    delete [] faces_contributing;
}

void set_face_tangents(Triangle_Model *model, int index,
		       Vector t0, Vector t1, Vector t2) {
    Face_Data *face = &model->faces[index];
    face->tangents[0] = t0;
    face->tangents[1] = t1;
    face->tangents[2] = t2;

    Vector p0, p1, p2;
    p0 = model->vertices[face->vertex_indices[0]];
    p1 = model->vertices[face->vertex_indices[1]];
    p2 = model->vertices[face->vertex_indices[2]];

    Vector v0 = p1.subtract(p0);
    Vector v1 = p2.subtract(p0);
    Vector cross = cross_product(v0, v1);

    cross.normalize();
    face->normals[0] = cross;
    face->normals[1] = cross;
    face->normals[2] = cross;

    face->cross_tangents[0] = cross_product(face->normals[0], t0);
    face->cross_tangents[1] = cross_product(face->normals[1], t1);
    face->cross_tangents[2] = cross_product(face->normals[2], t2);

    face->tangents[0] = cross_product(face->normals[0], 
				      face->cross_tangents[0]);
    face->tangents[1] = cross_product(face->normals[1], 
				      face->cross_tangents[1]);
    face->tangents[2] = cross_product(face->normals[2], 
				      face->cross_tangents[2]);
}

void set_face(Triangle_Model *model, int index,
	      int n0, int n1, int n2, 
	      double u0, double v0, 
	      double u1, double v1, double u2, double v2) {
    Face_Data *face = &model->faces[index];

    face->material_index = -1;
    face->vertex_indices[0] = n0;
    face->vertex_indices[1] = n1;
    face->vertex_indices[2] = n2;

    face->texture_vertices[0].set(u0, v0, 0);
    face->texture_vertices[1].set(u1, v1, 0);
    face->texture_vertices[2].set(u2, v2, 0);
}

inline int scale_and_clamp(double d) {
    d *= 255.0;
    d += (0.5 / 255.0);
    int result = (int)d;
    if (result < 0) result = 0;
    if (result > 255) result = 255;

    return result;
}

Vector spheremap_vector_from_coordinates(float u, float v) {
    float len = sqrt(u * u + v * v);
    float w;
    if (len >= 1.0f) {
        w = 0.0f;
    } else {
        w = sqrt(1.0f - len);
    }

    return Vector(u, v, w);
}

GLuint gl_texture_from_bitmap(char *bits, int width, int height) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    if (ALLOW_MIPMAPPING) {
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, 
			  GL_RGB, GL_UNSIGNED_BYTE, (void *)bits);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 
		     width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
		     (void *)bits);
    }

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return texture_id;
}

GLuint texture_from_image(Brdf_Image *image, char **bits_ret) {

    int width, height;
    width = image->getWidth();
    height = image->getHeight();
    
    HDC context = CreateCompatibleDC(0);
    assert(context);
    // XXX must free context later?

    BITMAPINFO BitmapInfo = { sizeof(BITMAPINFOHEADER) };
    BitmapInfo.bmiHeader.biWidth = width;
    BitmapInfo.bmiHeader.biHeight = height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 24;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    BitmapInfo.bmiHeader.biSizeImage = width * height * 3;

    void *pBits;
    HBITMAP OldBitmap = CreateDIBSection(context, &BitmapInfo, DIB_RGB_COLORS,
					 &pBits, 0, 0);
    assert(OldBitmap && pBits);

    char *cbits = (char *)pBits;

    int val;
    int i;
    for (i = 0; i < width * height; i++) {
        int red, green, blue;

	int y = i / width;
	int x = i % width;

	double fr, fg, fb;
	image->getPixel(x, y, fr, fg, fb);

	red = scale_and_clamp(fr);
	green = scale_and_clamp(fg);
	blue = scale_and_clamp(fb);

        cbits[i * 3 + 0] = red;
        cbits[i * 3 + 1] = green;
        cbits[i * 3 + 2] = blue;
    }


    OldBitmap = SelectBitmap(context, OldBitmap);
    GLuint texture_id = gl_texture_from_bitmap(cbits, width, height);

    *bits_ret = cbits;
    return texture_id;
}

void messy_create_texture(Brdf_Map_Info *info) {
    Brdf_Map *map = info->map;

    char *bits;

    Brdf_Image *image0 = map->get_image(0, Brdf_Layer::TEXTURE1);
    assert(image0 != NULL);
    info->texture_id_brdf0 = texture_from_image(image0, &bits);
    info->uploaded_bitmap0 = bits;

    Brdf_Image *image1 = map->get_image(0, Brdf_Layer::TEXTURE2);
    assert(image1 != NULL);
    info->texture_id_brdf1 = texture_from_image(image1, &bits);
    info->uploaded_bitmap1 = bits;

    Brdf_Image *image2 = map->get_image(0, Brdf_Layer::TEXTURE_AMBIENT);
    assert(image2 != NULL);
    info->texture_id_brdf_ambient = texture_from_image(image2, &bits);
}

void Brdf_Renderer::add_render_proc(Render_Proc *proc, char *name,
				    double max_delta) {
    render_procs[num_render_procs] = proc;
    render_proc_names[num_render_procs] = name;
    render_proc_deltas[num_render_procs] = max_delta;
    num_render_procs++;
}

bool Brdf_Renderer::lookup_extension_name(char *name) {
    int i;
    for (i = 0; i < num_extensions; i++) {
        if (strcmp(name, (char *)extension_list[i]) == 0) return true;
    }

    return false;
}

void Brdf_Renderer::init_gl_extensions() {
    const unsigned char *const extensions = glGetString(GL_EXTENSIONS);

    unsigned char *s = (unsigned char *)extensions;
    int index = 0;

    while (s) {
        if (index == MAX_EXTENSIONS) break;

        unsigned char *t = s;
	while (*t && !isspace(*t)) t++;
	int len = t - s;
	char *extension = new char[len + 1];
	memcpy(extension, s, len);
	extension[len] = '\0';
	s = t + 1;
	extension_list[index++] = extension;
    }

    num_extensions = index;

    extensions_found = 0;
    if (lookup_extension_name("GL_EXT_texture_env_combine")) {
        extensions_found |= EXTENSION_TEXTURE_ENV_COMBINE;
    }

    if (lookup_extension_name("GL_NV_texture_env_combine4")) {
        extensions_found |= EXTENSION_TEXTURE_ENV_COMBINE4;
    }

    // We're just not going to run without the glActiveTexture
    // stuff, so if we can't find it we will just assert, so there.

    void *proc;
    proc = wglGetProcAddress("glActiveTextureARB");
    *(void **)&glActiveTextureARB = proc;
    assert(proc != NULL);

    proc = wglGetProcAddress("glClientActiveTextureARB");
    *(void **)&glClientActiveTextureARB = proc;
    assert(proc != NULL);
}

Triangle_Model *make_cube() {
    const float R = 1;
    Triangle_Model *model = new Triangle_Model(8, 12, 0);
    model->vertices[0] = Vector(-R, +R, -R);
    model->vertices[1] = Vector(-R, -R, -R);
    model->vertices[2] = Vector(+R, -R, -R);
    model->vertices[3] = Vector(+R, +R, -R);
    model->vertices[4] = Vector(-R, +R, +R);
    model->vertices[5] = Vector(-R, -R, +R);
    model->vertices[6] = Vector(+R, -R, +R);
    model->vertices[7] = Vector(+R, +R, +R);

    model->vertex_normals[0] = model->vertices[0];
    model->vertex_normals[1] = model->vertices[1];
    model->vertex_normals[2] = model->vertices[2];
    model->vertex_normals[3] = model->vertices[3];
    model->vertex_normals[4] = model->vertices[4];
    model->vertex_normals[5] = model->vertices[5];
    model->vertex_normals[6] = model->vertices[6];
    model->vertex_normals[7] = model->vertices[7];

    model->vertex_normals[0].normalize();
    model->vertex_normals[1].normalize();
    model->vertex_normals[2].normalize();
    model->vertex_normals[3].normalize();
    model->vertex_normals[4].normalize();
    model->vertex_normals[5].normalize();
    model->vertex_normals[6].normalize();
    model->vertex_normals[7].normalize();

    set_face(model, 0,   0, 1, 2,    0, 1,  0, 0,  1, 0);
    set_face(model, 1,   0, 2, 3,    0, 1,  1, 0,  1, 1);
    set_face(model, 2,   3, 2, 6,    0, 1,  0, 0,  1, 0);
    set_face(model, 3,   3, 6, 7,    0, 1,  1, 0,  1, 1);
    set_face(model, 4,   7, 6, 5,    0, 1,  0, 0,  1, 0);
    set_face(model, 5,   7, 5, 4,    0, 1,  1, 0,  1, 1);
    set_face(model, 6,   4, 5, 1,    0, 1,  0, 0,  1, 0);
    set_face(model, 7,   4, 1, 0,    0, 1,  1, 0,  1, 1);
    set_face(model, 8,   0, 3, 7,    0, 1,  0, 0,  1, 0);
    set_face(model, 9,   0, 7, 4,    0, 1,  1, 0,  1, 1);
    set_face(model, 10,  2, 1, 5,    0, 1,  0, 0,  1, 0);
    set_face(model, 11,  2, 5, 6,    0, 1,  1, 0,  1, 1);

    set_face_tangents(model, 0,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 1,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 2,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 3,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 4,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 5,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 6,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));
    set_face_tangents(model, 7,   
		      Vector(0, 1, 0), Vector(0, 1, 0), Vector(0, 1, 0));

    set_face_tangents(model, 8,   
		      Vector(0, 0, 1), Vector(0, 0, 1), Vector(0, 0, 1));
    set_face_tangents(model, 9,   
		      Vector(0, 0, 1), Vector(0, 0, 1), Vector(0, 0, 1));
    set_face_tangents(model, 10,   
		      Vector(0, 0, 1), Vector(0, 0, 1), Vector(0, 0, 1));
    set_face_tangents(model, 11,   
		      Vector(0, 0, 1), Vector(0, 0, 1), Vector(0, 0, 1));

    return model;
}

void turn_vertex(Vector *t) {
    float new_u = -t->y;
    float new_v = t->x;

    t->x = new_u;
    t->y = new_v;
}

void turn_face_textures(Face_Data *face) {
    turn_vertex(&face->texture_vertices[0]);
    turn_vertex(&face->texture_vertices[1]);
    turn_vertex(&face->texture_vertices[2]);
}

void add_face(Triangle_Model *model, Vector *origin, 
	      float edge_length, int verts_per_side,
	      Vector *east, Vector *north, int turned) {

    int steps_per_side = verts_per_side - 1;

    float ds = edge_length / (double)steps_per_side;
    Vector dx = *east;
    dx = dx.scale(ds);
    Vector dy = *north;
    dy = dy.scale(ds);

    double duv = 1.0 / (double)steps_per_side;

    int m = verts_per_side;

    int face_index = model->nfaces;
    int vertex_index = model->nvertices;

    Vector cross = cross_product(dx, dy);

    Vector normal = cross;
    normal.normalize();

    Vector center = *origin;
    center = center.add(east->scale(edge_length * 0.5));
    center = center.add(north->scale(edge_length * 0.5));
    center = center.add(normal.scale(edge_length * -0.5));

    Vector tangent;
    if (turned) {
        tangent = *east;
    } else {
        tangent = *north;
    }

    double k = 0.35 * edge_length;

    int i, j;
    for (j = 0; j < verts_per_side; j++) {
        double v = duv * j;
        for (i = 0; i < verts_per_side; i++) {
	    double u = duv * i;

	    int n0 = model->nvertices + j * m + i;
	    int n1 = n0 + 1;
	    int n2 = model->nvertices + (j + 1) * m + i + 1;
	    int n3 = n2 - 1;

	    // Add n0 as a vertex
	    Vector v0 = *origin;
	    Vector ox = dx.scale(i);
	    Vector oy = dy.scale(j);

	    v0 = v0.add(ox);
	    v0 = v0.add(oy);

	    double um2 = (u * u) - u;
	    double vm2 = (v * v) - v;

	    double oz = 16 * k * (um2 * vm2);
	    Vector adjust = normal;
	    adjust = adjust.scale(oz);
	    v0 = v0.add(adjust);

	    double dz_du = 16 * k * vm2 * (-2 * u + 1.25);
	    double dz_dv = 16 * k * um2 * (-2 * v + 1.25);

	    dz_du = -16 * k * vm2 * (2 * u - 1);
	    dz_dv = -16 * k * um2 * (2 * v - 1);


	    double du_dx = 1.0 / edge_length;
	    double dv_dy = 1.0 / edge_length;

	    double dz_dx = dz_du * du_dx;
	    double dz_dy = dz_dv * dv_dy;

	    
	    Vector g0 = east->scale(dz_dx);
	    Vector g1 = north->scale(dz_dy);
	    Vector g2 = normal;

	    Vector gradient = g0.add(g1).add(g2);
	    gradient.normalize();

	    model->vertices[n0] = v0;
	    model->vertex_normals[n0] = gradient;

	    if ((i == 0) || (i == verts_per_side - 1)
		|| (j == 0) || (j == verts_per_side - 1)) {

	        Vector substitute = v0.subtract(center);
		substitute.normalize();
		model->vertex_normals[n0] = substitute;
		//		model->vertex_normals[n0] = Vector(1, 0, 0);
	    }

	    vertex_index++;

	    if (i == (verts_per_side - 1)) continue;
	    if (j == (verts_per_side - 1)) continue;

	    Face_Data *face;
	    face = &model->faces[face_index];
	    face->vertex_indices[0] = n0;
	    face->vertex_indices[1] = n2;
	    face->vertex_indices[2] = n1;
	    face->texture_vertices[0] = Vector(duv * i, duv * j, 0);
	    face->texture_vertices[1] = Vector(duv * (i + 1), duv * (j + 1), 0);
	    face->texture_vertices[2] = Vector(duv * (i + 1), duv * j, 0);
	    face->material_index = 0;
	    if (!turned) turn_face_textures(face);

	    face_index++;

	    face = &model->faces[face_index];
	    face->vertex_indices[0] = n0;
	    face->vertex_indices[1] = n3;
	    face->vertex_indices[2] = n2;
	    face->texture_vertices[0] = Vector(duv * i, duv * j, 0);
	    face->texture_vertices[1] = Vector(duv * i, duv * (j + 1), 0);
	    face->texture_vertices[2] = Vector(duv * (i + 1), duv * (j + 1), 0);
	    face->material_index = 0;
	    if (!turned) turn_face_textures(face);

	    face_index++;
	    
	}
    }

    for (i = model->nfaces; i < face_index; i++) {
        Face_Data *face;
	face = &model->faces[i];
	model->face_vertices[i * 3 + 0] = 
	    model->vertices[face->vertex_indices[0]];
	model->face_vertices[i * 3 + 1] = 
	    model->vertices[face->vertex_indices[1]];
	model->face_vertices[i * 3 + 2] = 
	    model->vertices[face->vertex_indices[2]];

	face->normals[0] = model->vertex_normals[face->vertex_indices[0]];
	face->normals[1] = model->vertex_normals[face->vertex_indices[1]];
	face->normals[2] = model->vertex_normals[face->vertex_indices[2]];

	face->tangents[0] = realign_tangent(&face->normals[0], &tangent);
	face->tangents[1] = realign_tangent(&face->normals[1], &tangent);
	face->tangents[2] = realign_tangent(&face->normals[2], &tangent);
    }

    model->nfaces = face_index;
    model->nvertices = vertex_index;
}

void add_dense_cube(Triangle_Model *model, Vector *origin,
		    float EDGE_LENGTH, float VERTS_PER_SIDE, int turned) {
    Vector x(1, 0, 0);
    Vector y(0, 1, 0);
    Vector z(0, 0, 1);

    Vector cursor = *origin;
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &x, &y, turned);

    cursor.set(EDGE_LENGTH, 0, 0);
    cursor = cursor.add(*origin);
    z.z = -1;
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &z, &y, turned);
    z.z = 1;

    cursor.set(0, 0, -EDGE_LENGTH);
    cursor = cursor.add(*origin);
    x.x = -1;
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &z, &y, turned);
    x.x = 1;

    cursor.set(0, EDGE_LENGTH, 0);
    cursor = cursor.add(*origin);
    z.z = -1;
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &x, &z, turned);
    z.z = 1;

    cursor.set(0, 0, -EDGE_LENGTH);
    cursor = cursor.add(*origin);
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &x, &z, turned);

    cursor.set(0, EDGE_LENGTH, -EDGE_LENGTH);
    cursor = cursor.add(*origin);
    y.y = -1;
    add_face(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, &x, &y, turned);
    y.y = 1;
}

Triangle_Model *make_dense_cubes() {
    const int VERTS_PER_SIDE = 11;

    int verts_per_face = VERTS_PER_SIDE * VERTS_PER_SIDE;
    int total_verts = verts_per_face * 6;

    int steps_per_side = VERTS_PER_SIDE - 1;
    int faces_per = 2 * steps_per_side * steps_per_side;
    int total_faces = faces_per * 6;

    total_faces *= 4;
    total_verts *= 4;

    const float EDGE_LENGTH = 0.8f;
    const float GAP = 0.1f;


    Triangle_Model *model = new Triangle_Model(total_verts, total_faces, 0);
    int orig_nvertices = model->nvertices;
    int orig_nfaces = model->nfaces;

    model->nvertices = 0;
    model->nfaces = 0;

    Vector cursor;

    float qx0 = -EDGE_LENGTH - GAP * 0.5;
    float qx1 = GAP * 0.5;

    float qy0 = qx0;
    float qy1 = qx1;

    cursor.set(qx0, qy0, 0);
    add_dense_cube(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, 0);

    cursor.set(qx1, qy0, 0);
    add_dense_cube(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, 1);

    cursor.set(qx0, qy1, 0);
    add_dense_cube(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, 1);

    cursor.set(qx1, qy1, 0);
    add_dense_cube(model, &cursor, EDGE_LENGTH, VERTS_PER_SIDE, 0);
    assert(model->nvertices <= orig_nvertices);
    assert(model->nfaces <= orig_nfaces);

    model->condense();
    //    model->recompute_normals_and_frames();

    return model;
}

GLuint get_texture(char *filename) {
    unsigned char *bitmap;
    int bitmap_x, bitmap_y;
    bool success = load_bmp_file(filename, &bitmap,
				 &bitmap_x, &bitmap_y);
    assert(success);

    GLuint texture = gl_texture_from_bitmap((char *)bitmap,
					    bitmap_x, bitmap_y);
    return texture;
}

void Brdf_Renderer::init_rendering_state() {
    num_brdf_maps = 0;
    num_render_procs = 0;
    current_render_proc = 0;

    init_gl_extensions();

    add_render_proc(render_vanilla_proc, "Vanilla");

    if (extensions_found & EXTENSION_TEXTURE_ENV_COMBINE) {
        add_render_proc(render_extensions_proc, "Extensions");
	current_render_proc = num_render_procs - 1;
    }

    /*
    if (extensions_found & EXTENSION_TEXTURE_ENV_COMBINE4) {
        add_render_proc(render_nv4_proc, "Extensions_NV_Combine4", 8.0);
	current_render_proc = num_render_procs - 1;
    }
    */

    add_render_proc(render_gouraud_vanilla_proc, "Gouraud, Vanilla");

    if (extensions_found & EXTENSION_TEXTURE_ENV_COMBINE) {
        add_render_proc(render_gouraud_extensions_proc, "Gouraud, Extensions");
    }

    assert(num_render_procs > 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /*
    screen_i = GetSystemMetrics(SM_CXSCREEN);
    screen_j = GetSystemMetrics(SM_CYSCREEN);
    glViewport(0, 0, screen_i, screen_j);
    */	             

    float Aspect = 4.0f / 3.0f;
    glFrustum(-Aspect, Aspect, -1., 1., 2.5, 10000.0);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    //    GLfloat ambient_i[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    //    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    Tcl_Loader *loader = new Tcl_Loader();
    Triangle_Model *model = loader->load("teapots8.tcl");

    Triangle_Model *small_model = loader->load("teapot4.tcl");
    big_test_model = model;
    big_test_model->material_indices[0] = 0;

    model = make_dense_cubes();
    anisotropy_test_model = model;
    anisotropy_test_model->material_indices[0] = 0;

    //    Triangle_Model *model = make_cube();
    test_models[0] = small_model;
    test_models[1] = small_model->copy();
    test_models[2] = small_model->copy();
    test_models[3] = small_model->copy();

    test_models[0]->material_indices[0] = 2;
    test_models[1]->material_indices[0] = 3;
    test_models[2]->material_indices[0] = 6;
    test_models[3]->material_indices[0] = 7;

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

}

Brdf_Map_Info *Brdf_Renderer::load_brdf_map(char *filename, 
					    char *displayable_name,
					    double target_lambda,
					    double ratio_bias) {
    assert(num_brdf_maps < MAX_BRDF_MAPS);
    Brdf_Map *map = new Brdf_Map(filename);
    if (target_lambda != -1.0) {
        map->rescale_for_target_lambda(target_lambda, ratio_bias);
    }

    map->convertToGLSphereMap();
    
    Brdf_Map_Info *info = new Brdf_Map_Info;
    map_info[num_brdf_maps] = info;

    info->map = map;
    info->name = displayable_name;

    messy_create_texture(info);

    info->texture_id_diffuse = default_texture_handle;
    info->ambient_light_scale = 1.0f;

    num_brdf_maps++;
    return info;
}

void load_matrix_into_gl(const Transformer &tr) {
    double coef[16];

    Transformation_Matrix *m = tr.current_transform;
    coef[0]  = m->coef[0][0];
    coef[1]  = m->coef[1][0];
    coef[2]  = m->coef[2][0];
    coef[3]  = m->coef[3][0];
    coef[4]  = m->coef[0][1];
    coef[5]  = m->coef[1][1];
    coef[6]  = m->coef[2][1];
    coef[7]  = m->coef[3][1];
    coef[8]  = m->coef[0][2];
    coef[9]  = m->coef[1][2];
    coef[10] = m->coef[2][2];
    coef[11] = m->coef[3][2];
    coef[12] = m->coef[0][3];
    coef[13] = m->coef[1][3];
    coef[14] = m->coef[2][3];
    coef[15] = m->coef[3][3];

    glMultMatrixd(coef);
}

void mipmap_or_not(int texture_id) {
    if (use_mipmaps) {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
			GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
}

void render_vanilla(Brdf_Renderer *ren,
		    Triangle_Model *model, Brdf_Map_Info *info) {
    glEnable(GL_LIGHT0);
    GLfloat ambient_i0[] = { 0, 0, 0, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    if (mag_workspace > 2) {
        set_full_material_color(GL_DIFFUSE);
    } else {
        set_scaled_down_material_color(GL_DIFFUSE);
    }

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);


    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf0);
    mipmap_or_not(info->texture_id_brdf0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    int stride = sizeof(Face_Data);

    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_0);

    if (do_channel_0) {
        glDrawElements(GL_TRIANGLES, nelements, 
		       GL_UNSIGNED_INT, model->condensed_indices);
	ren->passes_rendered++;

	glEnable(GL_BLEND);
	set_full_material_color(GL_DIFFUSE);
    }

    glDisable(GL_LIGHTING);

    if (mag_workspace > 1) {
        mag_workspace /= 2;
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    } else {
        glBlendFunc(GL_DST_COLOR, GL_ONE);
    }

    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf1);
    mipmap_or_not(info->texture_id_brdf1);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_1);
    
    if (do_channel_1) {
        glDrawElements(GL_TRIANGLES, nelements, 
		       GL_UNSIGNED_INT, model->condensed_indices);
	ren->passes_rendered++;
    }

    while (mag_workspace > 1) {
        set_scaled_down_material_color(GL_DIFFUSE);
	scale_down_workspace = 1.0;
        glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        glDrawElements(GL_TRIANGLES, nelements, 
		       GL_UNSIGNED_INT, model->condensed_indices);
	ren->passes_rendered++;
	mag_workspace >>= 1;
    }
}

void render_gouraud_vanilla(Brdf_Renderer *ren,
			    Triangle_Model *model, Brdf_Map_Info *info) {
    glEnable(GL_LIGHT0);
    GLfloat ambient_i0[] = { 0, 0, 0, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    set_full_material_color(GL_DIFFUSE);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnableClientState(GL_COLOR_ARRAY);

    int stride = sizeof(Face_Data);

    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);
    glColorPointer(3, GL_DOUBLE, 0, model->condensed_vertex_colors);

    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;

    glDisableClientState(GL_COLOR_ARRAY);
}

void render_gouraud_extensions(Brdf_Renderer *ren,
			       Triangle_Model *model, Brdf_Map_Info *info) {
    glEnable(GL_LIGHT0);
    GLfloat ambient_i0[] = { 0, 0, 0, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    set_full_material_color(GL_DIFFUSE);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);

    if (ambient_light) {
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	float amb = AMBIENT_INTENSITY;

	GLfloat ambient_i[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	ambient_i[0] = ambient_i[1] = ambient_i[2] = amb;
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, ambient_i);


        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glBindTexture(GL_TEXTURE_2D, info->texture_id_diffuse);
	mipmap_or_not(info->texture_id_diffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glEnable(GL_TEXTURE_2D);
	glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
			  model->condensed_texture_vertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_CONSTANT_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
    } else {
        glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    glEnableClientState(GL_COLOR_ARRAY);

    int stride = sizeof(Face_Data);

    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);
    glColorPointer(3, GL_DOUBLE, 0, model->condensed_vertex_colors);

    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;

    glDisableClientState(GL_COLOR_ARRAY);


    // Undo any multitexture effects
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
}


void render_extensions(Brdf_Renderer *ren,
		       Triangle_Model *model, Brdf_Map_Info *info) {
    glEnable(GL_LIGHT0);
    GLfloat ambient_i0[] = { 0, 0, 0, 1.0f };
    set_scaled_down_material_color(GL_DIFFUSE);
    //    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    glDisable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);

    int mag_factor = last_mag_factor;

    // Set up texture unit 0

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf0);
    mipmap_or_not(info->texture_id_brdf0);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (do_channel_0) {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
    } else {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    }

    // Set up texture unit 1
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, mag_factor);

    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf1);
    mipmap_or_not(info->texture_id_brdf1);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (do_channel_1) {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
    } else {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    }


    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);

    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;
    //    glDrawArrays(GL_TRIANGLES, 0, model->nfaces * 3);



    // Undo any multitexture effects
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void render_nv4(Brdf_Renderer *ren,
		Triangle_Model *model, Brdf_Map_Info *info) {
    glEnable(GL_LIGHT0);
    GLfloat ambient_i0[] = { 0, 0, 0, 1.0f };
    set_scaled_down_material_color(GL_DIFFUSE);
    //    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);

    glDisable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);

    int mag_factor = last_mag_factor;
    int use_nv4_extra_scaling = 0;
    if (mag_factor > 4) {
        assert(mag_factor == 8);
	use_nv4_extra_scaling = 1;
	mag_factor = 4;
    }

    // Set up texture unit 0

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf0);
    mipmap_or_not(info->texture_id_brdf0);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    GLfloat white_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (do_channel_0) {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
    } else {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, white_color);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
    }


    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);


    // Set up texture unit 1
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
    glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, mag_factor);

    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(GL_TEXTURE_2D, info->texture_id_brdf1);
    mipmap_or_not(info->texture_id_brdf1);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->face_brdf_channel_1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (do_channel_1) {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

	if (use_nv4_extra_scaling) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT);
	    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
	    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_TEXTURE);
	    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
	} else {
	    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
	    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
	    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_PREVIOUS_EXT);
	    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
	}
    } else {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, white_color);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
    }




    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);

    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;
    //    glDrawArrays(GL_TRIANGLES, 0, model->nfaces * 3);



    // Undo any multitexture effects
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void render_ambient_extensions(Brdf_Renderer *ren,
			       Triangle_Model *model, Brdf_Map_Info *info) {
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    float amb = AMBIENT_INTENSITY;

    GLfloat ambient_i[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    ambient_i[0] = ambient_i[1] = ambient_i[2] = amb;

    set_scaled_down_material_color(GL_AMBIENT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i);


    // Set up texture unit 0

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);


    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, info->texture_id_diffuse);
    mipmap_or_not(info->texture_id_diffuse);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->condensed_texture_vertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (do_channel_0) {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

    } else {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1);

    // Set up texture unit 1
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);


    glClientActiveTextureARB(GL_TEXTURE1_ARB);

    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);


    int i;
    int nelements = model->nfaces * 3;

    glVertexPointer(3, GL_DOUBLE, 0, model->condensed_vertices);
    glNormalPointer(GL_DOUBLE, 0, model->condensed_normals);

    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;


    // Undo any multitexture effects
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
    glDisable(GL_TEXTURE_2D);

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glClientActiveTextureARB(GL_TEXTURE0_ARB);
}

void render_ambient_vanilla(Brdf_Renderer *ren,
			    Triangle_Model *model, Brdf_Map_Info *info) {

    if (info->texture_id_diffuse == reject_this_texture) return;

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glEnable(GL_BLEND);

    float amb = AMBIENT_INTENSITY;

    glBlendFunc(GL_ONE, GL_ONE);

    GLfloat ambient_i[] = { 0.25f, 0.25f, 0.25f, 1.0f };
    ambient_i[0] = ambient_i[1] = ambient_i[2] = 
        amb * info->ambient_light_scale;

    set_full_material_color(GL_AMBIENT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, info->texture_id_diffuse);
    mipmap_or_not(info->texture_id_diffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glEnable(GL_TEXTURE_2D);

    glTexCoordPointer(2, GL_DOUBLE, sizeof(Vector), 
		      model->condensed_texture_vertices);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    int nelements = model->nfaces * 3;
    glDrawElements(GL_TRIANGLES, nelements, 
		   GL_UNSIGNED_INT, model->condensed_indices);
    ren->passes_rendered++;
}

void render_vanilla_proc(Brdf_Renderer *ren, Triangle_Model *model,
			 Brdf_Map_Info *info) {
    render_vanilla(ren, model, info);
    if (ambient_light) render_ambient_vanilla(ren, model, info);
}

const int LOW_WORD_OFFSET = 0;
static const double fixint_conv_factor = ((double)0x10000000)
                                        * ((double)0x01000000) * 1.5;

inline unsigned long FastInt(double d) {
    d += fixint_conv_factor;
    const unsigned long *const &num = (unsigned long *)&d + LOW_WORD_OFFSET;
    return *num;
}

void prepare_vertex_colors(Brdf_Renderer *ren, Triangle_Model *model,
			   Brdf_Map_Info *info) {
    float delta = last_delta;

    Brdf_Layer *layer = info->map->get_layer(0);
    Brdf_Image *image0 = layer->get_image(Brdf_Layer::TEXTURE1);
    Brdf_Image *image1 = layer->get_image(Brdf_Layer::TEXTURE2);

    int w0 = image0->getWidth();
    int h0 = image0->getHeight();
    int w1 = image1->getWidth();
    int h1 = image1->getHeight();

    float uscale0 = (float)w0;
    float vscale0 = (float)h0;
    float uscale1 = (float)w1;
    float vscale1 = (float)h1;

    int i;
    for (i = 0; i < model->num_condensed_items; i++) {
        Vector *c0 = &model->face_brdf_channel_0[i];
        Vector *c1 = &model->face_brdf_channel_1[i];

	if (c0->x < 0.0) c0->x = 0.0;
	if (c0->y < 0.0) c0->y = 0.0;
	if (c1->x < 0.0) c1->x = 0.0;
	if (c1->y < 0.0) c1->y = 0.0;

	int ix0, iy0, ix1, iy1;
	ix0 = FastInt(c0->x * uscale0);
	iy0 = FastInt(c0->y * uscale0);

	ix1 = FastInt(c1->x * uscale1);
	iy1 = FastInt(c1->y * uscale1);

	if (ix0 >= w0) ix0 = w0 - 1;
	if (iy0 >= h0) iy0 = h0 - 1;
	if (ix1 >= w1) ix1 = w1 - 1;
	if (iy1 >= h1) iy1 = h1 - 1;

	float r0, g0, b0, r1, g1, b1;

	int index0 = ix0 * image0->depth + iy0 * image0->width * image0->depth;
	assert(index0 >= 0);
	assert(index0 < image0->width * image0->height * image0->depth);

	r0 = image0->imageMemory[index0 + 0];
	g0 = image0->imageMemory[index0 + 1];
	b0 = image0->imageMemory[index0 + 2];

	int index1 = ix1 * image1->depth + iy1 * image1->width * image1->depth;
	r1 = image1->imageMemory[index1 + 0];
	g1 = image1->imageMemory[index1 + 1];
	b1 = image1->imageMemory[index1 + 2];

	Vector *dest = &model->condensed_vertex_colors[i];
	dest->x = r0 * r1 * delta;
	dest->y = g0 * g1 * delta;
	dest->z = b0 * b1 * delta;

	if (!(do_channel_0 + do_channel_1)) {
	    dest->x = dest->y = dest->z = 1.0;
	} else if (!do_channel_0) {
	    dest->x = r1 * delta * 0.5;
	    dest->y = g1 * delta * 0.5;
	    dest->z = b1 * delta * 0.5;
	} else if (!do_channel_1) {
	    dest->x = r0 * delta * 0.5;
	    dest->y = g0 * delta * 0.5;
	    dest->z = b0 * delta * 0.5;
	}

	if (dest->x > 1.0) dest->x = 1.0;
	if (dest->y > 1.0) dest->y = 1.0;
	if (dest->z > 1.0) dest->z = 1.0;

	assert(dest->x >= 0.0);
	assert(dest->y >= 0.0);
	assert(dest->z >= 0.0);
    }
}

void render_gouraud_vanilla_proc(Brdf_Renderer *ren, Triangle_Model *model,
				 Brdf_Map_Info *info) {

    last_delta = last_wanted_delta;

    prepare_vertex_colors(ren, model, info);

    render_gouraud_vanilla(ren, model, info);
    if (ambient_light) render_ambient_vanilla(ren, model, info);
}

void render_gouraud_extensions_proc(Brdf_Renderer *ren, Triangle_Model *model,
				 Brdf_Map_Info *info) {

    last_delta = last_wanted_delta;

    prepare_vertex_colors(ren, model, info);
    render_gouraud_extensions(ren, model, info);
}

void render_extensions_proc(Brdf_Renderer *ren, Triangle_Model *model,
			    Brdf_Map_Info *info) {
    render_extensions(ren, model, info);
    if (ambient_light) render_ambient_vanilla(ren, model, info);
}

void render_nv4_proc(Brdf_Renderer *ren, Triangle_Model *model,
		     Brdf_Map_Info *info) {
    render_nv4(ren, model, info);
    if (ambient_light) render_ambient_vanilla(ren, model, info);
}

void megaclamp(Vector *v) {
    if (v->x < 0.0) v->x = 0.0;
    if (v->y < 0.0) v->y = 0.0;
    if (v->x > 1.0) v->x = 1.0;
    if (v->y > 1.0) v->y = 1.0;
}

void Brdf_Renderer::draw_model(Triangle_Model *model) {
    // XXX This can happen when AppPaint gets called early somehow;
    // we should fix it so that no rendering gets called until
    // an init_time variable is set, signalling that initialization
    // is completed!
    if (num_brdf_maps == 0) return;

    assert(num_brdf_maps > 0);

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    if (intensity_choke) {
        const float m = 0.5;
        GLfloat choked[] = { m, m, m, 1 };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, choked);
    } else {
        GLfloat unchoked[] = { 1, 1, 1, 1 };
        glLightfv(GL_LIGHT0, GL_DIFFUSE, unchoked);
    }

    Transformer tr;
    tr.push(model->orientation, model->position);

    glPushMatrix();
    load_matrix_into_gl(tr);

    Vector model_to_eye, model_to_light;

    tr.invert_in_place();
    model_to_eye.set(0, 0, 0);
    model_to_eye = tr.transform_point(model_to_eye);

    model_to_light = the_light_pos;
    model_to_light = tr.transform_point_rotate_only(model_to_light);

    Vector *c0 = model->face_brdf_channel_0;
    Vector *c1 = model->face_brdf_channel_1;

    int mindex = model->material_indices[0];
    Brdf_Map_Info *info = map_info[mindex];
    Brdf_Map *map = info->map;

    if (update_brdf_coordinates) {
	Param *param = map->getBrdf_Parameterizer();
	param->update_coords(model, model_to_eye, model_to_light);
    } else {
        int i;
	for (i = 0; i < model->num_condensed_items; i++) {
	    c0[i].x = 0.5;
	    c0[i].y = 0.5;
	    c1[i].x = 0.5;
	    c1[i].y = 0.5;
	}
    }


    // Adjust rendering parameters so that the image will come out
    // at the proper intensity.

    Brdf_Layer *layer = info->map->get_layer(0);
    double delta = layer->getDelta();

    int mag_factor = 1;
    if (!do_channel_0) mag_factor <<= 1;
    if (!do_channel_1) mag_factor <<= 1;
    if (mag_factor < 1) mag_factor = 1;

    double max_delta = render_proc_deltas[current_render_proc];
    if (delta > max_delta) delta = max_delta;

    if (delta > 1.0) mag_factor = 2;
    if (delta > 2.0) mag_factor = 4;
    if (delta > 4.0) mag_factor = 8;

    double scale_down = delta / (double)mag_factor;

    unsigned int byte_value = double_to_byte(scale_down);
    double actual_scale_down = byte_value / 255.0;
    double actual_delta = actual_scale_down * mag_factor;

    last_wanted_delta = layer->getDelta();
    last_delta = actual_delta;
    last_mag_factor = mag_factor;
    mag_workspace = mag_factor;
    last_scale_down = scale_down;
    scale_down_workspace = last_scale_down;

    // Draw the stuff!

    passes_rendered = 0;
    Render_Proc *proc = render_procs[current_render_proc];
    proc(this, model, info);


    glPopMatrix();

    triangles_drawn += model->nfaces;
    frames_computed += model->num_condensed_items;
}

float GetFloatTime(void) {
    static __int64 StartTime = 0, Frequency;
    __int64 Timer;

    if (!StartTime) {
        QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
        QueryPerformanceCounter((LARGE_INTEGER*)&StartTime);
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&Timer);
    return float(double(Timer - StartTime)/double(Frequency));
}

void set_delta(Brdf_Map_Info *info, double delta) {
    Brdf_Map *map = info->map;

    int i;
    for (i = 0; i < map->getNumberOfLayers(); i++) {
        Brdf_Layer *layer = map->get_layer(i);
	layer->lambda = layer->delta = delta;
    }
}


int PASCAL WinMain(HINSTANCE hinst, HINSTANCE,
		   LPSTR pCommandLine, int ShowMode) {
    hInstance = hinst;

    Transformer tr;
    Rotation_Matrix rm;

    Quaternion ori(0.3, 1.7, 0.4, 1.1);
    Vector offset(10.0, 3.0, -13.1);
    ori.normalize();
    rm.set(ori);

    tr.push(&rm, Vector(0, 0, 0), offset);

    Vector pos0, pos1, pos2;
    pos0.set(-10.3, 111.11, 1234.56);
    pos1 = tr.transform_point(pos0);
    tr.invert_in_place();
    pos2 = tr.transform_point(pos1);

    // get the user's specified display parameters, or defaults
    // the shortened names work because of the default value
    
    FullscreenWidth = GetCommandLineArgInt("width", 
					   pCommandLine, FullscreenWidth);

    FullscreenHeight = GetCommandLineArgInt("height", 
					    pCommandLine, FullscreenHeight);


    // Create the window classes for the main window and the
    // children
    if(!BuildClasses()) return FALSE;

    // Create the main application window
    AppWindow = CreateWindow(
			     lpszMainWndClass,
			     AppName,
			     WS_OVERLAPPEDWINDOW,
			     0, 0,               // Size and dimensions of window
			     FullscreenWidth, FullscreenHeight,
			     NULL,
			     NULL,
			     hInstance,
			     NULL);

    assert(g_hDC != 0);

    // Display the window
    ShowWindow(AppWindow,SW_SHOW);
    UpdateWindow(AppWindow);

    RECT rect;
    GetClientRect(AppWindow, &rect);
    screen_i = rect.right;
    screen_j = rect.bottom;

    // Set the viewport to be the entire window
    wglMakeCurrent(g_hDC, g_hRC);
    glViewport(0, 0, screen_i, screen_j);



    Brdf_Renderer *renderer = new Brdf_Renderer();
    the_brdf_renderer = renderer;

    renderer->init_rendering_state();

    Brdf_Map *map;

    GLuint cloth_handle = get_texture("cloth001.bmp");
    reject_this_texture = cloth_handle;

    font_handle = get_texture("font.bmp");

    GLuint red_handle = get_texture("marsand001.bmp");
    GLuint bush_handle = get_texture("bush001.bmp");
    GLuint metal_handle = get_texture("groundstruct001.bmp");
    GLuint granite1_handle = get_texture("brwngran.bmp");
    //    GLuint wood1_handle = get_texture("wood1.bmp");
    GLuint wood1_handle = get_texture("wood-11.bmp");
    //   GLuint wood1_handle = get_texture("wood-08.bmp");
    GLuint gold_handle = get_texture("gold.bmp");

    renderer->default_texture_handle = cloth_handle;

    Brdf_Map_Info *mi;

    mi = renderer->load_brdf_map("velvet_oi_stretch.ascii.brdfmap", "Velvet");
    mi->texture_id_diffuse = red_handle;

    mi = renderer->load_brdf_map("goldcolor_stretch.ascii.brdfmap", "Gold", 4.0);
    mi->texture_id_diffuse = gold_handle;

    mi = renderer->load_brdf_map("vinylapp_stretch.ascii.brdfmap", "Vinyl", 4.0, 1.0);
    mi = renderer->load_brdf_map("wood_stretch.ascii.brdfmap", "Wood", 2.0);
    mi->texture_id_diffuse = wood1_handle;
    mi->ambient_light_scale = 0.65;

    //    mi = renderer->load_brdf_map("wool_stretch.ascii.brdfmap", "Lamb's Wool", 4.0);
    mi = renderer->load_brdf_map("feather_stretch.ascii.brdfmap", "Peacock Feather", 4.0);
    mi = renderer->load_brdf_map("wardapp128.ascii.brdfmap", "Ward's model", 4.0);
    mi->texture_id_diffuse = granite1_handle;

    mi = renderer->load_brdf_map("cylinderapp128.ascii.brdfmap", "Brushed Metal", 1.0);
    mi->ambient_light_scale = 0.65;
    mi->texture_id_diffuse = metal_handle;

    //    mi = renderer->load_brdf_map("skin.ascii.brdfmap", "Skin", 4.0);
    mi = renderer->load_brdf_map("orange.ascii.brdfmap", "Orange Peel", 6.0);

    mi = renderer->load_brdf_map("moss.ascii.brdfmap", "Moss", 1.0);
    mi->ambient_light_scale = 0.45;
    set_delta(mi, 2.2);
    mi->texture_id_diffuse = bush_handle;

    MSG msg;
    for (;;) {
      if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
      } else {
	  if (app_is_active) {
	      AppPaint(AppWindow,g_hDC);
	  } else {
	      WaitMessage();
	  }
        }
    }

    return msg.wParam;
}

/*----------------------------------------------------------------------------

AppPaint

*/

void compute_xy(Vector *result, int ix, int iy, int screen_i, int screen_j) {
    float fx = ix;
    float fy = iy;

    float Z_PLANE = -1;
    *result = Vector(fx, fy, Z_PLANE);
}

void add_letter_quad(char *s, Vector *uv, Vector *xy, Vector *rgb,
		     int i, int xs, int ys, 
		     int letter_width, int letter_height,
		     float cr, float cg, float cb,
		     int screen_i, int screen_j) {

    unsigned int c = s[i];
    unsigned int ci = (c % 16);
    unsigned int cj = (c / 16);

    int w = letter_width;
    int h = letter_height;

    int n0 = i * 4 + 0;
    int n1 = i * 4 + 1;
    int n2 = i * 4 + 2;
    int n3 = i * 4 + 3;

    uv[n0].x = 0.0;
    uv[n0].y = 1.0;
    uv[n1].x = 0.0;
    uv[n1].y = 0.0;
    uv[n2].x = 1.0;
    uv[n2].y = 0.0;
    uv[n3].x = 1.0;
    uv[n3].y = 1.0;

    compute_xy(&xy[n0], xs+0, ys+0, screen_i, screen_j);
    compute_xy(&xy[n1], xs+0, ys+h, screen_i, screen_j);
    compute_xy(&xy[n2], xs+w, ys+h, screen_i, screen_j);
    compute_xy(&xy[n3], xs+w, ys+0, screen_i, screen_j);

    rgb[n0] = Vector(cr, cg, cb);
    rgb[n1] = Vector(cr, cg, cb);
    rgb[n2] = Vector(cr, cg, cb);
    rgb[n3] = Vector(cr, cg, cb);


    glTexCoord2f(ci, cj);
    glVertex2f(xy[n0].x, xy[n0].y);

    glTexCoord2f(ci, cj + 1);
    glVertex2f(xy[n1].x, xy[n1].y);

    glTexCoord2f(ci + 1, cj + 1);
    glVertex2f(xy[n2].x, xy[n2].y);

    glTexCoord2f(ci + 1, cj);
    glVertex2f(xy[n3].x, xy[n3].y);
}

const int LETTER_WIDTH = 11;
const int LETTER_HEIGHT = 13;

void draw_line(HDC dc, int *x, int *y, char *s) {
    const int LETTER_PAD = 2;

    const int MAXLEN = 1024;
    Vector uv[MAXLEN * 4];
    Vector xy[MAXLEN * 4];
    Vector rgb[MAXLEN * 4];

    int len = strlen(s);
    if (len > MAXLEN) len = MAXLEN;

    float cr = 1.0f;
    float cg = 1.0f;
    float cb = 1.0f;

    int xs = 0;
    int ys = *y;

    glBegin(GL_QUADS);

    int i;
    for (i = 0; i < len; i++) {
        add_letter_quad(s, uv, xy, rgb, i, xs, ys, LETTER_WIDTH, LETTER_HEIGHT,
			cr, cg, cb, screen_i, screen_j);

	xs += LETTER_WIDTH;
    }

    glEnd();

    *y += LETTER_HEIGHT + LETTER_PAD;
}

void begin_help_text_mode() {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(LETTER_WIDTH / 256.0, LETTER_HEIGHT / 256.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_i, screen_j, 0, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, font_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
}

void end_help_text_mode() {
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

static int drawing_help_text = 0;
void draw_help_text(HDC dc, int *x, int *y) {
    if (!drawing_help_text) {
        draw_line(dc, x, y, "Press F1 for help on key commands.");
	return;
    }

    draw_line(dc, x, y, "Press F1 to eliminate help on key commands.");
    draw_line(dc, x, y, "A: Toggle diffuse texture.   T: Toggle updating BRDF texture coordinates.");
    draw_line(dc, x, y, "E, R: Cycle to previous or next available renderer.");
    draw_line(dc, x, y, "J, K: Cycle to previous or next BRDF map.");
    draw_line(dc, x, y, "0, 9: See different types of displays.");
    draw_line(dc, x, y, "ESCAPE: Quit");
    draw_line(dc, x, y, "SPACE: Toggle object rotation (default: on)");
    draw_line(dc, x, y, "L: Toggle light rotation (default: off)");
}


Triangle_Model *get_model_from_mode() {
    Triangle_Model *model;
    switch (object_viewing_mode) {
      case DRAWING_SINGLE_LARGE_TEAPOT:
	model = big_test_model;
	break;
      case DRAWING_MULTIPLE_TEAPOTS:
	model = anisotropy_test_model;
	break;
      case DRAWING_ANISOTROPIC_DEMO:
	model = anisotropy_test_model;
	break;
    }

    return model;
}

static char txt_channels[3] = "  ";
void draw_text(HDC dc) {
    if (the_brdf_renderer == NULL) return;

    int x = 10;
    int y = 10;

    float seconds = frame_time.get_value();
    float fps = 1.0f / seconds;

    char buf[BUFSIZ];
    sprintf(buf, "fps: %3.2f", fps);
    draw_line(dc, &x, &y, buf);

    if (show_fps_only) return;

    glColor3f(1, 1, 1);

    draw_help_text(dc, &x, &y);
    draw_line(dc, &x, &y, "Press 'f' for frame-counter-only (highest frame rate)");

    glColor3f(1, 1, 0);

    Brdf_Renderer *ren = the_brdf_renderer;

    Triangle_Model *model = get_model_from_mode();
    int mapno = model->material_indices[0];
    if (mapno >= ren->num_brdf_maps) return;  // XXX why?!?
    assert(mapno < ren->num_brdf_maps);

    Brdf_Map_Info *info = ren->map_info[mapno];

    if (intensity_choke) draw_line(dc, &x, &y, "INTENSITY_CHOKE");
    if (!update_brdf_coordinates) draw_line(dc, &x, &y, "NOT UPDATING BRDF COORDINATES");

    sprintf(buf, "Current material: %s", info->name);
    draw_line(dc, &x, &y, buf);


    if (use_mipmaps) {
        sprintf(buf, "Mipmaps: ON");
    } else {
        sprintf(buf, "Mipmaps: OFF");
    }
    draw_line(dc, &x, &y, buf);

    
    char *renderer = ren->render_proc_names[ren->current_render_proc];

    txt_channels[0] = ' ';
    txt_channels[1] = ' ';
    if (do_channel_0) txt_channels[0] = '1';
    if (do_channel_1) txt_channels[1] = '2';

    sprintf(buf, "Current renderer: %s", renderer);
    draw_line(dc, &x, &y, buf);
    draw_line(dc, &x, &y, txt_channels);


    sprintf(buf, "triangles: %d; passes: %d; vertex frames: %d",
	    the_brdf_renderer->triangles_drawn,
	    the_brdf_renderer->passes_rendered,
	    the_brdf_renderer->frames_computed);
    draw_line(dc, &x, &y, buf);

    sprintf(buf, "Light: %s%s",
	    ambient_light ? "Ambient " : "",
	    use_light_source ? "Directional" : "");
    draw_line(dc, &x, &y, buf);

    sprintf(buf, "delta %.3f (wanted %.3f) = mag %d scaledown %.3f",
	    last_delta, last_wanted_delta, last_mag_factor, last_scale_down);
    draw_line(dc, &x, &y, buf);
}

static double kx = 0.0;
static double ky = 0.0;

void draw_single_large_teapot(Brdf_Renderer *ren, double object_theta) {
    Triangle_Model *model = big_test_model;

    if (model) {
        model->orientation.set_from_axis_and_angle(0, 1, 0, object_theta);
	model->orientation.set_from_axis_and_angle(1, 1, 1, object_theta);
	model->position.set(kx, ky, -10.0);
	ren->draw_model(model);
    }
}

static float aniso_animation_time = 0;
static float aniso_last_time = 0;
static float aniso_object_theta = 0;


void draw_anisotropic_demo(Brdf_Renderer *ren, double object_theta) {
    Triangle_Model *model = anisotropy_test_model;

    float theta;

    float now = GetFloatTime();
    if (aniso_last_time == 0) aniso_last_time = now;

    float delta = now - aniso_last_time;
    aniso_last_time = now;

    Vector light(1, 0, 0);

    float dtheta;
    int itime = aniso_animation_time * 5;
    if ((itime % 9) > 4) {
        dtheta = 0;
    } else {
        dtheta = delta * M_PI * 0.5;
    }

    if (animate_orientation) {
        aniso_object_theta += dtheta;
	aniso_animation_time += delta;
    }

    if (model) {
        model->orientation.set_from_axis_and_angle(0, 0, 1, aniso_object_theta);
	model->position.set(0, 0, -5.0);
	ren->draw_model(model);
    }
}

void draw_multiple_teapots(Brdf_Renderer *ren, double object_theta) {
    Triangle_Model *first_model = test_models[0];
    if (!first_model) return;

    int i;
    for (i = 0; i < 4; i++) {
        Triangle_Model *model = test_models[i];
	double ox = 4.2;
	double oy = 2.9;

	int k = i;

	if ((k == 1) || (k == 2)) ox = -ox;
	if (k > 1) oy = -oy;

	assert(model != NULL);

        model->orientation.set_from_axis_and_angle(0, 1, 0, object_theta);
	model->orientation.set_from_axis_and_angle(1, 1, 1, object_theta);
	model->position.set(ox, oy, -20.0);
	ren->draw_model(model);
	//	glFlush();
    }
}

int AppPaint(HWND, HDC) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float LastTime = 0;
    static float AnimationTime = 0;

    
    float CurrentTime = GetFloatTime();
    if (LastTime == 0) {
        LastTime = CurrentTime;
	AnimationTime = CurrentTime;
    }

    float delta = CurrentTime - LastTime;
    frame_time.accumulate(delta);

    AnimationTime += CurrentTime - LastTime;

    LastTime = CurrentTime;

    Vector light(1, 0, 0);

    float dtheta = delta * M_PI * 0.5;
    if (animate_orientation) last_object_theta += dtheta;
    if (animate_light) last_light_theta += dtheta;

    float light_theta = last_light_theta;
    float object_theta = last_object_theta;
    
    Quaternion q;
    q.set_from_axis_and_angle(0, 1, 0, light_theta);
    light = light.rotate(q);
    the_light_pos = light;

    float gl_light_position[4];
    gl_light_position[0] = light.x;
    gl_light_position[1] = light.y;
    gl_light_position[2] = light.z;
    gl_light_position[3] = 0.0f;
    glLightfv(GL_LIGHT0, GL_POSITION, gl_light_position);

    the_brdf_renderer->triangles_drawn = 0;
    the_brdf_renderer->frames_computed = 0;


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float Aspect = 4.0f / 3.0f;
    glFrustum(-Aspect, Aspect, -1., 1., 2.5, 10000.0);
    glMatrixMode(GL_MODELVIEW);


    Brdf_Renderer *ren = the_brdf_renderer;
    if (object_viewing_mode == DRAWING_SINGLE_LARGE_TEAPOT) {
        draw_single_large_teapot(ren, object_theta);
    } else if (object_viewing_mode == DRAWING_MULTIPLE_TEAPOTS) {
        draw_multiple_teapots(ren, object_theta);
    } else if (object_viewing_mode == DRAWING_ANISOTROPIC_DEMO) {
        draw_anisotropic_demo(ren, object_theta);
    }

    extern void begin_help_text_mode();
    extern void end_help_text_mode();

    begin_help_text_mode();
    draw_text(g_hDC);
    end_help_text_mode();

    SwapBuffers(g_hDC);

    return TRUE;
}

void cycle_brdf_map(int delta) {
    Triangle_Model *model = get_model_from_mode();

    int mindex = model->material_indices[0];

    Brdf_Renderer *ren = the_brdf_renderer;
    mindex += ren->num_brdf_maps;
    mindex += delta;
    mindex %= ren->num_brdf_maps;

    model->material_indices[0] = mindex;
}

int unsigned GetCommandLineArgInt(char const *pArgument,
                                  char const *pCommandLine,
                                  int unsigned DefaultValue) {
    assert(pArgument && pCommandLine);
    int unsigned Value = DefaultValue;
    char const *pStart;
    if((pStart = strstr(pCommandLine,pArgument)) != 0)
    {
        char aFormat[1024];
        ostrstream Out(aFormat,sizeof(aFormat));
        Out<<pArgument<<"=%d"<<ends;
        sscanf(pStart,aFormat,&Value);
    }
    return Value;
}

bool GetCommandLineArgString(char const *pArgument,
                             char const *pCommandLine,
                             char *pStringBuffer) {
    assert(pArgument && pCommandLine && pStringBuffer);

    bool ReturnValue = false;
    char const *pStart;
    if((pStart = strstr(pCommandLine,pArgument)) != 0) {
        char aFormat[1024];
        ostrstream Out(aFormat,sizeof(aFormat));
        Out<<pArgument<<"=%s"<<ends;
        int Fields = sscanf(pStart,aFormat,pStringBuffer);

        if (Fields && (Fields != EOF)) {
            ReturnValue = true;
        }
    }

    return ReturnValue;
}





extern "C" void __cdecl _assert(void *pExpression, void *pFile,
				unsigned LineNumber) {
    char aBuffer[500];
    wsprintf(aBuffer,"Assertion: %s\nFile: %s, Line: %d\n" \
             "Hit Abort to exit, Retry to debug, Ignore to continue",
             pExpression,pFile,LineNumber);

    int Hit = MessageBox(AppWindow,aBuffer,"Assert!",MB_ABORTRETRYIGNORE |
                         MB_ICONHAND | MB_TOPMOST | MB_SYSTEMMODAL);

    if (Hit == IDABORT) exit(0);
    if (Hit == IDRETRY) DebugBreak();
}





Decaying_Average::Decaying_Average() {
    decay_constant = 0.88f;
    value = 1.0f;
}

void Decaying_Average::set_decay_constant(float decay) {
    decay_constant = decay;
}

void Decaying_Average::set_value(float _value) {
    value = _value;
}

void Decaying_Average::accumulate(float new_value) {
    value = (value * decay_constant) + (new_value * (1.0f - decay_constant));
}























/////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
/////////////////////////////////////////////////////////////////////////////////////

// Select the pixel format for a given device context. This function is identical
// to the above, but also supplies a depth buffer
void SetDCDepthPixelFormat(HDC hDC)
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size of this structure
		1,                                                              // Version of this structure    
		PFD_DRAW_TO_WINDOW |                    // Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |					// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,                       // Double buffered
		PFD_TYPE_RGBA,                          // RGBA Color mode
		24,                                     // Want 24bit color 
		0,0,0,0,0,0,                            // Not used to select mode
		0,0,                                    // Not used to select mode
		0,0,0,0,0,                              // Not used to select mode
		// Try to get away with smaller depth buffer to take advantage
		// of low end PC accelerator cards
		32,                                     // Size of depth buffer
		0,                                      // Not used to select mode
		0,                                      // Not used to select mode
		PFD_MAIN_PLANE,                         // Draw in main plane
		0,                                      // Not used to select mode
		0,0,0 };                                // Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	// Set the pixel format for the device context
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Window procedure, handles all messages for this window
LRESULT CALLBACK WndProcView(HWND    hWnd,
			     UINT    message,
			     WPARAM  wParam,
			     LPARAM  lParam)
{
    Brdf_Renderer *ren = the_brdf_renderer;

//// Local Variables ////////////////////////////////////////////////////////////////
	int tx,ty;
/////////////////////////////////////////////////////////////////////////////////////
	
	switch (message)
		{
		case WM_ACTIVATEAPP:
		        if (wParam) app_is_active = true;
		        else app_is_active = false;
		// Window creation, setup here
		case WM_CREATE:
		        g_hDC = GetDC(hWnd);

			// Set the pixel format
			SetDCDepthPixelFormat(g_hDC);

			// Create the rendering context and make it current
			g_hRC = wglCreateContext(g_hDC);
			assert(g_hRC != NULL);

			BOOL succ;
			succ = wglMakeCurrent(g_hDC, g_hRC);
			assert(succ);

			break;

		// Window is being destroyed, cleanup
		case WM_DESTROY:
			// Cleanup...
			// Deselect the current rendering context and delete it
			wglMakeCurrent(g_hDC,NULL);
			wglDeleteContext(g_hRC);

			// Destroy the palette if it was created 
			if(hPalette != NULL)
				DeleteObject(hPalette);

			// Release the device context
			ReleaseDC(hWnd,g_hDC);
			break;

		// Window is resized. Setup the viewing transformation
		case WM_SIZE:
			{
			int nWidth,nHeight;
			double dAspect;

			nWidth = LOWORD(lParam);  // width of client area 
			nHeight = HIWORD(lParam); // height of client area 

			screen_i = nWidth;
			screen_j = nHeight;
			if(nHeight == 0)		  // Don't allow divide by zero
				nHeight = 1;

			dAspect = (double)nWidth/(double)nHeight;

			// Make this rendering context current
			wglMakeCurrent(g_hDC, g_hRC);

			// Set the viewport to be the entire window
		    glViewport(0, 0, nWidth, nHeight);
	
			// Setup Perspective
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			/*
			// Establish viewing volume
			gluPerspective(CAMERA_FOV, dAspect,0.2f, 2000);
			*/

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			}
			break;

		case WM_PAINT:
			{

			// Validate the newly painted client area
			ValidateRect(hWnd,NULL);
			}
			break;


		// Windows is telling the application that it may modify
		// the system palette.  This message in essance asks the 
		// application for a new palette.
		case WM_QUERYNEWPALETTE:
			// If the palette was created.
			if(hPalette)
				{
				int nRet;

				// Selects the palette into the current device context
				SelectPalette(g_hDC, hPalette, FALSE);

				// Map entries from the currently selected palette to
				// the system palette.  The return value is the number 
				// of palette entries modified.
				nRet = RealizePalette(g_hDC);

				// Repaint, forces remap of palette in current window
				InvalidateRect(hWnd,NULL,FALSE);

				return nRet;
				}
			break;

	
		// This window may set the palette, even though it is not the 
		// currently active window.
		case WM_PALETTECHANGED:
			// Don't do anything if the palette does not exist, or if
			// this is the window that changed the palette.
			if((hPalette != NULL) && ((HWND)wParam != hWnd))
				{
				// Select the palette into the device context
				SelectPalette(g_hDC,hPalette,FALSE);

				// Map entries to system palette
				RealizePalette(g_hDC);
				
				// Remap the current colors to the newly realized palette
				UpdateColors(g_hDC);
				return 0;
				}
			break;


		// Handle Right Mouse Button Press
		case WM_RBUTTONDOWN:
			break;

		// Handle Right Mouse Button Release
		case WM_RBUTTONUP:
			break;

	default:   // Passes it on if unproccessed
	    return (DefWindowProc(hWnd, message, wParam, lParam));

	}

    return (0L);
}


void handle_keydown(int param) {
    Brdf_Renderer *ren = the_brdf_renderer;

    switch (param) {
      case VK_SPACE:
	animate_orientation = !animate_orientation;
	break;
    case 'L':
	animate_light = !animate_light;
	break;
    case 'M':
        use_mipmaps = !use_mipmaps;
	if (!ALLOW_MIPMAPPING) use_mipmaps = 0;
	break;
    case VK_ESCAPE:
	PostMessage(AppWindow,WM_CLOSE,0,0);
	break;
    case VK_F1:
	drawing_help_text = !drawing_help_text;
	break;
    case 'K':
	cycle_brdf_map(1);
	break;
    case 'J':
	cycle_brdf_map(-1);
	break;
    case 'T': {
	update_brdf_coordinates = !update_brdf_coordinates;
	break;
    }
    case 'F':
	show_fps_only = !show_fps_only;
	break;
    case 'R':
	ren->current_render_proc++;
	if (ren->current_render_proc == ren->num_render_procs)
	    ren->current_render_proc = 0;
	break;
    case 'E':
	ren->current_render_proc--;
	if (ren->current_render_proc < 0)
	    ren->current_render_proc = ren->num_render_procs - 1;
	break;
    case 'I':
	intensity_choke = !intensity_choke;
	break;
    case 'A':
	ambient_light = !ambient_light;
	break;
    case 'S':
	use_light_source = !use_light_source;
	break;

    case '1':
	do_channel_0 = !do_channel_0;
	break;
    case '2':
	do_channel_1 = !do_channel_1;
	break;
    case '7':
	kx += 0.1;
	break;
    case '8':
	ky += 0.1;
	break;
    case '9':
	object_viewing_mode--;
	if (object_viewing_mode < 0)
	    object_viewing_mode = NUM_OBJECT_VIEWING_MODES - 1;
	aniso_last_time = aniso_animation_time = 0;
	aniso_object_theta = 0;
	break;
    case '0':
	object_viewing_mode++;
	if (object_viewing_mode == NUM_OBJECT_VIEWING_MODES)
	    object_viewing_mode = 0;
	aniso_last_time = aniso_animation_time = 0;
	aniso_object_theta = 0;
	break;
    case 'O':
	do_overbright = !do_overbright;
	break;
    }
}


