#ifndef PATCHBOX_H
#define PATCHBOX_H

#include <math/Vector.h>

// This just holds an axis-aligned bounding box used in patch culling.
class PatchBox
{
public:
  PatchBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

  Vector corners[8];
  float minX, minY, minZ;
  float maxX, maxY, maxZ;
protected:
};

#endif //PATCHBOX_H