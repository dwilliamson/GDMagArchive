#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

#define MAX_MAP_MATRIX_SEARCH 16384
#define MAX_ITERATIONS (MAX_MAP_MATRIX_SEARCH-8)

#define HEURISTIC(x,y,xd,yd) \
(5*((long)(sqrt((x-xd)*(x-xd)+(y-yd)*(y-yd)))))

#define LOCKED_PATH_CUT_OFF_DEPTH 8

long total_open[NUMBER_OF_SEARCH_METHODS],total_iterations[NUMBER_OF_SEARCH_METHODS],total_closed[NUMBER_OF_SEARCH_METHODS],total_sort[NUMBER_OF_SEARCH_METHODS];


long direction_cost[8]={4,6,4,6,4,6,4,6};

typedef struct search_map_matrix_node
	{unsigned short g,position;
	 unsigned short h,depth;
	 struct search_map_matrix_node *parent,*right;
	 } search_map_matrix_node;

unsigned short *regular_path(unit_type *u,long xd,long yd,long *size,long search_mask) {
//xd and yd are tile coordinates
static search_map_matrix_node *open,*closed,*search_memory=NULL,*current,*previous,*best_node,*succ;
long p,gx,gy,block,iterations=0,search_memory_count=0,i,index,cx,cy,xx,yy,flag,goal_index;
unsigned short *path;
goal_index=xd+yd*MAP_X;
if (search_memory==NULL)
	search_memory=malloc(sizeof(search_map_matrix_node)*MAX_MAP_MATRIX_SEARCH);
search_memory_count=NULL;
closed=NULL;
open=&search_memory[search_memory_count++];
open->parent=NULL;
open->g=0;
open->depth=0;
open->position=u->x/32+u->y/32*MAP_X;
open->h=HEURISTIC(u->x/32,u->y/32,xd,yd);
open->right=NULL;
//Start the search.
while(1)
 {if (open==NULL)
	return NULL;//No solution
 //Take best node from open and place it on closed
 iterations+=8;
 if (iterations>=MAX_ITERATIONS)
	return NULL;
 best_node=open;
 open=open->right;
 previous=closed;closed=best_node;closed->right=previous;
 //Check if bestnode is the goal
 if (best_node->position==goal_index)
	{//Path found
	 current=best_node;
	 *size=best_node->depth+1;
	 if ((path=malloc(sizeof(unsigned short)*(best_node->depth+1)))==NULL)
		return NULL;
	 i=best_node->depth;
	 while(current!=NULL)
		{//Opposite since A* caused a reversed path
		 path[i--]=current->position;
		 current=current->parent;
		 }
	 return path;
	}
 //Generate successors
 cx=best_node->position%MAP_X;cy=best_node->position/MAP_X;
 total_iterations[REGULAR_ASTAR_METHOD]++;
 for(i=0;i<8;i++)
	{//Create succesor in search memory

	 xx=cx+direction_delta[i][0];
	 yy=cy+direction_delta[i][1];
	 index=xx+yy*MAP_X;
	 if (search_mask & AVOID_OBSTACLES)
		if (is_tile_blocked(xx,yy))
			continue;//Skip
	 if (search_mask & AVOID_UNITS)
		if (is_tile_busy(xx,yy))
			continue;//Skip
	 succ=&search_memory[search_memory_count++];
	 succ->g=best_node->g+direction_cost[i];
	 succ->parent=best_node;
	 succ->depth=best_node->depth+1;
	 succ->position=xx+yy*MAP_X;
	 succ->h=HEURISTIC(xx,yy,xd,yd);
	 //Check if succ is already on open
	 current=open;previous=NULL;
	 flag=0;
	 while(current!=NULL)
		{total_open[REGULAR_ASTAR_METHOD]++;
		 if (current->position==succ->position)
			{//Succ is on open.
			 if (succ->g<current->g)
				{current->parent=best_node;
				 current->g=succ->g;
				 current->depth=succ->depth;
				 //Remove current from open
				 if (previous!=NULL)
					previous->right=current->right;
				 else
				 //It's was the first node
					open=NULL;
				 //Throw away succesor
				 search_memory_count--;
				 succ=current;
				 flag=2;
				 break;
				 }
			 else
				{//Throw away successor
				search_memory_count--;
				flag=1;
				}
			 }
		 previous=current;
		 current=current->right;
		 }
	 if (!flag)
	 //Succ was not on open, was it on closed ?
		{current=closed;
		 while(current!=NULL)
			{total_closed[REGULAR_ASTAR_METHOD]++;
			 if (current->position==succ->position)
				{
				 //Throw away succesor
				 search_memory_count--;
				 flag=1;
				 break;
				 }
			current=current->right;
			}
		 }
	 else
		if (flag==2) flag=0;//If it was on open and better we want it to be reinserted
	 if (!flag)
	 //Succ was not on open and not on closed
		{//Insert succesor in open
		 if (open==NULL)
			{open=succ;
			 open->right=NULL;
			 }
		 else
			{current=open;
			 previous=NULL;
			 while(current!=NULL)
				{total_sort[REGULAR_ASTAR_METHOD]++;
				 if (current->g+current->h<succ->g+succ->h)
					{previous=current;
					 current=current->right;
					 }
				 else
					break;
				 }
			 if (previous!=NULL)
				{succ->right=previous->right;
				 previous->right=succ;
				 }
			 else
				{succ->right=open;
				 open=succ;
				}
			 }
		 }
	 skip_it:;
	} //End for loop
 } //End while(1) loop
return NULL;
}

