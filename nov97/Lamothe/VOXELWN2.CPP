// VOXELWN2.CPP /////////////////////////////////////////////////////////////////

// Windows 95/DirectX version with 640x480 double buffering

// INCLUDES ///////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // make sure certain headers are included correctly

#include <windows.h>         // include windows stuff
#include <windowsx.h>        
#include <mmsystem.h>        

#include <iostream.h>        // include C/C++ stuff
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <math.h> 
#include <malloc.h>
#include <string.h>
#include <io.h>
#include <fcntl.h> 
#include <signal.h>
#include <float.h>

#include <ddraw.h>          // include DirectDraw stuff

// DEFINES ////////////////////////////////////////////////////////////////////

#define WINDOW_CLASS_NAME "WINDOW_CLASS" // this is the name of the window class

// defines for screen parameters
// try 320x200, 320x240, they are way faster
#define SCREEN_WIDTH     640     // the width of the viewing surface
#define SCREEN_HEIGHT    480     // the height of the viewing surface
#define SCREEN_BPP       8       // the bits per pixel
#define SCREEN_COLORS    256     // the maximum number of colors

// defines for fixed point math
#define FIXP_SHIFT       12      // number of decimal places 20.12 	
#define FIXP_MUL         4096    // 2^12, used to convert reals
 
// defines for angles
#define PIE              ((double)3.14159265) // take a guess

#define ANGLE_360        (SCREEN_WIDTH * 360/60) // fov is 60 deg, so SCREEN_WIDTH * 360/60
#define ANGLE_180        (ANGLE_360/2)
#define ANGLE_120        (ANGLE_360/3)
#define ANGLE_90		 (ANGLE_360/4)
#define ANGLE_60         (ANGLE_360/6)
#define ANGLE_45         (ANGLE_360/8) 
#define ANGLE_30		 (ANGLE_360/12)
#define ANGLE_20         (ANGLE_360/18) 
#define ANGLE_15         (ANGLE_360/24)
#define ANGLE_10         (ANGLE_360/36)  
#define ANGLE_5          (ANGLE_360/72)
#define ANGLE_2          (ANGLE_360/180) 
#define ANGLE_1          (ANGLE_360/360) 
#define ANGLE_0          0
#define ANGLE_HALF_HFOV  ANGLE_30


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

#define START_PITCH      80*(SCREEN_HEIGHT/240)    // starting angular heading
#define START_HEADING    ANGLE_90

#define MAX_STEPS        200    // number of steps to cast ray

// TYPES /////////////////////////////////////////////////////////////////////
typedef unsigned char  UCHAR;

// the bitmap file structure /////////////////////////////////////////////////
typedef struct bitmap_file_tag
        {
        BITMAPFILEHEADER bitmapfileheader;  // this contains the bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info is
        PALETTEENTRY     palette[256];      // we will store the palette here
        UCHAR            *buffer;           // this is a pointer to the data
        } bitmap_file, *bitmap_file_ptr;

// MACROS /////////////////////////////////////////////////////////////////////

// these query the keyboard in real-time, WIN API stuff
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// PROTOTYPES ////////////////////////////////////////////////////////////////

int DD_Init(HWND hwnd);
int DD_Shutdown(void);
int DD_Set_Pal_Entry(int index, int red, int green, int blue);
int DD_Load_Pal_RGB(LPPALETTEENTRY colors, int num_colors);
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

LPDIRECTDRAW         lpdd         = NULL;		// dd object
LPDIRECTDRAWSURFACE  lpddsprimary = NULL;		// dd primary surface
LPDIRECTDRAWPALETTE  lpddpal      = NULL;		// a pointer to the created dd palette
PALETTEENTRY         color_palette[256];		// holds the shadow palette entries
DDSURFACEDESC        ddsd;						// a direct draw surface description struct
DDSCAPS              ddscaps;					// a direct draw surface capabilities struct
HRESULT              ddrval;					// result back from dd calls
HWND                 main_window_handle = NULL; // used to store the window handle
char                 *command_line;             // ptr to winmain command line
UCHAR                *video_buffer      = NULL, // pointer to video ram
                     *double_buffer     = NULL, // pointer to double buffer
					 *height_map_ptr   = NULL,  // pointer to height field height data
					 *color_map_ptr    = NULL;  // pointer to height field color data

