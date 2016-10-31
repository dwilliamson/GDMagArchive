
// VOXELD32.CPP //////////////////////////////////////////////////////////////

// This is the Watcom 10.6 version, it has a a lot of extra code and
// defines to make up for the lack of functionality DirectX comes
// with. I sound like I work for Microsoft...scarry!

// INCLUDES ///////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ctype.h>
#include <iostream.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <i86.h>
#include <conio.h>
#include <ctype.h>
#include <io.h>
#include <time.h>
#include <bios.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>

// DEFINES ////////////////////////////////////////////////////////////////////

// defines for screen parameters
#define GRAPHICS_MODE13  0x13    // 320x200x256
#define TEXT_MODE        0x03    // 80x25 text mode

#define SCREEN_WIDTH     320     // the width of the viewing surface
#define SCREEN_HEIGHT    200     // the height of the viewing surface
#define SCREEN_BPP       8       // the bits per pixel
#define SCREEN_COLORS    256     // the maximum number of colors

#define PC_NOCOLLAPSE     0x04   // do not match color to system palette */
#define COLOR_REGISTER_WR 0x3C8  // set write index at this I/O
#define COLOR_DATA        0x3C9  // the R/W data is here

// defines for fixed point math
#define FIXP_SHIFT       12      // number of decimal places 20.12 	
#define FIXP_MUL         4096    // 2^12, used to convert reals

// defines for angles
#define PIE              ((double)3.14159265) // take a guess

#define ANGLE_360        (1920) // fov is 60 deg, so SCREEN_WIDTH * 360/60
#define ANGLE_180        (ANGLE_360/2)
#define ANGLE_90         (ANGLE_360/4)
#define ANGLE_60         (ANGLE_360/6)
#define ANGLE_45         (ANGLE_360/8) 
#define ANGLE_30         (ANGLE_360/12)
#define ANGLE_20         (ANGLE_360/18) 
#define ANGLE_15         (ANGLE_360/24)
#define ANGLE_10         (ANGLE_360/36)  
#define ANGLE_5          (ANGLE_360/72)
#define ANGLE_2          (ANGLE_360/180) 
#define ANGLE_1          (ANGLE_360/360) 
#define ANGLE_0          0

// defines for height field
#define HFIELD_WIDTH      512	// width of height field data map
#define HFIELD_HEIGHT     512   // height of height field data map  
#define HFIELD_BIT_SHIFT  9     // log base 2 of 512
#define TERRAIN_SCALE_X2  3     // scaling factor for terrain

#define VIEWPLANE_DISTANCE (SCREEN_WIDTH/64)

#define MAX_ALTITUDE     1000   // maximum and minimum altitudes
#define MIN_ALTITUDE     50
#define MAX_SPEED        32     // maximum speed of camera

#define START_X_POS      256    // starting viewpoint position
#define START_Y_POS      256 
#define START_Z_POS      700

#define START_PITCH      80     // starting angular heading
#define START_HEADING    ANGLE_90

#define MAX_STEPS        200    // number of steps to cast ray

// keyboard stuff
#define VK_ESCAPE        1
#define VK_LEFT          75
#define VK_RIGHT         77
#define VK_UP            72
#define VK_DOWN          80
#define VK_PRIOR         73
#define VK_NEXT          81
#define VK_SPACE         57
#define VK_A             30
#define VK_Z             44

// defines for the keyboard interface

#define KEYBOARD_INTERRUPT  0x09  // the keyboard interrupt number
#define KEY_BUFFER          0x60  // the port of the keyboard buffer
#define KEY_CONTROL         0x61  // the port of the keyboard controller
#define PIC_PORT            0x20  // the port of the programmable
                                  // interrupt controller (PIC)

// TYPES /////////////////////////////////////////////////////////////////////
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef unsigned int   LONG;

// holds a single color palette entry
typedef struct  tagPALETTEENTRY
    {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
    } PALETTEENTRY, *LPPALETTEENTRY;

