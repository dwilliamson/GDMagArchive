#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>

#include "dmtypes.h"
#include "image.h"


//#define OPENGL_TEXTURE_LOADER
#define LOAD_SBI
#define LOAD_PNG
#define LOAD_JPEG


typedef   signed char   int8;
typedef unsigned char  uint8;

typedef   signed short  int16;
typedef unsigned short uint16;

typedef   signed int    int32;
typedef unsigned int   uint32;

typedef unsigned int   uint;

#ifdef LOAD_SBI
#include "sboxlib.h"
#endif


#ifdef LOAD_PNG
#include "png.h"           // interface to libpng.lib
#endif


#ifdef LOAD_JPEG
#undef FAR
#define INT32_DEFINED
#include "jpeglib.h"       // interface to libjpeg.lib
#endif


/////////////////////////////////////////////////////////////////////////////////////////////
//
// PSD - Photoshop file reader
//
//   modified version of Chris Hecker's PSD reader from Indie Game Jam 0

// PSD file header, everything is bigendian
#pragma pack(1)
typedef struct {
    uint32 Signature;
    uint16 Version;
    uint8  Reserved[6];
    uint16 Channels;
    uint32 Rows;
    uint32 Columns;
    uint16 Depth;
    uint16 Mode;
} PSDFile;

#define SWAP16(w) ((uint16)((((w)>>8)&0xFF)|(((w)<<8)&0xFF00)))
#define SWAP32(w) ((uint32)((((w)>>24)&0xFF)|(((w)<<24)&0xFF000000)|(((w)<<8)&0xFF0000)|(((w)>>8)&0xFF00)))

// followed by chunks (uint32 Length as first field):
// Color mode data section
// Image resources section
// Layer and Mask information section
// uint16 Compression = 0 for raw, = 1 for RLE
// Image data section, one channel at a time
// then bits, scanline order, no pad

Image *imageCreate(int width, int height, int channels)
{
   Image *res = malloc(sizeof(*res) + sizeof(res->data[0]) * width * height);
   res->width = width;
   res->height = height;
   res->channels = channels;
   return res;
}

static Image *loadPSD(void *data, int len, void *param)
{
   Image *res = NULL;

   if(memcmp(data, "8BPS", 4) == 0) {
       // it's a Photoshop PSD file
       uint8 *FileBytes = data;
       PSDFile *Psd = (PSDFile *) FileBytes;
       uint32 Width = SWAP32(Psd->Columns);
       uint32 Height = SWAP32(Psd->Rows);
       int channels = SWAP16(Psd->Channels);

       if((channels == 1 || channels == 3 || channels >= 4) && 
            (SWAP16(Psd->Version) == 1) && 
            (SWAP16(Psd->Depth) == 8))
       {
          uint16 Compression;
          int LineStride;
          int Channel;
          uint32 *data;
          unsigned int i,j;

          res = malloc(sizeof(*res) + sizeof(res->data[0]) * Width * Height);
          res->width = Width;
          res->height = Height;
          res->channels = channels;
          data = res->data;
          memset(data, 0, sizeof(*data) * Width * Height);

          LineStride = (Width);   // walk up the bitmap

          FileBytes += sizeof(PSDFile);
          // skip Color mode
          FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
          // skip Image resources
          FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
          // skip Layers
          FileBytes += SWAP32(*(uint32 *)(FileBytes)) + 4;
          Compression = SWAP16(*(uint16*)FileBytes);
          FileBytes += 2;
          if (channels > 4) channels = 4;
          for (Channel = 0; Channel < channels; ++Channel) {
             int shift = 8 * Channel;
             if(Compression == 0)
             {
                 // uncompressed data

                 // skip to the specified channel
                 uint32 ChannelSize = Width * Height;
                 uint8 *p = FileBytes + ChannelSize * Channel;

                 // rip out the specified channel
                 for(i = 0;i < Height;++i)
                 {
                     for(j = 0;j < Width;++j)
                     {
                         data[j+i*LineStride] |= *p++ << shift;
                     }
                 }
             }
             else
             {
                 uint8 *p;
                 uint32 *d = data;

                 // RLE compressed data
                 // header of uint16 line lengths
                 // n b+
                 // n=[00,7F]: copy n+1 b's
                 // n=[81,FF]: dupe b -n+1 times
                 // n=80: finished/skip

                 // figure out where this channel's scanlines start
                 uint BytesToChannel = 2*Height*Channel;
                 uint LineOffset = 2*Height*SWAP16(Psd->Channels);
                 int BytesToWrite = Width*Height;
                 int LineCount = 0;
                 uint i = 0;
                 while(i < BytesToChannel) {
                     LineOffset += SWAP16(*(uint16*)(FileBytes+i));
                     i += 2;
                 }
                 p = FileBytes + LineOffset;

                 // now we're at Channel
                 while(BytesToWrite > 0) {
                     // decompress run
                     int Count = *(int8*)p;
                     ++p;
                     if(Count >= 0) {
                         // copy
                         Count += 1;
                         BytesToWrite -= Count;
                         LineCount += Count;
                         while(Count--) {
                             *d++ |= *p++ << shift;
                         }
                     } else
                     if(Count > -128) {
                         // run
                         int i;
                         Count = -Count + 1;
                         BytesToWrite -= Count;
                         for (i=0; i < Count; ++i)
                            *d++ |= *p << shift;
                         ++p;
                         LineCount += Count;
                     }
                     // line end?
                     assert((uint32)(LineCount) <= Width);
                     if((uint32)(LineCount) >= Width) {
                         // go back to start of this line and then to next
                         LineCount = 0;
                         d += LineStride - Width;
                     }
                     assert(BytesToWrite >= 0);
                 }
             }
         }
      }
   }

   return res;
} 

