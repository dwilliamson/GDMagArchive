/*
 * "$Id: bitmap.c,v 1.2 1996/01/09 22:52:53 mike Exp mike $"
 *
 *   Device Independent Bitmap functions for OpenGL under MS Windows.
 *
 * Contents:
 *
 *   LoadDIBitmap()  - Load a DIB/BMP file from disk.
 *   SaveDIBitmap()  - Save a bitmap to a DIB/BMP file on disk.
 *   ReadDIBitmap()  - Read the current OpenGL viewport into a
 *                     24-bit RGB bitmap.
 *   PrintDIBitmap() - Print a bitmap to a GDI printer.
 *   ConvertRGB()    - Convert a DIB/BMP image to 24-bit RGB pixels.
 *
 * Revision History:
 *
 *   $Log: bitmap.c,v $
 *   Revision 1.2  1996/01/09  22:52:53  mike
 *   Added PrintDIBitmap.
 *
 *   Revision 1.1  1995/12/31  07:27:17  mike
 *   Initial revision
 */

/*
 * Include necessary headers.
 */

#include "stdafx.h"
#include "bitmap.h"


/*
 * 'LoadDIBitmap()' - Load a DIB/BMP file from disk.
 *
 * Returns a pointer to the bitmap if successful, NULL otherwise...
 */

void *
LoadDIBitmap(char	*filename,	/* I - File to load */
             BITMAPINFO	**info)		/* O - Bitmap information */
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
 * 'SaveDIBitmap()' - Save a bitmap to a DIB/BMP file on disk.
 *
 * Returns 0 if successful, non-zero otherwise...
 */

int
SaveDIBitmap(char       *filename,	/* I - File to save to */
	     BITMAPINFO *info,		/* I - Bitmap information */
             void       *bits)		/* I - Bitmap pixel bits */
{
  FILE			*fp;		/* Open file pointer */
  long			size,		/* Size of file */
			infosize,	/* Size of bitmap info */
			bitsize;	/* Size of bitmap pixels */
  BITMAPFILEHEADER	header;		/* File header */


 /*
  * Try opening the file; use "wb" mode to write this *binary* file.
  */

  if ((fp = fopen(filename, "wb")) == NULL)
    return (-1);

  if (info->bmiHeader.biSizeImage == 0)	/* Figure out the bitmap size */
    bitsize = (info->bmiHeader.biWidth *
               info->bmiHeader.biBitCount + 7) / 8 *
	      abs(info->bmiHeader.biHeight);
  else
    bitsize = info->bmiHeader.biSizeImage;

  infosize = sizeof(BITMAPINFOHEADER);
  switch (info->bmiHeader.biCompression)
  {
    case BI_BITFIELDS :
        infosize += 12;			/* Add 3 RGB doubleword masks */
        if (info->bmiHeader.biClrUsed == 0)
	  break;
    case BI_RGB :
        if (info->bmiHeader.biBitCount > 8 &&
            info->bmiHeader.biClrUsed == 0)
	  break;
    case BI_RLE8 :
    case BI_RLE4 :
        if (info->bmiHeader.biClrUsed == 0)
          infosize += (1 << info->bmiHeader.biBitCount) * 4;
	else
          infosize += info->bmiHeader.biClrUsed * 4;
	break;
  };

  size = sizeof(BITMAPFILEHEADER) + infosize + bitsize;

 /*
  * Write the file header, bitmap information, and bitmap pixel data...
  */

  header.bfType      = 'MB';		/* Non-portable... sigh */
  header.bfSize      = size;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits   = sizeof(BITMAPFILEHEADER) + infosize;

  if (fwrite(&header, 1, sizeof(BITMAPFILEHEADER), fp) < sizeof(BITMAPFILEHEADER))
  {
   /*
    * Couldn't write the file header - return...
    */

    fclose(fp);
    return (-1);
  };

  if (fwrite(info, 1, infosize, fp) < infosize)
  {
   /*
    * Couldn't write the bitmap header - return...
    */

    fclose(fp);
    return (-1);
  };

  if (fwrite(bits, 1, bitsize, fp) < bitsize)
  {
   /*
    * Couldn't write the bitmap - return...
    */

    fclose(fp);
    return (-1);
  };

 /*
  * OK, everything went fine - return...
  */

  fclose(fp);
  return (0);
}


/*
 * 'ReadDIBitmap()' - Read the current OpenGL viewport into a
 *                    24-bit RGB bitmap.
 *
 * Returns the bitmap pixels if successful and NULL otherwise.
 */

