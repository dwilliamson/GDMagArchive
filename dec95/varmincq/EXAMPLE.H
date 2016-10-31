#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"
#include "dos.h"

typedef unsigned char  uchar;
typedef unsigned int   uint;
typedef unsigned long  ulong;

#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef ASSERT
#define ASSERT(i) if (i) { fprintf(stderr, "assertion failed: %s %i\n", __FILE__, __LINE__); exit(100); }
#endif

/* quantize.c */

int quantize(uint *histogram, int max_colors, uchar *color_map, int *num_colors);

/* inv_cmap.c */

void inv_cmap_2( int colors, unsigned char *colormap[3], int bits,
                 unsigned long *dist_buf, unsigned char *rgbmap );