bitmap_file          height_bmp_file,           // holds the height data
                     color_bmp_file;            // holds the color data

int view_pos_x = START_X_POS,                   // view point x pos
    view_pos_y = START_Y_POS,                   // view point y pos
	view_pos_z = START_Z_POS,                   // view point z pos (altitude)

	view_ang_x = START_PITCH,                   // pitch 
	view_ang_y = START_HEADING,                 // heading, or yaw
	view_ang_z = 0,                             // roll, unused
	
	// this is very important, it is based on the fov and scaling
	// factors, it is the delta slope between two rays piercing
	// the viewplane that a single pixel from each other in the
	// same colum
	dslope = (int)(((double)1/(double)VIEWPLANE_DISTANCE)*FIXP_MUL), 

	cos_look[ANGLE_360],                        // trig lookup tables, really only
	sin_look[ANGLE_360];                        // need onem but 2 is easier

int       error_file_handle = -1;    // this is the file handle for errors
OFSTRUCT  error_file_data;           // this holds the file data for the
char      errbuffer[80];             // holds error strings

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

// DIRECT X FUNCTIONS /////////////////////////////////////////////////////////

int DD_Init(HWND hwnd)
{
// this function is responsible for initializing direct draw, setting 
// cooperation level, setting display mode, creating a primary 
// surface and attaching a pallete

// create the directdraw com object
if ((ddrval=DirectDrawCreate(NULL,&lpdd,NULL))!=DD_OK)
   {
   // shutdown any other dd objects and kill window
   DD_Shutdown();
   sprintf(errbuffer,"\nError:DirectDrawCreate(...) - Code (%d)",ddrval);
   Write_Error(errbuffer);
   return(0);
   } // end if

// now set the coop level to exclusive and set for full screen and mode x
if ((ddrval=lpdd->SetCooperativeLevel(hwnd, DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE |
                              DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX))!=DD_OK)
   {
   // shutdown any other dd objects and kill window
   DD_Shutdown();
   sprintf(errbuffer,"\nError:SetCooperativeLevel(...) - Code (%d)",ddrval);
   Write_Error(errbuffer);
   return(0);
   } // end if

// now set the display mode
if ((ddrval=lpdd->SetDisplayMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP))!=DD_OK)
   {
   // shutdown any other dd objects and kill window
   DD_Shutdown();
   sprintf(errbuffer,"\nError:SetDisplayMode(...) - Code (%d)",ddrval);
   Write_Error(errbuffer);
   return(0);
   } // end if

// Create the primary surface
ddsd.dwSize            = sizeof(ddsd);
ddsd.dwFlags           = DDSD_CAPS;
ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE;

if ((ddrval=lpdd->CreateSurface(&ddsd,&lpddsprimary,NULL))!=DD_OK)
   {
   // shutdown any other dd objects and kill window
   DD_Shutdown();
   sprintf(errbuffer,"\nError:CreateSurface(...) - Code (%d)",ddrval);
   Write_Error(errbuffer);
   return(0);
   } // end if

// create a palette full of black and attach it to the primary surface
for (int index=0; index<256; index++)
	{
	// set rgb to 0
	color_palette[index].peRed   = 0;
	color_palette[index].peGreen = 0;
	color_palette[index].peBlue  = 0;
	
	// set the no collapse flag very important
	// otherwise, I would have just done a memset!
	color_palette[index].peFlags = PC_NOCOLLAPSE;
	
	} // end for index

// now create the palette object, note that it is a member of the dd object itself
if ((ddrval=lpdd->CreatePalette((DDPCAPS_8BIT | DDPCAPS_INITIALIZE),
	color_palette,&lpddpal,NULL))!=DD_OK)
	{
	// shutdown any other dd objects and kill window
	DD_Shutdown();
	sprintf(errbuffer,"\nError: - CreatePalette(...) - Code (%d)",ddrval);
    Write_Error(errbuffer);
	return(0);
	} // end if

