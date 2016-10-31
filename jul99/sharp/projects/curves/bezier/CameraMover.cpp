#include <curves/bezier/CameraMover.h>
#include <curves/GlobalCamera.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include <iostream>
#include <gl/glut.h>

#ifndef M_PI
#define M_PI 3.141593f
#endif

bool CameraMover::mouseDown = false;
int CameraMover::mouseButton = GLUT_LEFT_BUTTON;
long CameraMover::lastButtonDownTime = 0;

bool CameraMover::haveMoved = false;
long CameraMover::locationLastTime = 0;
long CameraMover::lastFrameTime = 0;
int CameraMover::cenX = 0;
int CameraMover::cenY = 0;
float CameraMover::accumX = 0;
float CameraMover::accumY = 0;

void CameraMover::mouse (int button, int state, int x, int y)
{
  mouseDown = (state == GLUT_DOWN) ? true : false;
  mouseButton = button;
  lastButtonDownTime = timeGetTime();
}

void CameraMover::motion(int x, int y)
{
  if (!haveMoved)
  {
    cenX = glutGet(GLUT_WINDOW_X) + glutGet(GLUT_WINDOW_WIDTH)/2;
    cenY = glutGet(GLUT_WINDOW_Y) + glutGet(GLUT_WINDOW_HEIGHT)/2;
    glutWarpPointer(cenX, cenY);
    accumX = 0;
    accumY = 0;
    haveMoved = true;
  }
  else
  {
    if (cenX == x && cenY == y)
    {
      return;
    }

    float dx = cenX - x;
    float dy = cenY - y;

    accumX += dx;
    accumY += dy;

    glutWarpPointer(cenX, cenY);
  }
}

void CameraMover::update()
{
  long curTime = timeGetTime();
  if (lastFrameTime == 0) 
  {
    lastFrameTime = curTime;
    return;
  }

  float dt = (curTime - lastFrameTime) / 1000.0f;

  // Go forward if the left button is down, backwards if any other mouse button is down.
  float direction = mouseButton == GLUT_LEFT_BUTTON ? 1.0f : -1.0f;

  bool button1 = mouseDown && mouseButton == GLUT_LEFT_BUTTON;
  bool button2 = mouseDown && mouseButton != GLUT_LEFT_BUTTON;

  GlobalCamera::Instance()->getPosition().pitch(0.004 * accumY);
  GlobalCamera::Instance()->getPosition().rotateByAngles(0.004 * accumX, 0.0f);

  // Reset these since we've taken them into account now.
  accumX = 0;
  accumY = 0;

  if (mouseDown)
  {
    GlobalCamera::Instance()->getPosition().translateBy(GlobalCamera::Instance()->getPosition().getOrientation(), direction * dt * 40.0f);
  }

  lastFrameTime = curTime;
}

void CameraMover::initCallbacks()
{
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(motion);
}

