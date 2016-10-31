#include "../framework.h"
#include <math.h>
#include "../mesh.h"
#include "../app_shell/os_specific_opengl_headers.h"
#include <gl/glu.h>

#include "rendering.h"

struct Triangle_List_Mesh;

static bool vertex_buffer_extensions_initted = false;

void (__stdcall *glBindBufferARB)(GLenum, GLuint);
void (__stdcall *glGenBuffersARB)(GLsizei, GLuint *);
void (__stdcall *glBufferDataARB)(GLenum, GLuint, void *, GLenum);

const int MAX_EXTENSIONS = 400;
char *extension_list[MAX_EXTENSIONS];
int num_extensions;
int extensions_found = 0;

const int OFFSET_XYZ = 0;
const int OFFSET_NORMAL = 3 * sizeof(float);
const int OFFSET_TEX_COORD = OFFSET_NORMAL + 3 * sizeof(float);
const int VERTEX_STRIDE = OFFSET_TEX_COORD + 2 * sizeof(float);

#define GL_BUFFFER_OFFSET(i) ((char *)NULL + (i))


const unsigned int GL_ARRAY_BUFFER_ARB = 0x8892;
const unsigned int GL_ELEMENT_ARRAY_BUFFER_ARB = 0x8893;

const unsigned int GL_STREAM_DRAW_ARB = 0x88E0;
const unsigned int GL_STREAM_READ_ARB = 0x88E1;
const unsigned int GL_STREAM_COPY_ARB = 0x88E2;
const unsigned int GL_STATIC_DRAW_ARB = 0x88E4;
const unsigned int GL_STATIC_READ_ARB = 0x88E5;
const unsigned int GL_STATIC_COPY_ARB = 0x88E6;
const unsigned int GL_DYNAMIC_DRAW_ARB = 0x88E8;
const unsigned int GL_DYNAMIC_READ_ARB = 0x88E9;
const unsigned int GL_DYNAMIC_COPY_ARB = 0x88EA;

void rendering_3d(Matrix4 *tr, Projector *projector) {
    // The convention in my user-level code is that the X axis points
    // forward, the Y axis points to the left, the Z axis points up.
    // This is true for all objects, including the camera.

    // A lot of code (including OpenGL / GLU) uses this screwed-up
    // convention where the minus-Z axis is forward.  That's so
    // that the X axis in view space maps to the X axis in projected
    // space, etc.  That's fine for low level rendering but propagating
    // that throughout all your code is VERY CONFUSING since -Z being
    // forward is just a difficult basis to think about. 

    // So before feeding the matrix to opengl, I permute it from
    // my convention to glu's.

    float modelview_coefs[16];

    Matrix4 permute;
    permute.zero_matrix();
    permute.coef[2][0] = -1;
    permute.coef[0][1] = -1;
    permute.coef[1][2] = 1;
    permute.coef[3][3] = 1;

    Matrix4 result;
    transform_multiply(&permute, tr, &result);
    
    int i, j;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            float factor = result.coef[i][j];
            int index = j*4 + i;

            modelview_coefs[index] = factor;
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(modelview_coefs);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float w = projector->viewport_width;
    float h = projector->viewport_height;
    float viewport_ratio = w / h;

    float view_angle = projector->fov * (180 / M_PI) * (1.0f / viewport_ratio);

    gluPerspective(view_angle, viewport_ratio, 1.0f, // AAAprojector->z_viewport,
                   1000.0f);

}

bool lookup_extension_name(char *name) {
    int i;
    for (i = 0; i < num_extensions; i++) {
        if (strcmp(name, extension_list[i]) == 0) return true;
    }

    return false;
}

