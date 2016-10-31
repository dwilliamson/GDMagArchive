#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "bmp_internal.h"

#define MAX_BITMAP_SIZE (640 * 480)

inline int ABS(int i) {
    if (i < 0) return -i;
    return i;
}


struct close_on_return {
    close_on_return(FILE *data);
    ~close_on_return();

    FILE *close_me;
};

inline close_on_return::close_on_return(FILE *data) {
    close_me = data;
}

inline close_on_return::~close_on_return() {
    if (close_me) fclose(close_me);
}


unsigned char fread_1b(FILE *f, bool *ok){
  int r=99;

  if(f==NULL){
    r = EOF;
  }else{
    r = fgetc(f);
  }
  
  if(ok!=NULL && *ok!=false){
    if(r==EOF)   *ok = false;
    else         *ok = true;
  }

  return ((unsigned char)r);
}

int fread_2b(FILE *f, bool *ok){
  short  s_res=99;
  int    err,res;
  short  r1,r2;

  if(f==NULL){
    err = EOF;
  }else{

    r1 = (short) fgetc(f);
    err =        fgetc(f);

    r2 = (short) err;

    s_res =                r2;
    s_res = (s_res << 8) + r1;
  }

  if(ok!=NULL && *ok!=false){
    if(err==EOF) *ok = false;
    else         *ok = true;
  }

  res = s_res;
  return(res);
}

long fread_4b(FILE *f, bool *ok){
  long  res=99;
  int   r1,r2,r3,r4;

  if(f==NULL){
    r4 = EOF;
  }else{
    r1 = fgetc(f);
    r2 = fgetc(f);
    r3 = fgetc(f);
    r4 = fgetc(f);

    res =              (unsigned char) r4;
    res = (res << 8) | (unsigned char) r3;
    res = (res << 8) | (unsigned char) r2;
    res = (res << 8) | (unsigned char) r1;
  }

  if(ok!=NULL && *ok!=false){
    if(r4==EOF) *ok = false;
    else        *ok = true;
  }

  return(res);
}



static bool load_bmp_header(FILE *bmp_file, bmp_header *hdr){
  bool ok=true;
  
  hdr->type      = fread_2b(bmp_file,&ok); 
  hdr->size      = fread_4b(bmp_file,&ok); 
  hdr->reserved1 = fread_2b(bmp_file,&ok); 
  hdr->reserved2 = fread_2b(bmp_file,&ok); 
  hdr->offBits   = fread_4b(bmp_file,&ok);
  
  return(ok);
}

static bool load_bmp_info(FILE *bmp_file, bmp_info *inf){
  bool ok=true;

  inf->size          = fread_4b(bmp_file,&ok);
  inf->width         = fread_4b(bmp_file,&ok);
  inf->height        = fread_4b(bmp_file,&ok);
  inf->planes        = fread_2b(bmp_file,&ok);
  inf->bitCount      = fread_2b(bmp_file,&ok);
  inf->compression   = fread_4b(bmp_file,&ok);
  inf->sizeImage     = fread_4b(bmp_file,&ok);
  inf->xPelsPerMeter = fread_4b(bmp_file,&ok);
  inf->yPelsPerMeter = fread_4b(bmp_file,&ok);
  inf->clrUsed       = fread_4b(bmp_file,&ok);
  inf->clrImportant  = fread_4b(bmp_file,&ok);

  if (inf->clrUsed == 0) {
      switch (inf->bitCount) {
        case 1:
	  inf->clrUsed = 2;
	  break;
        case 4:
	  inf->clrUsed = 16;
	  break;
        case 8:
	  inf->clrUsed = 256;
	  break;
      }
  }

  return(ok);
}

static bool load_rgb(FILE *bmp_file, rgb_quad *quad){
    bool ok = true;

    quad->blue  = fread_1b(bmp_file, &ok);
    quad->green = fread_1b(bmp_file, &ok);
    quad->red   = fread_1b(bmp_file, &ok);

    fread_1b(bmp_file, &ok);  // unused

    return ok;		
}

bool read_24bit_line(FILE *bmp_file, char *loc, int width) {
    char color;
    int nchars = ((width * 3 + 3) / 4) * 4;
    bool ok = true;

    int i;
    for (i = 0; i < width; i++) {
	char red, green, blue;

	blue = fread_1b(bmp_file, &ok);
	green = fread_1b(bmp_file, &ok);
	red = fread_1b(bmp_file, &ok);

	*(loc + 0) = red;
	*(loc + 1) = green;
	*(loc + 2) = blue;

	loc += 3;
	nchars -= 3;
    }

    while (nchars-- > 0) fread_1b(bmp_file, &ok);

    return ok;
}

bool read_8bit_line(FILE *bmp_file, char *loc, int width) {
    char color;
    bool ok=true;
    int  nchars = ((width + 3) / 4) * 4;

    int i;
    for (i = 0; i < width; i++) {

        color = fread_1b(bmp_file, &ok);
	*loc = color;

        loc++;
    }

    while (i < nchars) {
	fread_1b(bmp_file,&ok);
	i++;
    }

    return ok;
}

bool read_4bit_line(FILE *bmp_file, char *loc, int width) {
    int nchars = ((((width + 1) / 2) + 3) / 4) * 4;
    bool ok = true;
    int i;
    char color, value;

    for (i = 0; i < width; i++) {
        if (i & 0x1) {
            color = value & 0xf;
        } else {
            value = fread_1b(bmp_file, &ok);
	    if (!ok) return false;
            color = (value >> 4) & 0xf;
        }

	*loc++ = color;
    }

    while (i < nchars){
      fread_1b(bmp_file, &ok);
      i++;
    }

    return ok;
}

