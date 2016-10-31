/*
 *      A Practical Implementation of Xiaolin Wu's Color Quantizer
 *           (See Graphics Gems, Volume Two, Pages 126-133)
 *                     October 23, 1995 - v2
 *
 * Author:
 *
 * Rich Geldreich, Jr.
 *
 * Description:
 *
 * This module is an implementation of a high-speed, low-memory
 * and relatively easy to understand statistical color quantizer.
 * Its operation is similar to other Heckbert-style quantizers,
 * except that each box is weighted by variance (instead of by the
 * much more naive methods of weighting boxes by either population, size,
 * or a combination of the two), and each box is splitted on the
 * axis which will minimize the sum of the variances of both new boxes.
 *
 * Notes:
 *
 * (1) Int's are assumed to be at least 32-bits wide.
 *
 * (2) The area sum table approach to gathering color statistics is not
 * implemented here to conserve memory. Instead, a brute force method
 * of gathering color statistics is employed, which is surprisingly fast
 * and easily optimized in assembler.
 *
 * (3) A binary tree based priority list is employed to speed up the
 * search for the box with the largest variance. This differs from the
 * usual technique of using a simple linear search.
 *
 * (4) Although floating point math is used in the variance() function,
 * this may be easily replaced with high-precision fixed point math on
 * machines with weak floating point math capability.
 *
 * (5) The output of this function is an array of 8-bit palette entries.
 * It is up to you to map the original image's true-color (24-bit) pixels
 * to palettized (8-bit or less) pixels.  Spencer W. Thomas's  inverse
 * colormap functions serve this purpose very well. (See INV_CMAP.C)
 *
 */
