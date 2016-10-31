#ifndef MARCHINGCUBE_H
#define MARCHINGCUBE_H

//
// MarchingCube
//
// MarchingCube is the basic data structure for tessellation.  It's a cubelet whose corners
// are evaluated as either in or out of the surface, and from that a polygonization for the
// inside of the cubelet is formed (by CubePolygonizer.)
//
// MarchingCube is depended upon by SurfaceWalker and CubePolygonizer.
//

#include <fluid/Tess/IntVector.h>

class MarchingCube
{
public:
  MarchingCube();

  // Called per-tessellation to set the axis-aligned box of space we're parameterizing over.
  void setBoundingBox(float xMin, float xRange, float yMin, float yRange, float zMin, float zRange);

  // Called per-tessellation to specify the density of smallest-cells in the space.
  void setCellDensity(int cellsX, int cellsY, int cellsZ);

  // An alternate way to tell it the same thing.
  void setCellWorldSize(float cellSizeX, float cellSizeY, float cellSizeZ);

  // Called during tessellation to set the size of the active cubelet of space. The size argument is 
  // needed because the cubelet can span multiple cells if we're recursing and this cube is higher
  // up in the octree, i.e. not a leaf node.  We only need to know size once for all 3 axes, since it
  // will always be specifying a cube (in parameter space.)
  void setCurrentCube(int xLoc, int yLoc, int zLoc, int size);

  // Access methods.
  // We need to pick some order for the vertices of a cubelet, so here it is (I use 0 to denote
  // the minimum of the two possible values and 1 to denote the maximum, so the point (1,1,1)
  // isn't in world space at (1,1,1), but at (maxX, maxY, maxZ).)
  //
  // 0: (0,0,0)
  // 1: (1,0,0)
  // 2: (1,1,0)
  // 3: (0,1,0)
  // 4: (0,0,1)
  // 5: (1,0,1)
  // 6: (1,1,1)
  // 7: (0,1,1)
  const float* getCorner(int) const;
  const IntVector& getLocation() const;

protected:
  // Computed from setCurrentCube.
  float curCorners[8][3];
  IntVector curLoc;

  // The range of our parameter space and the number of cells inside.
  float xMin, xRange, yMin, yRange, zMin, zRange;
  float xStep, yStep, zStep;
};

#endif //MARCHINGCUBE_H