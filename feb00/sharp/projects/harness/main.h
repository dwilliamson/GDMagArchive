//
//  Project driver file.
//
//      Brian Sharp, 6/2/98
//      bsharp@acm.org
//
//  This file contains the prototypes for the basic 
//  driver setup for all my 3D OpenGL-based personal
//  projects. The app* functions are not defined by 
//  the driver; the application must implement them.
//

// Used for initialization.
void init();
void appInit(int, char**);

// Used to handle keystrokes.
void key(unsigned char, int, int);
void appKey(unsigned char, int, int);

// Used when drawing each frame.
void draw();
void appDraw();

// It's main.
int main(int, char**);

// This is used so that the application can choose its own window title.
const char* appWindowName();

// This is used to let the application clean up before dying.
void appCleanup();