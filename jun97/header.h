#define member(el,a,b) \
((a<b) ? (el>a && el<b) : (el>b && el<a))

#define memberc(el,a,b) \
((a<b) ? (el>=a && el<=b) : (el>=b && el<=a))

#define NR_OF_COLOR_TABLES 256


//Application specific position of tiles in images.000
#define GRAS_START 0
#define GRAS_END 15
#define MOUSE_CURSOR 36
#define UNIT_START 16
#define UNIT_DIR_ANIMATION_SIZE 4
#define OBSTACLE_START 37
#define OBSTACLE_END 43

#define CL_WHITE 243

#define U_SELECTED 1

#define NO_PATH_REQUESTED 0xffff

#define TILE_BUSY 1
#define TILE_DESTINATION 2
#define TILE_BLOCKED 4

#define set_tile(x,y,what) map[(x)+(y)*MAP_X].parameters|=(what)
#define set_tile_not(x,y,what) map[(x)+(y)*MAP_X].parameters&=0xffff ^ (what)
#define is_tile(x,y,what) (map[(x)+(y)*MAP_X].parameters & (what))

#define set_tile_busy(x,y) set_tile(x,y,TILE_BUSY)
#define set_tile_not_busy(x,y) set_tile_not(x,y,TILE_BUSY)
#define is_tile_busy(x,y) is_tile(x,y,TILE_BUSY)

#define set_tile_destination(x,y) set_tile(x,y,TILE_DESTINATION)
#define set_tile_not_destination(x,y) set_tile_not(x,y,TILE_DESTINATION)
#define is_tile_destination(x,y) is_tile(x,y,TILE_DESTINATION)

#define set_tile_blocked(x,y) set_tile(x,y,TILE_BLOCKED)
#define set_tile_not_blocked(x,y) set_tile_not(x,y,TILE_BLOCKED)
#define is_tile_blocked(x,y) is_tile(x,y,TILE_BLOCKED)
#define is_tile_clear(x,y) is_tile(x,y,TILE_BLOCKED | TILE_BUSY)


//Path methods
#define LOCKED_PATH_METHOD 0
#define LOCKED_CUTTED_PATH_METHOD 1
#define GREEDY_PATH_METHOD 2
#define DELAYED_LOCKED_CUTTED_PATH_METHOD 3
#define STEP_BASED_PATH_METHOD 4
#define NUMBER_OF_PATH_METHODS 5

//Search methods
#define REGULAR_ASTAR_METHOD 0
#define OPTIMIZED_ASTAR_METHOD 1
#define OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD 2
#define SEARCH_ALL 3
#define OPTIMIZED_ASTAR_DISTRIBUTED_METHOD 4
#define OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD 5
#define NUMBER_OF_SEARCH_METHODS 6


#define AVOID_UNITS 1
#define AVOID_OBSTACLES 2

//A* defines

#define ON_OPEN 1
#define ON_CLOSED 2
//Bit 2 and 3 are reserved to identify which search the node belongs
//to in bidirectional search (usage 1<<(SEARCH_ID_BIT+search) )
#define SEARCH_ID_BIT 2






