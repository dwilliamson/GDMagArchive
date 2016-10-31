#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <assert.h>
#include <string.h>

#define __MSC__
#include <glide.h>



/*
typedef struct {
  float  sow;                   // s texture ordinate (s over w) 
  float  tow;                   // t texture ordinate (t over w) 
  float  oow;                   // 1/w (used mipmapping - really 0xfff/w) 
}  GrTmuVertex;

/*typedef struct
{
  float x, y;         // X and Y in screen space 
  float ooz;          // 65535/Z (used for Z-buffering) 
  float oow;          // 1/W (used for W-buffering, texturing) 
  float r, g, b, a;   // R, G, B, A [0..255.0] 
  float z;            // Z is ignored 
  GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} GrVertex;*/

/*typedef struct{
	float x, y, z;	// X, Y, Z 
	float r, g, b;	// R, G, B 
	float ooz;	// 65535/Z (used for Z-buffering) 
	float a;	// Alpha 
	float oow;	// 1/W (used for W-buffering, texturing)
	GrTmuVertex tmuvtx[GLIDE_NUM_TMU];
} MyVertex;	// old GrVertex */


int		OpenRaster(void);
void	CloseRaster(void);
void	ClearBuffer(unsigned long Color);