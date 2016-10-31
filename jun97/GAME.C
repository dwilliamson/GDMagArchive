//Main game loop

#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

extern long total_open[],total_iterations[],total_closed[],total_sort[];
extern long direction_cost[];
long direction_default[2][8]={{4,6,4,6,4,6,4,6},{5,5,5,5,5,5,5,5}};

long game_loop(void) {
static long init=0,x=0,y=0,i,SHOW_INFO=1;
POINT ptcursor;RECT r;
//If search method is distributed make sure counters are resetted
switch(SEARCH_METHOD)
	{case OPTIMIZED_ASTAR_DISTRIBUTED_METHOD:
	 case OPTIMIZED_BIDIRECTIONAL_ASTAR_DISTRIBUTED_METHOD:
	 for(i=0;i<NUMBER_OF_SEARCH_METHODS;i++)
		total_open[i]=total_iterations[i]=total_closed[i]=total_sort[i]=0;
	 break;
	 }
//Get mouse position
GetCursorPos(&ptcursor);
ScreenToClient(mainwindow,&ptcursor);
mouse_x=max(0,min(ptcursor.x,SCREEN_X-32));
mouse_y=max(0,min(ptcursor.y,SCREEN_Y-32));

//Display map and units on screen
show_map(x,y);
show_units(x,y);
user_interface(&x,&y);
if (SHOW_INFO)
	{printg(0,0,CL_WHITE,"%s",game_title);
	 printg(0,10,CL_WHITE,"Path method:%s Search method:%s Clutter:%d",path_method_names[PATH_METHOD],search_method_names[SEARCH_METHOD],CLUTTER_OBSTACLES);
	 for(i=0;i<SEARCH_ALL;i++)
		printg(0,20+i*10,CL_WHITE,"Method:%s It:%d Cl:%d Op:%d St:%d",search_method_names[i],total_iterations[i],total_closed[i],total_open[i],total_sort[i]);
	 printg(0,20+i*10,CL_WHITE,"Max iterations=%d",MAX_SEARCH_ITERATIONS);
	 }
show_mouse_cursor();
warp_page();
switch(key_pressed)
	{case 32://Set an obstacle
		 set_obstacle((x+mouse_x)/32,(y+mouse_y)/32);
		 break;
	 case -1://No key pressed
		  break;
	 case 'u'://Add unit
		 add_unit(x+mouse_x,y+mouse_y);
		 break;
	 case 'p'://Switch between path methods
		 switch_path_method((PATH_METHOD+1)%NUMBER_OF_PATH_METHODS);
		 break;
	 case 's'://Switch beween search methods
		 SEARCH_METHOD=(SEARCH_METHOD+1)%NUMBER_OF_SEARCH_METHODS;
		 search_busy=-1;
		 break;
	 case 27:key_pressed=-1;//We can process a new key now
		 //mouse_click=0;//if there was a click it's over now
		 exit_demonstration();
		 return 0;
	 case 'd':
		 //Show A* fields
		 SHOW_DATA=!SHOW_DATA;
		 break;
	 case 'i':
		//Show search info
		 SHOW_INFO=!SHOW_INFO;
		 break;
	 case 'c':for(i=0;i<NUMBER_OF_SEARCH_METHODS;i++)
			total_open[i]=total_iterations[i]=total_closed[i]=total_sort[i]=0;
		 break;
	 case '+':CLUTTER_OBSTACLES=min(CLUTTER_OBSTACLES+1,100);
		 if (!set_map(CLUTTER_OBSTACLES))
			{exit_demonstration();return 0;}
		 break;
	 case '-':CLUTTER_OBSTACLES=max(CLUTTER_OBSTACLES-1,1);
		 if (!set_map(CLUTTER_OBSTACLES))
			{exit_demonstration();return 0;}
		 break;
	 case '*':MAX_SEARCH_ITERATIONS+=128;
		 break;
	 case '/':MAX_SEARCH_ITERATIONS=max(MAX_SEARCH_ITERATIONS-128,2);
		 break;
	 case 't':for(i=0;i<8;i++)
			direction_cost[i]=direction_default[0][i];
		 break;
	 case 'T':for(i=0;i<8;i++)
			direction_cost[i]=direction_default[1][i];
		 break;
	 }
key_pressed=-1;//We can process a new key now
mouse_click=0;//if there was a click it's over now
return 1;
}





