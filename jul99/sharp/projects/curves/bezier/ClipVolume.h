#ifndef CLIPVOLUME_H
#define CLIPVOLUME_H

#include <math/Plane.h>
#include <math/Vector.h>

class PatchBox;
class GlobalCamera;

class ClipVolume
{
public:
  ClipVolume(const GlobalCamera&);

  bool contains(const PatchBox&) const;

protected:
  Plane topPlane, bottomPlane, rightPlane, leftPlane, nearPlane, farPlane;
  Vector eyePt;
};

#endif //CLIPVOLUME_H