struct Scripting_System;

const int NUM_WORLDS = 3;

void world_init(int world_index);
void world_update(int world_index, double dt);
void world_draw(int world_index);
void world_handle_key_down(int world_index, int key);

extern Scripting_System *scripting_system;  // Global variable that we play with
