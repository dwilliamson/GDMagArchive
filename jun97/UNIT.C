//unit.c,  misc code

#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

extern long total_open[],total_iterations[],total_closed[],total_sort[];

void add_unit(long x,long y) {
//Add a unit on pixel position x,y
unit_type *u;
unit_list=realloc(unit_list,sizeof(unit_type)*(number_of_units+1));
u=&unit_list[number_of_units];
u->x=u->xd=(x/32)*32;
u->y=u->yd=(y/32)*32;
u->original_goal=u->final_goal=(x/32)+(y/32)*MAP_X;
u->frame=0;
u->direction=rand()%8;
u->parameter=0;
u->walk_count=0;
u->speed=1<<rand()%5;
u->path_size=0;
u->leader_path_size=0;
u->request_path=NO_PATH_REQUESTED;
u->delay=0;
u->index=number_of_units++;
}

void clear_unit_selection(void) {
//Clear unit selection
long i=0;
for(i=0;i<number_of_units;i++)
	unit_list[i].parameter&=0xffff ^ U_SELECTED;
selected_units=0;
}

void move_selected_units(long x,long y) {
long i;
if (!number_of_units) return;
for(i=0;i<number_of_units;i++)
	if (unit_list[i].parameter & U_SELECTED)
		{
		switch(PATH_METHOD)
			{case DELAYED_LOCKED_CUTTED_PATH_METHOD:
				unit_list[i].delay=rand()%32;
			 default:
				unit_list[i].original_goal=unit_list[i].request_path=x+y*MAP_X;
			 }
		}


}



long get_unit_frame_tile_index(long direction,long frame) {
//Get tile that corresponds with unit frame walking in direction
//If return value is negative unit should be shown flipped
long index;
if (direction<5)
	index=UNIT_START+direction*UNIT_DIR_ANIMATION_SIZE+frame;
else
	index=-(UNIT_START+(8-direction)*UNIT_DIR_ANIMATION_SIZE+frame);
return index;
}

void move_to(unit_type *u,long xd,long yd) {
//Move unit to neighbouring tile at xd,yd
long dx=xd-u->x/32,dy=yd-u->y/32,i=0;
for(;i<8;i++)
	if ((direction_delta[i][0]==dx) && (direction_delta[i][1]==dy))
		break;
if (i==8) return;//Shouldn't happen
u->walk_count=32/u->speed;
u->xd=xd;
u->yd=yd;
u->direction=i;
u->frame=0;
}

long next_unit_frame(unit_type *u) {
//Advance frame, and move unit.  Return 0 if no more frames to go else 1
long i;
//Are we delayed ?
if (u->delay>0) return 1;
//Are we moving because of walk_count ?
if (u->walk_count>0)
	{//It is moving.
	 u->x+=u->speed*direction_delta[u->direction][0];
	 u->y+=u->speed*direction_delta[u->direction][1];
	 u->frame=(u->frame+1)%UNIT_DIR_ANIMATION_SIZE;
	 if (--u->walk_count>0)
		return 1;
	 }
return 0;
}

