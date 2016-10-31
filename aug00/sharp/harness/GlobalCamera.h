#ifndef GLOBALCAMERA_H
#define GLOBALCAMERA_H

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

  void makeCurrent();

  // Used to take larger screenshots; can shear so it's only rendering a quarter of the full frame.
  void setOffset(bool offset, int xSign, int ySign) { shearCam = offset; xShear = xSign, yShear = ySign; }

protected:
  GlobalCamera();

  static GlobalCamera* singleton;

  float fovY;
  Position pos;

  float farPlane;

  bool shearCam;
  int xShear, yShear;
};

#endif