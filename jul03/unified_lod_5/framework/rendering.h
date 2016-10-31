void rendering_3d(Matrix4 *tr, Projector *pr);
void emit_mesh(Triangle_List_Mesh *mesh);


enum {
    EXTENSION_ARB_VERTEX_BUFFER_OBJECT = 0x1
};

int init_gl_extensions();
