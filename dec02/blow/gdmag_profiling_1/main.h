// This program slows itself down artifically in order
// to provide somewhat reproducible profiling results
// across various speeds of host machine.  These COST_*
// variables are the amount of time, in seconds, to
// chew up when performing a particular task.
const float COST_STREAM_MUSIC = 0.006;
const float COST_AI_PATHFIND = 0.024;
const float COST_RENDER_ENTITY = 0.0005;
const float COST_RENDER_GROUND = 0.012;


// Stuff used for control and world size.
const float DEGREES_PER_MICKEY_X = 0.2f;
const float DEGREES_PER_MICKEY_Y = 0.2f;
const float PI = 3.141592653589;
const float MOVEMENT_SPEED = 20.0f;
const float GROUND_SIZE = 50.0f;


extern Vector3 view_pos;
extern Vector3 view_direction;
extern Vector3 light_direction;

extern float view_theta;
extern float view_phi;
extern float cos_viewcone_angle;

extern struct Loaded_Texture_Info ground_texture;

void busy_wait(float seconds);


