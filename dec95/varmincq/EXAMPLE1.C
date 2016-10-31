/*
 * EXAMPLE1.C - October 23, 1995 - v2
 *
 * Example Program #1 - Reads and writes files created by PICLAB.
 */

/*----------------------------------------------------------------------------*/
/* example1.c                                                                 */
/*----------------------------------------------------------------------------*/
#include "example.h"
/*----------------------------------------------------------------------------*/
#define HIST_BIT   (6)
#define HIST_MAX   (1 << HIST_BIT)
#define R_STRIDE   (HIST_MAX * HIST_MAX)
#define G_STRIDE   (HIST_MAX)
#define HIST_CELLS (HIST_MAX * HIST_MAX * HIST_MAX)
#define HIST_SHIFT (8 - HIST_BIT)
/*----------------------------------------------------------------------------*/
void *cmalloc_msg(uint i)
{
  void *p = malloc(i);

  if (!p)
  {
    if (!i)
      return NULL;
    else
    {
      fprintf(stderr, "error: out of memory!\n");
      exit(1);
    }
  }

  memset(p, 0, i);

  return p;
}
/*----------------------------------------------------------------------------*/
int main(int arg_c, char *arg_v[])
{
  int i, j, src_x_size, src_y_size, max_colors, num_colors;
  FILE *src_r_file, *src_g_file, *src_b_file;
  FILE *dst_file, *dst_map;
  uchar *color_map = cmalloc_msg(256 * 3);
  uchar *cmap[3];
  uchar *inv_cmap = cmalloc_msg(HIST_CELLS);
  uint *histogram = cmalloc_msg(HIST_CELLS * sizeof(uint));
  ulong *dist_buf = cmalloc_msg(HIST_CELLS * sizeof(uint));
  char *tmp = cmalloc_msg(512);
  uchar *r_buf, *g_buf, *b_buf, *d_buf;

  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  if (arg_c != 6)
  {
    fprintf(stderr, "usage: <src file> <dst file> <x res> <y res> <max dst colors>\n");
    exit(1);
  }

  cmap[0] = cmalloc_msg(256);
  cmap[1] = cmalloc_msg(256);
  cmap[2] = cmalloc_msg(256);

  src_x_size = atoi(arg_v[3]);
  src_y_size = atoi(arg_v[4]);
  max_colors = atoi(arg_v[5]);

  if ((src_x_size < 1) || (src_y_size < 1))
  {
    fprintf(stderr, "error: invalid source file resolution!\n");
    exit(1);
  }

  if ((max_colors < 1) || (max_colors > 256))
  {
    fprintf(stderr, "error: invalid number of destination colors!\n");
    exit(1);
  }

  sprintf(tmp, "%s.r8", arg_v[1]);
  src_r_file = fopen(tmp, "rb"); ASSERT(!src_r_file);

  sprintf(tmp, "%s.g8", arg_v[1]);
  src_g_file = fopen(tmp, "rb"); ASSERT(!src_g_file);

  sprintf(tmp, "%s.b8", arg_v[1]);
  src_b_file = fopen(tmp, "rb"); ASSERT(!src_b_file);

  sprintf(tmp, "%s.r8", arg_v[2]);
  dst_file = fopen(tmp, "wb"); ASSERT(!dst_file);

  sprintf(tmp, "%s.map", arg_v[2]);
  dst_map = fopen(tmp, "wb"); ASSERT(!dst_map);

  setvbuf(src_r_file, NULL, _IOFBF, 8192);
  setvbuf(src_g_file, NULL, _IOFBF, 8192);
  setvbuf(src_b_file, NULL, _IOFBF, 8192);
  setvbuf(dst_file,   NULL, _IOFBF, 8192);

  r_buf = cmalloc_msg(src_x_size);
  g_buf = cmalloc_msg(src_x_size);
  b_buf = cmalloc_msg(src_x_size);
  d_buf = cmalloc_msg(src_x_size);

  printf("building histogram...\n");

  for (i = 0; i < src_y_size; i++)
  {
    ASSERT(fread(r_buf, 1, src_x_size, src_r_file) != src_x_size);
    ASSERT(fread(g_buf, 1, src_x_size, src_g_file) != src_x_size);
    ASSERT(fread(b_buf, 1, src_x_size, src_b_file) != src_x_size);

    for (j = 0; j < src_x_size; j++)
    {
      int r = r_buf[j] >> HIST_SHIFT;
      int g = g_buf[j] >> HIST_SHIFT;
      int b = b_buf[j] >> HIST_SHIFT;

      histogram[(r * R_STRIDE) + (g * G_STRIDE) + b]++;
    }
  }

  printf("quantizing to a maximum of %i colors...\n", max_colors);

  if (quantize(histogram, max_colors, color_map, &num_colors))
    ASSERT(TRUE);

  printf("quantized to %i colors\n", num_colors);

  printf("building inverse colormap...\n");

  for (i = 0; i < num_colors; i++)
  {
    cmap[0][i] = color_map[i * 3 + 0];
    cmap[1][i] = color_map[i * 3 + 1];
    cmap[2][i] = color_map[i * 3 + 2];
  }

  inv_cmap_2(num_colors, cmap, HIST_BIT, dist_buf, inv_cmap);

  printf("writing palettized image...\n");

  for (i = 0; i < num_colors; i++)
    fprintf(dst_map, "%i %i %i\n", color_map[i * 3 + 0], color_map[i * 3 + 1], color_map[i * 3 + 2]);

  rewind(src_r_file);
  rewind(src_g_file);
  rewind(src_b_file);

  for (i = 0; i < src_y_size; i++)
  {
    ASSERT(fread(r_buf, 1, src_x_size, src_r_file) != src_x_size);
    ASSERT(fread(g_buf, 1, src_x_size, src_g_file) != src_x_size);
    ASSERT(fread(b_buf, 1, src_x_size, src_b_file) != src_x_size);

    for (j = 0; j < src_x_size; j++)
    {
      int r = r_buf[j] >> HIST_SHIFT;
      int g = g_buf[j] >> HIST_SHIFT;
      int b = b_buf[j] >> HIST_SHIFT;

      d_buf[j] = inv_cmap[(r * R_STRIDE) + (g * G_STRIDE) + b];
    }

    ASSERT(fwrite(d_buf, 1, src_x_size, dst_file) != src_x_size);
  }

  fcloseall();

  free(color_map);
  free(inv_cmap);
  free(histogram);
  free(dist_buf);
  free(tmp);
  free(cmap[0]);
  free(cmap[1]);
  free(cmap[2]);
  free(r_buf);
  free(g_buf);
  free(b_buf);
  free(d_buf);

  return 0;
}
/*----------------------------------------------------------------------------*/

