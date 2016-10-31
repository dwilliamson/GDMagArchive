#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653598979f
#endif

class Vector
{
public:
  Vector(float x=1, float y=0, float z=0);
  
  Vector& operator+=(const Vector&);
  Vector& operator-=(const Vector&);
  Vector& operator*=(float scalar);
  Vector operator-() const;
  bool operator==(const Vector&);
  
  float getMagnitude() const;
  float getMagnitudeSquared() const;
  void setMagnitude(float);

  float getPhi() const;
  float getTheta() const;
  void setPhi(float);
  void setTheta(float);
  
  void normalize();
  void rotate(float, float);
  
  // Arbitrary-axis rotation -- rotate the argument around us by the specified angle in radians.
  void rotate(Vector& target, float radians) const;
  
  void cross(const Vector& rhs, Vector& out) const;
  float dot(const Vector& rhs) const;
  
  static float degreesToRadians(float degrees);
  static float radiansToDegrees(float radians);
  
  float x;
  float y;
  float z;

protected:
  void getRotMatrix(float*, float) const;
};

#endif // VECTOR_H