// holds file header for bitmap .BMP
typedef struct tagBITMAPFILEHEADER
        {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
        } BITMAPFILEHEADER;

// holds all the information for a .BMP
typedef struct tagBITMAPINFOHEADER
        {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
        } BITMAPINFOHEADER;


// the bitmap file structure /////////////////////////////////////////////////
typedef struct bitmap_file_tag
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info is
        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data
        } bitmap_file, *bitmap_file_ptr;

// MACROS /////////////////////////////////////////////////////////////////////


// PROTOTYPES ////////////////////////////////////////////////////////////////

void Set_Graphics_Mode(int mode);

void _interrupt _far  Keyboard_Driver();

void Keyboard_Install_Driver(void);

void Keyboard_Remove_Driver(void);

int Set_Pal_Entry(int index, int red, int green, int blue);

int Load_Pal_RGB(LPPALETTEENTRY colors, int num_colors);

int Timer_Query(void);

int Load_Bitmap_File(bitmap_file_ptr bitmap, char *filename);

int Unload_Bitmap_File(bitmap_file_ptr bitmap);

void Build_Tables(void);

void Game_Init(void);

void Game_Main(void);

void Game_Shutdown(void);

int Open_Error(char *filename);

int Close_Error(void);

int Write_Error(char *string);

// GLOBALS ////////////////////////////////////////////////////////////////////

int   game_state        = 1;     // state of game, 0 stop, 1 run

char  **command_line;            // ptr to command line
int     command_parms;           // number of command parms

UCHAR *video_buffer     = (UCHAR *)0xA0000L,  // pointer to video ram

      *double_buffer    = NULL,  // pointer to double buffer
      *height_map_ptr   = NULL,  // pointer to height field height data
      *color_map_ptr    = NULL;  // pointer to height field color data

PALETTEENTRY color_palette[256]; // shadow of color palette

bitmap_file height_bmp_file,     // holds the height data
            color_bmp_file;      // holds the color data

int view_pos_x = START_X_POS,    // view point x pos
    view_pos_y = START_Y_POS,    // view point y pos
    view_pos_z = START_Z_POS,    // view point z pos (altitude)

    view_ang_x = START_PITCH,    // pitch
    view_ang_y = START_HEADING,  // heading, or yaw
    view_ang_z = 0,              // roll, unused
	
// this is very important, it is based on the fov and scaling
// factors, it is the delta slope between two rays piercing
// the viewplane that a single pixel from each other in the
// same colum
dslope = (int)(((double)1/(double)VIEWPLANE_DISTANCE)*FIXP_MUL),

cos_look[ANGLE_360],              // trig lookup tables, really only
sin_look[ANGLE_360];              // need onem but 2 is easier

FILE      *fp_error;              // file handle to error output

char      errbuffer[80];          // holds error strings

void (_interrupt _far *Old_Keyboard_ISR)();  // holds old keyboard interrupt handler

unsigned char key_bits;

int raw_scan_code=0;  // the global keyboard scan code

// this holds the keyboard state table which tracks the state of every key
// on the keyboard

int keyboard_state[128];

// this tracks the number of keys that are currently pressed, helps
// with keyboard testing logic

int keys_active = 0;


// INLINE FUNCTIONS ///////////////////////////////////////////////////////////

inline int COS_LOOK(int theta)
{
// this inline function returns the cosine of the sent angle taking into
// consideration angles > 360 and < than 0

if (theta < 0)
   return(cos_look[theta + ANGLE_360]);
else
if (theta >= ANGLE_360)
   return(cos_look[theta - ANGLE_360]);
else
   return(cos_look[theta]);

} // end COS_LOOK

///////////////////////////////////////////////////////////////////////////////