void freeImage(Image *i)
{
   free(i);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// BMP - Windows bitmap file reader
//    only 24/32-bit
//

#if 0  // can avoid #including windows.h with this
#pragma pack(1)
typedef struct
{
    uint16  bfType; 
    uint32  bfSize; 
    uint16  bfReserved1; 
    uint16  bfReserved2; 
    uint32  bfOffBits; 
} BITMAPFILEHEADER; 

typedef struct
   uint32  biSize; 
   long    biWidth; 
   long    biHeight; 
   uint16  biPlanes; 
   uint16  biBitCount 
   uint32  biCompression; 
   uint32  biSizeImage; 
   long    biXPelsPerMeter; 
   long    biYPelsPerMeter; 
   uint32  biClrUsed; 
   uint32  biClrImportant; 
} BITMAPINFOHEADER; 
#endif
 
Image *loadBMP(void *data, int len, void *param)
{
   BITMAPFILEHEADER *f = data;
   uint8 *bits = (uint8 *) data + f->bfOffBits;
   BITMAPINFO *g = (void *) (f+1);
   Image *i = NULL;
   int w,h;
   if (g->bmiHeader.biSize == sizeof(g->bmiHeader)) {
      int j;
      // bitmapinfo case
      w = g->bmiHeader.biWidth;
      h = g->bmiHeader.biHeight;
      i = malloc(sizeof(*i) + w*h*sizeof(i->data[0]));
      i->channels = 4;
      i->width = w;
      i->height = h;
      assert(g->bmiHeader.biCompression == BI_RGB);
      if (g->bmiHeader.biBitCount == 24) {
         for (j=0; j < w*h; ++j) {
            i->data[j] = bits[j*3+2] + 256*bits[j*3+1] + 256*256*bits[j*3+0];
         }
      } else {
         assert(g->bmiHeader.biBitCount == 32);
         for (j=0; j < w*h; ++j) {
            i->data[j] = bits[j*4+2] + 256*bits[j*4+1] + 256*256*bits[j*4+0];
         }
      }
   } else {
      assert(0);
   }
   return i;
}

#ifdef LOAD_SBI
/////////////////////////////////////////////////////////////////////////////////////////////
//
// SBI - Sean's bitmap image format
//
// sbi is an image format with embedded mipmaps... however, they don't
// get loaded here, since this is an image interface not a texture interface

