#include <math/Vector.h>
#include <math/Math.h>
#include <math.h>

// This is a stupid "converting from double to float, possible loss of data" warning.
#pragma warning (disable: 4244)

bool Vector::operator==(const Vector& s) {
  return (x==s.x && y==s.y && z==s.z);
}

float Vector::getPhi() const
{
  // Quick check to prevent div by 0.
  if (getMagnitudeSquared() < 0.001)
  {
    return 0;
  }

  // This is it; we know we're not degenerate.
  return acos(z/getMagnitude());
}

float Vector::getTheta() const
{
  // Quick check to prevent div by 0.
  if (x*x + y*y < 0.0001)
  {
    return 0;
  }

  float r = sqrt(x*x + y*y);
  float theta = acos(x/r);

  // We want it from 0->2Math::Pi, not -Math::Pi->Math::Pi, so make sure to wrap it right.
  if (y < 0)
  {
    theta = 2*Math::Pi-theta;
  }

  // That's it!
  return theta;
}

void Vector::setPhi(float phi) 
{
  // Normalize the new value.
  while(phi >= 2*Math::Pi)
  {
    phi -= 2*Math::Pi;
  }
  while(phi < 0)
  {
    phi += 2*Math::Pi;
  }

  float theta = getTheta();
  float magnitude = getMagnitude();
  
  x = magnitude*sin(phi)*cos(theta);
  y = magnitude*sin(phi)*sin(theta);
  z = magnitude*cos(phi);
}

void Vector::setTheta(float theta) 
{
  // Normalize the new value.
  while (theta >= 2*Math::Pi)
  {
    theta -= 2*Math::Pi;
  }
  while (theta < 0)
  {
    theta += 2*Math::Pi;
  }

  float phi = getPhi();
  float magnitude = getMagnitude();

  x = magnitude*sin(phi)*cos(theta);
  y = magnitude*sin(phi)*sin(theta);
}

void Vector::rotate(float addTheta, float addPhi) 
{
  float phi = getPhi() + addPhi;
  float theta = getTheta() + addTheta;
  float magnitude = getMagnitude();

  // Normalize the new value.
  while(phi >= 2*Math::Pi)
  {
    phi -= 2*Math::Pi;
  }
  while(phi < 0)
  {
    phi += 2*Math::Pi;
  }

  // Normalize the new value.
  while(theta >= 2*Math::Pi)
  {
    theta -= 2*Math::Pi;
  }
  while(theta < 0)
  {
    theta += 2*Math::Pi;
  }

  x = magnitude*sin(phi)*cos(theta);
  y = magnitude*sin(phi)*sin(theta);
  z = magnitude*cos(phi);
}
  
// Note: 'angle' is in radians.
void Vector::rotate(Vector& target, float radians) const 
{
  float m[9];
  getRotMatrix(m, radians);
  float nx, ny, nz, tx, ty, tz;
  
  tx = target.x;
  ty = target.y;
  tz = target.z;
  
#define M(row,col)  m[row*3 + col]
  nx = tx*M(0,0) + ty*M(1,0) + tz*M(2,0);
  ny = tx*M(0,1) + ty*M(1,1) + tz*M(2,1);
  nz = tx*M(0,2) + ty*M(1,2) + tz*M(2,2);
#undef M
  
  target.x = nx;
  target.y = ny;
  target.z = nz;
}

void Vector::getRotMatrix(float* out, float radians) const 
{
  float sinAngle = sin(radians);
  float cosAngle = cos(radians);
  
  Vector unitThis(*this);
  unitThis.normalize();

  // This looks gross, but it works; I found a similar function in the Mesa
  // source except (a) it was long and even harder to read, and (b) it was under GPL.
  // I've rederived it here; the derivation involves rotating this vector through
  // a rotation matrix to align it with one of the basis vectors (I used the x axis)
  // and then generating the rotation matrix around that basis vector (easy trig) and
  // then pushing it through the inverse rotation to bring it back to this vector.
  // It's a pain to derive but the terms clean themselves up nicely into this:
  out[0*3 + 0] = (unitThis.x * unitThis.x * (1.0f - cosAngle)) + cosAngle;
  out[1*3 + 0] = (unitThis.x * unitThis.y * (1.0f - cosAngle)) - unitThis.z * sinAngle;
  out[2*3 + 0] = (unitThis.z * unitThis.x * (1.0f - cosAngle)) + unitThis.y * sinAngle;
                                         
  out[0*3 + 1] = (unitThis.x * unitThis.y * (1.0f - cosAngle)) + unitThis.z * sinAngle;
  out[1*3 + 1] = (unitThis.y * unitThis.y * (1.0f - cosAngle)) + cosAngle;
  out[2*3 + 1] = (unitThis.y * unitThis.z * (1.0f - cosAngle)) - unitThis.x * sinAngle;
                                         
  out[0*3 + 2] = (unitThis.z * unitThis.x * (1.0f - cosAngle)) - unitThis.y * sinAngle;
  out[1*3 + 2] = (unitThis.y * unitThis.z * (1.0f - cosAngle)) + unitThis.x * sinAngle;
  out[2*3 + 2] = (unitThis.z * unitThis.z * (1.0f - cosAngle)) + cosAngle;
}
  
void Vector::cross(const Vector& rhs, Vector& out) const
{
  out.x = y*rhs.z - z*rhs.y;
  out.y = z*rhs.x - x*rhs.z;
  out.z = x*rhs.y - y*rhs.x;
}

float Vector::degreesToRadians(float degrees) 
{
  return degrees * Math::Pi / 180.0f;
}

float Vector::radiansToDegrees(float radians) 
{
  return radians * 180.0f / Math::Pi;
}
