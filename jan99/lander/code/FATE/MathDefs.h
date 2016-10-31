#define XRES	640
#define YRES	480

#define MAX_ANGLE       4096
#define WORLD_SEXTANT   (MAX_ANGLE >> 4)
#define WORLD_OCTANT    (MAX_ANGLE >> 3)
#define WORLD_QUADRANT  (MAX_ANGLE >> 2)

#define COSINE(a)	(gSine[(a + WORLD_QUADRANT) % MAX_ANGLE])

#define MAX_DISTANCE    2000
#define NEAR_Z_CLIP     10

#define PRECISION       16
#define PRECMULT        (1<<PRECISION)

#define WORLD_PREC		4
#define WORLD_MULT		(1<<WORLD_PREC)

#define INT_TO_FIXED28(a)	((a) * WORLD_MULT)

#define M_PI        3.14159265358979323846
#define MAXPOLY             20000
#define MAXPOINT            20000
#define ABS(a)   (((a < 0) ? -a : a))
#define FABS(a)  (((a < 0.0) ? -a : a))
#define SGN(a)   ((a < 0) ? -1 : 1)
#define MAX(a,b)   ((a > b) ? a : b)
#define MIN(a,b)   ((a < b) ? a : b)
#define CROSS3D(a,b,r)                  \
	(r).x = (a).y * (b).z - (a).z * (b).y; \
	(r).y = (a).z * (b).x - (a).x * (b).z; \
	(r).z = (a).x * (b).y - (a).y * (b).x

#define DotProduct(xl,yl,xt,yt,xb,yb)                  \
	((xt - xl) * (yl - yb) + (yl - yt) * (xl - xb))


// KEYS THAT ARE NOT DEFINED
#define VK_PERIOD			0xbe

#define POLY_TYPE_QUAD			0
#define POLY_TYPE_TRI			1
#define POLY_TYPE_COLOR			2
#define POLY_TYPE_TRANS			4
#define POLY_TYPE_2SIDED		8
#define POLY_TYPE_SORT_FRONT	16
#define POLY_TYPE_SORT_AVE		32
#define POLY_TYPE_SORT_BACK		64
#define POLY_TYPE_ERROR			128
#define POLY_TYPE_SELECTED		256
#define POLY_TYPE_TEXTURING		512

void initSinTable();
extern long gSine[MAX_ANGLE];
