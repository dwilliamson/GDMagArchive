#include <harness/OriginOrbitCamera.h>
#include <harness/GlobalCamera.h>
#include <harness/PlatformSpecific.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include <iostream>

#ifndef M_PI
#define M_PI 3.141593f
#endif

OriginOrbitCamera::OriginOrbitCamera()
{
  mouseDown = false;
  mouseButton = GLUT_LEFT_BUTTON;

  haveMoved = false;
  locationLastTime = 0;
  cenX = 0;
  cenY = 0;
  accumX = 0;
  accumY = 0;
}

void OriginOrbitCamera::mouse (int button, int state, int x, int y)
{
  mouseDown = (state == GLUT_DOWN) ? true : false;
  mouseButton = button;
}

void OriginOrbitCamera::motion(int x, int y)
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

void OriginOrbitCamera::update(float dt)
{
  // Go forward if the left button is down, backwards if any other mouse button is down.
  float direction = mouseButton == GLUT_LEFT_BUTTON ? 1.0f : -1.0f;

  bool button1 = mouseDown && mouseButton == GLUT_LEFT_BUTTON;
  bool button2 = mouseDown && mouseButton != GLUT_LEFT_BUTTON;

  // Flip the position so it's looking at the camera from the origin.
  Position originToCamera = GlobalCamera::Instance()->getPosition();
  originToCamera += originToCamera.getOrientation();
  originToCamera.setOrientation(-originToCamera.getOrientation());

  originToCamera.pitch(-0.004 * accumY);
  originToCamera.rotateByAngles(0.004 * accumX, 0.0f);

  // Take into account any motion, which just moves in and out along the orientation vector.
  if (mouseDown)
  {
    Vector oriVec = originToCamera.getOrientation();
    oriVec.setMagnitude(oriVec.getMagnitude() - direction * dt * 10.0f);
    originToCamera.setOrientation(oriVec);
  }

  // Now flip again so it's looking the right direction again.
  originToCamera += originToCamera.getOrientation();
  originToCamera.setOrientation(-originToCamera.getOrientation());

  // Pop that back in the camera and we're set.
  GlobalCamera::Instance()->getPosition() = originToCamera;

  // Reset these since we've taken them into account now.
  accumX = 0;
  accumY = 0;
}