inline int SIN_LOOK(int theta)
{
// this inline function returns the sine of the sent angle taking into
// consideration angles > 360 and < than 0

if (theta < 0)
   return(sin_look[theta + ANGLE_360]);
else
if (theta >= ANGLE_360)
   return(sin_look[theta - ANGLE_360]);
else
   return(sin_look[theta]);

} // end SIN_LOOK

//////////////////////////////////////////////////////////////////////////////

int Timer_Query(void)
{
// this function is used to record the current time

int *clock = (int *)0x0000046CL; // address of timer

return((int)*clock);

} // end Timer_Query

//////////////////////////////////////////////////////////////////////////////

void _interrupt _far  Keyboard_Driver()
{
// this function is used as the new keyboard driver. once it is installed
// it will continually update a keyboard state table that has an entry for
// every key on the keyboard, when a key is down the appropriate entry will be
// set to 1, when released the entry will be reset. any key can be querried by
// accessing the keyboard_state[] table with the make code of the key as the
// index

// unsigned char key_bits;

// re-enable interrupts

_enable();

// get the key that was read

raw_scan_code = inp(KEY_BUFFER);

// get the keyboard control register bits

key_bits = inp(KEY_CONTROL) | 0x82;

// reset the keyboard

outp(KEY_CONTROL, key_bits);
outp(KEY_CONTROL, (key_bits & 0x7f));

// reset the programmable interrupt controller

outp(PIC_PORT, 0x20);


// update the keyboard table

// test if the scan code was a make code or a break code

if (raw_scan_code <128)
   {
   // index into table and set this key to the "on" state if it already isn't
   // on

   if (keyboard_state[raw_scan_code]==0)
      {
      // there is one more active key

      keys_active++;

      // update the state table

      keyboard_state[raw_scan_code] = 1;

      } // end if key wasn't already pressed

   } // end if a make code
else
   {
   // must be a break code, therefore turn the key "off"
   // note that 128 must be subtracted from the raw key to access the correct
   // element of the state table

   if (keyboard_state[raw_scan_code-128]==1)
      {
      // there is one less active key

      keys_active--;

      // update the state table

      keyboard_state[raw_scan_code-128] = 0;

      } // end if key wasn't already pressed

   } // end else

} // end Keyboard_Driver

///////////////////////////////////////////////////////////////////////////////

void Keyboard_Install_Driver(void)
{
// this function installs the new keyboard driver

int index; // used as loop variable

// clear out the keyboard state table

for (index=0; index<128; index++)
    keyboard_state[index]=0;

// save the old keyboard driver

Old_Keyboard_ISR = _dos_getvect(KEYBOARD_INTERRUPT);

// install the new keyboard driver

_dos_setvect(KEYBOARD_INTERRUPT, Keyboard_Driver);

} // end Keyboard_Install_Driver

///////////////////////////////////////////////////////////////////////////////

void Keyboard_Remove_Driver(void)
{

// this functions restores the old keyboard driver (DOS version) with the
// previously saved vector

_dos_setvect(KEYBOARD_INTERRUPT, Old_Keyboard_ISR);

} // end Keyboard_Remove_Driver

//////////////////////////////////////////////////////////////////////////////

void Set_Graphics_Mode(int mode)
{

// use the video interrupt 10h and the C interrupt function to set
// the video mode

union REGS inregs,outregs;

inregs.h.ah = 0;                    // set video mode sub-function
inregs.h.al = (unsigned char)mode;  // video mode to change to

int386(0x10, &inregs, &outregs);

} // end Set_Graphics_Mode

/////////////////////////////////////////////////////////////////////////////

int Load_Pal_RGB(LPPALETTEENTRY colors, int num_colors)
{
// this function loads the front buffer palette and the shadow palette with
// the sent array of RGB colors

for (int index=0; index<num_colors; index++)
    if (!Set_Pal_Entry(index,colors[index].peRed,
                             colors[index].peGreen,
                             colors[index].peBlue))
       return(0);

// return success
return(1);

} // end Load_Pal_RGB

