#ifndef POLYGONSAMPLER_H
#define POLYGONSAMPLER_H

//
// PolygonSampler
//
// PolygonSampler is used to construct heightfield out of a polygonal mesh.  It can take a bunch of
// polygons and then be asked to intersect a vertical ray with that batch of polygons, returning the
// value of the height of the highest polygon at that point.
//

#include <vector>
#include <math/Vector.h>

class PolygonSampler
{
public:
  PolygonSampler();

  // Used to pass in the polys this guy uses to sample from.
  void setPolygonMesh(const std::vector<Vector>& vertices, const std::vector<int>& indices);

  // Used to set a value returned for a point that doesn't intersect any polygons.
  void setOutOfBoundsHeight(float height);

  // Used after setting a polygonal mesh to query for the height of the mesh at a point.
  float getHeightAt(float x, float y);

protected:
  std::vector<Vector> vertices;
  std::vector<int> indices;
  float outOfBoundsHeight;
};

#endif //POLYGONSAMPLER_H