Image *loadSBI(FILE *f, int len)
{
   int32 *buf, w, h;
   uint8 *data;
   int pal=0;
   Image *im;
   SboxHandle *sbox;
   if (SboxReadOpenFromFile(&sbox, f, FALSE, "sB0xImageFormat"))
      return NULL;
   if (SboxkitFindString(sbox, "888 image") == SBOXKIT_NOTFOUND) { 
      if (SboxkitFindString(sbox, "palette 8->24") == SBOXKIT_NOTFOUND) { SboxReadClose(sbox); return NULL; }
      pal = 1;
   }
   // image header
   if (SboxkitGetByString(&buf, sbox, "size") != 8) { SboxReadClose(sbox); return NULL; }
   w = buf[0];
   h = buf[1];
   im = malloc(sizeof(*im) + w*h*sizeof(im->data[0]));
   im->width = w;
   im->height = h;
   // image data
   if (!pal) {
      uint8 *pixels;
      int i;
      if (SboxkitGetByString(&data, sbox, "888 image") == 0) { SboxReadClose(sbox); return NULL; }
      pixels = (void *) im->data;
      for (i=0; i < w*h; ++i) {
         pixels[i*4+0] = data[i*3+0];
         pixels[i*4+1] = data[i*3+1];
         pixels[i*4+2] = data[i*3+2];
         pixels[i*4+3] = 255;
      }
   } else {
      uint8 palette[768], *pixels;
      int i,j;
      if (SboxkitGetByString(&data, sbox, "palette 8->24") != 768) { SboxReadClose(sbox); return NULL; }
      memcpy(palette, data, 768);
      if (SboxkitGetByString(&data, sbox, "8-bit paletted image") == 0) { SboxReadClose(sbox); return NULL; }
      pixels = (void *) im->data;
      for (i=0; i < w*h; ++i) {
         for (j=0; j < 3; ++j)
            pixels[i*4+j] = palette[data[i]*3+j];
         pixels[i*4+3] = 255;
      }
   }
   SboxReadClose(sbox);
   return im;
}
#endif


#ifdef LOAD_JPEG
/////////////////////////////////////////////////////////////////////////////////////////////
//
// JPG - JPEG file reader using Independent Jpeg Group's library
//

static Image *loadJPEG(FILE *f, int len)
{
   Image *m = NULL;
   uint8 *buffer;
   uint32 *data;

   struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

   m = malloc(sizeof(*m) + sizeof(m->data[0]) * cinfo.output_width * cinfo.output_height);
   m->width = cinfo.output_width;
   m->height = cinfo.output_height;

   buffer = malloc(m->width * 3);

   data = m->data;
   while (cinfo.output_scanline < cinfo.output_height) {
      uint i;
      jpeg_read_scanlines(&cinfo, &buffer, 1);
      for (i=0; i < cinfo.output_width; ++i)
         *data++ = (buffer[i*3+2] << 16) + (buffer[i*3+1] << 8) + buffer[i*3];
   }

   free(buffer);

   return m;
}
#endif

#ifdef LOAD_PNG
/////////////////////////////////////////////////////////////////////////////////////////////
//
// PNG - Portable Network Graphics using PNG reference implementation
//

#define PNG_BYTES_TO_CHECK 4
static int check_if_png(char *file_name, FILE **fp)
{
   if ((*fp = fopen(file_name, "rb")) == NULL)
      return 0;
}

static Image *loadPNG(FILE *f, int len)
{
   char buf[PNG_BYTES_TO_CHECK];
   Image *b;
   png_structp png_ptr;
   png_infop info_ptr;
   uint32 width, height,channels;
   png_bytep *row_pointers;
   uint x,y;

   if (fread(buf, 1, PNG_BYTES_TO_CHECK, f) != PNG_BYTES_TO_CHECK)
      return 0;

   if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK))
      return 0;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
   if (png_ptr == NULL) {
      return NULL;
   }
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return NULL;
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      return NULL;
   }

   png_init_io(png_ptr, f);
   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);
   width    = info_ptr->width;
   height   = info_ptr->height;
   channels = info_ptr->channels;

   row_pointers = png_get_rows(png_ptr, info_ptr);
   b = malloc(sizeof(*b) + width * height * 4);
   b->width = width;
   b->height = height;
   b->channels = channels;

   for (y=0; y < height; ++y) {
      uint8 *p = row_pointers[y];
      uint32 *q = b->data + y * width;
      if (channels == 3) {
         for (x=0; x < width; ++x, p += 3)
            q[x] = (p[0]) + (p[1] << 8) + (p[2] << 16);
      } else {
         for (x=0; x < width; ++x, p += 4)
            q[x] = (p[0]) + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
      }
   }

   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
   return b;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