//////////////////////////////////////////////////////////////////////////////

int Set_Pal_Entry(int index, int red, int green, int blue)
{
// this function sets a palette entry with the sent color, note that
// it would be better to send a PALETTEENTRY, but this is more generic
// you might want to overload this function?

PALETTEENTRY color;	// used to build up color

// set RGB value in structure
color.peRed    = (BYTE)red;
color.peGreen  = (BYTE)green;
color.peBlue   = (BYTE)blue;
color.peFlags  = PC_NOCOLLAPSE;

// set the color palette entry on card

// tell vga card which color register to update
outp(COLOR_REGISTER_WR, index);

// update the color register RGB triple, note the same port is used each time
// the hardware will make sure each of the components is stored in the
// proper location
outp(COLOR_DATA,(red >> 2));
outp(COLOR_DATA,(green >> 2));
outp(COLOR_DATA,(blue >> 2));

// make copy in shadow palette
memcpy(&color_palette[index],
       &color,
       sizeof(PALETTEENTRY));

// return success
return(1);

} // end Set_Pal_Entry

// GENERAL FUNCTIONS //////////////////////////////////////////////////////////

void Build_Tables(void)
{
// this function builds all the lookup tables

for (int curr_angle=0; curr_angle < ANGLE_360; curr_angle++)
    {
    double angle_rad = 2*PIE*(double)curr_angle/(double)ANGLE_360;

    // compute sin and cos and convert to fixed point
    cos_look[curr_angle] = (int)(cos(angle_rad) * FIXP_MUL);
    sin_look[curr_angle] = (int)(sin(angle_rad) * FIXP_MUL);

    } // end for index

} // end Build_Tables

///////////////////////////////////////////////////////////////////////////////

int Load_Bitmap_File(bitmap_file_ptr bitmap, char *filename)
{
// this function opens a bitmap file and loads the data into bitmap
// this function will only work with non-compressed 8 bit palettized images
// it uses file handles instead of streams just for a change, no reason

#define BITMAP_ID        0x4D42       // this is the universal id for a bitmap

int      file_handle,                 // the file handle
         index;                       // looping index

// open the file if it exists
if ((file_handle = open(filename,O_RDONLY | O_BINARY))==-1)
   return(0);
 
// now load the bitmap file header
read(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));

// test if this is a bitmap file
if (bitmap->bitmapfileheader.bfType!=BITMAP_ID)
   {
   // close the file
   close(file_handle);
 
   // return error
   return(0);

   } // end if

// we know this is a bitmap, so read in all the sections

// load the bitmap file header
read(file_handle, &bitmap->bitmapinfoheader,sizeof(BITMAPINFOHEADER));

// now the palette
read(file_handle, &bitmap->palette,256*sizeof(PALETTEENTRY));

// now set all the flags in the palette correctly and fix the reverse BGR
for (index=0; index<256; index++)
    {
    int temp_color = bitmap->palette[index].peRed;

    bitmap->palette[index].peRed   = bitmap->palette[index].peBlue;
    bitmap->palette[index].peBlue  = temp_color;
    bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
    } // end for index

// finally the image data itself
_lseek(file_handle,-(int)(bitmap->bitmapinfoheader.biSizeImage),SEEK_END);

// allocate the memory for the image
if (!(bitmap->buffer = new UCHAR [bitmap->bitmapinfoheader.biSizeImage]))
   {
   // close the file
   close(file_handle);

   // return error
   return(0);

   } // end if

// now read it in
read(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);

// bitmaps are usually upside down, so flip the image
int biWidth  = bitmap->bitmapinfoheader.biWidth,
    biHeight = bitmap->bitmapinfoheader.biHeight;

// allocate the temporary buffer
UCHAR *flip_buffer = new UCHAR[biWidth*biHeight];

// copy image to work area
memcpy(flip_buffer,bitmap->buffer,biWidth*biHeight);

