enum Render_State_Type {
    RS_SOLID,
    RS_FADING
};

void setup_mesh_render_opacity(float opacity, bool use_zfunc_lessequal);
void setup_mesh_render_state(Render_State_Type type, bool wireframe);