void check_for_collisions(unit_type *u) {
//Check if moving to the next position in the path will cause a unit
//collision
long xd,yd,size,xdd,ydd;
unsigned short *path;
xd=u->path[u->path_position]%MAP_X;
yd=u->path[u->path_position]/MAP_X;
if (is_tile_busy(xd,yd))
	{//Yep, red alert
	 //We can either wait, or move around it
	 xdd=u->original_goal%MAP_X;ydd=u->original_goal/MAP_X;
	 if (destination_adjust(&xdd,&ydd))
		{switch(SEARCH_METHOD)
			{case REGULAR_ASTAR_METHOD:
				path=regular_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;
			 case OPTIMIZED_ASTAR_METHOD:
			 case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
				path=fast_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				if (search_busy!=-1)
					{//If search still busy, kill path
					 //and let it be recalculated by
					 //determine_path in parse_unit()
					 free(u->path);
					 u->path_size=0;
					 u->path=NULL;
					 u->path_position=0;
					 return;
					 }
			 break;
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD:
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
				path=bidirectional_fast_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				if (search_busy!=-1)
					{//If search still busy, kill path
					 //and let it be recalculated by
					 //determine_path in parse_unit()
					 free(u->path);
					 u->path_size=0;
					 u->path=NULL;
					 u->path_position=0;
					 return;
					 }
			 break;
			 case SEARCH_ALL:
				path=regular_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				free(path);
				path=fast_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				free(path);
				path=bidirectional_fast_path(u,xdd,ydd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;
			 }
		 if (path!=NULL)
			{//We have a new path
			 free(u->path);
			 u->path_size=size;
			 u->path=path;
			 u->path_position=1;
			 u->final_goal=xdd+ydd*MAP_X;
			 }
		 else
			{//Don't try anymore
			 free(u->path);
			 u->path_size=0;
			 u->path=NULL;
			 u->path_position=0;
			 }
		 }
	 else
		u->delay=1+rand()%32;
	}
}


void unit_follow_path(unit_type *u) {
//Unit follows it path using one of the methods
//Some methods need evaluation during the path, others just need to follow
//their path as the path search already avoided collisions
switch(PATH_METHOD)
	{case STEP_BASED_PATH_METHOD:
		check_for_collisions(u);
		if (u->path_size==0)
		//If path has been killed, no need to follow
			return;
		if (u->delay>0)
		//Unit has to wait, don't move further in path
			return;
		//It is now certain that we are going there. Occupy tile
		set_tile_busy(u->path[u->path_position]%MAP_X,u->path[u->path_position]/MAP_X);
	 default:
	 //Move to next position in path
		move_to(u,u->path[u->path_position]%MAP_X,u->path[u->path_position]/MAP_X);
	}
if (++u->path_position>=u->path_size)
	{//What do we do when we are at the end of the path
	 switch(PATH_METHOD)
		{case DELAYED_LOCKED_CUTTED_PATH_METHOD:
		 case LOCKED_CUTTED_PATH_METHOD:
		 case GREEDY_PATH_METHOD:
		 //With some methods we need to recalculate path if we
		 //haven't arrived at the final goal yet
			if (u->final_goal!=u->path[u->path_position-1])
				u->request_path=u->original_goal;
		 default:u->path_size=0;
			 free(u->path);
			 break;
		 }
	 }
}

void parse_unit(unit_type *u) {
//Parse several unit fields
//Unremark the following lines to stop all units if search becomes
//distributed
/*switch(SEARCH_METHOD)
	{case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
	 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
	 if ((search_busy!=-1) && (search_busy!=u->index))
		//Search is busy. We do nothing until search has
		//been resolved
		 return;
	 break;
	 }*/
if (u->delay>0)
	{u->delay--;
	 set_tile_busy(u->x/32,u->y/32);
	 return;
	 }
if (u->walk_count>0)
	{//Advance
	 if (next_unit_frame(u))
		return;
	 }
//Remark here if first lines were unremarked
if ((u->request_path!=NO_PATH_REQUESTED) && (search_busy!=-1) && (search_busy!=u->index))
		{
		 //Lock next position
		 u->delay=1+rand()%8;
		 set_tile_busy(u->x/32,u->y/32);
		 return;
		 }
else
//End remark here
if ((u->request_path!=NO_PATH_REQUESTED) || (search_busy==u->index))
	{if (determine_path(u))
		{//We have a path
		 switch(SEARCH_METHOD)
			{case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
			 if (search_busy!=-1)
				{//Path is too long to be caluclated
				 //during this run.  We'll continue
				 //in the next frame.
				 set_tile_busy(u->x/32,u->y/32);
				 return;
				 }
			}
		 switch(PATH_METHOD)
			{case STEP_BASED_PATH_METHOD:
			 //For step based methods we leave the occupying
			 //to the routine which moves the unit because we
			 //are never sure if indeed a unit is going to
			 //move to the expected location
			 break;
			 default:
				occupy_path(u);
			 }
		 }
	}
if (u->path_size>0)
	{//Free current position
	 set_tile_not_busy(u->x/32,u->y/32);
	 unit_follow_path(u);
	 return;
	 }
//Lock next position
set_tile_busy(u->x/32,u->y/32);
}

void show_units(long x,long y) {
//Copy units to screenbuffer
//X and Y are pixel coordinates of upper left corner to show
long i,rx,ry,tile_index;unit_type *u;
for(i=0;i<number_of_units;i++)
	{u=&unit_list[i];
	 //Parse unit commands
	 parse_unit(u);
	 rx=u->x-x;ry=u->y-y;
	 if ((member(rx,-32,SCREEN_X)) && (member(ry,-32,SCREEN_Y)))
		{//Get index of tile
		 tile_index=get_unit_frame_tile_index(u->direction,u->frame);
		 //Copy unit to screen
		 if (tile_index<0)
			flip_clip_copy_player_image(rx,ry,32,32,&image_memory[(-tile_index)*1024],4);
		 else
			clip_copy_player_image(rx,ry,32,32,&image_memory[tile_index*1024],4);
		 //Check if unit is selected
		 if (select_left_x!=-1)
			{if (((member(select_left_x,rx,rx+32))
			 || (member(rx,select_left_x,select_right_x)))
			 &&
			 ((member(select_left_y,ry,ry+32))
			 || (member(ry,select_left_y,select_right_y))))
				{//Select unit
				 u->parameter|=U_SELECTED;
				 selected_units=1;
				 }
			 else
				//Deselect unit
				u->parameter&=0xffff^U_SELECTED;
			 }
		 if (u->parameter & U_SELECTED)
			v_rectangle(rx,ry,rx+32,ry+32,CL_WHITE);
		 }
	 }
}


void switch_path_method(long new_method) {
//Switch between path methods.  To do this all paths have to be stopped etc...
long i;unit_type *u;
switch(PATH_METHOD)
	{case STEP_BASED_PATH_METHOD:
	 case DELAYED_LOCKED_CUTTED_PATH_METHOD:
	 case LOCKED_PATH_METHOD:
	 case LOCKED_CUTTED_PATH_METHOD:
	 case GREEDY_PATH_METHOD:
		for(i=0;i<number_of_units;i++)
			{u=&unit_list[i];
			 if (u->path_size>0)
				{free(u->path);
				 u->path_size=0;
				 }
			 u->request_path=NO_PATH_REQUESTED;
			 u->walk_count=0;
			 u->delay=0;
			 u->x=(u->x/32)*32;
			 u->y=(u->y/32)*32;
			 u->original_goal=u->final_goal=u->x/32+(u->y/32)*MAP_X;
			 }
		break;
	 }
for(i=0;i<MAP_X*MAP_Y;map[i++].parameters&=0xffff^TILE_BUSY);
PATH_METHOD=new_method;
search_busy=-1;
}