// now attach the palette to the primary surface
if ((ddrval=lpddsprimary->SetPalette(lpddpal))!=DD_OK)
   {
   // shutdown any other dd objects and kill window
   DD_Shutdown();
   sprintf(errbuffer,"\nError: SetPalette(...) - Code (%d)",ddrval);
   Write_Error(errbuffer);
   return(0);
   } // end if

// return success if we got this far
return(1);

} // end DD_Init

///////////////////////////////////////////////////////////////////////////////

int DD_Shutdown(void)
{
// this function tests for dd components that have been created and releases
// them back to the operating system

// test if the dd object exists
if (lpdd)
   {
   // test if there is a primary surface
   if (lpddsprimary)
      {
      // release the memory and set pointer to NULL
      lpddsprimary->Release();
      lpddsprimary = NULL;
      } // end if

   // now release the dd object itself
   lpdd->Release();
   lpdd = NULL;

   // return success
   return(1);

   } // end if
else
   return(0);

} // end DD_Shutdown

//////////////////////////////////////////////////////////////////////////////

int DD_Load_Pal_RGB(LPPALETTEENTRY colors, int num_colors)
{
// this function loads the front buffer palette and the shadow palette with
// the sent array of RGB colors

for (int index=0; index<num_colors; index++)
    if (!DD_Set_Pal_Entry(index,colors[index].peRed,colors[index].peGreen,colors[index].peBlue))
       return(0);

// return success
return(1);

} // end DD_Load_Pal_RGB

//////////////////////////////////////////////////////////////////////////////

int DD_Set_Pal_Entry(int index, int red, int green, int blue)
{
// this function sets a palette entry with the sent color, note that
// it would be better to send a PALETTEENTRY, but this is more generic
// you might want to overload this function?

PALETTEENTRY color;	// used to build up color

// set RGB value in structure
color.peRed		= (BYTE)red;
color.peGreen	= (BYTE)green;
color.peBlue	= (BYTE)blue;
color.peFlags   = PC_NOCOLLAPSE;

// set the color palette entry
lpddpal->SetEntries(0,index,1,&color);

// make copy in shadow palette
memcpy(&color_palette[index],
       &color,
       sizeof(PALETTEENTRY));

// return success
return(1);

} // end DD_Set_Pal_Entry

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

OFSTRUCT file_data;                   // the file data information

// open the file if it exists
if ((file_handle = OpenFile(filename,&file_data,OF_READ))==-1)
   return(0);
 
// now load the bitmap file header
_lread(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));

// test if this is a bitmap file
if (bitmap->bitmapfileheader.bfType!=BITMAP_ID)
   {
   // close the file
   _lclose(file_handle);
 
   // return error
   return(0);

   } // end if

// we know this is a bitmap, so read in all the sections

// load the bitmap file header
_lread(file_handle, &bitmap->bitmapinfoheader,sizeof(BITMAPINFOHEADER));

// now the palette
_lread(file_handle, &bitmap->palette,256*sizeof(PALETTEENTRY));

// now set all the flags in the palette correctly and fix the reverse BGR
for (index=0; index<256; index++)
    {
    int temp_color = bitmap->palette[index].peRed;
    bitmap->palette[index].peRed  = bitmap->palette[index].peBlue;
    bitmap->palette[index].peBlue = temp_color;
	bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
    } // end for index

// finally the image data itself
_lseek(file_handle,-(int)(bitmap->bitmapinfoheader.biSizeImage),SEEK_END);

// allocate the memory for the image
if (!(bitmap->buffer = new UCHAR [bitmap->bitmapinfoheader.biSizeImage]))
   {
   // close the file
   _lclose(file_handle);

   // return error
   return(0);

   } // end if

// now read it in
_lread(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);

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
_lclose(file_handle);

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

