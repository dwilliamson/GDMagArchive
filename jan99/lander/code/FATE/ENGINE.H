/*****************************************************************

  Engine.H 
  Game Engine Structures
*****************************************************************/

#ifndef ENGINE_H
#define ENGINE_H

#define MAX_FRAMES		100			/* MAXIMUM FRAMES PER DESC */

typedef long fixed28;

typedef unsigned char uchar;

// STRUCTURE DEFINITIONS...
// 2D POINT TYPE FOR SECTORS...FIXED POINT X AND Y
typedef struct POINT2D
{
	 long x, y;
} tPoint2D;

typedef struct POINT3D
{
	 long x, y, z;
} tPoint3D;

/*****************************************
 INSIDE SECTOR STRUCTURE:
*****************************************/

	/* quadrant id's, incremental angles, accumulated angle values */
typedef short quadrant_type;

	/* result value from point in polygon test */
typedef enum pt_poly_relation {INSIDE, OUTSIDE} pt_poly_relation;

	/* determine the quadrant of a polygon point
	   relative to the test point */
#define quadrant(vertex, x, y) \
  ( (vertex.x > x) ? ((vertex.y > y) ? 0 : 3) : ( (vertex.y > y) ? 1 : 2) )

	/* determine x intercept of a polygon edge
	   with a horizontal line at the y value of the test point */
#define x_intercept(pt1, pt2,  yy) \
  (pt2.x - ( ((pt2.y - yy) * (pt1.x - pt2.x)) / (pt1.y - pt2.y) ) )

	/* adjust delta */
#define adjust_delta(delta, vertex, next_vertex, xx, yy)		\
  switch (delta) {							\
	    /* make quadrant deltas wrap around */			\
    case  3:	delta = -1; break;					\
    case -3:	delta =	 1; break;					\
	    /* check if went around point cw or ccw */			\
    case  2: case -2: if (x_intercept(vertex, next_vertex, yy) > xx)	\
		    delta =  - (delta);					\
		break;							\
    }


/* 2D EDGE STRUCTURE */
typedef struct {
	tPoint2D pos;	/* Coords of Start */
	short	nextedge,prevedge,backedge;
	short	sector,backsector;
	short	tex1,tex2;
	uchar	Uoff,Voff,Uscale,Vscale;
	long	shade;
	long	flags;
	long	pad1,pad2,pad3;
} tEdge;

#define SECTOR_FLAGS_CLOSED		1
#define SECTOR_FLAGS_VISITED	2

/* 2D SECTOR STRUCTURE */
typedef struct {
	short	edge,edgecnt;	/* FIRST EDGE # and COUNT */
	long	ceil_height,floor_height;
	short	floor_flags,floor_tex;
	uchar	floor_Uoff,floor_Voff,floor_Uscale,floor_Vscale;
	long	floor_shade;
	short	ceil_flags,ceil_tex;
	uchar	ceil_Uoff,ceil_Voff,ceil_Uscale,ceil_Vscale;
	long	ceil_shade;
	long	flags,pad2,pad3;		// FOR LATER EXPANSION
} tSector;

#endif