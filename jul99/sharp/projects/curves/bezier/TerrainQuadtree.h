#ifndef TERRAINQUADTREE_H
#define TERRAINQUADTREE_H

#include <curves/bezier/TerrainTreeNode.h>
#include <gl/glut.h>

class BezierPatch;
class CentralPatchTessellator;

class TerrainQuadtree
{
public:
  TerrainQuadtree();

  // This sets the size of the quadtree to edgeSize x edgeSize, where edgeSize must be a power
  // of two.
  void setSize(int edgeSize);

  // Adds the patch at the specified location in the grid.
  void setPatch(const BezierPatch&, int x, int y);

  // Called to setup culling data (this is the application's way of telling us it's done
  // filling in the patches.  If the patches are regenerated, this should again be called
  // after resetting all of them.
  void finalizePatches();

  // Uses GlobalCamera to cull out the patches that are invisible, draws those that remain.
  void draw();

  void setFixInterCracks(bool newVal);
  bool getFixInterCracks() const;

  // Utility / evaluation functions.
  bool contains(float x, float y);
  float getValueAt(float x, float y);
  Vector getNormalAt(float x, float y);
  Vector getGradientAt(float x, float y);
  void getLightmapAt(float x, float y, unsigned int& texName, float& u, float& v);

  // Functions that pass through to the patches or the tessellators.
  void setGranularity( float newVal );
  float getGranularity() const;
  void setMode( GLenum newVal );
  GLenum getMode() const;
  void setFixCracks( bool newVal );
  bool getFixCracks() const;
  void setMaxDepth( int newVal ) const;
  int getMaxDepth() const;
  void setUseLightmap(bool newVal);
  void setUseTexture(bool newVal);
  bool getUseLightmap() const;
  bool getUseTexture() const;
  void setTexture(int newVal);
  int getTexture() const;

protected:
  // Used to ensure there are no inter-patch cracking issues 
  void fixInterPatchCracks(const std::vector<std::pair<int,int> >& visibles);

  TerrainTreeNode topNode;
  std::vector<BezierPatch> patches;
  std::vector<CentralPatchTessellator> tessellators;
  int dim;

  bool fixInterCracks;
};

#endif //TERRAINQUADTREE_H