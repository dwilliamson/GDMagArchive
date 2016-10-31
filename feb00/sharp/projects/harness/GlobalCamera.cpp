#include <harness/GlobalCamera.h>

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
  halfFovY = halfFovY * 3.1415 / 180.0f;
  float halfFovX = atan( tan(halfFovY) * 1.333f );
  halfFovX = halfFovX * 180.0f / 3.1415;
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
}