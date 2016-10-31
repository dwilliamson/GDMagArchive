#ifndef CUBEPOLYGONIZER_H
#define CUBEPOLYGONIZER_H

//
// CubePolygonizer
//
// CubePolygonizer is responsible for taking a MarchingCube and an isovalue and generating
// polygons inside the cube according to the cube corners' function values.  It deposits
// resulting polygons in the ImplicitMesh.  CubePolygonizer can also be asked whether a
// cubelet lies on the surface without actually depositing its polygons into the mesh if so,
// useful for stepping out from a potential point one cubelet at a time to find a cubelet
// on the surface.
//
// CubePolygonizer depends on SurfaceSampler to get the values of the cubelet corners and the
// surface normals of the resulting vertices, and depends on ImplicitMesh to hand over the new
// polygons.  It also depends on MarchingCube as the primary data structure.  SurfaceWalker
// depends on CubePolygonizer as it uses it at every step of the surface walk to polygonize
// the current cubelet.
//
// It would be possible to make CubePolygonizer an abstract base class for a Strategy, but 
// unless the need arises to support multiple different kinds of polygonizers (marching
// cubes vs. marching tetrahedrons, say) I probably won't.
//

// Stupid STL name-mangling warning.
#pragma warning (disable: 4786)

#include <fluid/SurfRep/SurfaceSampler.h>
#include <fluid/Render/ImplicitMesh.h>
#include <fluid/Tess/MarchingCube.h>
#include <fluid/Utility/IntHash.h>

class CubePolygonizer
{
public:
  CubePolygonizer();

  // Used to hand this guy pointers to objects he needs to know about.
  void setSurfaceSampler(const SurfaceSampler*);
  void setImplicitMesh(ImplicitMesh*);

  // Called to clear our cache (i.e. to tell us that we're starting a surface over.)
  void beginTessellation();
  
  // Neighbor bitmasks used if we need to do surface-following.
  enum NeighborFlags { NEIGHBOR_NEG_X=1, NEIGHBOR_POS_X=2, NEIGHBOR_NEG_Y=4, 
                       NEIGHBOR_POS_Y=8, NEIGHBOR_NEG_Z=16, NEIGHBOR_POS_Z=32 };

  // Polygon addition routine: big lookup table function.
  // The way the neighbor return value is formatted: bits 0 and 1 determine whether
  // the cube to your negative and positive x side lie on the surface, respectively,
  // bits 2 and 3 determine negative and positive y, and 4 and 5 are negative and positive z.
  enum CubeletLocation { ON_SURFACE, INSIDE_SURFACE, OUTSIDE_SURFACE };
  CubeletLocation polygonize(const MarchingCube* cube, float surfaceThreshold, int& neighborsOut, bool addPolys = true);

  // Optionally called at the end of the frame to draw debug info in an ortho projection.
  void drawDebugStatistics();

protected:
  // Cubelet-corner sampling function.
  void evaluateCubelet(float[]);

  // Used to interpolate an edge of the current cubelet.
  void vertexInterp(int corner0, int corner1, float[], float surfaceThreshold, int& out);

  // Surface evaluator.
  const SurfaceSampler* sampler;

  // Mesh where we put new polys.
  ImplicitMesh* mesh;

  // We hold onto this during a polygonize() call so internal functions can use it.
  const MarchingCube* currCube;

  // We use this to optimize the function calculation -- to cache them so we don't recalculate the
  // same corner up to eight times for the surrounding cubelets that use it.
  IntHash<float> cornerValueCache;

  // We use this to cache vertex interpolation and normal calculation.
  IntHash<int> edgeVertexCache;
};

#endif //CUBEPOLYGONIZER_H