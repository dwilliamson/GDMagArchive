#ifndef __GLOBAL__

//Windows and user interface
HWND mainwindow=0;//The main window handle
HANDLE hinstance;
char game_title[128];
long key_pressed=-1;
long game_active=0;
HPALETTE gamepal=NULL;//Windows version of the gamepal

//Palette
unsigned char *lights=NULL;
unsigned char **look_up_pal=NULL;

//Direct draw
LPDIRECTDRAW lpdd=NULL;
LPDIRECTDRAWSURFACE lpddsprimary=NULL;
LPDIRECTDRAWPALETTE lpddpal=NULL;

//Gfx
long SCREEN_X=640,SCREEN_Y=480;
unsigned char *videobuffer=NULL;
unsigned char *image_memory=NULL;

//Map

map_tile_type *map=NULL;
long MAP_X=64,MAP_Y=64;
long SHOW_DATA=0;

//Mouse
long mouse_x=0,mouse_y=0,mouse_code=0,mouse_click=0,mouse_double_click=0;
long select_left_x=-1,select_left_y=-1,select_right_x=-1,select_right_y=-1,selected_units=0;

//Unit
long direction_delta[8][2]={{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1}};
long number_of_units=0;
unit_type *unit_list=NULL;

//Ai
long search_busy=-1,search_busy_goals[2];
map_tile_type *search_busy_open[2];
long MAX_SEARCH_ITERATIONS=2048;

//tests
long PATH_METHOD=0,SEARCH_METHOD=0;
unsigned char *path_method_names[NUMBER_OF_PATH_METHODS]={
"Locked paths","Locked cutted paths","Greedy paths","Delayed locked cutted paths","Step based"};
unsigned char *search_method_names[NUMBER_OF_SEARCH_METHODS]={
"Regular A*","Optimized A*","Bidirectional optimized A*","All non distributed search methods","Distributed optimized A*","Distributed optimized bidirectional A*"};
long CLUTTER_OBSTACLES=30;

#else
//Windows and user interface
extern HWND mainwindow;//The main window handle
extern HANDLE hinstance;
extern char game_title[];
extern long key_pressed;
extern long game_active;
extern HPALETTE gamepal;//Windows version of the gamepal

//Palette
extern unsigned char *lights;
extern unsigned char **look_up_pal;

//Direct draw
extern LPDIRECTDRAW lpdd;
extern LPDIRECTDRAWSURFACE lpddsprimary;
extern LPDIRECTDRAWPALETTE lpddpal;

//Gfx
extern long SCREEN_X,SCREEN_Y;
extern unsigned char *videobuffer;
extern unsigned char *image_memory;

//Map

extern map_tile_type *map;
extern long MAP_X,MAP_Y;
extern long SHOW_DATA;

//Mouse
extern long mouse_x,mouse_y,mouse_code,mouse_click,mouse_double_click;
extern long select_left_x,select_left_y,select_right_x,select_right_y,selected_units;

//Unit
extern long direction_delta[][2];
extern long number_of_units;
extern unit_type *unit_list;

//Ai
extern long search_busy,search_busy_goals[];
extern map_tile_type *search_busy_open[];
extern long MAX_SEARCH_ITERATIONS;


//Tests
extern long PATH_METHOD,SEARCH_METHOD;
extern unsigned char *path_method_names[NUMBER_OF_PATH_METHODS];
extern unsigned char *search_method_names[NUMBER_OF_SEARCH_METHODS];
extern long CLUTTER_OBSTACLES;

#endif
