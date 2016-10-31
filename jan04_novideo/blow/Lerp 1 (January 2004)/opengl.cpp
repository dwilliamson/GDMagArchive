#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "primitives_private.h"

#include "os_specific_opengl_headers.h"
#include "app_shell.h"

#include "lerp_window.h"
#include "module_macros.h"

/*
const int buf_len = 1024;
static char error_buf[buf_len];
*/


void proc_clear(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int clear_type;
    bool success = coerce_to_integer(interp, record, 0, &clear_type);
    if (!success) return;

    glClear(clear_type);
}

void proc_loadIdentity(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 0)) return;
    glLoadIdentity();
}

void proc_matrixMode(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int type;
    bool success = coerce_to_integer(interp, record, 0, &type);
    if (!success) return;

    glMatrixMode(type);
}

void proc_bindTexture(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 2)) return;
    int mode, handle;
    bool success;
    
    success = coerce_to_integer(interp, record, 0, &mode);
    if (!success) return;
    success = coerce_to_integer(interp, record, 1, &handle);
    if (!success) return;

    glBindTexture(mode, handle);
}

void proc_blendFunc(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 2)) return;
    int i0, i1;
    bool success;
    
    success = coerce_to_integer(interp, record, 0, &i0);
    if (!success) return;
    success = coerce_to_integer(interp, record, 1, &i1);
    if (!success) return;

    glBlendFunc(i0, i1);
}

void proc_enable(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int type;
    bool success = coerce_to_integer(interp, record, 0, &type);
    if (!success) return;

    glEnable(type);
}

void proc_disable(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int type;
    bool success = coerce_to_integer(interp, record, 0, &type);
    if (!success) return;

    glDisable(type);
}

void proc_depthFunc(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int arg;
    bool success = coerce_to_integer(interp, record, 0, &arg);
    if (!success) return;

    glDepthFunc(arg);
}

void proc_shadeModel(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int arg;
    bool success = coerce_to_integer(interp, record, 0, &arg);
    if (!success) return;

    glShadeModel(arg);
}

void proc_colorMaterial(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 2)) return;
    int arg0, arg1;
    bool success = coerce_to_integer(interp, record, 0, &arg0);
    success &= coerce_to_integer(interp, record, 1, &arg1);
    if (!success) return;

    glColorMaterial(arg0, arg1);
}

void proc_texEnvi(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;
    int arg0, arg1, arg2;
    bool success = coerce_to_integer(interp, record, 0, &arg0);
    success &= coerce_to_integer(interp, record, 1, &arg1);
    success &= coerce_to_integer(interp, record, 2, &arg2);
    if (!success) return;

    glTexEnvi(arg0, arg1, arg2);
}

void proc_texParameteri(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;
    int arg0, arg1, arg2;
    bool success = coerce_to_integer(interp, record, 0, &arg0);
    success &= coerce_to_integer(interp, record, 1, &arg1);
    success &= coerce_to_integer(interp, record, 2, &arg2);
    if (!success) return;

    glTexParameteri(arg0, arg1, arg2);
}

void proc_begin(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    int primitive_type;
    bool success = coerce_to_integer(interp, record, 0, &primitive_type);
    if (!success) return;

    glBegin(primitive_type);
}

void proc_end(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 0)) return;
    glEnd();
}

void proc_vertex3f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;

    float f0, f1, f2;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    if (!success) return;

    glVertex3f(f0, f1, f2);
}

void proc_normal3f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;

    float f0, f1, f2;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    if (!success) return;

    glNormal3f(f0, f1, f2);
}

void proc_color3f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;

    float f0, f1, f2;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    if (!success) return;

    glColor3f(f0, f1, f2);
}

void proc_color4f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 4)) return;

    float f0, f1, f2, f3;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    success &= coerce_to_float(interp, record, 3, &f3);
    if (!success) return;

    glColor4f(f0, f1, f2, f3);
}

