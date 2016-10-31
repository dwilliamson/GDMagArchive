#ifndef INCLUDE_DEMO_H
#define INCLUDE_DEMO_H


int force_draw;

typedef int bool;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned int   uint32;
typedef          int    int32;
typedef unsigned short uint16;
typedef          short  int16;
typedef unsigned char  uint8;
typedef   signed char   int8;


extern void screenInit(int w, int h);
extern void demoResizeViewport(int w, int h);
extern int  demoProcessCharacter(int ch);
extern int  demoRunLoopmode(float tm);
extern void demoInit(void);
extern void demoDraw(void);

extern void gameMouse(int event, int x, int y, int shift, int control);

#endif
