struct Segregated_Terrain_Pieces {
    Auto_Array <Static_Terrain_Tree *> solid;
    Auto_Array <Static_Terrain_Tree *> fading;
};


void collect_visible_terrain(Static_Terrain_Tree *tree, 
                             Segregated_Terrain_Pieces *pieces);

const float VIEWING_DISTANCE = 1200.0f;

