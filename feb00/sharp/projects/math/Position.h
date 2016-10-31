#ifndef POSITION_H
#define POSITION_H

#include <math/Vector.h>

class Position : public Vector 
{
public:
  Position(const Vector& sourceLocation = Vector(0,0,0),
           const Vector& sourceOrientation = Vector(1,0,0),
           const Vector& sourceUpVector = Vector(0,0,1));

  const Vector& getOrientation() const;
  const Vector& getUpVector() const;

  void setUpVector(const Vector& newVal);
  void setOrientation(const Vector& newVal);

  void translateBy(float, float, float);
  void translateBy(const Vector&);
  void translateBy(const Vector&, float amt);

  void pitch(float radians);

  void rotateByAngles(float addTheta, float addPhi);

protected:
  Vector orientation;
  Vector upVector;
};

#endif // POSITION_H
