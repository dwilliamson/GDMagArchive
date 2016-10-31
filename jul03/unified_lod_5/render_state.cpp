#include "framework.h"

#include "make_world.h"
#include "mesh.h"
#include "mesh_reducer.h"
#include "mesh_topology_handler.h"

#include "os_specific_opengl_headers.h"
#include "framework/rendering.h"
#include "render_state.h"

#include <math.h>

const float POLYGON_OFFSET_MAGNITUDE = -1.1f;
Vector3 light_direction(-0.7f, 0.0f, 0.5f);

extern bool color_code_by_lod;
extern bool drawing_obvious_seams;

// Set up a single directional light to shine on our mesh.
void setup_mesh_lighting() {
    glEnable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColor4f(1, 1, 1, 1);

    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { 0, 0, 0, 0 };

    // Light from some arbitrary direction in world space.

    light_pos[0] = 0.1f;
    light_pos[1] = 0.7f;
    light_pos[2] = 0.3f;

    // Set the various GL lighting properties.
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    GLfloat unchoked[] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, unchoked);

    const float s = 0.4f;
    GLfloat ambient_i0[] = { s, s, s, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_i0);
}

void setup_mesh_render_opacity(float alpha, bool use_zfunc_lessequal) {
    glColor4f(1, 1, 1, alpha);

    if (use_zfunc_lessequal) {
        glDepthFunc(GL_LEQUAL);
    } else {
        glDepthFunc(GL_LESS);
    }
}

// Set up the render state for drawing a mesh.
void setup_mesh_render_state(Render_State_Type type, bool wireframe) {
    glEnable(GL_DEPTH_TEST);

    if (wireframe) {
        glDisable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(POLYGON_OFFSET_MAGNITUDE, 0);

        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, 1);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        return;
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    setup_mesh_lighting();

    if (color_code_by_lod) glDisable(GL_TEXTURE_2D);

    switch (type) {
    case RS_SOLID:
        glDepthFunc(GL_LESS);
        glDisable(GL_BLEND);
        glColor4f(1, 1, 1, 1);
        break;
    case RS_FADING:
        glDepthFunc(GL_LESS);
//        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1, 1, 1, 1);
        break;
    default:
        assert(0);
        break;
    }
}
