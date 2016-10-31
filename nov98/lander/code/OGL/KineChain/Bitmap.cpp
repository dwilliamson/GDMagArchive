///////////////////////////////////////////////////////////////////////////////
//
// Bitmap.cpp : implementation file
//
// Purpose:	Implementation of Windows BMP Loader
//
// Created:
//		JL 7/1/98		
//
// Notes:		This code was originally from the OpenGL SuperBible 
//				by Richard Wright Jr. and Michael Sweet
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bitmap.h"


/*
 * 'LoadDIBitmap()' - Load a DIB/BMP file from disk.
 *
 * Returns a pointer to the bitmap if successful, NULL otherwise...
 */

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadDIBitmap
// Purpose:		Load texture images for the bone
// Arguments:	Name of the file to load, bitmap info 
void *LoadDIBitmap(char	*filename,BITMAPINFO	**info)
{
  FILE			*fp;		/* Open file pointer */
  void			*bits;		/* Bitmap pixel bits */
  long			bitsize,	/* Size of bitmap */
			infosize;	/* Size of header information */
  BITMAPFILEHEADER	header;		/* File header */


 /*
  * Try opening the file; use "rb" mode to read this *binary* file.
  */

  if ((fp = fopen(filename, "rb")) == NULL)
    return (NULL);

 /*
  * Read the file header and any following bitmap information...
  */

  if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1)
  {
   /*
    * Couldn't read the file header - return NULL...
    */

    fclose(fp);
    return (NULL);
  };

  if (header.bfType != 'MB')	/* Check for BM reversed... */
  {
   /*
    * Not a bitmap file - return NULL...
    */

    fclose(fp);
    return (NULL);
  };

  infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
  if ((*info = (BITMAPINFO *)malloc(infosize)) == NULL)
  {
   /*
    * Couldn't allocate memory for bitmap info - return NULL...
    */

    fclose(fp);
    return (NULL);
  };

  if (fread(*info, 1, infosize, fp) < infosize)
  {
   /*
    * Couldn't read the bitmap header - return NULL...
    */

    free(*info);
    fclose(fp);
    return (NULL);
  };

 /*
  * Now that we have all the header info read in, allocate memory for the
  * bitmap and read *it* in...
  */

  if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
    bitsize = ((*info)->bmiHeader.biWidth *
               (*info)->bmiHeader.biBitCount + 7) / 8 *
  	      abs((*info)->bmiHeader.biHeight);

  if ((bits = malloc(bitsize)) == NULL)
  {
   /*
    * Couldn't allocate memory - return NULL!
    */

    free(*info);
    fclose(fp);
    return (NULL);
  };

  if (fread(bits, 1, bitsize, fp) < bitsize)
  {
   /*
    * Couldn't read bitmap - free memory and return NULL!
    */

    free(*info);
    free(bits);
    fclose(fp);
    return (NULL);
  };

 /*
  * OK, everything went fine - return the allocated bitmap...
  */

  fclose(fp);
  return (bits);
}


/*
 * 'ConvertRGB()' - Convert a DIB/BMP image to 24-bit RGB pixels.
 *
 * Returns an RGB pixel array if successful and NULL otherwise.
 */

GLubyte * ConvertBitsToGL(BITMAPINFO *info,void *bits)
{
  int		i, j,			/* Looping vars */
  		bitsize,		/* Total size of bitmap */
		width;			/* Aligned width of bitmap */
  GLubyte	*newbits;		/* New RGB bits */
  GLubyte	*from, *to,		/* RGB looping vars */
		temp;			/* Temporary var for swapping */


 /*
  * Allocate memory for the RGB bitmap...
  */

  width   = 3 * info->bmiHeader.biWidth;
  width   = (width + 3) & ~3;	
  bitsize = width * info->bmiHeader.biHeight;
  if ((newbits = (GLubyte *)calloc(bitsize, 1)) == NULL)
    return (NULL);

 /*
  * Copy the original bitmap to the new array, converting as necessary.
  */

  switch (info->bmiHeader.biCompression)
  {
    case BI_RGB :
        if (info->bmiHeader.biBitCount == 24)
	{
         /*
          * Swap red & blue in a 24-bit image...
          */

          for (i = 0; i < info->bmiHeader.biHeight; i ++)
	    for (j = 0, from = ((GLubyte *)bits) + i * width,
	             to = newbits + i * width;
		 j < info->bmiHeader.biWidth;
		 j ++, from += 3, to += 3)
            {
              to[0] = from[2];
              to[1] = from[1];
              to[2] = from[0];
            };
	};
	break;
    case BI_RLE4 :
    case BI_RLE8 :
    case BI_BITFIELDS :
        break;
  };

  return (newbits);
}
