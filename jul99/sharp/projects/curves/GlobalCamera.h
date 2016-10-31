#ifndef GLOBALCAMERA_H
#define GLOBALCAMERA_H

#include <curves/CurvePoint.h>
#include <math/Position.h>

class GlobalCamera {
public:
  static GlobalCamera* Instance();

  void setFovY(float newVal);
  float getFovY() const;
  float getFovX() const;

  float getFarPlane() const;
  void setFarPlane(float newVal);

  Position& getPosition();
  const Position& getPosition() const;

protected:
  GlobalCamera();

  static GlobalCamera* singleton;

  float fovY;
  Position pos;

  float farPlane;
};

#endif