void proc_light3(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 5)) return;

    int i0, i1;
    float f0, f1, f2;

    bool success = true;
    success &= coerce_to_integer(interp, record, 0, &i0);
    success &= coerce_to_integer(interp, record, 1, &i1);
    success &= coerce_to_float(interp, record, 2, &f0);
    success &= coerce_to_float(interp, record, 3, &f1);
    success &= coerce_to_float(interp, record, 4, &f2);
    if (!success) return;

    float arg[4];
    arg[0] = f0;
    arg[1] = f1;
    arg[2] = f2;
    arg[3] = 0;
    glLightfv(i0, i1, arg);
}

void proc_light4(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 6)) return;

    int i0, i1;
    float f0, f1, f2, f3;

    bool success = true;
    success &= coerce_to_integer(interp, record, 0, &i0);
    success &= coerce_to_integer(interp, record, 1, &i1);
    success &= coerce_to_float(interp, record, 2, &f0);
    success &= coerce_to_float(interp, record, 3, &f1);
    success &= coerce_to_float(interp, record, 4, &f2);
    success &= coerce_to_float(interp, record, 5, &f3);
    if (!success) return;

    float arg[4];
    arg[0] = f0;
    arg[1] = f1;
    arg[2] = f2;
    arg[3] = f3;
    glLightfv(i0, i1, arg);
}


void proc_translatef(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 3)) return;

    float f0, f1, f2;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    if (!success) return;

    glTranslatef(f0, f1, f2);
}

void proc_lookAt(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 9)) return;

    float f0, f1, f2, f3, f4, f5, f6, f7, f8;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    success &= coerce_to_float(interp, record, 3, &f3);
    success &= coerce_to_float(interp, record, 4, &f4);
    success &= coerce_to_float(interp, record, 5, &f5);
    success &= coerce_to_float(interp, record, 6, &f6);
    success &= coerce_to_float(interp, record, 7, &f7);
    success &= coerce_to_float(interp, record, 8, &f8);
    if (!success) return;

    gluLookAt(f0, f1, f2, f3, f4, f5, f6, f7, f8);
}

void proc_perspective(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 4)) return;

    float f0, f1, f2, f3;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    success &= coerce_to_float(interp, record, 3, &f3);
    if (!success) return;

    gluPerspective(f0, f1, f2, f3);
}

void proc_ortho(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 6)) return;

    float f0, f1, f2, f3, f4, f5;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    success &= coerce_to_float(interp, record, 3, &f3);
    success &= coerce_to_float(interp, record, 4, &f4);
    success &= coerce_to_float(interp, record, 5, &f5);
    if (!success) return;

    glOrtho(f0, f1, f2, f3, f4, f5);
}

void proc_texCoord2f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 2)) return;

    float f0, f1;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    if (!success) return;

    glTexCoord2f(f0, f1);
}

void proc_vertex2f(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 2)) return;

    float f0, f1;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    if (!success) return;

    glVertex2f(f0, f1);
}

void proc_clearColor(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 4)) return;

    float f0, f1, f2, f3;
    bool success = true;
    success &= coerce_to_float(interp, record, 0, &f0);
    success &= coerce_to_float(interp, record, 1, &f1);
    success &= coerce_to_float(interp, record, 2, &f2);
    success &= coerce_to_float(interp, record, 3, &f3);
    if (!success) return;

    glClearColor(f0, f1, f2, f3);
}


void proc_makeCurrent(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;

    Database *owner = (Database *)record->registers[1]->read();
    if (!is_window_pointer(interp, owner)) {
        interp->report_error("Argument 0 must be a window!\n");
        return;
    }

    First_Class *fc = owner->lookup_named_slot(interp, glib->resources_atom);
    if (!fc) return;  // @Robustness: Report an error
    if (fc->type != ARG_BLOB) return;

    Private_Window_Blob *blob = (Private_Window_Blob *)fc;

    wglMakeCurrent(blob->dc, blob->glrc);
    glib->current_opengl_dc = blob->dc;
    glib->current_opengl_glrc = blob->glrc;
}

