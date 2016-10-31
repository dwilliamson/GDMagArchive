#ifndef INC_IMAGE_H
#define INC_IMAGE_H

typedef unsigned int uint32;

typedef struct st_Image Image;

struct st_Image
{
   int width;
   int height;
   int channels;
   uint32 data[];
};

extern void freeImage(Image *i);
extern Image *imageCreate(int width, int height, int channels);

#endif