long destination_adjust(long *xd,long *yd) {
long xxd=*xd,yyd=*yd,x,y,xx,yy,r,lx,rx,ly,ry,index;
if (!is_tile_clear(xxd,yyd))
	return 1;
for(r=1;r<8;r++)
		{lx=max(xxd-r,1);rx=min(MAP_X-1,xxd+r);
		 ly=max(yyd-r,1);ry=min(MAP_Y-1,yyd+r);
		 for(xx=lx;xx<=rx;xx++)
			for(yy=ly;yy<=ry;yy++)
				if (!is_tile_clear(xx,yy))
					{*xd=xx;
					 *yd=yy;
					 return 1;
					 }
		 }
return 0;
}


void occupy_path(unit_type *u) {
//Occupy the positions on the map we will be passing
long i=u->path_position;
for(;i<u->path_size;i++)
	set_tile_busy(u->path[i]%MAP_X,u->path[i]/MAP_X);
}

void clear_path(unit_type *u) {
//Free up the positions on the map in the path
long i=u->path_position;
for(;i<u->path_size;i++)
	set_tile_not_busy(u->path[i]%MAP_X,u->path[i]/MAP_X);
}


BOOL determine_path(unit_type *u) {
unsigned short *path;long size,x,y,xd,yd;
x=u->x/32;
y=u->y/32;
if (search_busy==-1)
	{xd=u->request_path%MAP_X;
	 yd=u->request_path/MAP_X;
	 //If there was a previous path, free it
	 if (u->path_size>0)
		{switch(PATH_METHOD)
			{case LOCKED_PATH_METHOD:
			 case DELAYED_LOCKED_CUTTED_PATH_METHOD:
			 case LOCKED_CUTTED_PATH_METHOD:
			 case GREEDY_PATH_METHOD:
				clear_path(u);
			 case STEP_BASED_PATH_METHOD:
				free(u->path);
				u->path_size=0;
			 break;
			 }
		 }
	 //Clear request
	 u->request_path=NO_PATH_REQUESTED;
	 if ((xd==x) && (yd==y))
		{u->original_goal=u->final_goal=x+y*MAP_X;
		 return FALSE;//Unit is already there
		 }
	 //Adjust the destination so that no two units have the same destination
	 if (!destination_adjust(&xd,&yd))
		{u->original_goal=u->final_goal=x+y*MAP_X;
		 return FALSE;
		 }
	 }
else
	{xd=search_busy_goals[0]%MAP_X;
	 yd=search_busy_goals[0]/MAP_X;
	 }
//Get the path
switch(PATH_METHOD)
	{case LOCKED_PATH_METHOD:
	 case DELAYED_LOCKED_CUTTED_PATH_METHOD:
	 case LOCKED_CUTTED_PATH_METHOD:
	 case GREEDY_PATH_METHOD:
		switch(SEARCH_METHOD)
			{case REGULAR_ASTAR_METHOD:
				path=regular_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;
			 case OPTIMIZED_ASTAR_METHOD:
			 case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
				path=fast_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD:
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
				path=bidirectional_fast_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;
			 case SEARCH_ALL:
				path=regular_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				free(path);
				path=fast_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
				free(path);
				path=bidirectional_fast_path(u,xd,yd,&size,AVOID_UNITS | AVOID_OBSTACLES);
			 break;

			 }
	 break;
	 case STEP_BASED_PATH_METHOD:
		switch(SEARCH_METHOD)
			{case REGULAR_ASTAR_METHOD:
				path=regular_path(u,xd,yd,&size,AVOID_OBSTACLES);
			 break;
			 case OPTIMIZED_ASTAR_METHOD:
			 case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
				path=fast_path(u,xd,yd,&size,AVOID_OBSTACLES);
			 break;
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD:
			 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
				path=bidirectional_fast_path(u,xd,yd,&size,AVOID_OBSTACLES);
			 case SEARCH_ALL:
				path=regular_path(u,xd,yd,&size,AVOID_OBSTACLES);
				free(path);
				path=fast_path(u,xd,yd,&size,AVOID_OBSTACLES);
				free(path);
				path=bidirectional_fast_path(u,xd,yd,&size,AVOID_OBSTACLES);
			 break;
			 }
	 break;
	 }
//If search is distributed return TRUE because we don't know yet if there
//is a path or not
if (search_busy!=-1)
	{u->final_goal=xd+yd*MAP_X;
	 return TRUE;
	 }
//If we didn't find a path, pretend the unit didn't hear well
if (path==NULL)
	{u->final_goal=x+y*MAP_X;
	 return FALSE;
	 }
//Put path in unit
u->path=path;
switch(PATH_METHOD)
	{case LOCKED_PATH_METHOD:
	 case STEP_BASED_PATH_METHOD:
		u->path_size=size;
		break;
	 case DELAYED_LOCKED_CUTTED_PATH_METHOD:
	 case LOCKED_CUTTED_PATH_METHOD:
		u->path_size=min(size,LOCKED_PATH_CUT_OFF_DEPTH);
		break;
	 case GREEDY_PATH_METHOD:
		u->path_size=2;
		break;
	 }
u->path_position=1;//Position 0 is start position
u->final_goal=xd+yd*MAP_X;
return TRUE;
}







