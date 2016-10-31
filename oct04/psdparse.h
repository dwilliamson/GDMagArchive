typedef unsigned int uint32;

typedef struct
{
   int w;
   int h;
   uint32 data[1];
} PSDbitmap;

extern PSDbitmap *loadPSD( char *FileName);
