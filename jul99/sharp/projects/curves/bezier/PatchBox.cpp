#include <curves/bezier/PatchBox.h>

PatchBox::PatchBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
  this->minX = minX;
  this->maxY = maxY;
  this->minY = minY;
  this->maxX = maxX;
  this->minZ = minZ;
  this->maxZ = maxZ;

  // Now make each corner point for convenient culling.
  corners[0].x = minX;
  corners[1].x = minX;
  corners[2].x = minX;
  corners[3].x = minX;
  corners[4].x = maxX;
  corners[5].x = maxX;
  corners[6].x = maxX;
  corners[7].x = maxX;

  corners[0].y = minY;
  corners[1].y = minY;
  corners[2].y = maxY;
  corners[3].y = maxY;
  corners[4].y = minY;
  corners[5].y = minY;
  corners[6].y = maxY;
  corners[7].y = maxY;

  corners[0].z = minZ;
  corners[1].z = maxZ;
  corners[2].z = minZ;
  corners[3].z = maxZ;
  corners[4].z = minZ;
  corners[5].z = maxZ;
  corners[6].z = minZ;
  corners[7].z = maxZ;
}