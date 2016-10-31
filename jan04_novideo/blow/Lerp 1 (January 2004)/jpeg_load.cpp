#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "jpeg_load.h"

extern "C" {
#include <jpeglib.h>
}

const J_DITHER_MODE DITHER_MODE = JDITHER_FS;
//const J_DITHER_MODE DITHER_MODE = JDITHER_NONE;
//const J_DITHER_MODE DITHER_MODE = JDITHER_ORDERED;

const bool TWO_PASS_QUANTIZE = TRUE;
//const bool TWO_PASS_QUANTIZE = FALSE;

bool load_jpeg_file(char *filename, unsigned char **bitmap_result, 
                    int *width_result, int *height_result) {

    FILE *f = fopen(filename, "rb");
    if (f == NULL) return false;

    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);

    cinfo.dither_mode = DITHER_MODE;
    cinfo.two_pass_quantize = TWO_PASS_QUANTIZE;

    jpeg_start_decompress(&cinfo);

    int width = cinfo.image_width;
    int height = cinfo.image_height;
    int bpp = cinfo.num_components;

    JSAMPARRAY buffer;
    JDIMENSION buffer_height;

    char *data = (char *)malloc(width * height * bpp);
//    char *pos = data + width * height * bpp;
    char *pos = data;
    buffer_height = height;

    bool success = true;

    // XXX Maybe this will be faster if we read more than one
    // scanline at a time (requires setting up array of char **).
    while (cinfo.output_scanline < cinfo.output_height) {
        buffer = (unsigned char **)&pos;
        int num_scanlines = jpeg_read_scanlines(&cinfo, buffer, 1);
        pos += width * bpp;

        if (num_scanlines == 0) {
            success = false;
            break;
        }

        buffer_height -= num_scanlines;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(f);

    if (jerr.num_warnings) success = false;
    if (bpp != 3) success = false;

    if (!success) {
        free(data);
        return false;
    }

    *width_result = width;
    *height_result = height;
    *bitmap_result = (unsigned char *)data;

    return true;
}
