
// Entity is an item in the game world.

struct Entity {
    Entity();

    Vector3 position;
    Vector3 extents;
    float theta;

    int texture_handle;
};

const int MAX_ENTITIES = 1000;
extern int num_entities;
extern Entity *entities[MAX_ENTITIES];


void draw_entities();
void add_entity(Entity *);
