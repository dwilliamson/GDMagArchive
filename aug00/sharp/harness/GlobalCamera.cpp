#include <harness/GlobalCamera.h>
#include <gl/glut.h>
#include <math/Math.h>

#include <windows.h>
#include <mmsystem.h>

// Stupid double-to-float-conversion, possible-loss-of-data warning.
#pragma warning (disable: 4244)

GlobalCamera* GlobalCamera::singleton = 0;

GlobalCamera* GlobalCamera::Instance()
{
  if ( singleton == 0 )
  {
    singleton = new GlobalCamera;
  }
  return singleton;
}

void GlobalCamera::setFovY(float newVal)
{
  fovY = newVal;
}

float GlobalCamera::getFovY() const
{
  return fovY;
}

float GlobalCamera::getFovX() const
{
  float halfFovY = getFovY() / 2.0;
  halfFovY = halfFovY * Math::Pi / 180.0f;
  float halfFovX = atan( tan(halfFovY) * 1.333f );
  halfFovX = halfFovX * 180.0f / Math::Pi;
  return 2.0f * halfFovX;
}

float GlobalCamera::getFarPlane() const
{
  return farPlane;
}

void GlobalCamera::setFarPlane(float newVal)
{
  farPlane = newVal;
}

Position& GlobalCamera::getPosition()
{
  return pos;
}

const Position& GlobalCamera::getPosition() const
{
  return pos;
}

GlobalCamera::GlobalCamera() : pos()
{
  fovY = 68;
  farPlane = 200;
  shearCam = false;
}

void GlobalCamera::makeCurrent()
{
  // Set up our projection matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (!shearCam)
  {
    gluPerspective(getFovY(), 1.33f, 0.5f, farPlane);
  }
  else
  {
    // This is a hack: this 0.242 is somehow derived from 40 degrees and a 1.33 aspect ratio.  I didn't
    // bother to figure out how, though.
    float v = 0.242f;
    glFrustum(xShear < 0 ? -v : 0, xShear > 0 ? v : 0, yShear < 0 ? -v/1.33 : 0, yShear > 0 ? v/1.33 : 0, 0.5f, 200);
  }

  // Clear our matrix, reposition the viewer.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  float eyeX = getPosition().x;
  float eyeY = getPosition().y;
  float eyeZ = getPosition().z;
  float oriX = eyeX + getPosition().getOrientation().x;
  float oriY = eyeY + getPosition().getOrientation().y;
  float oriZ = eyeZ + getPosition().getOrientation().z;
  float upvX = getPosition().getUpVector().x;
  float upvY = getPosition().getUpVector().y;
  float upvZ = getPosition().getUpVector().z;

  gluLookAt(eyeX,eyeY,eyeZ, oriX,oriY,oriZ, upvX, upvY, upvZ);
}