void proc_swapBuffers(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 0)) return;

    SwapBuffers(glib->current_opengl_dc);
}

/*

void update_error() {
    DWORD error = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error,
                  0, error_buf, buf_len, NULL);
}
*/


#define GlConstant(s) MEMBER(s, integer, GL_##s);

void register_opengl_primitives(Lerp_Interp *interp) {
    Schema *ns = make_schema(interp, 100);
    ns->type_name = interp->parser->make_atom("GL_Type");
    int schema_cursor = 0;

    register_window_primitives(interp);

    MEMBER_PROC(vertex2f);
    MEMBER_PROC(vertex3f);
    MEMBER_PROC(normal3f);
    MEMBER_PROC(color3f);
    MEMBER_PROC(color4f);
    MEMBER_PROC(texCoord2f);
    MEMBER_PROC(begin);
    MEMBER_PROC(end);
    MEMBER_PROC(clear);
    MEMBER_PROC(loadIdentity);
    MEMBER_PROC(matrixMode);
    MEMBER_PROC(enable);
    MEMBER_PROC(disable);
    MEMBER_PROC(clearColor);
    MEMBER_PROC(swapBuffers);
    MEMBER_PROC(bindTexture);
    MEMBER_PROC(blendFunc);

    MEMBER_PROC(makeCurrent);

    MEMBER_PROC(lookAt);
    MEMBER_PROC(translatef);
    MEMBER_PROC(perspective);
    MEMBER_PROC(ortho);
    MEMBER_PROC(depthFunc);
    MEMBER_PROC(shadeModel);
    MEMBER_PROC(colorMaterial);
    MEMBER_PROC(texEnvi);
    MEMBER_PROC(texParameteri);

    MEMBER_PROC(light3);
    MEMBER_PROC(light4);

    GlConstant(COLOR_BUFFER_BIT);
    GlConstant(DEPTH_BUFFER_BIT);
    GlConstant(TRIANGLES);
    GlConstant(TRIANGLE_STRIP);
    GlConstant(TRIANGLE_FAN);
    GlConstant(QUADS);

    GlConstant(MODELVIEW);
    GlConstant(PROJECTION);
    GlConstant(TEXTURE);

    GlConstant(BLEND);
    GlConstant(FRONT);
    GlConstant(BACK);
    GlConstant(FRONT_AND_BACK);
    GlConstant(ALWAYS);
    GlConstant(LINEAR);
    GlConstant(LIGHTING);
    GlConstant(CULL_FACE);
    GlConstant(TEXTURE_2D);
    GlConstant(AMBIENT);
    GlConstant(DIFFUSE);
    GlConstant(AMBIENT_AND_DIFFUSE);
    GlConstant(SRC_ALPHA);
    GlConstant(ONE_MINUS_SRC_ALPHA);
    GlConstant(NEAREST);
    GlConstant(TEXTURE_WRAP_S);
    GlConstant(TEXTURE_WRAP_T);
    GlConstant(TEXTURE_MIN_FILTER);
    GlConstant(TEXTURE_MAG_FILTER);

    GlConstant(GREATER);
    GlConstant(LESS);
    GlConstant(DEPTH_TEST);
    GlConstant(SMOOTH);
    GlConstant(COLOR_MATERIAL);
    GlConstant(TEXTURE_ENV);
    GlConstant(TEXTURE_ENV_MODE);
    GlConstant(MODULATE);
    GlConstant(REPEAT);
    GlConstant(CLAMP);

    GlConstant(LIGHT0);
    GlConstant(LIGHT1);
    GlConstant(LIGHT2);
    GlConstant(LIGHT3);
    GlConstant(LIGHT4);
    GlConstant(LIGHT5);
    GlConstant(LIGHT6);
    GlConstant(LIGHT7);

    GlConstant(POSITION);
    GlConstant(LIGHT_MODEL_AMBIENT);

    trim_schema(ns, schema_cursor);

    install_module(interp, ns, "GL");
}


