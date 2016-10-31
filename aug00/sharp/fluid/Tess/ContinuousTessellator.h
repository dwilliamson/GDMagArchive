#ifndef CONTINUOUSTESSELLATOR_H
#define CONTINUOUSTESSELLATOR_H

//
// ContinuousTessellator
//
// ContinuousTessellator is responsible for tracing over the implicit surface to find every 
// cubelet that lies on the surface (and having CubePolygonizer polygonize all those cubelets.)
//
// ContinuousTessellator depends on CubeTracker to maintain the list of considered cubes to 
// avoid cycles in recursion, on CubePolygonizer to polygonize each cubelet as it's considered,
// and PotentialPoints to find an initial cube on the surface and to determine when it's
// done walking the entire surface.
//

// Stupid STL name-mangling warning
#pragma warning (disable: 4786)

#include <fluid/Tess/SurfaceTessellator.h>
#include <fluid/Tess/IntVector.h>
#include <fluid/Utility/IntHash.h>

class ContinuousTessellator : public SurfaceTessellator
{
public:
  // Optionally called at the end of the frame to draw debug info in an ortho projection.
  virtual void drawDebugStatistics();

protected:
  virtual void childTessellate(const PotentialPoints&, float cubeletSize, float surfaceThreshold);

  // Used so child classes can specify how many debug levels they want to expose.
  virtual int getNumDebugLevels() { return 2; }

  // This is the function that starts the polygonization from a potential point.
  void walkSurfaceFromPoint(const PotentialPoints&, int startingPoint, const Vector& minCorner, float cubeletSize, float surfaceThreshold);

  // This is the function that recurses over the surface, polygonizing it cube by cube.
  void recurseOverSurface(unsigned short[], float);

  // Used by tessellate to set the cube to the right parameter space.
  Vector determineBoundingBox(const PotentialPoints& points, float cubeletSize);

  // This is a table used to track which points we've already considered the surfaces of.
  std::vector<bool> pointsConsidered;

  // This is a table of which cells have been considered.  This will likely need
  // to be optimized into something more efficient later.
  IntHash<int> cellsConsidered;

  // This is used as a counter to track the next unused surface id.
  int curSurfaceId;
};

#endif //CONTINUOUSTESSELLATOR_H