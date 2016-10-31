#include <harness/CameraMover.h>
#include <gl/glut.h>
#include <iostream>

CameraPersona* CameraMover::persona = NULL;

void CameraMover::mouse (int button, int state, int x, int y)
{
  if (persona)
  {
    persona->mouse(button, state, x, y);
  }
}

void CameraMover::motion(int x, int y)
{
  if (persona)
  {
    persona->motion(x,y);
  }
}

void CameraMover::keyboard(unsigned char key, int x, int y)
{
  if (persona)
  {
    persona->keyboard(key, x, y);
  }
}

void CameraMover::special(int key, int x, int y)
{
  if (persona)
  {
    persona->special(key, x, y);
  }
}

void CameraMover::update()
{
  if (persona)
  {
    persona->update();
  }
}

void CameraMover::initCallbacks()
{
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion);
  glutSpecialFunc(special);
}

void CameraMover::registerPersona(CameraPersona* cp)
{
  persona = cp;
}
