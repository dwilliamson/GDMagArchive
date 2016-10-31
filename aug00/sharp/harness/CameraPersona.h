#ifndef CAMERAPERSONA_H
#define CAMERAPERSONA_H

class CameraPersona
{
public:
  virtual void mouse (int button, int state, int x, int y);
  virtual void motion(int x, int y);
  virtual void keyboard(unsigned char key, int x, int y);
  virtual void special(int key, int x, int y);

  // This is the only function without a default (null) implementation.
  virtual void update(float dt) = 0;

protected:
};

#endif //CAMERAPERSONA_H