// flip vertically
for (index=0; index<biHeight; index++)
    memcpy(&bitmap->buffer[((biHeight-1) - index)*biWidth],&flip_buffer[index * biWidth], biWidth);

// release the working memory
delete [] flip_buffer;

// close the file
close(file_handle);

// return success
return(1);

} // end Load_Bitmap_File

///////////////////////////////////////////////////////////////////////////////

int Unload_Bitmap_File(bitmap_file_ptr bitmap)
{
// this function releases all memory associated with "bitmap"
if (bitmap->buffer)
   {
   // release memory
   delete [] bitmap->buffer;

   // reset pointer
   bitmap->buffer = NULL;

   } // end if

// return success
return(1);

} // end Unload_Bitmap_File

///////////////////////////////////////////////////////////////////////////////

void Render_Terrain(int vp_x,     
                    int vp_y,
                    int vp_z,
                    int vp_ang_x,
                    int vp_ang_y,
                    int vp_ang_z,
                    UCHAR *dest_buffer)
{
// this function renders the terrain at the given position and orientation

int xr,                     // used to compute the point the ray intersects the
    yr,                     // the height data
    curr_column,            // current screen column being processed
    curr_step,              // current step ray is at
    raycast_ang,            // current angle of ray being cast
        dx,dy,dz,           // general deltas for ray to move from pt to pt
        curr_voxel_scale,   // current scaling factor to draw each voxel line
        column_height,      // height of the column intersected and being rendered
	curr_row,           // number of rows processed in current column
	x_ray,y_ray,z_ray,  // the position of the tip of the ray
	map_addr;           // temp var used to hold the addr of data bytes
		
UCHAR color,            // color of pixel being rendered
      *dest_column_ptr; // address screen pixel being rendered 
	  
// convert needed vars to fixed point
vp_x = (vp_x << FIXP_SHIFT);
vp_y = (vp_y << FIXP_SHIFT);
vp_z = (vp_z << FIXP_SHIFT);

// push down destination buffer to bottom of screen
dest_buffer += (SCREEN_WIDTH * (SCREEN_HEIGHT-1)); 

// compute starting angle
raycast_ang = vp_ang_y + ANGLE_30;

// cast a ray for each column of the screen
for (curr_column=0; curr_column < SCREEN_WIDTH-1; curr_column++)
	{
	// seed starting point for cast
	x_ray = vp_x;
	y_ray = vp_y;
	z_ray = vp_z;

        // compute deltas to project ray at
        // note the spherical cancelation factor
	dx = COS_LOOK(raycast_ang) << 1;
	dy = SIN_LOOK(raycast_ang) << 1;
	
	// dz is a bit complex, remember dz is the slope of the ray we are casting
	// therefore, we need to take into consideration the down angle, or
	// x axis angle, the more we are looking down the larger the intial dz
	// must be

	dz = dslope * (vp_ang_x - SCREEN_HEIGHT);
	
	// reset current voxel scale 
	curr_voxel_scale = 0;

	// reset row
	curr_row = 0;

	// get starting address of bottom of current video column 
	dest_column_ptr = dest_buffer;

	// enter into casting loop
	for (curr_step = 0; curr_step < MAX_STEPS; curr_step++)
            {
            // compute pixel in height map to process
            // note that the ray is converted back to an int
            // and it is clipped to to stay positive and in range
            xr = (x_ray  >> FIXP_SHIFT);
            yr = (y_ray  >> FIXP_SHIFT);

            xr = (xr & (HFIELD_WIDTH-1));
            yr = (yr & (HFIELD_HEIGHT-1));

            map_addr = (xr + (yr << HFIELD_BIT_SHIFT));

            // get current height in height map, note the conversion to fixed point
            // and the added multiplication factor used to scale the mountains
            column_height = (height_map_ptr[map_addr] << (FIXP_SHIFT+TERRAIN_SCALE_X2));

            // test if column height is greater than current voxel height for current step
            // from intial projection point
            if (column_height > z_ray)
               {
               // we know that we have intersected a voxel column, therefore we must
               // render it until we have drawn enough pixels on the display such that
               // thier projection would be correct for the height of this voxel column
               // or until we have reached the top of the screen

               // get the color for the voxel
               color = color_map_ptr[map_addr];

               // draw vertical column voxel
               while(1)
                    {
                    // draw a pixel
                    *dest_column_ptr = color;

                    // now we need to push the ray upward on z axis, so increment the slope
                    dz+=dslope;

                    // now translate the current z position of the ray by the current voxel
                    // scale per unit
                    z_ray+=curr_voxel_scale;

                    // move up one video line
                    dest_column_ptr-=SCREEN_WIDTH;

                    // test if we are done with column
                    if (++curr_row >= SCREEN_HEIGHT)
                       {
                       // force exit of outer steping loop
                       // chezzy, but better than GOTO!
                       curr_step = MAX_STEPS;
                       break;

                       } // end if

                    // test if we can break out of the loop
                    if (z_ray > column_height) break;

                    } // end while

               } // end if

            // update the position of the ray
            x_ray+=dx;
            y_ray+=dy;
            z_ray+=dz;

            // update the current voxel scale, remember each step out means the scale increases
            // by the delta scale
            curr_voxel_scale+=dslope;

            } // end for curr_step

	// advance video pointer to bottom of next column
	dest_buffer++;

	// advance to next angle
	raycast_ang--;	

	} // end for curr_col

} // end Render_Terrain

