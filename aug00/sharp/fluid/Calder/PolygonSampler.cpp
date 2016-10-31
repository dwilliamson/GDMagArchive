#include <fluid/calder/PolygonSampler.h>

PolygonSampler::PolygonSampler()
{
  outOfBoundsHeight = 0;
}

// Used to pass in the polys this guy uses to sample from.
void PolygonSampler::setPolygonMesh(const std::vector<Vector>& verticesIn, const std::vector<int>& indicesIn)
{
  vertices = verticesIn;
  indices = indicesIn;
}

// Used to set a value returned for a point that doesn't intersect any polygons.
void PolygonSampler::setOutOfBoundsHeight(float height)
{
  outOfBoundsHeight = height;
}

// Used after setting a polygonal mesh to query for the height of the mesh at a point.
float PolygonSampler::getHeightAt(float x, float y)
{
  Vector testPt(x,y,0);

  bool foundPoly = false;
  float maxHeight = outOfBoundsHeight;

  // Figure out which it lies in, saving the highest.
  for (int index=0; index<indices.size()/3; index++)
  {
    const Vector& p0 = vertices[indices[3*index+0]];
    const Vector& p1 = vertices[indices[3*index+1]];
    const Vector& p2 = vertices[indices[3*index+2]];

    Vector p01Norm = p1 - p0;
    Vector p12Norm = p2 - p1;
    Vector p20Norm = p0 - p2;

    float swap;

    swap = p01Norm.x;
    p01Norm.x = p01Norm.y;
    p01Norm.y = -swap;
    p01Norm.z = 0;
    p01Norm.normalize();

    swap = p12Norm.x;
    p12Norm.x = p12Norm.y;
    p12Norm.y = -swap;
    p12Norm.z = 0;
    p12Norm.normalize();

    swap = p20Norm.x;
    p20Norm.x = p20Norm.y;
    p20Norm.y = -swap;
    p20Norm.z = 0;
    p20Norm.normalize();

    // If the dot products are all the same sign it's inside the poly.
    Vector p0p = testPt - p0;
    p0p.normalize();

    Vector p1p = testPt - p1;
    p1p.normalize();

    Vector p2p = testPt - p2;
    p2p.normalize();

    float edge0Dir = p0p.dot(p01Norm);
    float edge1Dir = p1p.dot(p12Norm);
    float edge2Dir = p2p.dot(p20Norm);

    // Catch ones that are really close to edges.
    if (fabs(edge0Dir) < 0.000001) edge0Dir = 0;
    if (fabs(edge1Dir) < 0.000001) edge1Dir = 0;
    if (fabs(edge2Dir) < 0.000001) edge2Dir = 0;
    
    // Inside this poly?  If so, find the height.
    if ((edge0Dir >= 0 && edge1Dir >= 0 && edge2Dir >= 0) || (edge0Dir <= 0 && edge1Dir <= 0 && edge2Dir <= 0))
    {
      // Find the normal.
      Vector polyNorm;
      (p1 - p0).cross(p2 - p0, polyNorm);
      polyNorm.normalize();

      // Find the offset.  Dotting the normal with any of the vertices will do the trick.
      float planeD = -polyNorm.dot(p0);

      // Now solve for the height at the (x,y) we care about.
      float thisHeight = (-polyNorm.x*x - polyNorm.y*y - planeD) / polyNorm.z;

      // If we've already found other intersections make sure this is the highest, otherwise
      // just set it straight up.
      if (!foundPoly || (thisHeight > maxHeight))
      {
        maxHeight = thisHeight;
      }
      
      // We found one...
      foundPoly = true;
    }
  }

  return maxHeight;
}
