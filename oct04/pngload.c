#ifdef SCREENSHOT
#include <stdlib.h>
#include "png.h"
#include "psdparse.h"


/* write a png file */
void write_png(char *file_name, PSDbitmap *bm)
{
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;
   png_bytepp rows=NULL;

   /* open the file */
   fp = fopen(file_name, "wb");
   if (fp == NULL)
      return;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
   if (png_ptr == NULL)
   {
      fclose(fp);
      return;
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
      free(rows);
      return;
   }
   info_ptr->width = bm->w;
   info_ptr->height = bm->h;
   info_ptr->color_type = PNG_COLOR_TYPE_RGBA;
   info_ptr->bit_depth = 8;
   info_ptr->rowbytes = bm->w * 4;
   info_ptr->valid = 0;
   info_ptr->interlace_type = PNG_INTERLACE_NONE;
   info_ptr->compression_type = PNG_COMPRESSION_TYPE_DEFAULT;

   rows = malloc(sizeof(rows[0]) * bm->h);
   { int i; for(i=0; i < bm->h; ++i) rows[bm->h-1 - i] = (png_bytep) &bm->data[bm->w * i]; }
   png_set_rows(png_ptr, info_ptr, rows);

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return;
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   png_write_png(png_ptr, info_ptr, 0, png_voidp_NULL);

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);
   free(rows);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}

#endif