void *
ReadDIBitmap(BITMAPINFO **info)		/* O - Bitmap information */
{
  long		i, j,			/* Looping var */
  		bitsize,		/* Total size of bitmap */
		width;			/* Aligned width of a scanline */
  GLint		viewport[4];		/* Current viewport */
  void		*bits;			/* RGB bits */
  GLubyte	*rgb,			/* RGB looping var */
		temp;			/* Temporary var for swapping */

 /*
  * Grab the current viewport...
  */

  glGetIntegerv(GL_VIEWPORT, viewport);

 /*
  * Allocate memory for the header and bitmap...
  */

  if ((*info = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER))) == NULL)
  {
   /*
    * Couldn't allocate memory for bitmap info - return NULL...
    */

    return (NULL);
  };

  width   = viewport[2] * 3;		/* Real width of scanline */
  width   = (width + 3) & ~3;		/* Aligned to 4 bytes */
  bitsize = width * viewport[3];	/* Size of bitmap, aligned */

  if ((bits = calloc(bitsize, 1)) == NULL)
  {
   /*
    * Couldn't allocate memory for bitmap pixels - return NULL...
    */

    free(*info);
    return (NULL);
  };

 /*
  * Read pixels from the framebuffer...
  */

  glFinish();				/* Finish all OpenGL commands */
  glPixelStorei(GL_PACK_ALIGNMENT, 4);	/* Force 4-byte alignment */
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

  glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE,
               bits);

 /*
  * Swap red and blue for the bitmap...
  */

  for (i = 0; i < viewport[3]; i ++)
    for (j = 0, rgb = ((GLubyte *)bits) + i * width;
         j < viewport[2];
	 j ++, rgb += 3)
    {
      temp   = rgb[0];
      rgb[0] = rgb[2];
      rgb[2] = temp;
    };

 /*
  * Finally, initialize the bitmap header information...
  */

  (*info)->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  (*info)->bmiHeader.biWidth         = viewport[2];
  (*info)->bmiHeader.biHeight        = viewport[3];
  (*info)->bmiHeader.biPlanes        = 1;
  (*info)->bmiHeader.biBitCount      = 24;
  (*info)->bmiHeader.biCompression   = BI_RGB;
  (*info)->bmiHeader.biSizeImage     = bitsize;
  (*info)->bmiHeader.biXPelsPerMeter = 2952; /* 75 DPI */
  (*info)->bmiHeader.biYPelsPerMeter = 2952; /* 75 DPI */
  (*info)->bmiHeader.biClrUsed       = 0;
  (*info)->bmiHeader.biClrImportant  = 0;

  return (bits);
}


/*
 * 'PrintDIBitmap()' - Print a bitmap to a GDI printer.
 */

int
PrintDIBitmap(HWND       owner,		/* I - Owner/parent window */
              BITMAPINFO *info,		/* I - Bitmap information */
              void       *bits)		/* I - Bitmap pixel bits */
{
  PRINTDLG	pd;			/* Print dialog information */
  long		xsize,			/* Size of printed image */
		ysize,
		xoffset,		/* Offset from edges for image */
		yoffset;
  RECT		rect;			/* Page rectangle */
  DOCINFO	di;			/* Document info */
  HDC		hdc;			/* Device context for bitmap */
  HBITMAP	bitmap;			/* Bitmap image */
  HBRUSH	brush;			/* Background brush for page */
  HCURSOR	busy,			/* Busy cursor */
		oldcursor;		/* Old cursor */


 /*
  * Range check...
  */

  if (info == NULL || bits == NULL)
    return (0);

 /*
  * Initialize a PRINTDLG structure before displaying a standard Windows
  * print dialog...
  */

  memset(&pd, 0, sizeof(pd));
  pd.lStructSize = sizeof(pd);
  pd.hwndOwner   = owner;
  pd.Flags       = PD_RETURNDC;
  pd.hInstance   = NULL;									    
  if (!PrintDlg(&pd))
    return (0);		/* User chose 'cancel'... */

 /*
  * OK, user wants to print, so set the cursor to 'busy' and start the
  * print job...
  */

  busy      = LoadCursor(NULL, IDC_WAIT);
  oldcursor = SetCursor(busy);

  SetMapMode(pd.hDC, MM_TEXT);
  di.cbSize      = sizeof(DOCINFO);
  di.lpszDocName = "OpenGL Image";
  di.lpszOutput  = NULL;

  StartDoc(pd.hDC, &di);
  StartPage(pd.hDC);

 /*
  * Clear the background to white...
  */

  rect.top    = 0;
  rect.left   = 0;
  rect.right  = GetDeviceCaps(pd.hDC, HORZRES);
  rect.bottom = GetDeviceCaps(pd.hDC, VERTRES);
  brush       = CreateSolidBrush(0x00ffffff);
  FillRect(pd.hDC, &rect, brush);

 /*
  * Stretch the bitmap to fit the page...
  */

  hdc    = CreateCompatibleDC(pd.hDC);
  bitmap = CreateDIBitmap(hdc, &(info->bmiHeader), CBM_INIT, bits, info,
                          DIB_RGB_COLORS);
  SelectObject(hdc, bitmap);

  xsize = rect.right;
  ysize = xsize * info->bmiHeader.biHeight / info->bmiHeader.biWidth;
  if (ysize > rect.bottom)
  {
    ysize = rect.bottom;
    xsize = ysize * info->bmiHeader.biWidth / info->bmiHeader.biHeight;
  };

  xoffset = (rect.right - xsize) / 2;
  yoffset = (rect.bottom - ysize) / 2;

  StretchBlt(pd.hDC, xoffset, yoffset, xsize, ysize,
             hdc, 0, 0, info->bmiHeader.biWidth, info->bmiHeader.biHeight,
             SRCCOPY);

 /*
  * That's it.  End the print job and free anything we allocated...
  */

  EndPage(pd.hDC);
  EndDoc(pd.hDC);
  DeleteDC(pd.hDC);

  DeleteObject(bitmap);
  DeleteObject(brush);
  DeleteObject(busy);
  DeleteDC(hdc);

 /*
  * Restore the cursor and return...
  */

  SetCursor(oldcursor);

  return (1);
}


