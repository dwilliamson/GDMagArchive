#include "../framework.h"
#include <math.h>
#include "../mesh.h"
#include "../app_shell/os_specific_opengl_headers.h"
#include <gl/glu.h>

struct Triangle_List_Mesh;

void rendering_3d(Transformation_Matrix *tr, Projector *projector) {
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

    Transformation_Matrix permute;
    permute.zero_matrix();
    permute.coef[2][0] = -1;
    permute.coef[0][1] = -1;
    permute.coef[1][2] = 1;
    permute.coef[3][3] = 1;

    Transformation_Matrix result;
    transform_multiply(&permute, tr, &result);
    
    int i, j;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            float factor = result.coef[i][j];
            int index = j*4 + i;

            modelview_coefs[index] = factor;
        }
    }

/*
    int i, j;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            float factor = tr->coef[i][j];
            int index = j*4 + i;

            modelview_coefs[index] = factor;
        }
    }
*/

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

void rendering_2d_right_handed(bool unit_scale = false) {
}

void emit_mesh(Triangle_List_Mesh *mesh) {
    int i;
    for (i = 0; i < mesh->num_triangle_lists; i++) {
        Triangle_List_Info *list_info = &mesh->triangle_list_info[i];

        Mesh_Material_Info *material_info = &mesh->material_info[list_info->material_index];
        if (material_info->texture_index != -1) {
            glBindTexture(GL_TEXTURE_2D, material_info->texture_index);
        }

        // @Functionality todo: Make material current.  Right now we are
        // basically just ignoring the mesh materials and rendering
        // the mesh in a shade of blue.

        glBegin(GL_TRIANGLES);

        int j;
        for (j = 0; j < list_info->num_vertices; j++) {
            int index = mesh->indices[list_info->start_of_list + j];
            Vector3 position = mesh->vertices[index];

            // XXXXX Speed this up!
            Vector3 z(0, 0, 1);
            z.rotate(mesh->tangent_frames[index]);
            glNormal3fv((float *)&z);
            glTexCoord2fv((float *)&mesh->uvs[index]);
            glVertex3fv((float *)&position);
        }

        glEnd();
    }
}
