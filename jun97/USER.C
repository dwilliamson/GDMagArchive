#include "..\\include\\move.h"
#define __GLOBAL__
#include "..\\include\\globals.h"
#include "..\\include\\prototyp.h"

void user_interface(long *x,long *y) {
//Do things related to user interface like moving units and scrolling the map
long xm,ym,move_screen=1;
if ((mouse_code & 1) && (selected_units!=2))
	{
	 get_select_unit_rectangle();
	 show_select_unit_rectangle();
	 move_screen=0;
	}
else
	{select_left_x=-1;
	 select_right_x=-1;
	 }
if (mouse_click==1)
	{if (selected_units==1)
		selected_units=2;
	 else
	 if (selected_units==2)
		move_selected_units((*x+mouse_x)/32,(*y+mouse_y)/32);
	}
if (mouse_code & 2)
	{if (selected_units)
		{clear_unit_selection();
		 move_screen=0;
		 }
	 }
if (move_screen)
	{
	 xm=*x;ym=*y;
	 if (mouse_x<32)
		{if (xm>32) xm-=32;}
	 else
	 if (mouse_x>SCREEN_X-64)
		if (xm+32<=(MAP_X*32-SCREEN_X))
				xm+=32;
	 if (mouse_y<32)
		{if (ym>32) ym-=32;}
	 else
	 if (mouse_y>SCREEN_Y-64)
		if (ym+32<=MAP_Y*32-SCREEN_Y)
			ym+=32;
	 *x=xm;*y=ym;
	 }
}
