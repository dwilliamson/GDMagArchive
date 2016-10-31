#ifndef PUIPERSONA_H
#define PUIPERSONA_H

//
// PuiPersona
//
// This class just hands all mouse input over to PUI (the picoscopic user interface by Steve Baker) and 
// is used when menus are activated.
//

#include <plib/pu.h>
#include <harness/CameraPersona.h>

class PuiPersona : public CameraPersona
{
public:
  virtual void mouse (int button, int state, int x, int y) { puMouse(button, state, x, y); }
  virtual void motion(int x, int y) { puMouse(x, y); }

  // This is the only function without a default (null) implementation.
  virtual void update(float dt) {};

protected:
};

#endif //PUIPERSONA_H