///////////////////////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
// save command line
command_line  = argv;
command_parms = argc;

// initialize game
Game_Init();

// enter main event loop
while(game_state)
     {
     // call main logic module
     Game_Main();
     } // end while

// shutdown game
Game_Shutdown();


} // end main

// HERE ARE OUR GAME CONSOLE FUNCTIONS ///////////////////////////////////////////////////////

void Game_Init(void)
{
// do any initialization here

char *height_file_name, // holds file names for data
     *color_file_name;

// open error system
Open_Error("errors.txt");

// set graphics mode to mode 13
Set_Graphics_Mode(GRAPHICS_MODE13);

// test if user sent data files
height_file_name = command_line[1];
color_file_name = command_line[2];

if (command_parms < 3)
   {
   // load in default height maps
   Load_Bitmap_File(&height_bmp_file,"heightd2.bmp");

   // load in default color map
   Load_Bitmap_File(&color_bmp_file,"heightc2.bmp");
   } // end if
else
   {
   Write_Error("\nloading bitmaps");
   // load in requested height map
   Load_Bitmap_File(&height_bmp_file,height_file_name);

   // load in requested color map
   Load_Bitmap_File(&color_bmp_file,color_file_name);
   } // end else

Write_Error("\nsetting palette");

// set palette to new colors
Load_Pal_RGB(color_bmp_file.palette, SCREEN_COLORS);

// alias access ptr
color_map_ptr = color_bmp_file.buffer;

// alias access ptr
height_map_ptr = height_bmp_file.buffer;


Write_Error("\nallocating buffer");
// allocate double buffer
double_buffer = new UCHAR[SCREEN_WIDTH * SCREEN_HEIGHT];

Write_Error("\nbuilding tabless");

// build the tables
Build_Tables();

Write_Error("\ninstalling keyboard");

// install the isr keyboard driver
Keyboard_Install_Driver();

} // end Game_Init

/////////////////////////////////////////////////////////////////////////////////////////////

void Game_Shutdown(void)
{
// cleanup and release all resources here

// release double buffer
delete [] double_buffer;

// unload bitmaps
Unload_Bitmap_File(&color_bmp_file);
Unload_Bitmap_File(&height_bmp_file);

// set mode vback to text
Set_Graphics_Mode(TEXT_MODE);

// restore the old keyboard driver
Keyboard_Remove_Driver();

// close down error system
Close_Error();

} // end Game_Shutdown

