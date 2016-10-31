/*
 * EXAMPLE2.C - October 23, 1995 - v2
 *
 * Example Program #2 - Reads and writes files created by PowerView.
 */

/*----------------------------------------------------------------------------*/
/* example2.c                                                                 */
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
#define TRUECOLOR (2048)
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
  FILE *src_file, *dst_file;
  uchar *color_map = cmalloc_msg(256 * 3);
  uchar *cmap[3];
  uchar *inv_cmap = cmalloc_msg(HIST_CELLS);
  uint *histogram = cmalloc_msg(HIST_CELLS * sizeof(uint));
  ulong *dist_buf = cmalloc_msg(HIST_CELLS * sizeof(uint));
  char *tmp = cmalloc_msg(512);
  uchar *d_buf;

  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  if (arg_c != 4)
  {
    fprintf(stderr, "usage: <src file> <dst file> <max dst colors>\n");
    exit(1);
  }

  cmap[0] = cmalloc_msg(256);
  cmap[1] = cmalloc_msg(256);
  cmap[2] = cmalloc_msg(256);

  sprintf(tmp, "%s.raw", arg_v[1]);
  src_file = fopen(tmp, "rb"); ASSERT(!src_file);

  sprintf(tmp, "%s.raw", arg_v[2]);
  dst_file = fopen(tmp, "wb"); ASSERT(!dst_file);

  setvbuf(src_file, NULL, _IOFBF, 8192);
  setvbuf(dst_file, NULL, _IOFBF, 8192);

  max_colors = atoi(arg_v[3]);

  if ((max_colors < 1) || (max_colors > 256))
  {
    fprintf(stderr, "error: invalid number of destination colors!\n");
    exit(1);
  }

  ASSERT(fread(tmp, 1, 8, src_file) != 8);

  tmp[8] = '\0';

  if (strcmp(tmp, "MAJESTIC") != 0)
  {
    fprintf(stderr, "error: source file is invalid\n");
    exit(1);
  }

  src_x_size = fgetc(src_file); src_x_size += (int)fgetc(src_file) << 8;
  src_y_size = fgetc(src_file); src_y_size += (int)fgetc(src_file) << 8;

  if ((src_x_size < 1) || (src_y_size < 1))
  {
    fprintf(stderr, "error: invalid source file resolution!\n");
    exit(1);
  }

  i = fgetc(src_file); i += (int)fgetc(src_file) << 8;

  if (i != TRUECOLOR)
  {
    fprintf(stderr, "error: source file is not true color\n");
    exit(1);
  }

  ASSERT(fread(tmp, 1, 6, src_file) != 6);

  tmp[6] = '\0';

  if (strcmp(tmp, "GRUDGE") != 0)
  {
    fprintf(stderr, "error: source file is invalid\n");
    exit(1);
  }

  d_buf = cmalloc_msg(src_x_size);

  printf("building histogram...\n");

  for (i = 0; i < src_y_size; i++)
    for (j = 0; j < src_x_size; j++)
    {
      int b, g, r;

      b = fgetc(src_file) >> HIST_SHIFT;
      g = fgetc(src_file) >> HIST_SHIFT;
      r = fgetc(src_file) >> HIST_SHIFT;

      histogram[(r * R_STRIDE) + (g * G_STRIDE) + b]++;
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

  fprintf(dst_file, "MAJESTIC");

  fputc(src_x_size & 0xFF, dst_file);
  fputc((src_x_size >> 8) & 0xFF, dst_file);

  fputc(src_y_size & 0xFF, dst_file);
  fputc((src_y_size >> 8) & 0xFF, dst_file);

  fputc(num_colors & 0xFF, dst_file);
  fputc((num_colors >> 8) & 0xFF, dst_file);

  ASSERT(fwrite(color_map, 1, num_colors * 3, dst_file) != num_colors * 3);

  fprintf(dst_file, "GRUDGE");

  fseek(src_file, 8 + 2 + 2 + 2 + 6, SEEK_SET);

  for (i = 0; i < src_y_size; i++)
  {
    for (j = 0; j < src_x_size; j++)
    {
      int b, g, r;

      b = fgetc(src_file) >> HIST_SHIFT;
      g = fgetc(src_file) >> HIST_SHIFT;
      r = fgetc(src_file) >> HIST_SHIFT;

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
  free(d_buf);

  return 0;
}
/*----------------------------------------------------------------------------*/

