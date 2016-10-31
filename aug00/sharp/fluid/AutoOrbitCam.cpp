#include <fluid/AutoOrbitCam.h>
#include <harness/GlobalCamera.h>
#include <harness/PlatformSpecific.h>
#include <math/Math.h>
#include <gl/glut.h>
#include <mmsystem.h>
#include <iostream>

AutoOrbitCam::AutoOrbitCam()
{
  mouseDown = false;
  mouseButton = GLUT_LEFT_BUTTON;

  phiMovingUp = true;
}

void AutoOrbitCam::mouse (int button, int state, int x, int y)
{
  mouseDown = (state == GLUT_DOWN) ? true : false;
  mouseButton = button;
}

void AutoOrbitCam::update(float dt)
{
  // Hardcode rotation rates and bounds for now.  Rates are in radians / sec.
  const float thetaRate = 2.0f * Math::Pi / 50.0f;

  const float maxPhi = Math::Pi * 0.40f;
  const float minPhi = Math::Pi * 0.12f;

  const float phiRate = (maxPhi - minPhi) / 20.0f;

  // Go forward if the left button is down, backwards if any other mouse button is down.
  float direction = mouseButton == GLUT_LEFT_BUTTON ? 1.0f : -1.0f;

  bool button1 = mouseDown && mouseButton == GLUT_LEFT_BUTTON;
  bool button2 = mouseDown && mouseButton != GLUT_LEFT_BUTTON;

  // Flip the position so it's looking at the camera from the origin.
  Position originToCamera = GlobalCamera::Instance()->getPosition();
  originToCamera += originToCamera.getOrientation();
  originToCamera.setOrientation(-originToCamera.getOrientation());

  // Figure out the angles to rotate.
  float curPhi = originToCamera.getOrientation().getPhi();
  if (curPhi > maxPhi)
  {
    phiMovingUp = true;
  }
  else if (curPhi < minPhi)
  {
    phiMovingUp = false;
  }

  float dTheta = thetaRate * dt;
  float dPhi = phiRate * dt * (phiMovingUp ? 1 : -1);

  originToCamera.pitch(dPhi);
  originToCamera.rotateByAngles(dTheta, 0.0f);

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
}