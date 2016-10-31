/*
 * "$Id: loadtex.c,v 1.2 1996/01/09 22:52:53 mike Exp mike $"
 *
 */

/*
 * Include necessary headers.
 */

#include "stdafx.h"
#include "loadtex.h"


///////////////////////////////////////////////////////////////////////////////
// This stuff forward is the TGA Loading Routines
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Name: LoadTGAFile()
// Desc: Given a filename of a TGA file, it loads the image in BITMAP form
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

	// JL TEST SMALL TEXTURES ONLY
//	dwWidth = 2;
//	dwHeight = 2;
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
					to[0] = from[0];
					to[1] = from[1];
					to[2] = from[2];
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


