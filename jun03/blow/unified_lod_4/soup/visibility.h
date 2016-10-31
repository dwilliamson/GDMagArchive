struct Segregated_World_Pieces {
    Auto_Array <World_Block *> solid;
    Auto_Array <World_Block *> fading;
};


void collect_visible_world(World_Block *tree, 
                           Segregated_World_Pieces *pieces);

const float VIEWING_DISTANCE = 1200.0f;

