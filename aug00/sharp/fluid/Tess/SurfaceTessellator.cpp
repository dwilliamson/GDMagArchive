#include <fluid/Tess/SurfaceTessellator.h>

SurfaceTessellator::SurfaceTessellator()
{
  cube = 0;
  polygonizer = 0;
  debugLevel = 0;
}

// Used to toggle debug stuff like drawing the octree...
void SurfaceTessellator::incrementDebugLevel(int amount)
{
  debugLevel += amount;
  
  // Since amount can be negative, and mod of a negative number isn't what we want,
  // make sure debugLevel is positive.
  while (debugLevel < 0)
  {
    debugLevel += getNumDebugLevels()+1;
  }
  
  debugLevel = debugLevel % (getNumDebugLevels()+1);
}

int SurfaceTessellator::getDebugLevel() const
{
  return debugLevel;
}

// Called once to hand this guy the cube he uses.
void SurfaceTessellator::setMarchingCube(MarchingCube* cubeIn)
{
  cube = cubeIn;
}

void SurfaceTessellator::setCubePolygonizer(CubePolygonizer* poly)
{
  polygonizer = poly;
}

// This is the function that actually tessellates the surface.  It calls
// childTessellate but also does some bookkeeping.
void SurfaceTessellator::tessellate(const PotentialPoints& points, float cubeletSize, float surfaceThreshold)
{
  // Clear this guy out each frame.
  getPolygonizer()->beginTessellation();

  childTessellate(points, cubeletSize, surfaceThreshold);
}