// IMC - hacked IMage Cluster format
//   an IMC is a text file with 16 file names
//   the IMC loader automatically builds a single image with those 16 files
//       laid out in a 4x4 grid (only works if they're the same size)
//
//   this is just a hack I made to turn a set of 128x128 textures into
//   512x512 textures without using photoshop, since I didn't have much
//   photoshop mojo at the time and I had a lot of textures like this I
//   wanted to use
//
//   This was written before I wrote the string loader; I would switch
//   to using that now instead if I rewrote it.

static Image *loadCluster(void *raw, int len, void *param)
{
   uint8 *data = raw;
   // a cluster is a 4x4 grid of filenames; we assemble a meta-texture
   // from this fixed arrangement

   Image *grid[4][4], *out;
   char *files[4][4];
   int i,j;
   char *p = data, *q;
   data[len] = 0; // data manager promises us space for this sentinel
   memset(files, 0, sizeof(files));
   memset(grid, 0, sizeof(grid));
   for (i=0; i < 16; ++i) {
      q = strchr(p, '\n');
      if (q) {
         if (q[-1] == '\r') --q;
         *q = 0;
         files[0][i] = p;
         p = q+1;
         while (*p == '\n' || *p == '\r') {
            ++p;
         }
      }
   }
   if (i != 16) return NULL;
   for (i=0; i < 16; ++i) {
      grid[0][i] = getImage(files[0][i]);
      if (grid[0][i] == NULL) break;
   }
   out = NULL;
   if (i == 16) {
      for (i=1; i < 16; ++i)
         if (grid[0][i]->width != grid[0][0]->width || grid[0][i]->height != grid[0][i]->height)
            break;
      if (i == 16) {
         int w = grid[0][0]->width;
         int h = grid[0][0]->height;
         out = malloc(sizeof(*out) + 16*w*h*sizeof(out->data[0]));
         out->width = 4*w;
         out->height = 4*h;
         out->channels = grid[0][0]->channels;
         for (j=0; j < 4; ++j) {
            for (i=0; i < 4; ++i) {
               int k;
               for (k=0; k < h; ++k)
                  memcpy(&out->data[(j*h + k)*4*w + i*w],
                         &grid[j][i]->data[k*w],
                         sizeof(out->data[0])*w);

            }
         }
      }
   }
   for (i=0; i < 16; ++i)
      if (grid[i])
         dmFree(files[0][i], DT_IMAGE);
   return out;
}

#ifdef OPENGL_TEXTURE_LOADER

/////////////////////////////////////////////////////////////////////////////////////////////
//
// OpenGL texture
//
//   This is a dependent loader which turns an image into a texture.
//   If you try to load a texture with a name for which there is an
//   image file, the image file will be loaded, and then the converter.
//
//   The data manager will automatically hot load the texture if the
//   image is changed on disk.

#include <gl/gl.h>
#include <gl/glu.h>

static GLuint createTex(int w, int h, uint32 *data)
{
   GLuint x, code;
   glGenTextures(1, &x);
   glBindTexture(GL_TEXTURE_2D, x);
   code = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, w,h, GL_RGBA, GL_UNSIGNED_BYTE, data);
   assert(code == 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   return x;
}

static void *convertImageToTexture(void *p, int size, void *params)
{
   GLuint tex;
   Image *im = p;
   tex = createTex(im->width, im->height, im->data);
   #ifdef GL_EXTENSIONS_AVAILABLE
   if (params) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }
   #endif
   return (void *) tex;
}