int xr,                 // used to compute the point the ray intersects the
    yr,                 // the height data
    curr_column,        // current screen column being processed
    curr_step,          // current step ray is at
    raycast_ang,        // current angle of ray being cast
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

// compute starting angle, at current angle plus half field of view
raycast_ang = vp_ang_y + ANGLE_HALF_HFOV;

// cast a ray for each column of the screen
for (curr_column=0; curr_column < SCREEN_WIDTH-1; curr_column++)
	{
	// seed starting point for cast
	x_ray = vp_x;
	y_ray = vp_y;
	z_ray = vp_z;

	// compute deltas to project ray at, note the spherical cancelation factor
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

// WINDOWS CALLBACK FUNCTION //////////////////////////////////////////////////		             

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
// this is the main message handler of the system

HDC			hdc;		// handle to graphics context
PAINTSTRUCT ps;			// used to hold the paint info

// what is the message?
switch(msg)
	{	
	case WM_CREATE:
		{
		// do windows inits here
		return(0);
		} break;

	case WM_PAINT:
		{
		// this message occurs when your window needs repainting
		hdc = BeginPaint(hwnd,&ps);	 
		EndPaint(hdc,&ps);
		
		return(0);
   		} break;

	case WM_DESTROY:
		{
		// this message is sent when your window is destroyed
		PostQuitMessage(0);
		return(0);
		} break;

	default:break;

    } // end switch

// let windows process any messages that we didn't take care of 
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////////////////////////

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{
WNDCLASS		winclass;	// this holds the windows class info
HWND			hwnd;		// this holds the handle of our new window
MSG				msg;		// this holds a generic message

// first fill in the window class stucture
winclass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
winclass.hbrBackground	= GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL;
winclass.lpszClassName	= WINDOW_CLASS_NAME;

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(0,
							WINDOW_CLASS_NAME,				// class
							"Voxel Demo!",			        // title
							WS_VISIBLE | WS_OVERLAPPED,     // flags
					 		0,0,							// x,y
							SCREEN_WIDTH,					// size of window
							SCREEN_HEIGHT,				
							NULL,							// parent
							NULL,							// menu
							hinstance,						// instance
							NULL)))							// creation parms
return(0);

// hide the mouse cursor
ShowCursor(0);

// save the window handle and command line
main_window_handle = hwnd;
command_line = (char *)lpcmdline;

// initialize direct draw
if (!DD_Init(hwnd))
	{
	DestroyWindow(hwnd);
	Close_Error();
	return(0);
	} // end if

// initialize game
Game_Init();

// enter main event loop
while(1)
	{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{ 
		// test if this is a quit
        if (msg.message == WM_QUIT)
           break;
	
		// translate any accelerator keys
		TranslateMessage(&msg);

		// send the message to the window proc
		DispatchMessage(&msg);
		} // end if
	else
		{
		// do asynchronous processing here

		// acquire dd pointer to video ram, note it is always linear
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		lpddsprimary->Lock(NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR,NULL);
		video_buffer = (UCHAR *)ddsd.lpSurface;

		// call main logic module with valid video_buffer
		Game_Main();

		// release dd pointer to video ram
		lpddsprimary->Unlock(ddsd.lpSurface);
		
		} // end else

	} // end while

// shut down direct draw
DD_Shutdown();

// shutdown game
Game_Shutdown();

// return to Windows
return(msg.wParam);

} // end WinMain

// HERE ARE OUR GAME CONSOLE FUNCTIONS ///////////////////////////////////////////////////////

