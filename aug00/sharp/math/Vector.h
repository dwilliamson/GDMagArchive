#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <math/Math.h>

class Vector
{
public:
  __inline Vector(float x=1, float y=0, float z=0);
  
  __inline Vector& operator+=(const Vector&);
  __inline Vector& operator-=(const Vector&);
  __inline Vector& operator*=(float scalar);
  __inline Vector operator-() const;

  __inline Vector operator-(const Vector&) const;
  __inline Vector operator+(const Vector&) const;
  __inline Vector operator*(float scalar) const;

  bool operator==(const Vector&);
  
  __inline float getMagnitude() const;
  __inline float getMagnitudeSquared() const;
  __inline void setMagnitude(float);

  float getPhi() const;
  float getTheta() const;
  void setPhi(float);
  void setTheta(float);
  
  __inline void normalize();

  // This is faster but you lose some precision (it uses a fast approximate inverse sqrt)
  __inline void fastNormalize();
  void rotate(float, float);
  
  // Arbitrary-axis rotation -- rotate the argument around us by the specified angle in radians.
  void rotate(Vector& target, float radians) const;
  
  void cross(const Vector& rhs, Vector& out) const;
  __inline float dot(const Vector& rhs) const;
  
  static float degreesToRadians(float degrees);
  static float radiansToDegrees(float radians);
  
  float x;
  float y;
  float z;

protected:
  void getRotMatrix(float*, float) const;
};

// Inline functions
__inline Vector::Vector(float x, float y, float z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

__inline Vector& Vector::operator+=(const Vector& rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

__inline Vector& Vector::operator-=(const Vector& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

__inline Vector& Vector::operator*=(float scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

__inline Vector Vector::operator-() const
{
  Vector out;
  out.x = -x;
  out.y = -y;
  out.z = -z;
  return out;
}

__inline Vector Vector::operator+(const Vector& rhs) const
{
  Vector result;
  result.x = x + rhs.x;
  result.y = y + rhs.y;
  result.z = z + rhs.z;
  return result;
}

__inline Vector Vector::operator-(const Vector& rhs) const
{
  Vector result;
  result.x = x - rhs.x;
  result.y = y - rhs.y;
  result.z = z - rhs.z;
  return result;
}

__inline Vector Vector::operator*(float scalar) const
{
  Vector result;
  result.x = x * scalar;
  result.y = y * scalar;
  result.z = z * scalar;
  return result;
}

__inline float Vector::getMagnitude() const {
  return (float)sqrt(x*x + y*y + z*z);
}

__inline float Vector::getMagnitudeSquared() const {
  return (x*x + y*y + z*z);
}

__inline void Vector::setMagnitude(float newM) {
  normalize();
  x = x * newM;
  y = y * newM;
  z = z * newM;
}

__inline void Vector::normalize() 
{
  float invR = 1.0f/getMagnitude();
  x = x*invR;
  y = y*invR;
  z = z*invR;
}

__inline void Vector::fastNormalize() 
{
  float invR = Math::fastInverseSqrt(getMagnitudeSquared());
  x = x*invR;
  y = y*invR;
  z = z*invR;
}

__inline float Vector::dot(const Vector& rhs) const
{
  return x*rhs.x + y*rhs.y + z*rhs.z;
}

#endif // VECTOR_H