/*----------------------------------------------------------------------------*/
#include "example.h"
/*----------------------------------------------------------------------------*/
#define HIST_BIT   (6)
#define HIST_MAX   (1 << HIST_BIT)
#define R_STRIDE   (HIST_MAX * HIST_MAX)
#define G_STRIDE   (HIST_MAX)
#define HIST_SHIFT (8 - HIST_BIT)
/*----------------------------------------------------------------------------*/
typedef struct
{
  uint variance;         /* weighted variance */
  uint total_weight;     /* total weight */
  uint tt_sum;           /* tt_sum += r*r+g*g+b*b*weight over entire box */
  uint t_ur;             /* t_ur += r*weight over entire box */
  uint t_ug;             /* t_ug += g*weight over entire box */
  uint t_ub;             /* t_ub += b*weight over entire box */
  int  ir, ig, ib;       /* upper and lower bounds */
  int  jr, jg, jb;
} box;
/*----------------------------------------------------------------------------*/
static box * *heap;     /* priority queue */
static int heap_size;
static box *boxes;      /* box list */
static int num_boxes;
static uint *hist;      /* histogram */
/*----------------------------------------------------------------------------*/
/* Shrinks box to minimum possible size.                                      */
/*----------------------------------------------------------------------------*/
static void shrink_box(int ir, int ig, int ib,
                       int jr, int jg, int jb,
                       int *lr, int *lg, int *lb,
                       int *hr, int *hg, int *hb)
{
  int r, g, b;
  uint *rp, *gp, *bp, *s;

  s = hist + (ir * R_STRIDE + ig * G_STRIDE + ib);

  rp = s;

  for (r = ir; r <= jr; r++)
  {
    gp = rp;

    for (g = ig; g <= jg; g++)
    {
      bp = gp;

      for (b = ib; b <= jb; b++)
        if (*bp++) { *lr = r; goto lr_done; }

      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

lr_done:

  gp = s;

  for (g = ig; g <= jg; g++)
  {
    rp = gp;

    for (r = ir; r <= jr; r++)
    {
      bp = rp;

      for (b = ib; b <= jb; b++)
        if (*bp++) { *lg = g; goto lg_done; }

      rp += R_STRIDE;
    }

    gp += G_STRIDE;
  }

lg_done:

  bp = s;

  for (b = ib; b <= jb; b++)
  {
    rp = bp;

    for (r = ir; r <= jr; r++)
    {
      gp = rp;

      for (g = ig; g <= jg; g++, gp += G_STRIDE)
        if (*gp) { *lb = b; goto lb_done; }

      rp += R_STRIDE;
    }

    bp++;
  }

lb_done:

  s = hist + (jr * R_STRIDE + jg * G_STRIDE + jb);

  rp = s;

  for (r = jr; r >= ir; r--)
  {
    gp = rp;

    for (g = jg; g >= ig; g--)
    {
      bp = gp;

      for (b = jb; b >= ib; b--)
        if (*bp--) { *hr = r; goto hr_done; }

      gp -= G_STRIDE;
    }

    rp -= R_STRIDE;
  }

hr_done:

  gp = s;

  for (g = jg; g >= ig; g--)
  {
    rp = gp;

    for (r = jr; r >= ir; r--)
    {
      bp = rp;

      for (b = jb; b >= ib; b--)
        if (*bp--) { *hg = g; goto hg_done; }

      rp -= R_STRIDE;
    }

    gp -= G_STRIDE;
  }

hg_done:

  bp = s;

  for (b = jb; b >= ib; b--)
  {
    gp = bp;

    for (g = jg; g >= ig; g--)
    {
      rp = gp;

      for (r = jr; r >= ir; r--, rp -= R_STRIDE)
        if (*rp) { *hb = b; goto hb_done; }

      gp -= G_STRIDE;
    }

    bp--;
  }

hb_done:

  return;
}
/*----------------------------------------------------------------------------*/
/* Standard binary tree based priorty queue manipulation functions.           */
/*----------------------------------------------------------------------------*/
static void down_heap(void)
{
  uint i, j, q;
  box *p;

  p = heap[1];
  q = p->variance;

  for (i = 1; ; )
  {
    if ((j = i << 1) > (uint)heap_size)
      break;

    if (j < (uint)heap_size)
    {
      if (heap[j]->variance < heap[j + 1]->variance)
        j++;
    }

    if (q >= heap[j]->variance)
      break;

    heap[i] = heap[j];

    i = j;
  }

  heap[i] = p;
}
/*----------------------------------------------------------------------------*/
static void insert_heap(box *p)
{
  uint i, j, q;

  q = p->variance;
  j = ++heap_size;

  for ( ; ; )
  {
    if ((i = j >> 1) == 0)
      break;

    if (heap[i]->variance >= q)
      break;

    heap[j] = heap[i];

    j = i;
  }

  heap[j] = p;
}
/*----------------------------------------------------------------------------*/
/* Returns "worst" box, or NULL if no more splittable boxes remain. The worst */
/* box is the box with the largest variance.                                  */
/*----------------------------------------------------------------------------*/
static box *worst_box(void)
{
  if (heap_size == 0)
    return NULL;
  else
    return heap[1];
}
/*----------------------------------------------------------------------------*/
/* Calculate statistics over the specified box. This is an implementation of  */
/* the "brute force" method of gathering statistics described earlier.        */
/*----------------------------------------------------------------------------*/
static void sum(int ir, int ig, int ib,
                int jr, int jg, int jb,
                uint *total_weight,
                uint *tt_sum, uint *t_ur, uint *t_ug, uint *t_ub)
{
  int i, j, r, g, b;
  uint rs, ts;
  uint w, tr, tg, tb;
  uint *rp, *gp, *bp;

  j = 0;

  tr = tg = tb = i = 0;

  rp = hist + ((ir * R_STRIDE) + (ig * G_STRIDE) + ib);

  for (r = ir; r <= jr; r++)
  {
    rs = r * r;
    gp = rp;

    for (g = ig; g <= jg; g++)
    {
      ts = rs + (g * g);
      bp = gp;

      for (b = ib; b <= jb; b++)
        if (*bp++)                /* was this cell used at all? */
        {
          w   = *(bp - 1);        /* update statistics */
          j  += w;
          tr += r * w;
          tg += g * w;
          tb += b * w;
          i  += (ts + b * b) * w;
        }

      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

  *total_weight = j;
  *tt_sum       = i;
  *t_ur         = tr;
  *t_ug         = tg;
  *t_ub         = tb;
}
/*----------------------------------------------------------------------------*/
static uint variance(uint tw, uint tt_sum,
                     uint t_ur, uint t_ug, uint t_ub)
{
  double temp;

  /* the following calculations can be performed in fixed point
   * if needed - just be sure to preserve enough precision!
   */

  temp  = (double)t_ur * (double)t_ur;
  temp += (double)t_ug * (double)t_ug;
  temp += (double)t_ub * (double)t_ub;
  temp /= (double)tw;

  return ((uint)((double)tt_sum - temp));
}
/*----------------------------------------------------------------------------*/
/* Splits box along the axis which will minimize the two new box's overall    */
/* variance. A search on each axis is used to locate the optimum split point. */
/*----------------------------------------------------------------------------*/
static void split_box(box *old_box)
{
  int i, j;
  box *new_box;

  uint total_weight;
  uint tt_sum, t_ur, t_ug, t_ub;
  int  ir, ig, ib, jr, jg, jb;

  uint total_weight1;
  uint tt_sum1, t_ur1, t_ug1, t_ub1;
  int  ir1, ig1, ib1, jr1, jg1, jb1;

  uint total_weight2;
  uint tt_sum2, t_ur2, t_ug2, t_ub2;
  int  ir2, ig2, ib2, jr2, jg2, jb2;

  uint total_weight3;
  uint tt_sum3, t_ur3, t_ug3, t_ub3;

  uint lowest_variance, variance_r, variance_g, variance_b;
  int  pick_r, pick_g, pick_b;

  new_box = boxes + num_boxes;
  num_boxes++;

  total_weight          = old_box->total_weight;
  tt_sum                = old_box->tt_sum;
  t_ur                  = old_box->t_ur;
  t_ug                  = old_box->t_ug;
  t_ub                  = old_box->t_ub;
  ir                    = old_box->ir;
  ig                    = old_box->ig;
  ib                    = old_box->ib;
  jr                    = old_box->jr;
  jg                    = old_box->jg;
  jb                    = old_box->jb;

  /* left box's initial statistics */

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  /* right box's initial statistics */

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  /* Note: One useful optimization has been purposefully omitted from the
   * following loops. The variance function is always called twice per
   * iteration to calculate the new total variance. This is a waste of time
   * in the possibly common case when the new split point did not shift any
   * new points from one box into the other. A simple test can be added to
   * remove this inefficiency.
   */

  /* locate optimum split point on red axis */

  variance_r = 0xFFFFFFFF;

  for (i = ir; i < jr; i++)
  {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */

    sum(i, ig, ib, i, jg, jb,
        &total_weight3, &tt_sum3, &t_ur3, &t_ug3, &t_ub3);

#ifdef DEBUGGING
    if (total_weight3 > total_weight)
      ASSERT(TRUE)
#endif

    /* update left and right box's statistics */

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

#ifdef DEBUGGING
    if ((total_weight1 + total_weight2) != total_weight)
      ASSERT(TRUE)
#endif

    /* calculate left and right box's overall variance */

    total_variance = variance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     variance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    /* found better split point? if so, remember it */

    if (total_variance < variance_r)
    {
      variance_r = total_variance;
      pick_r = i;
    }
  }

  /* left box's initial statistics */

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  /* right box's initial statistics */

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  /* locate optimum split point on green axis */

  variance_g = 0xFFFFFFFF;

  for (i = ig; i < jg; i++)
  {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */

    sum(ir, i, ib, jr, i, jb,
        &total_weight3, &tt_sum3, &t_ur3, &t_ug3, &t_ub3);

#ifdef DEBUGGING
    if (total_weight3 > total_weight)
      ASSERT(TRUE)
#endif

    /* update left and right box's statistics */

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

#ifdef DEBUGGING
    if ((total_weight1 + total_weight2) != total_weight)
      ASSERT(TRUE)
#endif

    /* calculate left and right box's overall variance */

    total_variance = variance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     variance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    /* found better split point? if so, remember it */

    if (total_variance < variance_g)
    {
      variance_g = total_variance;
      pick_g = i;
    }
  }

  /* left box's initial statistics */

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  /* right box's initial statistics */

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  variance_b = 0xFFFFFFFF;

  /* locate optimum split point on blue axis */

  for (i = ib; i < jb; i++)
  {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */

    sum(ir, ig, i, jr, jg, i,
        &total_weight3, &tt_sum3, &t_ur3, &t_ug3, &t_ub3);

#ifdef DEBUGGING
    if (total_weight3 > total_weight)
      ASSERT(TRUE)
#endif

    /* update left and right box's statistics */

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

#ifdef DEBUGGING
    if ((total_weight1 + total_weight2) != total_weight)
      ASSERT(TRUE)
#endif

    /* calculate left and right box's overall variance */

    total_variance = variance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     variance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    /* found better split point? if so, remember it */

    if (total_variance < variance_b)
    {
      variance_b = total_variance;
      pick_b = i;
    }
  }

  /* now find out which axis should be split */

  lowest_variance = variance_r;
  i = 0;

  if (variance_g < lowest_variance)
  {
    lowest_variance = variance_g;
    i = 1;
  }

  if (variance_b < lowest_variance)
  {
    lowest_variance = variance_b;
    i = 2;
  }

  /* split box on the selected axis */

  ir1 = ir; ig1 = ig; ib1 = ib;
  jr2 = jr; jg2 = jg; jb2 = jb;

  switch (i)
  {
    case 0:
    {
      jr1 = pick_r + 0; jg1 = jg; jb1 = jb;
      ir2 = pick_r + 1; ig2 = ig; ib2 = ib;
      break;
    }
    case 1:
    {
      jr1 = jr; jg1 = pick_g + 0; jb1 = jb;
      ir2 = ir; ig2 = pick_g + 1; ib2 = ib;
      break;
    }
    case 2:
    {
      jr1 = jr; jg1 = jg; jb1 = pick_b + 0;
      ir2 = ir; ig2 = ig; ib2 = pick_b + 1;
      break;
    }
  }

  /* shrink the new boxes to their minimum possible sizes */

  shrink_box(ir1, ig1, ib1, jr1, jg1, jb1,
            &ir1, &ig1, &ib1, &jr1, &jg1, &jb1);

  shrink_box(ir2, ig2, ib2, jr2, jg2, jb2,
            &ir2, &ig2, &ib2, &jr2, &jg2, &jb2);

  /* update statistics */

  sum(ir1, ig1, ib1, jr1, jg1, jb1,
      &total_weight1, &tt_sum1, &t_ur1, &t_ug1, &t_ub1);

  total_weight2         = total_weight - total_weight1;
  tt_sum2               = tt_sum - tt_sum1;
  t_ur2                 = t_ur - t_ur1;
  t_ug2                 = t_ug - t_ug1;
  t_ub2                 = t_ub - t_ub1;

  /* create the new boxes */

  old_box->variance     = variance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1);
  old_box->total_weight = total_weight1;
  old_box->tt_sum       = tt_sum1;
  old_box->t_ur         = t_ur1;
  old_box->t_ug         = t_ug1;
  old_box->t_ub         = t_ub1;
  old_box->ir           = ir1;
  old_box->ig           = ig1;
  old_box->ib           = ib1;
  old_box->jr           = jr1;
  old_box->jg           = jg1;
  old_box->jb           = jb1;

  new_box->variance     = variance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);
  new_box->total_weight = total_weight2;
  new_box->tt_sum       = tt_sum2;
  new_box->t_ur         = t_ur2;
  new_box->t_ug         = t_ug2;
  new_box->t_ub         = t_ub2;
  new_box->ir           = ir2;
  new_box->ig           = ig2;
  new_box->ib           = ib2;
  new_box->jr           = jr2;
  new_box->jg           = jg2;
  new_box->jb           = jb2;

  /* enter all splittable boxes into the priory queue */

  i = 0;
  if ((jr1 - ir1) + (jg1 - ig1) + (jb1 - ib1)) i = 2;
  if ((jr2 - ir2) + (jg2 - ig2) + (jb2 - ib2)) i++;

  switch (i)
  {
    case 0:
    {
      heap[1] = heap[heap_size];

      heap_size--;

      if (heap_size)
        down_heap();

      break;
    }
    case 1:
    {
      heap[1] = new_box;

      down_heap();

      break;
    }
    case 2:
    {
      down_heap();

      break;
    }
    case 3:
    {
      down_heap();

      insert_heap(new_box);

      break;
    }
  }
}
/*----------------------------------------------------------------------------*/
/* Creates new colormap.                                                      */
/*----------------------------------------------------------------------------*/
static void make_color_map(uchar *color_map)
{
  int i;
  box *p;
  uint total_weight;

  p = boxes;

  for (i = 0; i < num_boxes; i++, p++)
  {
    total_weight = p->total_weight;

    *color_map++ = (uchar)(((p->t_ur << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
    *color_map++ = (uchar)(((p->t_ug << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
    *color_map++ = (uchar)(((p->t_ub << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
  }
}
/*----------------------------------------------------------------------------*/
/* Create initial box, initialize heap.                                       */
/*----------------------------------------------------------------------------*/
static int initialize(int colors)
{
  uint total_weight;
  uint tt_sum, t_ur, t_ug, t_ub;
  int ir, ig, ib, jr, jg, jb;

  if ((heap = malloc(sizeof(box *) * (colors + 1))) == NULL)
    return TRUE;

  if ((boxes = malloc(sizeof(box) * colors)) == NULL)
    return TRUE;

  /* shrink initial box to minimum possible size */
  shrink_box(0, 0, 0, HIST_MAX - 1, HIST_MAX - 1, HIST_MAX - 1, &ir, &ig, &ib, &jr, &jg, &jb);

  /* calculate the initial box's statistics */
  sum(ir, ig, ib, jr, jg, jb, &total_weight, &tt_sum, &t_ur, &t_ug, &t_ub);

  boxes->total_weight = total_weight;
  boxes->variance     = 1;
  boxes->tt_sum       = tt_sum;
  boxes->t_ur         = t_ur;
  boxes->t_ug         = t_ug;
  boxes->t_ub         = t_ub;
  boxes->ir           = ir;
  boxes->ig           = ig;
  boxes->ib           = ib;
  boxes->jr           = jr;
  boxes->jg           = jg;
  boxes->jb           = jb;

  /* enter box into heap if it's splittable */

  num_boxes           = 1;
  heap_size           = 0;

  if ((jr - ir) + (jg - ig) + (jb - ib))
  {
    heap[1] = boxes;
    heap_size = 1;
  }

  return FALSE;
}
/*----------------------------------------------------------------------------*/
int quantize(uint *histogram, int max_colors, uchar *color_map, int *num_colors)
{
  int status = FALSE;
  box *p;

  heap  = NULL;
  boxes = NULL;

  hist  = histogram;

  if ((status = initialize(max_colors)) != 0)
    goto reduce_error;

  while (num_boxes < max_colors)
  {
    if ((p = worst_box()) == NULL)
      break;

    split_box(p);
  }

  make_color_map(color_map);

  *num_colors = num_boxes;

reduce_error:

  free(heap);
  free(boxes);

  return status;
}
/*----------------------------------------------------------------------------*/