int init_gl_extensions() {
    const unsigned char *const extensions = glGetString(GL_EXTENSIONS);

    unsigned char *s = (unsigned char *)extensions;
    int index = 0;

    while (*s) {
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
    if (lookup_extension_name("GL_ARB_vertex_buffer_object")) {
        extensions_found |= EXTENSION_ARB_VERTEX_BUFFER_OBJECT;
    }

    return extensions_found;
}



void init_vertex_buffers(Triangle_List_Mesh *mesh) {
    if (!vertex_buffer_extensions_initted) {
        vertex_buffer_extensions_initted = true;

        if (extensions_found & EXTENSION_ARB_VERTEX_BUFFER_OBJECT) {
            *(void **)&glBindBufferARB = (void *)wglGetProcAddress("glBindBufferARB");
            *(void **)&glGenBuffersARB = (void *)wglGetProcAddress("glGenBuffersARB");
            *(void **)&glBufferDataARB = (void *)wglGetProcAddress("glBufferDataARB");        
        }
    }

    // Generate some normals for this mesh (used even if we don't have
    // vertex buffers).
    if (mesh->normals == NULL) {
        Vector3 *normals = new Vector3[mesh->num_vertices];
        int i;
        for (i = 0; i < mesh->num_vertices; i++) {
            Vector3 z(0, 0, 1);
            z.rotate(mesh->tangent_frames[i]);
            normals[i] = z;
        }
        mesh->normals = normals;
    }


    // If we didn't set a valid function pointer for glBindBufferARB,
    // then bail on this... we don't have the vertex buffer extension 
    // available to us, which means the rest of the system will just
    // have to use slow rendering.
    if (!glBindBufferARB) return;

    GLuint index_buffer, vertex_buffer;
    glGenBuffersARB(1, &index_buffer);
    glGenBuffersARB(1, &vertex_buffer);

    const int VERTEX_SIZE_IN_FLOATS = 8; // 3 for xyz, 3 for normal, 2 for uv

    float *vertex_buffer_data = new float[mesh->num_vertices * VERTEX_SIZE_IN_FLOATS];
    unsigned short *index_buffer_data = new unsigned short[mesh->num_indices];

    int i;
    for (i = 0; i < mesh->num_indices; i++) {
        assert(mesh->indices[i] < 65536);  // Because we're packing them into shorts!
        index_buffer_data[i] = mesh->indices[i];
    }

    // Pack all the vertex data into one array to give to the hardware.
    for (i = 0; i < mesh->num_vertices; i++) {
        float *v = vertex_buffer_data + i * VERTEX_SIZE_IN_FLOATS;
        v[0] = mesh->vertices[i].x;
        v[1] = mesh->vertices[i].y;
        v[2] = mesh->vertices[i].z;
        v[3] = mesh->normals[i].x;
        v[4] = mesh->normals[i].y;
        v[5] = mesh->normals[i].z;
        v[6] = mesh->uvs[i].x;
        v[7] = mesh->uvs[i].y;
    }


    int index_buffer_size = mesh->num_indices * sizeof(index_buffer_data[0]);
    int vertex_buffer_size = mesh->num_vertices * VERTEX_SIZE_IN_FLOATS * sizeof(float);
    
    // Create index buffer
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, index_buffer);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, index_buffer_size,
                    index_buffer_data, GL_STATIC_DRAW_ARB);

    // Create vertex buffer
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertex_buffer);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertex_buffer_size,
                    vertex_buffer_data, GL_STATIC_DRAW_ARB);

    delete [] vertex_buffer_data;
    delete [] index_buffer_data;

    mesh->index_buffer = index_buffer;
    mesh->vertex_buffer = vertex_buffer;
}

void emit_mesh(Triangle_List_Mesh *mesh) {
    if (!mesh->vertex_buffer) {
        init_vertex_buffers(mesh);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    if (mesh->vertex_buffer) {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->index_buffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vertex_buffer);

        glVertexPointer(3, GL_FLOAT, VERTEX_STRIDE, GL_BUFFFER_OFFSET(OFFSET_XYZ)); 
        glTexCoordPointer(2, GL_FLOAT, VERTEX_STRIDE, GL_BUFFFER_OFFSET(OFFSET_TEX_COORD)); 
        glNormalPointer(GL_FLOAT, VERTEX_STRIDE, GL_BUFFFER_OFFSET(OFFSET_NORMAL));
    } else {
        if (glBindBufferARB) {
            // In case we have vertex buffers for some meshes, but not
            // this one, we need to turn off vertex buffer usage.  (i.e.
            // the extension is enabled, but we didn't generate vertex 
            // buffers for all meshes).  That should not happen in this
            // demo app, but I am doing this for robustness.
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }

        glVertexPointer(3, GL_FLOAT, 0, mesh->vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs);
        glNormalPointer(GL_FLOAT, 0, mesh->normals);
    }

    int i;
    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *list_info = &mesh->triangle_list_info[i];

        Mesh_Material_Info *material_info = &mesh->material_info[list_info->material_index];
        if (material_info->texture_index != -1) {
            extern bool textures_disabled;
            if (textures_disabled) {
                extern Loaded_Texture_Info white_texture;
                glBindTexture(GL_TEXTURE_2D, white_texture.texture_handle);
            } else {
                glBindTexture(GL_TEXTURE_2D, material_info->texture_index);
            }
        }

        if (mesh->vertex_buffer) {
            glDrawElements(GL_TRIANGLES, list_info->num_vertices, 
                           GL_UNSIGNED_SHORT,
                           GL_BUFFFER_OFFSET(list_info->start_of_list * sizeof(unsigned short)));

        } else {
            glDrawElements(GL_TRIANGLES, list_info->num_vertices, 
                           GL_UNSIGNED_INT,
                           mesh->indices + list_info->start_of_list);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}