/*
 * 'ConvertRGB()' - Convert a DIB/BMP image to 24-bit RGB pixels.
 *
 * Returns an RGB pixel array if successful and NULL otherwise.
 */

GLubyte *
ConvertRGB(BITMAPINFO *info,		/* I - Original bitmap information */
           void       *bits)		/* I - Original bitmap pixels */
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


///////////////////////////////////////////////////////////////////////////////
// This stuff forward is the TGA Loading Routines
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Name: LoadTGAFile()
// Desc: Given a filename of a TGA file, it loads the image in BITMAP form
//-----------------------------------------------------------------------
GLubyte *LoadTGAFile( TCHAR* strFilename,tTGAHeader_s *header)
{
/// Local Variables ///////////////////////////////////////////////////////////
	short			BPP;
	unsigned char	*buffer,*buffer2;
	int				loop,loop2;
	int bitsize;		/* Total size of bitmap */
	GLubyte	*newbits;		/* New RGB bits */
	GLubyte	*from, *to;		/* RGB looping vars */
	int		i, j,			/* Looping vars */
	width;			/* Aligned width of bitmap */
    FILE* file;
///////////////////////////////////////////////////////////////////////////////

    // Open the file and read the header
	file = fopen( strFilename, TEXT("rb") );
    if( NULL == file )
        return NULL;

    if ( fread( header, sizeof( tTGAHeader_s ), 1, file ) != 1 )
    {
        fclose( file );
        return NULL;
    }

    // Parse the TGA header
    DWORD dwWidth, dwHeight;
	dwWidth = (DWORD)header->d_width;
	dwHeight = (DWORD)header->d_height;
	BPP = (short)header->d_pixel_size;          // 16, 24, or 32

	// JL TEST SMALL TEXTURES ONLY
//	dwWidth = 2;
//	dwHeight = 2;
    // Create a bitmap to load the data into

	bitsize = dwWidth * dwHeight * (BPP/8);
	if ((newbits = (GLubyte *)calloc(bitsize, 1)) == NULL)
	{
        fclose( file );
        return NULL;
	}
 	buffer = (unsigned char *)malloc(dwWidth*dwHeight*(BPP / 8));
    if ( fread( buffer, dwWidth*dwHeight*(BPP / 8), 1, file ) != 1 )
	{
        fclose( file );
		free(buffer);
		free(newbits);
        return NULL;
	}

	width   = (BPP / 8) * dwWidth;

    for (i = 0; i < dwHeight; i ++)
		for (j = 0, from = ((GLubyte *)buffer) + i * width,
	        to = newbits + i * width;
			j < dwWidth;
			j ++, from += (BPP / 8), to += (BPP / 8))
        {
				if (BPP == 24)
				{
					to[0] = from[2];
					to[1] = from[1];
					to[2] = from[0];
				}
				else
				{
					to[0] = from[2];
					to[1] = from[1];
					to[2] = from[0];
					to[3] = from[3];
				}
        };
	// SINCE TGA IS UPSIDE DOWN, I HAVE TO REVERSE THIS DAMN THING
/*	for (loop = 0; loop < dwHeight; loop++)
	{
		memcpy(	&newbits[loop * dwWidth * (BPP / 8)],
				&buffer[(dwHeight - (loop + 1)) * dwWidth * (BPP / 8)],
				dwWidth * (BPP / 8));
	}
*/
	free(buffer);
//	free(buffer2);
    fclose( file );

    return newbits;
}


/*
 * End of "$Id: bitmap.c,v 1.2 1996/01/09 22:52:53 mike Exp mike $".
 */
