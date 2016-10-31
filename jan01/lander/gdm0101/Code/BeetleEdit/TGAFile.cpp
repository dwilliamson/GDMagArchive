/*
 * TGAFile.c
 *
 */

/*
 * Include necessary headers.
 */

#include "stdafx.h"
#include "TGAFile.h"


///////////////////////////////////////////////////////////////////////////////
// This stuff forward is the TGA Loading Routines
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Name: LoadTGAFile()
// Desc: Given a filename of a TGA file, it loads the image in TGA form
//-----------------------------------------------------------------------
unsigned char *LoadTGAFile( char * strFilename,	tTGAHeader_s *header)
{
/// Local Variables ///////////////////////////////////////////////////////////
	short			BPP;
	unsigned char	*buffer;
	int bitsize;		/* Total size of bitmap */
	BYTE	*newbits;		/* New RGB bits */
	BYTE	*from, *to;		/* RGB looping vars */
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

    // Create a bitmap to load the data into

	bitsize = dwWidth * dwHeight * (BPP/8);
	if ((newbits = (BYTE *)calloc(bitsize, 1)) == NULL)
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
		for (j = 0, from = ((BYTE *)buffer) + i * width,
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

	free(buffer);
//	free(buffer2);
    fclose( file );

    return newbits;
}

//-----------------------------------------------------------------------
// Name: SaveTGAFile()
// Desc: Given a filename of a TGA file, it saves the file in TGA format
//-----------------------------------------------------------------------
void SaveTGAFile( char * strFilename,	int width, int height, int bpp, unsigned char *data)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int stepwidth;
	BYTE	*buffer;		/* New RGB bits */
	BYTE	*from, *to;		/* RGB looping vars */
	int		i, j;			/* Looping vars */
    FILE* file;
	tTGAHeader_s header;
///////////////////////////////////////////////////////////////////////////////

	header.d_iif_size = 0;
	header.d_cmap_type = 0;
	header.d_image_type = 2;
	header.pad[0] = 0;
	header.pad[1] = 0;
	header.pad[2] = 0;
	header.pad[3] = 0;
	header.pad[4] = 0;
	header.d_x_origin = 0;
	header.d_y_origin = 0;
	header.d_width = width;
	header.d_height = height;
	header.d_pixel_size = bpp * 8;
	header.d_image_descriptor = 0;

    // Open the file and write the header
	file = fopen( strFilename, TEXT("wb") );
    if( NULL == file )
        return;

    if ( fwrite( &header, sizeof( tTGAHeader_s ), 1, file ) != 1 )
    {
        fclose( file );
        return;
    }

    // Write the data
 	buffer = (unsigned char *)malloc(width*height*bpp);
	stepwidth   = bpp * width;

    for (i = 0; i < height; i ++)
		for (j = 0, from = (data) + i * stepwidth,
	        to = buffer + i * stepwidth;
			j < width;
			j ++, from += bpp, to += bpp)
        {
				if (bpp == 3)
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

    if ( fwrite( buffer, width*height*bpp, 1, file ) != 1 )
	{
        fclose( file );
		free(buffer);
        return;
	}

    fclose( file );
	free(buffer);

    return;
}



