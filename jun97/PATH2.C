#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"


#define HEURISTIC(x,y,xd,yd) \
(5*((long)(sqrt((x-xd)*(x-xd)+(y-yd)*(y-yd)))))

extern long total_open[],total_iterations[],total_closed[],total_sort[];
extern long direction_cost[];

long current_search_id=0;


unsigned short *fast_path(unit_type *u,long xd,long yd,long *size,long search_mask) {
//This is a version of A* which uses the searchspace for storing its
//nodes. The big advantage is that one iteration takes O(1) instead of
//O(n).  For n iterations this then comes to O(n) instead of O(ný) which
//is decidedly faster.
//xd and yd are tile coordinates
static map_tile_type *open,*closed,*current,*previous,*best_node,*succ;
unsigned short *path;long goal_index,cx,cy,xx,yy,i,index,g,h,iterations=0;
if (search_busy==-1)
	{//This is a new search
	 current_search_id++;
	 goal_index=xd+yd*MAP_X;
	 //Create root node
	 open=&map[u->x/32+u->y/32*MAP_X];
	 open->parent=NULL;
	 open->g=0;
	 open->h=HEURISTIC(u->x/32,u->y/32,xd,yd);
	 open->depth=0;
	 open->id=current_search_id;
	 open->ai_parameters=ON_OPEN;
	 open->right=NULL;
	 open->left=NULL;
	 }
else
	{open=search_busy_open[0];
	 goal_index=search_busy_goals[0];
	 search_busy=-1;
	 xd=goal_index%MAP_X;yd=goal_index/MAP_X;
	 }
//Start the search.
while(1)
 {if (open==NULL)
	{
	 return NULL;//No solution
	 }
 total_iterations[OPTIMIZED_ASTAR_METHOD]++;
 if (SEARCH_METHOD==OPTIMIZED_ASTAR_DISTRIBUTED_METHOD)
	{if (total_iterations[OPTIMIZED_ASTAR_METHOD]>MAX_SEARCH_ITERATIONS)
		{//Ok, we still haven't reached the goal as the search is still busy
		 //Let's postpone further searching until the next frame
		 search_busy=u->index;
		 search_busy_open[0]=open;
		 search_busy_goals[0]=goal_index;
		 return NULL;
		 }
	 }
 //Take best node from open and place it on closed
 best_node=open;
 open=open->right;
 if (open!=NULL)
	 open->left=NULL;
 best_node->ai_parameters=ON_CLOSED;
 //Check if bestnode is the goal
 if (best_node->position==goal_index)
	{
	 //FOUND GOAL
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
 cx=best_node->position%MAP_X;
 cy=best_node->position/MAP_X;
 for(i=0;i<8;i++)
	{//Do not exceed maximum allowed iterations
	 iterations++;
	 //Check if succesor is valid
	 xx=cx+direction_delta[i][0];
	 yy=cy+direction_delta[i][1];
	 index=xx+yy*MAP_X;
	 if (search_mask & AVOID_OBSTACLES)
		if (is_tile_blocked(xx,yy))
			continue;//Skip
	 if (search_mask & AVOID_UNITS)
		if (is_tile_busy(xx,yy))
			continue;//Skip
	 //Create succesor
	 succ=&map[index];
	 g=best_node->g+direction_cost[i];
	 h=HEURISTIC(xx,yy,xd,yd);
	 //Has this node been generated during this search
	 //Due to the way we store these things these are all
	 //O(1) operations rather than O(n) operations so that
	 //overall time complexity is O(n) instead of O(ný)
	 //(Actually it used to be 2*(n(n+1)/2)=n(n+1)=ný+n=O(ný))
	 if (succ->id==current_search_id)
		{//Yes, it has been generated already
		 //Check if already on open
		 if (succ->ai_parameters & ON_OPEN)
			{
			//It is on open
			if (g<succ->g)
				{//Better path found to open
				 succ->parent=best_node;
				 succ->g=g;
				 succ->depth=best_node->depth+1;
				 //Remove it from queue
				 if (succ->left!=NULL)
					succ->left->right=succ->right;
				 else
					open=NULL;//It was the first node
				 if (succ->right!=NULL)
					succ->right->left=succ->left;
				 }
			else
				{//If it is on open but is g value is better
				 //than the current g value we do not
				 //generate the succesor
				 continue;
				 }
			}
		 else
		 if (succ->ai_parameters & ON_CLOSED)
			{
			//It is on closed. Let's throw it away since
			//we don't want to propagate
			continue;
			}
		 }
	 else
		 {//Succesor wasn't generated yet during this search
		  succ->id=current_search_id;
		  succ->g=g;
		  succ->parent=best_node;
		  succ->depth=best_node->depth+1;
		  succ->h=h;
		  succ->ai_parameters=ON_OPEN;
		  }
	 //Insert succesor in open
	 if (open==NULL)
		{open=succ;
		 open->right=NULL;
		 open->left=NULL;
		 }
	 else
		{current=open;
		 previous=NULL;
		 while(current!=NULL)
			{total_sort[OPTIMIZED_ASTAR_METHOD]++;
			 if (current->g+current->h<succ->g+succ->h)
				{previous=current;
				 current=current->right;
				 }
			  else
				break;
			 }
		 if (previous!=NULL)
			{succ->right=previous->right;
			 if (succ->right!=NULL)
				succ->right->left=succ;
			 previous->right=succ;
			 succ->left=previous;
			 }
		 else
			{succ->right=open;
			 succ->left=NULL;
			 open->left=succ;
			 open=succ;
			 }
		 }
	} //End for loop
 } //End while(1) loop
return NULL;
}


unsigned short *bidirectional_fast_path(unit_type *u,long xd,long yd,long *size,long search_mask) {
//Bidirectional version of the previous fast A* algorithm.  BA* is a good
//idea in real time strategy games because it decides quicker if there
//is a solution yes or no.  The classic problem associated with BA*
//is the fact that the two searches might miss eachother and in fact
//it turns out to be the case that on average BA* takes a few more
//iterations than A*(though sometimes it is indeed faster).  However,
//A* is a real sucker when there is no solution, and BA* is a lot better
//in that respect.
//xd and yd are tile coordinates
static map_tile_type *open[2],*current_open,*current,*previous,*best_node,*succ;
unsigned short *path;long goal_index[2],x,y,cx,cy,xx,yy,i,index,g,h,search,iterations=0;
char s[256];
x=u->x/32;y=u->y/32;
if (search_busy==-1)
	{current_search_id++;
	 goal_index[0]=xd+yd*MAP_X;
	 goal_index[1]=x+y*MAP_X;
	 //Create root nodes
	 open[0]=&map[goal_index[1]];
	 open[1]=&map[goal_index[0]];
	 open[0]->h=open[1]->h=HEURISTIC(x,y,xd,yd);
	 for(search=0;search<2;search++)
		{open[search]->parent=NULL;
		 open[search]->g=0;
		 open[search]->depth=0;
		 open[search]->id=current_search_id;
		 open[search]->ai_parameters=ON_OPEN | (1<<(SEARCH_ID_BIT+search));
		 open[search]->right=NULL;
		 open[search]->left=NULL;
		 }
	 }
else
	{
	 for(search=0;search<2;search++)
		{open[search]=search_busy_open[search];
		 goal_index[search]=search_busy_goals[search];
		 }
	 search_busy=-1;
	 xd=goal_index[0]%MAP_X;yd=goal_index[0]/MAP_X;
	 }
//Start the search.
while(1)
 {
 total_iterations[OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD]+=2;
 if (SEARCH_METHOD==OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD)
	{if (total_iterations[OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD]>MAX_SEARCH_ITERATIONS)
		{for(search=0;search<2;search++)
			{search_busy_open[search]=open[search];
			 search_busy_goals[search]=goal_index[search];
			 }
		 search_busy=u->index;
		 return NULL;
		 }
	}
 for(search=0;search<2;search++)
   {
   current_open=open[search];
   if (current_open==NULL)
	return NULL;//No solution
   //Take best node from current_open and place it on closed
   best_node=current_open;
   current_open=current_open->right;
   if (current_open!=NULL)
	 current_open->left=NULL;
   best_node->ai_parameters=ON_CLOSED | (1<<(SEARCH_ID_BIT+search));
   //Generate successors
   cx=best_node->position%MAP_X;
   cy=best_node->position/MAP_X;

   for(i=0;i<8;i++)
	{//Do not exceed maximum allowed iterations
	 iterations++;
	 //Check if succesor is valid
	 xx=cx+direction_delta[i][0];
	 yy=cy+direction_delta[i][1];
	 index=xx+yy*MAP_X;
	 if (search_mask & AVOID_OBSTACLES)
		if (is_tile_blocked(xx,yy))
			continue;//Skip
	 if (search_mask & AVOID_UNITS)
		if (is_tile_busy(xx,yy))
			continue;//Skip
	 //Create succesor
	 succ=&map[index];
	 g=best_node->g+direction_cost[i];
	 h=HEURISTIC(xx,yy,goal_index[search]%MAP_X,goal_index[search]/MAP_X);
	 //Has this node been generated during this search
	 //Due to the way we store these things these are all
	 //O(1) operations rather than O(n) operations so that
	 //overall time complexity is O(n) instead of O(ný)
	 //(Actually it used to be 2*(n(n+1)/2)=n(n+1)=ný+n=O(ný))
	 if (succ->id==current_search_id)
		{//Yes, it has been generated already
		 //Since this is a bidirectional search it could well
		 //be that it belongs to the other search in which case
		 //we've found our goal.
		 if (!(succ->ai_parameters & (1<<(SEARCH_ID_BIT+search))))
			{//It doesn't belong to this search so it has
			 //to belong to the other search. We found our
			 //solution !
			 *size=best_node->depth+4+succ->depth;
			 if ((path=malloc(sizeof(unsigned short)*(*size)))==NULL)
				return NULL;
			 //The first search is backwards, the second
			 //search is frontwards
			 //First do the foreward search
			 switch(search)
				{case 0:current=best_node;
					i=best_node->depth;
					break;
				 case 1:current=succ;
					i=succ->depth;
					break;
				 }
			 while(i>=0)
				 {//Opposite since A* caused a reversed path
				 path[i--]=current->position;
				 current=current->parent;
				 }
			 //Now do the backward search
			 switch(search)
				{case 0:current=succ;
					i=best_node->depth+1;
					break;
				 case 1:current=best_node;
					i=succ->depth+1;
					break;
				 }
			 while(current!=NULL)
				 {
				 path[i++]=current->position;
				 current=current->parent;
				 }
			 *size=i;
			 return path;
			 }
		 //Check if already on current_open
		 if (succ->ai_parameters & ON_OPEN)
			{
			//It is on current_open
			if (g<succ->g)
				{//Better path found to current_open
				 succ->parent=best_node;
				 succ->g=g;
				 succ->depth=best_node->depth+1;
				 //Remove it from queue
				 if (succ->left!=NULL)
					succ->left->right=succ->right;
				 else
					current_open=NULL;//It was the first node
				 if (succ->right!=NULL)
					succ->right->left=succ->left;
				 }
			else
				{//If it is on current_open but is g value is better
				 //than the current g value we do not
				 //generate the succesor
				 continue;
				 }
			}
		 else
		 if (succ->ai_parameters & ON_CLOSED)
			{
			//It is on closed. For now let's throw it away
			continue;
			}
		 }
	 else
		 {//Succesor wasn't generated yet during this search
		  succ->id=current_search_id;
		  succ->g=g;
		  succ->parent=best_node;
		  succ->depth=best_node->depth+1;
		  succ->h=h;
		  succ->ai_parameters=ON_OPEN | (1<<(SEARCH_ID_BIT+search));
		  }
	 //Insert succesor in current_open
	 if (current_open==NULL)
		{current_open=succ;
		 current_open->right=NULL;
		 current_open->left=NULL;
		 }
	 else
		{current=current_open;
		 previous=NULL;
		 while(current!=NULL)
			{total_sort[OPTIMIZED_BIDIRECTIONAL_ASTAR_METHOD]++;
			 if (current->g+current->h<succ->g+succ->h)
				{previous=current;
				 current=current->right;
				 }
			  else
				break;
			 }
		 if (previous!=NULL)
			{succ->right=previous->right;
			 if (succ->right!=NULL)
				succ->right->left=succ;
			 previous->right=succ;
			 succ->left=previous;
			 }
		 else
			{succ->right=current_open;
			 succ->left=NULL;
			 current_open->left=succ;
			 current_open=succ;
			 }
		 }
	} //End for loop
   open[search]=current_open;//Open might have changed
   }//End search loop
 } //End while(1) loop
return NULL;
}