static void freeTexture(void *p)
{
   GLuint tex = (GLuint) p;
   glDeleteTextures(1, &tex);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  The remainder of this file (except the final loader-installing routine)
//  is devoted to code which does post-processing of images on load. I wrote
//  this before I'd discovered Photoshop's adjustment layers, which allow for
//  this kind of non-destructive editing. On the other hand, it's still pretty
//  cool, and the reload cycle is sometimes actually better than in Photoshop.
//  Even though I hotload textures, if you, say, bring up the PS levels control
//  and start playing with it with preview on, you can't just hit ctrl-S to save
//  and view it in-game; you have to exit the levels control, hit ctrl-S, then
//  go double-click the levels adjustment layer to tweak it again.
//
//  Also, this system lets me reuse the same art, recolored and desaturated and
//  such, in a pretty trivial way. This was interesting to me several years ago
//  when I wanted to keep download size small.

#define lerp(x,a,b)   ((x) < (a) ? 0 : (x) > (b) ? 1 : ((x)-(a))/(float)((b)-(a)))

static int french_alpha(int r, int g, int b)
{
   return (int) (255-255*lerp(r, g*0.75, g*1.25));
};

void imageSetAlpha(int w, int h, uint32 *data, int (*f)(int,int,int), int alpha)
{
   uint8 *pixels = (uint8 *) data;
   int i;
   for (i=0; i < w*h; ++i) {
      if (f)
         pixels[i*4+3] = f(pixels[i*4+0], pixels[i*4+1], pixels[i*4+2]);
      else
         pixels[i*4+3] = alpha;
   }
}

static int (*afuncs[])(int,int,int) =
{
   NULL,
   french_alpha,
};

void imageSetAlphaFunc(Image *im, int func)
{
   if (func > 0 && func < sizeof(afuncs)/sizeof(afuncs[0])) {
      imageSetAlpha(im->width, im->height, im->data, afuncs[func], 0);
   }
}

#ifndef ftoi
#define ftoi(x)   ((int) (x))
#endif

void imageDesaturate(int w, int h, uint32 *data, float desaturate)
{
   uint8 *pixels = (uint8 *) data;
   int i,j,z;
   for (i=0; i < w*h; ++i) {
      z = (pixels[i*4+0] + 2*pixels[i*4+1] + pixels[i*4+2]) >> 2;
      for (j=0; j < 3; ++j)
         pixels[i*4+j] = ftoi(pixels[i*4+j] * (1-desaturate) + z * desaturate);
   }
}

void imageLighten(int w, int h, uint32 *data, float strength)
{
   uint8 *pixels = (uint8 *) data;
   float alpha = 1-strength;
   int i,j;
   for (i=0; i < w*h; ++i) {
      for (j=0; j < 3; ++j)
         pixels[i*4+j] = (int) (255 - (255 - pixels[i*4+j])*alpha);
   }
}

void imageBrighten(int w, int h, uint32 *data, float strength)
{
   uint8 *pixels = (uint8 *) data;
   int i;
   for (i=0; i < w*h; ++i) {
      int r = (int) (pixels[i*4+0] * strength);
      int g = (int) (pixels[i*4+1] * strength);
      int b = (int) (pixels[i*4+2] * strength);
      if (r > 255 || b > 255 || g > 255) {
         int m = (r > b ? r : b);
         m = (m > g ? m : g);
         r = r * 255 / m;
         g = g * 255 / m;
         b = b * 255 / m;
      }
      assert(r >= 0 && r < 256);
      assert(g >= 0 && g < 256);
      assert(b >= 0 && b < 256);
      pixels[i*4+0] = r;
      pixels[i*4+1] = g;
      pixels[i*4+2] = b;
   }
}

void imageReduceContrast(int w, int h, uint32 *data, float icontrast)
{
   uint8 *pixels = (uint8 *) data;
   int i;
   float lum=0;
   for (i=0; i < w*h; ++i) {
      lum += (pixels[i*4+0] + 2*pixels[i*4+1] + pixels[i*4+2]) >> 2;
   }
   // reduce contrast by moving all pixels towards the average luminosity
   lum /= (w*h);
   // lum = average luminosity
   for (i=0; i < w*h; ++i) {
      pixels[i*4+0] = ftoi(pixels[i*4+0] * (1-icontrast) + lum*icontrast);
      pixels[i*4+1] = ftoi(pixels[i*4+1] * (1-icontrast) + lum*icontrast);
      pixels[i*4+2] = ftoi(pixels[i*4+2] * (1-icontrast) + lum*icontrast);
   }
}

void imageRecolor(Image *im, float rs, float gs, float bs)
{
   int i;
   uint8 *data = (uint8 *) im->data;
   if (rs <= 1 && gs <= 1 && bs <= 1) {
      for (i=0; i < im->width * im->height; ++i) {
         data[i*4+0] = ftoi(rs * data[i*4+0]);
         data[i*4+1] = ftoi(gs * data[i*4+1]);
         data[i*4+2] = ftoi(bs * data[i*4+2]);
      }
   } else {
      for (i=0; i < im->width * im->height; ++i) {
         int r = ftoi(rs * data[i*4+0]);
         int g = ftoi(gs * data[i*4+1]);
         int b = ftoi(bs * data[i*4+2]);
         data[i*4+0] = r > 255 ? 255 : r;
         data[i*4+1] = g > 255 ? 255 : g;
         data[i*4+2] = b > 255 ? 255 : b;
      }
   }
}

static Image *loadRecolored(char *name)
{
   Image *im;
   char realname[128];

   // parse out the real asset name
   char *r = strchr(name, ' ');

   if (r == NULL) return NULL;

   // can't stop a \0 into the passed-in name, so copy
   strncpy(realname, name, r-name);
   realname[r-name] = 0;
   ++r;

   // load without cacheing... ideally this should be
   // something like "load cached, but eject from cache first"
   im = dmUncachedLoad(realname, DT_IMAGE, NULL);
   if (im == NULL)
      return NULL;

   while (*r) {
      int i0;
      float p0,p1,p2;
      char *s = strchr(r, ' ');
      if (s != r) {
         if (sscanf(r, "recolor:%f,%f,%f", &p0,&p1,&p2) == 3)
            imageRecolor(im, p0,p1,p2);
         else if (sscanf(r, "desat:%f", &p0) == 1)
            imageDesaturate(im->width, im->height, im->data, p0);
         else if (sscanf(r, "alpha:%f", &p0) == 1)
            imageSetAlpha(im->width, im->height, im->data, NULL, (int) (255*p0));
         else if (sscanf(r, "contrast:%f", &p0) == 1)
            imageReduceContrast(im->width, im->height, im->data, p0);
         else if (sscanf(r, "alphafunc:%d", &i0) == 1)
            imageSetAlphaFunc(im, i0);
         else if (sscanf(r, "brighten:%f", &p0) == 1)
            imageBrighten(im->width, im->height, im->data, p0);
         else if (sscanf(r, "lighten:%f", &p0) == 1)
            imageLighten(im->width, im->height, im->data, p0);
         else { /* @TODO: print error!!! */ }
      }

      if (s == NULL) break;
      r = s+1;
   }

   return im;
}

void imageInit(void)
{
   DM_REGISTER(DT_IMAGE, "psd" , NULL,  loadPSD, freeImage);
   DM_REGISTER(DT_IMAGE, "bmp" , NULL,  loadBMP, freeImage);

   #ifdef LOAD_JPEG
   DM_REGISTER(DT_IMAGE, "jpg" , loadJPEG, NULL, freeImage);
   DM_REGISTER(DT_IMAGE, "jpeg", loadJPEG, NULL, freeImage);
   #endif

   #ifdef LOAD_SBI
   DM_REGISTER(DT_IMAGE, "sbi" , loadSBI,  NULL, freeImage);
   #endif

   #ifdef LOAD_PNG
   DM_REGISTER(DT_IMAGE, "png" , loadPNG,  NULL, freeImage);
   #endif

   DM_REGISTER(DT_IMAGE, "imc" , NULL,  loadCluster, freeImage);

   DM_REGISTER_NAMED(DT_IMAGE, loadRecolored, NULL, freeImage);

   #ifdef OPENGL_TEXTURE_LOADER
   dmRegisterFormatHandler(DT_TEX, DT_IMAGE, NULL, NULL, NULL, convertImageToTexture, freeTexture);
   #endif
}
