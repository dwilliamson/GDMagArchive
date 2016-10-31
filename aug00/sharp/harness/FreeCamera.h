#ifndef FREECAMERA_H
#define FREECAMERA_H

#include <harness/CameraPersona.h>

class FreeCamera : public CameraPersona
{
public:
  FreeCamera();

  virtual void mouse (int button, int state, int x, int y);
  virtual void motion(int x, int y);
  virtual void update(float dt);

protected:
  bool mouseDown;
  int mouseButton;

  bool haveMoved;
  long locationLastTime;
  int cenX, cenY;
  float accumX, accumY;
};

#endif //FREECAMERA_H