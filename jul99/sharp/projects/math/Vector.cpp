#include <math/Vector.h>
#include <math.h>

// This is a stupid "converting from double to float, possible loss of data" warning.
#pragma warning (disable: 4244)

Vector::Vector(float x, float y, float z)
{
  this->x = x;
  this->y = y;
  this->z = z;
}

Vector& Vector::operator+=(const Vector& rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

Vector& Vector::operator-=(const Vector& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

Vector& Vector::operator*=(float scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

Vector Vector::operator-() const
{
  Vector out;
  out.x = -x;
  out.y = -y;
  out.z = -z;
  return out;
}

bool Vector::operator==(const Vector& s) {
  return (x==s.x && y==s.y && z==s.z);
}

float Vector::getMagnitude() const {
  return sqrt(x*x + y*y + z*z);
}

float Vector::getMagnitudeSquared() const {
  return (x*x + y*y + z*z);
}

void Vector::setMagnitude(float newM) {
  normalize();
  x = x * newM;
  y = y * newM;
  z = z * newM;
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

  // We want it from 0->2M_PI, not -M_PI->M_PI, so make sure to wrap it right.
  if (y < 0)
  {
    theta = 2*M_PI-theta;
  }

  // That's it!
  return theta;
}

void Vector::setPhi(float phi) 
{
  // Normalize the new value.
  while(phi >= 2*M_PI)
  {
    phi -= 2*M_PI;
  }
  while(phi < 0)
  {
    phi += 2*M_PI;
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
  while (theta >= 2*M_PI)
  {
    theta -= 2*M_PI;
  }
  while (theta < 0)
  {
    theta += 2*M_PI;
  }

  float phi = getPhi();
  float magnitude = getMagnitude();

  x = magnitude*sin(phi)*cos(theta);
  y = magnitude*sin(phi)*sin(theta);
}

void Vector::normalize() 
{
  float invR = 1.0f/getMagnitude();
  x = x*invR;
  y = y*invR;
  z = z*invR;
}

void Vector::rotate(float addTheta, float addPhi) 
{
  float phi = getPhi() + addPhi;
  float theta = getTheta() + addTheta;
  float magnitude = getMagnitude();

  // Normalize the new value.
  while(phi >= 2*M_PI)
  {
    phi -= 2*M_PI;
  }
  while(phi < 0)
  {
    phi += 2*M_PI;
  }

  // Normalize the new value.
  while(theta >= 2*M_PI)
  {
    theta -= 2*M_PI;
  }
  while(theta < 0)
  {
    theta += 2*M_PI;
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
  /* This function contributed by Erich Boleyn (erich@uruk.org) */
  /* This function used from the Mesa OpenGL code (matrix.c)  */
  float mag, s, c;
  float vx, vy, vz, xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
  
  s = sin(radians);
  c = cos(radians);
  
  mag = getMagnitude();
  
  vx = x / mag;
  vy = y / mag;
  vz = z / mag;
  
#define M(row,col)  out[row*3 + col]
  
  /*
  *     Arbitrary axis rotation matrix.
  *
  *  This is composed of 5 matrices, Rz, Ry, T, Ry', Rz', multiplied
  *  like so:  Rz * Ry * T * Ry' * Rz'.  T is the final rotation
  *  (which is about the X-axis), and the two composite transforms
  *  Ry' * Rz' and Rz * Ry are (respectively) the rotations necessary
  *  from the arbitrary axis to the X-axis then back.  They are
  *  all elementary rotations.
  *
  *  Rz' is a rotation about the Z-axis, to bring the axis vector
  *  into the x-z plane.  Then Ry' is applied, rotating about the
  *  Y-axis to bring the axis vector parallel with the X-axis.  The
  *  rotation about the X-axis is then performed.  Ry and Rz are
  *  simply the respective inverse transforms to bring the arbitrary
  *  axis back to it's original orientation.  The first transforms
  *  Rz' and Ry' are considered inverses, since the data from the
  *  arbitrary axis gives you info on how to get to it, not how
  *  to get away from it, and an inverse must be applied.
  *
  *  The basic calculation used is to recognize that the arbitrary
  *  axis vector (x, y, z), since it is of unit length, actually
  *  represents the sines and cosines of the angles to rotate the
  *  X-axis to the same orientation, with theta being the angle about
  *  Z and phi the angle about Y (in the order described above)
  *  as follows:
  *
  *  cos ( theta ) = x / sqrt ( 1 - z^2 )
  *  sin ( theta ) = y / sqrt ( 1 - z^2 )
  *
  *  cos ( phi ) = sqrt ( 1 - z^2 )
  *  sin ( phi ) = z
  *
  *  Note that cos ( phi ) can further be inserted to the above
  *  formulas:
  *
  *  cos ( theta ) = x / cos ( phi )
  *  sin ( theta ) = y / cos ( phi )
  *
  *  ...etc.  Because of those relations and the standard trigonometric
  *  relations, it is pssible to reduce the transforms down to what
  *  is used below.  It may be that any primary axis chosen will give the
  *  same results (modulo a sign convention) using thie method.
  *
  *  Particularly nice is to notice that all divisions that might
  *  have caused trouble when parallel to certain planes or
  *  axis go away with care paid to reducing the expressions.
  *  After checking, it does perform correctly under all cases, since
  *  in all the cases of division where the denominator would have
  *  been zero, the numerator would have been zero as well, giving
  *  the expected result.
  */
  
  xx = vx * vx;
  yy = vy * vy;
  zz = vz * vz;
  xy = vx * vy;
  yz = vy * vz;
  zx = vz * vx;
  xs = vx * s;
  ys = vy * s;
  zs = vz * s;
  one_c = 1.0F - c;
  
  M(0,0) = (one_c * xx) + c;
  M(1,0) = (one_c * xy) - zs;
  M(2,0) = (one_c * zx) + ys;
  
  M(0,1) = (one_c * xy) + zs;
  M(1,1) = (one_c * yy) + c;
  M(2,1) = (one_c * yz) - xs;
  
  M(0,2) = (one_c * zx) - ys;
  M(1,2) = (one_c * yz) + xs;
  M(2,2) = (one_c * zz) + c;
  
#undef M
}
  
void Vector::cross(const Vector& rhs, Vector& out) const
{
  out.x = y*rhs.z - z*rhs.y;
  out.y = z*rhs.x - x*rhs.z;
  out.z = x*rhs.y - y*rhs.x;
}

float Vector::dot(const Vector& rhs) const
{
  return x*rhs.x + y*rhs.y + z*rhs.z;
}

float Vector::degreesToRadians(float degrees) 
{
  return degrees * M_PI / 180.0f;
}

float Vector::radiansToDegrees(float radians) 
{
  return radians * 180.0f / M_PI;
}
