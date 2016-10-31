//misc.c,  misc code

#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

extern long current_search_id;

BOOL init_gfx_mode(void) {
//Initialize screen buffer
if ((videobuffer=malloc(640L*480L))==NULL)
	return FALSE;
SCREEN_X=640;SCREEN_Y=480;
return TRUE;
}

void exit_demonstration(void) {
//Exit program
ShowCursor(TRUE);
ReleaseCapture();
DestroyWindow(mainwindow);
}

BOOL read_images(void) {
//Read in tiles from tile list
//The tile list is simply a raw chunk of 32X32 tiles
FILE *fi=fopen("static\\images.000","rb");
long size;
if (fi==NULL) return FALSE;
//Get amount of tiles
fseek(fi,0L,SEEK_END);
size=ftell(fi);
if (!size) return FALSE;
//Allocate memory and read tiles
if ((image_memory=malloc(size))==NULL)
	return FALSE;
fseek(fi,0L,SEEK_SET);
if (fread(image_memory,size,1,fi)!=1)
	return FALSE;
fclose(fi);
return TRUE;
}

BOOL set_map(long clutter) {
//Fill the map
long i=0;
if ((map=realloc(map,MAP_X*MAP_Y*sizeof(map_tile_type)))==NULL)
	return FALSE;
memset(map,0,MAP_X*MAP_Y*sizeof(map_tile_type));
for(;i<MAP_X*MAP_Y;i++)
	{map[i].tile_index=GRAS_START+rand()%(GRAS_END-GRAS_START);
	 map[i].position=i;
	 }
//Block borders so that we don't have to check in path searching
for(i=0;i<MAP_X;i++)
	{map[i].parameters|=TILE_BLOCKED;
	 map[i+(MAP_Y-1)*MAP_X].parameters|=TILE_BLOCKED;
	 }
for(i=0;i<MAP_Y;i++)
	{map[i*MAP_X].parameters|=TILE_BLOCKED;
	 map[i*MAP_X+MAP_X-1].parameters|=TILE_BLOCKED;
	 }
for(i=0;i<MAP_X*MAP_Y;i++)
	if (rand()%100<clutter)
		{map[i].tile_index=OBSTACLE_START+rand()%(OBSTACLE_END-OBSTACLE_START);
		 map[i].parameters|=TILE_BLOCKED;
		 }
return TRUE;
}

void set_obstacle(long x,long y) {
//Set a random obstacle at x,y
if ((member(x,0,MAP_X)) && (member(y,0,MAP_Y)))
	{map[x+y*MAP_X].parameters|=TILE_BLOCKED;
	 map[x+y*MAP_X].tile_index=OBSTACLE_START+rand()%(OBSTACLE_END-OBSTACLE_START);
	 }
}

void show_map(long x,long y) {
//Copy map to screenbuffer
//X and Y are pixel coordinates of upper left corner to show
long xt=x/32,yt=y/32,xx=xt*32-x,yy=yt*32-y,index,si,xs=xt,xxs=xx;
while(yy<SCREEN_Y)
	{si=yt*MAP_X+xt;
	 while(xx<SCREEN_X)
		{index=map[si].tile_index;
		 clip_warp_image(xx,yy,32,32,&image_memory[index*1024]);
		 if (SHOW_DATA)
			{if (is_tile_busy(xt,yt))
				printg(xx,yy,243,"B");
			 if (map[si].id==current_search_id)
				{if (map[si].ai_parameters & ON_OPEN)
					printg(xx,yy+10,16,"O");
				 else
					printg(xx,yy+10,211,"C");
				 }
			}
		 xx+=32;
		 xt++;
		 si++;
		 }
	 yt++;
	 yy+=32;
	 xx=xxs;
	 xt=xs;
	 }
}


void show_mouse_cursor(void) {
//Copy mouse cursor to screen buffer
clip_copy_image(mouse_x,mouse_y,32,32,&image_memory[MOUSE_CURSOR*1024]);
}

void show_select_unit_rectangle(void) {
if (select_left_x!=-1)
	{if (select_right_x==-1)
		 v_rectangle(select_left_x,select_left_y,mouse_x,mouse_y,CL_WHITE);
	 else
		 v_rectangle(select_left_x,select_left_y,select_right_x,select_right_y,CL_WHITE);
	 }
}

void get_select_unit_rectangle(void) {
//Get the selection rectangle
if (select_left_x==-1)
	{select_left_x=mouse_x;
	 select_left_y=mouse_y;
	 select_right_x=select_left_x+32;
	 select_right_y=select_left_y+32;
	 }
else
	{select_right_x=mouse_x;
	 select_right_y=mouse_y;
	 if (select_right_x<=select_left_x)
		select_right_x=select_left_x+32;
	 if (select_right_y<=select_left_y)
		select_right_y=select_left_y+32;
	 }
}





