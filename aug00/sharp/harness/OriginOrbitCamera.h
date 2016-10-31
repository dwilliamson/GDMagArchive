#ifndef ORIGINORBITCAMERA_H
#define ORIGINORBITCAMERA_H

#include <harness/CameraPersona.h>

class OriginOrbitCamera : public CameraPersona
{
public:
  OriginOrbitCamera();

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

#endif //ORIGINORBITCAMERA_H