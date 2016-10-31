#ifndef CAMERAMOVER_H
#define CAMERAMOVER_H

class CameraMover
{
public:
  static void mouse (int button, int state, int x, int y);
  static void motion(int x, int y);
  static void update();

  static void initCallbacks();
protected:

  static bool mouseDown;
  static int mouseButton;
  static long lastButtonDownTime;

  static bool haveMoved;
  static long locationLastTime;
  static long lastFrameTime;
  static int cenX, cenY;
  static float accumX, accumY;
};

#endif //CAMERAMOVER_H