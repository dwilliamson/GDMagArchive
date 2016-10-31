//Prototype.h, prototypes

//Interface to video.lib
extern void near clip_warp_image(long ,long ,long ,long ,unsigned char*);
#pragma aux clip_warp_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] \
		modify          [edi];

extern void near clip_copy_image(long ,long ,long ,long ,unsigned char*);
#pragma aux clip_copy_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] \
		modify          [edi];

extern void near clip_copy_player_image(long ,long ,long ,long ,unsigned char*,long );
#pragma aux clip_copy_player_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] [edi];

extern void near flip_clip_warp_image(long ,long ,long ,long ,unsigned char*);
#pragma aux flip_clip_warp_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] \
		modify          [edi];

extern void near flip_clip_copy_image(long ,long ,long ,long ,unsigned char*);
#pragma aux flip_clip_copy_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] \
		modify          [edi];

extern void near flip_clip_copy_player_image(long ,long ,long ,long ,unsigned char*,long );
#pragma aux flip_clip_copy_player_image "*_"   \
		parm caller     [ebx] [eax] [edx] [ecx] [esi] [edi];

//Interface to print.lib
void printg(long ,long ,long ,char *,...);
long init_active_font(char*);

//Interface to line.lib
void v_xc_line(long ,long ,long ,long ,long );
void v_rectangle(long ,long ,long ,long ,unsigned char );
