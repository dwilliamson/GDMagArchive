#include <math/Plane.h>
#include <math.h>

Plane::Plane(const Vector& normal, float offset)
{
  // We store the normal normalized.
  abc = normal;
  d = offset / abc.getMagnitude();
  abc.normalize();
}

float Plane::getA() const { return abc.x; }
float Plane::getB() const { return abc.y; }
float Plane::getC() const { return abc.z; }
float Plane::getD() const { return d; }

void Plane::setA(float newVal) { abc.x = newVal; }
void Plane::setB(float newVal) { abc.y = newVal; }
void Plane::setC(float newVal) { abc.z = newVal; }
void Plane::setD(float newVal) { d = newVal; }

bool Plane::isInside(const Vector& test) const
{
  float planeEqVal = abc.x*test.x + abc.y*test.y + abc.z*test.z + d;
  if (planeEqVal > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}