/////////////////////////////////////////////////////////////////////////////////////////////

void Game_Main(void)
{
// main control function like C/C++ MAIN()

// note the use of statics, since this function will be entered and exited
static int autopilot_on    = 1,   // state of autopilot
           autopilot_timer = 50,  // counts time til heading change
           autopilot_turn  = 0,   // heading direction
           speed           = 8;   // current speed of view camera

// the word "auto" encoded as bits, 16 left if you want to add!
static unsigned int autobmp[] = {0x65770000,0x75250000,0x57270000}; 

// record starting time of frame
int frame_start_time = Timer_Query();

// is user exiting?
if (keyboard_state[VK_ESCAPE])
   {
   game_state = 0;
   return;
   } // end if



// first clear double buffer
memset(double_buffer,0, SCREEN_WIDTH*SCREEN_HEIGHT);

// change heading, note these are the only controls that
// disengage autopilot
if (keyboard_state[VK_LEFT])
   {
   // rotate CC
   view_ang_y+=ANGLE_5;
   autopilot_on    = 0;
   autopilot_timer = 0;
   } // end if
else
if (keyboard_state[VK_RIGHT])
   {
   // rotate CW
   view_ang_y-=ANGLE_5;
   autopilot_on    = 0;
   autopilot_timer = 0;
   } // end if

// change altitude
if (keyboard_state[VK_A])
   view_pos_z+=8;
else
if (keyboard_state[VK_Z])
   view_pos_z-=8;

// change speed
if (keyboard_state[VK_UP])
   speed+=2;
else
if (keyboard_state[VK_DOWN])
   speed-=2;

// change pitch 
if (keyboard_state[VK_PRIOR])
   view_ang_x+=2;
else
if (keyboard_state[VK_NEXT])
   view_ang_x-=2;

// stop the camera totally
if (keyboard_state[VK_SPACE])
   {
   autopilot_on    = 0;
   autopilot_timer = 0;
   speed           = 0;
   } // end if

// process autopilot //////////////////////////////////////////////////////////
if (autopilot_on)
   {
   if (autopilot_timer==0 && (rand()%100) > 95)
      {
      autopilot_timer = 10+rand()%30;
      autopilot_turn  = -1 + 2*(rand()%2);
      speed           = 2+(rand()%28);

      if (autopilot_turn==0)
              autopilot_timer*=2;

      } // end if
   else
      {
      if (--autopilot_timer < 0)
         {
         // disengage autopilot
         autopilot_turn  = 0;
         autopilot_timer = 0;
         } // end if
      else
         {
         // do autopilot
         if ((rand()%10)==1)
            speed+=(-1+rand()%2);

         // turn player
         if (autopilot_turn==-1)
            view_ang_y+=ANGLE_2;
         else
         if (autopilot_turn==1)
            view_ang_y-=ANGLE_2;

         // follow terrain a bit, get height of whats coming up
         int terr_x = view_pos_x + ((24*COS_LOOK(view_ang_y)) >> FIXP_SHIFT),
             terr_y = view_pos_y + ((24*SIN_LOOK(view_ang_y)) >> FIXP_SHIFT);

         // clip values
         terr_x = (terr_x & (HFIELD_WIDTH-1));
         terr_y = (terr_y & (HFIELD_HEIGHT-1));

         // get height data
         int height = 64+(height_map_ptr[terr_x + (terr_y << HFIELD_BIT_SHIFT)] << TERRAIN_SCALE_X2);

         // test if we need to change altitude
         if (view_pos_z < height)
            {
            // climb
            view_pos_z+=(10*(1+(speed >> 2)));
            view_ang_x+=(2*(1+(speed >> 2)));
            } // end if
         else
         if (view_pos_z > (height+15))
            view_pos_z-=2; // fall

         // adjust pitch of trajectory
         if (view_ang_x > START_PITCH)
            view_ang_x--;

         } // end else

      } // end else

   // draw the word "auto" in a clever way:)
   UCHAR col = rand()%256;

   for (unsigned int count=0, scanbit=0x80000000; count < 32; count++)
       {
       if ((autobmp[0] & scanbit)) double_buffer[count]                  = col;
       if ((autobmp[1] & scanbit)) double_buffer[count+SCREEN_WIDTH]     = col;
       if ((autobmp[2] & scanbit)) double_buffer[count+(SCREEN_WIDTH*2)] = col;
       scanbit>>=1;
       } // end for

   } // end autopilot_on
else
   {
   // try and turn on autopilot
   if (++autopilot_timer > 1000)
      {
      autopilot_on    = 1;
      autopilot_timer = 0;
      speed           = 4+rand()%28;
      } // end if

   } // end if

//end autopilot////////////////////////////////////////////////////////////////

// move viewpoint
view_pos_x += ((speed*COS_LOOK(view_ang_y)) >> FIXP_SHIFT);
view_pos_y += ((speed*SIN_LOOK(view_ang_y)) >> FIXP_SHIFT);

// keep viewpoint in playfield
if (view_pos_x >=HFIELD_WIDTH) 
   view_pos_x = 0;
else
if (view_pos_x < 0)	
   view_pos_x = HFIELD_WIDTH-1;

if (view_pos_y >=HFIELD_HEIGHT)	
   view_pos_y = 0;
else
if (view_pos_y < 0)	
   view_pos_y = HFIELD_HEIGHT-1;

// test heading
if (view_ang_y >= ANGLE_360) 
   view_ang_y-=ANGLE_360;
else
if (view_ang_y <  ANGLE_0) 
   view_ang_y+=ANGLE_360;

// test speed
if (speed > MAX_SPEED) 
   speed = MAX_SPEED;
else
if (speed < -MAX_SPEED) 
   speed = -MAX_SPEED;

// test altitude
if ((view_pos_z+=8) > MAX_ALTITUDE) 
   view_pos_z = MAX_ALTITUDE;
else
if ((view_pos_z-=8) < MIN_ALTITUDE) 
   view_pos_z = MIN_ALTITUDE;

// draw the next frame of terrain to double buffer
Render_Terrain(view_pos_x, 
              view_pos_y,
              view_pos_z,
              view_ang_x,
              view_ang_y,
              view_ang_z,
              double_buffer);

// copy frame in double buffer to primary display buffer
memcpy(video_buffer, double_buffer,SCREEN_WIDTH*SCREEN_HEIGHT);

// lock to 30 fps, this is totally inaccurate!
while((Timer_Query() - frame_start_time) < 1);

} // end Game_Main

//////////////////////////////////////////////////////////////////////////////

int Open_Error(char *filename)
{
// this function opens the error file
if (strcmp(strupr(filename),"NULL.TXT")==0 || !filename)
   return(-1);

// else attempt to open the file
if ((fp_error=fopen(filename,"w"))!=NULL)
   {
   // write the open error message
   Write_Error("Error Message System Open.");
   return(1);
   } // end if
else
   return(-1);

} // end Open_Error

///////////////////////////////////////////////////////////////////////////////

int Close_Error(void)
{
// this function closes the error file if it is open
if (fp_error)
   {
   // send close message
   Write_Error("\nError Message System Closed.\n");

   // close the file and return success
   fclose(fp_error);
   return(1);

   } // end if
else
   return(0);

} // end Close_Error

///////////////////////////////////////////////////////////////////////////////

int Write_Error(char *string)
{
// this function writes a string to the error file
if (fp_error)
   {
   // write the string and return success
   fprintf(fp_error,"%s",string);
   return(1);

   } // end if
else
   return(0);

} // end Write_Error

////////////////////////////////////////////////////////////////////////////////
