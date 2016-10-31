#include <math/Position.h>
#include <iostream>
#include <iomanip>

Position::Position(const Vector& sourceLocation, 
                   const Vector& sourceOrientation, 
                   const Vector& sourceUpVector)
                   : Vector(sourceLocation), 
                     orientation(sourceOrientation)
{
  // We want to guarantee orthogonality.
  Vector left;
  sourceUpVector.cross(orientation, left);
  orientation.cross(left, upVector);
  upVector.normalize();
}

const Vector& Position::getOrientation() const
{
	return orientation;
}

const Vector& Position::getUpVector() const
{
	return upVector;
}

void Position::setUpVector(const Vector& newVal) 
{
  // We want to guarantee orthogonality.
  Vector left;
  newVal.cross(orientation, left);
  orientation.cross(left, upVector);
  upVector.normalize();
}

void Position::setOrientation(const Vector& newVal) 
{
  orientation = newVal;
}

void Position::translateBy(float addX, float addY, float addZ) 
{
  x += addX;
  y += addY;
  z += addZ;
}

void Position::translateBy(const Vector& add) 
{
  x += add.x;
  y += add.y;
  z += add.z;
}

void Position::translateBy(const Vector& direction, float amt)
{
  Vector dir(direction);
  dir.setMagnitude(amt);
  translateBy(dir);
}

void Position::pitch(float radians) 
{
  // Get the vector perpendicular to the orientation and up vectors.
  Vector left;
  orientation.cross(upVector, left);

  // Spin both our vectors around the left vector.
  left.rotate(orientation, radians);
  left.rotate(upVector, radians);
}

void Position::rotateByAngles(float addTheta, float addPhi) 
{
  // Rotate both our vectors by the specified amounts.
  upVector.rotate(addTheta, addPhi);
  orientation.rotate(addTheta, addPhi);
}