void Game_Init(void)
{
// do any initialization here

char height_file_name[16]="", // holds file names for data
     color_file_name[16]="";

// test if user sent data files
sscanf(command_line,"%s %s",height_file_name,color_file_name);

if (strlen(height_file_name)==0 || strlen(color_file_name)==0)
	{
	// load in default height maps
	Load_Bitmap_File(&height_bmp_file,"heightd2.bmp");

	// load in default color map
	Load_Bitmap_File(&color_bmp_file,"heightc2.bmp");
	} // end if
else
	{
	// load in requested height map
	Load_Bitmap_File(&height_bmp_file,height_file_name);

	// load in requested color map
	Load_Bitmap_File(&color_bmp_file,color_file_name);
	} // end else

// set palette to new colors
DD_Load_Pal_RGB(color_bmp_file.palette, SCREEN_COLORS);

// alias access ptr
color_map_ptr = color_bmp_file.buffer;

// alias access ptr
height_map_ptr = height_bmp_file.buffer;

// allocate double buffer
double_buffer = new UCHAR[SCREEN_WIDTH * SCREEN_HEIGHT];

// build the tables
Build_Tables();

Open_Error("errors.txt");

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

} // end Game_Shutdown

/////////////////////////////////////////////////////////////////////////////////////////////

void Game_Main(void)
{
// main control function like C/C++ MAIN()

// note the use of statics, since this function will be entered and exited
static int autopilot_on    = 1,   // state of autopilot
           autopilot_timer = 50, // counts time til heading change
		   autopilot_turn  = 0,   // heading direction
		   speed           = 8;   // current speed of view camera

// the word "auto" encoded as bits, 16 left if you want to add!
static unsigned int autobmp[] = {0x65770000,0x75250000,0x57270000}; 

// record starting time of frame
DWORD frame_start_time = GetTickCount();

// is user exiting?
if (KEY_DOWN(VK_ESCAPE))
	{
	PostMessage(main_window_handle,WM_DESTROY,0,0); 
	return;
	} // end if

// first clear double buffer
memset(double_buffer,0, SCREEN_WIDTH*SCREEN_HEIGHT);

// change heading, note these are the only controls that
// disengage autopilot
if (KEY_DOWN(VK_LEFT)) 
	{
	// rotate CC
	view_ang_y+=ANGLE_5; 
	autopilot_on    = 0; 
	autopilot_timer = 0;
	} // end if
else
if (KEY_DOWN(VK_RIGHT))	
	{ 
	// rotate CW
	view_ang_y-=ANGLE_5;
	autopilot_on    = 0;  
	autopilot_timer = 0;
	} // end if

// change altitude
if (KEY_DOWN('A')) 
	view_pos_z+=8;
else
if (KEY_DOWN('Z')) 
	view_pos_z-=8;

// change speed
if (KEY_DOWN(VK_UP)) 
	speed+=2;
else
if (KEY_DOWN(VK_DOWN)) 
	speed-=2;

// change pitch 
if (KEY_DOWN(VK_PRIOR)) 
	view_ang_x+=2;
else
if (KEY_DOWN(VK_NEXT)) 
	view_ang_x-=2;

// stop the camera totally
if (KEY_DOWN(VK_SPACE)) 
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
while((GetTickCount() - frame_start_time) <= 30);

} // end Game_Main

//////////////////////////////////////////////////////////////////////////////

int Open_Error(char *filename)
{
// this function opens the error file
if (strcmp(strupr(filename),"NULL.TXT")==0 || !filename)
   return(error_file_handle = -1);

// else attempt to open the file
if ((error_file_handle =
     OpenFile(filename,&error_file_data,OF_CREATE | OF_WRITE))!=-1)
   {
   // write the open error message
   Write_Error("Error Message System Open.");
   return(error_file_handle);
   } // end if
else
   return(-1);

} // end Open_Error

///////////////////////////////////////////////////////////////////////////////

int Close_Error(void)
{
// this function closes the error file if it is open
if (error_file_handle!=-1)
   {
   // send close message
   Write_Error("\nError Message System Closed.\n");
   // close the file and return success
   _lclose(error_file_handle);
   return(1);

   } // end if
else
   return(0);

} // end Close_Error

///////////////////////////////////////////////////////////////////////////////

int Write_Error(char *string)
{
// this function writes a string to the error file
if (error_file_handle!=-1)
   {
   // write the string and return success
   _lwrite(error_file_handle, string,strlen(string)+1);
   return(1);

   } // end if
else
   return(0);

} // end Write_Error

////////////////////////////////////////////////////////////////////////////////
