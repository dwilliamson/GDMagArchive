//move.h,  main header file 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <direct.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include "..\\include\\define.h"

typedef struct map_tile_type
  {
  //Visuals
  unsigned short tile_index;
  //Attributes affecting the tile
  unsigned short parameters;
  //A* related data
  unsigned short g,depth;
  unsigned short h,position;
  unsigned short id;
  unsigned short ai_parameters;
  struct map_tile_type *parent,*left,*right;
  } map_tile_type;

typedef struct
 {
 //Visuals
 short x,y;//X and y pixel positions of unit
 short xd,yd;//Destination pixel positions of unit
 short frame;//Current frame of unit
 short direction;//Current direction of unit
 short parameter;//Bit 0->unit selected
 //Paths
 unsigned short request_path;//Destination request
 unsigned short *path;
 unsigned short path_size,path_position;
 unsigned short *leader_path;
 unsigned short leader_path_size,leader_path_position;
 unsigned short final_goal;
 unsigned short original_goal;
 //Other
 short walk_count;//Frames to pass before we reach next tile
 short speed;//Pixels/Frame
 short delay;//How many frames are we delayed
 short index;//Index in unit_list
 } unit_type;



