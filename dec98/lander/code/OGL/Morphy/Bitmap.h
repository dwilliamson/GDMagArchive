/*
 * "$Id: bitmap.h,v 1.2 1996/01/09 22:52:53 mike Exp mike $"
 *
 *   Device Independent Bitmap definitions for OpenGL under MS Windows.
 *
 * Revision History:
 *
 *   $Log: bitmap.h,v $
 *   Revision 1.2  1996/01/09  22:52:53  mike
 *   Added PrintDIBitmap.
 *
 *   Revision 1.1  1995/12/31  07:26:54  mike
 *   Initial revision
 */

/*
 * Include necessary headers.
 */

/*
 * Make this header file work with C and C++ source code...
 */
#include <GL/gl.h>
#include <GL/glu.h>

typedef struct
{
   unsigned char  d_iif_size;            // IIF size (after header), usually 0
   unsigned char  d_cmap_type;           // ignored
   unsigned char  d_image_type;          // should be 2
   unsigned char  pad[5];

   unsigned short d_x_origin;
   unsigned short d_y_origin;
   unsigned short d_width;
   unsigned short d_height;

   unsigned char  d_pixel_size;          // 16, 24, or 32
   unsigned char  d_image_descriptor;    // Bits 3-0: size of alpha channel
                                         // Bit 4: must be 0 (reserved)
                                         // Bit 5: should be 0 (origin)
                                         // Bits 6-7: should be 0 (interleaving)
} tTGAHeader_s;

extern void	*LoadDIBitmap(char *filename, BITMAPINFO **info);
extern int	SaveDIBitmap(char *filename, BITMAPINFO *info, void *bits);
extern void	*ReadDIBitmap(BITMAPINFO **info);
extern int	PrintDIBitmap(HWND owner, BITMAPINFO *info, void *bits);
GLubyte *LoadTGAFile( TCHAR* strFilename,tTGAHeader_s *header);

extern GLubyte	*ConvertRGB(BITMAPINFO *info, void *bits);

/*
 * End of "$Id: bitmap.h,v 1.2 1996/01/09 22:52:53 mike Exp mike $".
 */
