#define BI_RLE8         1

struct bmp_header {
  int    type;
  long   size;
  int    reserved1;
  int    reserved2;
  long   offBits;
};

struct bmp_info {
  long   size;
  long   width;
  long   height;
  int    planes;
  int    bitCount;
  long   compression;
  long   sizeImage;
  long   xPelsPerMeter;
  long   yPelsPerMeter;
  long   clrUsed;
  long   clrImportant;
};

struct rgb_quad {
    char blue;
    char green;
    char red;
};

