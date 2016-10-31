#ifndef FREECAMERA_H
#define FREECAMERA_H

#include <harness/CameraPersona.h>

class FreeCamera : public CameraPersona
{
public:
  FreeCamera();

  virtual void mouse (int button, int state, int x, int y);
  virtual void motion(int x, int y);
  virtual void update();

protected:
  bool mouseDown;
  int mouseButton;
  long lastButtonDownTime;

  bool haveMoved;
  long locationLastTime;
  long lastFrameTime;
  int cenX, cenY;
  float accumX, accumY;
};

#endif //FREECAMERA_H