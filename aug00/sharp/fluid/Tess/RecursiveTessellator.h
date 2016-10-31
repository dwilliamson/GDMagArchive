#ifndef RECURSIVETESSELLATOR_H
#define RECURSIVETESSELLATOR_H

//
// RecursiveTessellator
//
// RecursiveTessellator is responsible for recursing down an enclosing octree to find every 
// cubelet that lies on the surface (and having CubePolygonizer polygonize all those cubelets.)
//
// RecursiveTessellator depends on CubePolygonizer to polygonize each cubelet as it's considered,
// and PotentialPoints to use in culling and deciding when to recurse.
//

#include <fluid/Tess/SurfaceTessellator.h>

class RecursiveTessellator : public SurfaceTessellator
{
public:
protected:
  virtual void childTessellate(const PotentialPoints&, float cubeletSize, float surfaceThreshold);

  // Used so child classes can specify how many debug levels they want to expose.
  virtual int getNumDebugLevels() { return 3; }

  // Recursive routine called by tessellate to recurse down the octree.
  void recursiveTessellate(const PotentialPoints&, int[], int, float surfaceThreshold);

  // Used by tessellate to set the cube to the right parameter space.  Returns the
  // number of cubelets along each axis.
  int determineBoundingBox(const PotentialPoints&, float cubeletSize);

  // Used by tessellate to determine whether a given cube contains any surface info we care about.
  bool shouldRecurseOnCube(const PotentialPoints&, float surfaceThreshold);
};

#endif //RECURSIVETESSELLATOR_H