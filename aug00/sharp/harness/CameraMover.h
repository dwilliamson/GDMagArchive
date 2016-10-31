#ifndef CAMERAMOVER_H
#define CAMERAMOVER_H

#include <harness/CameraPersona.h>

class CameraMover
{
public:
  static void mouse (int button, int state, int x, int y);
  static void motion(int x, int y);
  static void keyboard(unsigned char key, int x, int y);
  static void special(int key, int x, int y);

  static void update(float dt);

  static void initCallbacks();

  static void registerPersona(CameraPersona*);
protected:

  static bool mouseDown;
  static int mouseButton;
  static long lastButtonDownTime;

  static bool haveMoved;
  static long locationLastTime;
  static long lastFrameTime;
  static int cenX, cenY;
  static float accumX, accumY;

  static CameraPersona* persona;
};

#endif //CAMERAMOVER_H