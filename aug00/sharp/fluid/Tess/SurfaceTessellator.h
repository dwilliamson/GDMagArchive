#ifndef SURFACETESSELLATOR_H
#define SURFACETESSELLATOR_H

//
// SurfaceTessellator is the abstract base class for all implicit surface tessellators.  
// Given the literature, the two obvious candidates for implementation are a recursive
// tessellator and a continuous tessellator.  It interacts with the PotentialPoints for culling
// purposes, although it uses the CubePolygonizer to do all actual surface evaluation.  It
// uses the MarchingCube to set it up to the right size to hand it off to the CubePolygonizer.
//
// It also has hooks for debug information -- incrementing the debug level will wrap at the highest
// level.  This number can be used for determining what "extra stuff" to draw, like a wireframe
// of the cubelets considered or a point indicator where each of the PotentialPoints is.
//

#include <fluid/SurfRep/PotentialPoints.h>
#include <fluid/Tess/MarchingCube.h>
#include <fluid/Tess/CubePolygonizer.h>

#include <assert.h>

class SurfaceTessellator
{
public:
  SurfaceTessellator();

  // Used to toggle debug stuff like drawing the octree...
  void incrementDebugLevel(int amount = 1);
  int getDebugLevel() const;

  // Called once to hand this guy the cube he uses and the polygonizer.
  void setMarchingCube(MarchingCube*);
  void setCubePolygonizer(CubePolygonizer*);

  // This is the function that actually tessellates the surface.  It calls
  // childTessellate but also does some bookkeeping.
  void tessellate(const PotentialPoints&, float cubeletSize, float surfaceThreshold);

  // Optionally called at the end of the frame to draw debug info in an ortho projection.
  virtual void drawDebugStatistics() {}

protected:
  // This is the function the child overloads...
  virtual void childTessellate(const PotentialPoints&, float cubeletSize, float surfaceThreshold) = 0;

  // We use inline access methods just so we can assert that the data members are initialized.
  __inline MarchingCube* getCube();
  __inline CubePolygonizer* getPolygonizer();

  // Used so child classes can specify how many debug levels they want to expose.
  virtual int getNumDebugLevels() = 0;

private:
  // Indicates whether we should draw debug info like the octree.
  int debugLevel;

  MarchingCube* cube;
  CubePolygonizer* polygonizer;
};

// Inline functions.
__inline MarchingCube* SurfaceTessellator::getCube()
{
  assert(cube != 0);
  return cube;
}

__inline CubePolygonizer* SurfaceTessellator::getPolygonizer()
{
  assert(polygonizer != 0);
  return polygonizer;
}

#endif //SURFACETESSELLATOR_H