bool read_1bit_line(FILE *bmp_file, char *loc, int width) {
    int nchars = ((((width + 7) / 8) + 3) / 4) * 4;

    int i;
    char color, value;
    bool ok = true;	

    while (width > 0) {
	nchars--;
	int x = fread_1b(bmp_file, &ok);
	int m = 0x80;

	while ((width > 0) && (m != 0)) {
	    unsigned char value = (x & m) ? 1 : 0;
	    *loc = value;

	    loc++;

	    m >>= 1;
	    width--;
	}
    }

    while (nchars) {
	fread_1b(bmp_file, &ok);
	nchars--;
    }

    return ok;
}

static char *load_bmp(FILE *bmp_file,
		      bmp_header *header, bmp_info *info) {

    int  res;
    char *loc;
    bool ok;
    int width,height,x,y;

    char *memory;

    res = fseek(bmp_file, header->offBits, SEEK_SET);
    if (res != 0) return NULL;

    width  = ABS(info->width);
    height = ABS(info->height);

    switch (info->bitCount) {
      case 1:
      case 4:
	assert(0);
	break;
      case 8:
	memory = (char *)malloc(width * height);
	loc = memory + (width * (height - 1));
	break;
      case 24:
	memory = (char *)malloc(width * height * 3);
	loc = memory + (width * (height - 1)) * 3;
	break;
      default:
	assert(0);
    }

    switch (info->bitCount) {
      case 1:
        for (y = 0; y < height; y++) {
	    ok = read_1bit_line(bmp_file, loc, width);
	    loc -= width;
	    if (!ok) goto fail;
	}

	break;
      case 4:
        for (y = 0; y < height; y++){
	    ok = read_4bit_line(bmp_file, loc, width);
	    loc -= width;
	    if (!ok) goto fail;
	}

	break;
      case 8:
        for (y = 0; y < height; y++) {
            ok = read_8bit_line(bmp_file, loc, width);
            loc -= width;
	    if (!ok) goto fail;
        }

	break;
      case 24:
        for (y = 0; y < height; y++) {
            ok = read_24bit_line(bmp_file, loc, width);
            loc -= width * 3;
	    if (!ok) goto fail;
        }

	break;
      default:
	assert(0);
    }

    if (!ok) goto fail;

    return memory;

  fail:

    if (memory) free(memory);
    return NULL;
}


bool load_bmp_palette(char *filename, char *dest_palette) {
    bmp_header header;
    bmp_info   info;
    FILE      *bmp_file;
    int i;

    bool ok = true;

    bmp_file = fopen(filename, "rb");
    if (bmp_file == NULL) return false;

    ok = load_bmp_header(bmp_file, &header);
    if (!ok) goto end_load;

    ok = load_bmp_info(bmp_file, &info);
    if (!ok) goto end_load;

    if (info.bitCount != 8) return false;
    if (info.clrUsed > 256) return false;

    for (i = 0; i < info.clrUsed; i++) {
        rgb_quad quad;
        load_rgb(bmp_file, &quad);
	
	dest_palette[i * 3 + 0] = quad.red;
	dest_palette[i * 3 + 1] = quad.green;
	dest_palette[i * 3 + 2] = quad.blue;
    }

    fclose(bmp_file);
    return true;

  end_load:
    fclose(bmp_file);
    return false;
}

bool is_valid_resolution(int bit_count) {
    switch (bit_count) {
      case 1:
	return true;
      case 2:
	return true;
      case 4:
	return true;
      case 8:
	return true;
      case 24:
	return true;
      default:
	return false;
    }
}

bool load_bmp_file(char *file_name, 
		   unsigned char **bit_map, 
		   int *x_size, int *y_size) {

    bmp_header header;
    bmp_info   info;
    long       bitmap_size;
    int        i,x,y;
    char      *mem;
    rgb_quad  *quad_array;
    bool       ok;
    FILE      *bmp_file;

    ok  = true;
    mem = NULL;
    quad_array = NULL;

    bmp_file = fopen(file_name,"rb");
    if (bmp_file == NULL) return false;
    close_on_return a(bmp_file);

    ok = load_bmp_header(bmp_file, &header);
    if (!ok) return false;

    ok = load_bmp_info(bmp_file, &info);
    if (!ok) return false;

    if (!is_valid_resolution(info.bitCount)) {
	fprintf(stderr, "Bitmap has invalid bitcount (it's %d).\n", info.bitCount);
	return false;
    }

    x = ABS(info.width);
    y = ABS(info.height);

    bitmap_size = x * y;

    if (bitmap_size > MAX_BITMAP_SIZE) {
	fprintf(stderr, "Bitmap larger than 640 x 480.... what the hell?\n");
	return false;
    }

    if (info.bitCount != 24) {
	assert(info.clrUsed != 0);
  
	quad_array = new rgb_quad[info.clrUsed];

	for (i = 0; i < info.clrUsed; i++) {
	    load_rgb(bmp_file, &quad_array[i]);
	}
    }

    mem = load_bmp(bmp_file, &header, &info);
    assert(mem != NULL);

    char *product;
    const int bpp = 3;
    int product_bytes = bitmap_size * bpp;
    product = (char *)malloc(product_bytes);

    if (info.bitCount == 24) {
        memcpy(product, mem, product_bytes);
    } else {
        assert(0);
        memcpy(product, mem, bitmap_size);
    }

    *x_size  = x;
    *y_size  = y;
    *bit_map = (unsigned char *)product;

    if (quad_array) delete [] quad_array;

    free(mem);
    return ok;
}



