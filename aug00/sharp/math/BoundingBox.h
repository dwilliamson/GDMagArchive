#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

// This is a little helper class to stor the dimensions of an axis-aligned box.

#include <math/Vector.h>

class BoundingBox
{
public:
  BoundingBox(float minX = 0, float minY = 0, float minZ = 0, float maxX = 0, float maxY = 0, float maxZ = 0)
  {
    this->minX = minX;
    this->minY = minY;
    this->minZ = minZ;
    this->sizeX = maxX-minX;
    this->sizeY = maxY-minY;
    this->sizeZ = maxZ-minZ;
  }

  bool isInside(const Vector& pt)
  {
    return (pt.x >= minX && pt.x <= minX+sizeX &&
            pt.y >= minY && pt.y <= minY+sizeY && 
            pt.z >= minZ && pt.z <= minZ+sizeZ);
  }

  float minX, minY, minZ, sizeX, sizeY, sizeZ;
protected:
};

#endif //BOUNDINGBOX_H