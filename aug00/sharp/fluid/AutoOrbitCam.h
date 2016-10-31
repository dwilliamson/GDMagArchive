#ifndef AUTOORBITCAM_H
#define AUTOORBITCAM_H

#include <harness/CameraPersona.h>

class AutoOrbitCam : public CameraPersona
{
public:
  AutoOrbitCam();

  virtual void mouse (int button, int state, int x, int y);
  virtual void update(float dt);

protected:
  bool mouseDown;
  int mouseButton;

  bool phiMovingUp;
};

#endif //AUTOORBITCAM_H