#include <curves/bezier/TerrainQuadtree.h>
#include <curves/bezier/BezierPatch.h>
#include <curves/bezier/UniformPatchTessellator.h>
#include <curves/bezier/CentralPatchTessellator.h>
#include <curves/bezier/ClipVolume.h>
#include <curves/GlobalCamera.h>
#include <curves/CurvePoint.h>
#include <assert.h>

// Stupid STL too-long-for-name-mangler warning.
#pragma warning (disable: 4786)

TerrainQuadtree::TerrainQuadtree()
{
  dim = 0;
  fixInterCracks = true;
}

// This sets the size of the quadtree to edgeSize x edgeSize, where edgeSize must be a power
// of two.
void TerrainQuadtree::setSize(int edgeSize)
{
  // Make sure it's a power of two.  Within reason.  We don't support 1024x1024 grids of terrain.
  if (edgeSize != 1 && edgeSize != 2 && edgeSize != 4 && edgeSize != 8 && edgeSize != 16 &&
      edgeSize != 32 && edgeSize != 64 && edgeSize != 128 && edgeSize != 256 && edgeSize != 512)
  {
    assert(false);
  }

  // Now resize it.
  patches.resize(edgeSize*edgeSize);
  tessellators.resize(edgeSize*edgeSize);
  dim = edgeSize;
}

// Adds the patch at the specified location in the grid.
void TerrainQuadtree::setPatch(const BezierPatch& patch, int x, int y)
{
  patches[x + y*dim] = patch;
}

// Called to setup culling data (this is the application's way of telling us it's done
// filling in the patches.  If the patches are regenerated, this should again be called
// after resetting all of them.
void TerrainQuadtree::finalizePatches()
{
  // Okay, now we have to build a tree out of these patches.
#define INDEX(x,y,size) ((x) + (y)*(size))
  
  // Start by making our leaves.  A dim*dim array of TerrainTreeNodes, with bounding boxes
  // of their respective patches.
  std::vector<TerrainTreeNode*> leaves;
  leaves.resize(dim*dim);
  for (int nodeCt=0; nodeCt<dim*dim; nodeCt++)
  {
    leaves[nodeCt] = new TerrainTreeNode;

    // Get the minima and maxima of this patch.
    float minX = 9999999, maxX = -9999999, minY = 9999999, maxY = -9999999, minZ = 9999999, maxZ = -9999999;
    for (int u=0; u<4; u++)
    {
      for (int v=0; v<4; v++)
      {
        minX = min(minX, patches[nodeCt].getControlPoint(u,v)->getX());
        maxX = max(maxX, patches[nodeCt].getControlPoint(u,v)->getX());
        minY = min(minY, patches[nodeCt].getControlPoint(u,v)->getY());
        maxY = max(maxY, patches[nodeCt].getControlPoint(u,v)->getY());
        minZ = min(minZ, patches[nodeCt].getControlPoint(u,v)->getZ());
        maxZ = max(maxZ, patches[nodeCt].getControlPoint(u,v)->getZ());
      }
    }

    PatchBox box(minX, maxX, minY, maxY, minZ, maxZ);
    leaves[nodeCt]->setBox(box);
  }

  // Now make another array like leaves, and two pointers that we can swap.
  std::vector<TerrainTreeNode*> treeLevel;
  std::vector<TerrainTreeNode*>* curLevel = &treeLevel;
  std::vector<TerrainTreeNode*>* lowerLevel = &leaves;

  // Now, run through lowerLevel and add each 2x2 square of nodes as children of a node in
  // curLevel.
  int curEdgeSize = dim;

  // Now, loop, creating levels until we're done.
  do
  {
    curEdgeSize /= 2;
    for (int y=0; y<curEdgeSize; y++)
    {
      for (int x=0; x<curEdgeSize; x++)
      {
        curLevel->push_back(new TerrainTreeNode());
        (*curLevel)[INDEX(x,y,curEdgeSize)]->setBranches(
                                              (*lowerLevel)[INDEX((x*2)+1,(y*2)+1,curEdgeSize*2)],
                                              (*lowerLevel)[INDEX((x*2)+1,(y*2)+0,curEdgeSize*2)],
                                              (*lowerLevel)[INDEX((x*2)+0,(y*2)+1,curEdgeSize*2)],
                                              (*lowerLevel)[INDEX((x*2)+0,(y*2)+0,curEdgeSize*2)]
                                                        );
      }
    }

    // Empty the lower level and swap pointers.
    lowerLevel->clear();
    std::vector<TerrainTreeNode*>* swap = curLevel;
    curLevel = lowerLevel;
    lowerLevel = swap;

  } while (curEdgeSize > 2);

  assert(curEdgeSize == 2);

  // Okay, we now have our result in lowerLevel.  Pass it to topNode.
  topNode.setBranches((*lowerLevel)[INDEX(1,1,curEdgeSize)],
                      (*lowerLevel)[INDEX(1,0,curEdgeSize)],
                      (*lowerLevel)[INDEX(0,1,curEdgeSize)],
                      (*lowerLevel)[INDEX(0,0,curEdgeSize)]);

  // Done!
}

// Uses GlobalCamera to cull out the patches that are invisible, draws those that remain.
void TerrainQuadtree::draw()
{
  std::vector<std::pair<int,int> > visibles;
  ClipVolume cameraVol(*(GlobalCamera::Instance()));
  topNode.getVisibleLeaves(cameraVol, 0, dim, 0, dim, visibles);

  // Turn crack-fixing off because we have to put it off until after we've fixed inter-patch
  // cracking.
  bool fixCracks = getFixCracks();
  for (int vis=0; vis<visibles.size(); vis++)
  {
    tessellators[visibles[vis].first + dim*visibles[vis].second].setFixCracks(false);
  }

  // Now, tessellate the visible patches.
  for (vis=0; vis<visibles.size(); vis++)
  {
    patches[visibles[vis].first + dim*visibles[vis].second].tessellate(&(tessellators[visibles[vis].first + dim*visibles[vis].second]));
  }

  // Now fix any cracking that occurred.
  if (fixInterCracks)
  {
    fixInterPatchCracks(visibles);
  }

  // We fix internal cracks after we've fixed inter-cracking.
  for (vis=0; vis<visibles.size(); vis++)
  {
    CentralPatchTessellator* t = &tessellators[visibles[vis].first + dim*visibles[vis].second];
    if (fixCracks)
    {
      t->fixCracks(t->getMinXMinYIndex(), t->getMaxXMinYIndex(), t->getMinXMaxYIndex(), t->getMaxXMaxYIndex());
    }
  }

  // Turn crack-fixing back to what it was.
  for (vis=0; vis<visibles.size(); vis++)
  {
    tessellators[visibles[vis].first + dim*visibles[vis].second].setFixCracks(fixCracks);
  }

  // Now draw the results.
  for (vis=0; vis<visibles.size(); vis++)
  {
    patches[visibles[vis].first + dim*visibles[vis].second].draw(&(tessellators[visibles[vis].first + dim*visibles[vis].second]));
  }
}

void TerrainQuadtree::setFixInterCracks(bool newVal)
{
  fixInterCracks = newVal;
}

bool TerrainQuadtree::getFixInterCracks() const
{
  return fixInterCracks;
}

void TerrainQuadtree::fixInterPatchCracks(const std::vector<std::pair<int,int> >& visibles)
{
  // Since we know that our camera volume is convex, the visible patches must form 
  // a convex polygon of patches (where each patch is a pixel in the rasterized polygon).
  // Therefore, we do this like an inverse rasterizer by building edge tables from the
  // polygon data.  From that, we know the neighbors of any cell and whether they're visible.
  
  // Start with exagerrated minimum and maximum y values
  int yMin =  9999999;
  int yMax = -9999999;
  // For each element in the visible list
  for (int ct=0; ct<visibles.size(); ct++)
  {
    // If its y is less than the minimum, update the minimum.
    yMin = min(yMin, visibles[ct].second);

    // If its  is greater than the maximum, update the maximum.
    yMax = max(yMax, visibles[ct].second);
  }// Done finding min and max .

  // Now allocate tables for the minimum and maximum x values.
  int* xMins = new int[yMax - yMin + 1];
  int* xMaxs = new int[yMax - yMin + 1];
  // Initialize the values to exagerratedly wrong values (very large mins, very small maxes.)
  for (ct=0; ct<=yMax-yMin; ct++)
  {
    xMins[ct] =  9999999;
    xMaxs[ct] = -9999999;
  }

  // For each element in the visible list,
  for (ct=0; ct<visibles.size(); ct++)
  {
    // If its x is less than the minimum x for its y value, update the minimum.
    xMins[visibles[ct].second-yMin] = min(xMins[visibles[ct].second-yMin], visibles[ct].first);

    // If its x is greater than the maximum x for its y value, update the maximum.
    xMaxs[visibles[ct].second-yMin] = max(xMaxs[visibles[ct].second-yMin], visibles[ct].first);
  } // Done finding min and max x values.

  // Those are our edge tables.  Now, run through and for each visible patch
  for (ct=0; ct<visibles.size(); ct++)
  {
    int curX = visibles[ct].first;
    int curY = visibles[ct].second;

    // See if the patch to its +x is visible.  If so...
    if (xMaxs[curY-yMin] >= curX + 1)
    {
      // ... fix any cracks between them.
      CentralPatchTessellator* t0 = &(tessellators[curX   + (curY*dim)]);
      CentralPatchTessellator* t1 = &(tessellators[curX+1 + (curY*dim)]);
//      t0->fixInterPatchCracks(t0->getMaxXMinYIndex(), t0->getMaxXMaxYIndex(), 
//                              t1->getMinXMinYIndex(), t1->getMinXMaxYIndex(), 
//                              t1);
      int t0v = t0->getNumEdgeVerts()-1;
      int t1v = t1->getNumEdgeVerts()-1;
      t0->fixInterPatchCracks(t0v,0,t0v,t0v,  0,0,0,t1v,  t1);
    }
    // See if the patch to its +y is visible.  If so...
    if (curY+1 <= yMax && xMaxs[curY+1-yMin] >= curX && xMins[curY+1-yMin] <= curX)
    {
      // ... fix any cracks between them.
      // ... fix any cracks between them.
      CentralPatchTessellator* t0 = &(tessellators[curX + ((curY+0)*dim)]);
      CentralPatchTessellator* t1 = &(tessellators[curX + ((curY+1)*dim)]);
//      t0->fixInterPatchCracks(t0->getMinXMaxYIndex(), t0->getMaxXMaxYIndex(), 
//                              t1->getMinXMinYIndex(), t1->getMaxXMinYIndex(), 
//                              t1);
      int t0v = t0->getNumEdgeVerts()-1;
      int t1v = t1->getNumEdgeVerts()-1;
      t0->fixInterPatchCracks(0,t0v,t0v,t0v,  0,0,t1v,0,  t1);
    }
  }

  delete[] xMins;
  delete[] xMaxs;
}

// Just tell them whether the (x,y) is above / below us or not (off the side).
bool TerrainQuadtree::contains(float x, float y)
{
  const PatchBox& box = topNode.getBox();
  if (box.minX <= x && box.maxX >= x && box.minY <= y && box.maxY >= y)
  {
    return true;
  }
  return false;
}

float TerrainQuadtree::getValueAt(float x, float y)
{
  const PatchBox& box = topNode.getBox();

  // Now find the patch it's in.
  float widthScalar = box.maxX - box.minX;
  float heightScalar = box.maxY - box.minY;

  float patchX = (x-box.minX) / widthScalar;
  float patchY = (y-box.minY) / heightScalar;

  patchX *= dim;
  patchY *= dim;

  int px = (int)patchX;
  int py = (int)patchY;

  return patches[px + py*dim].getValueAt(x,y);
}

Vector TerrainQuadtree::getNormalAt(float x, float y)
{
  const PatchBox& box = topNode.getBox();

  // Now find the patch it's in.
  float widthScalar = box.maxX - box.minX;
  float heightScalar = box.maxY - box.minY;

  float patchX = (x-box.minX) / widthScalar;
  float patchY = (y-box.minY) / heightScalar;

  patchX *= dim;
  patchY *= dim;

  int px = (int)patchX;
  int py = (int)patchY;

  return patches[px + py*dim].getNormalAt(x,y);
}

Vector TerrainQuadtree::getGradientAt(float x, float y)
{
  const PatchBox& box = topNode.getBox();

  // Now find the patch it's in.
  float widthScalar = box.maxX - box.minX;
  float heightScalar = box.maxY - box.minY;

  float patchX = (x-box.minX) / widthScalar;
  float patchY = (y-box.minY) / heightScalar;

  patchX *= dim;
  patchY *= dim;

  int px = (int)patchX;
  int py = (int)patchY;

  return patches[px + py*dim].getGradientAt(x,y);
}

void TerrainQuadtree::getLightmapAt(float x, float y, unsigned int& texName, float& u, float& v)
{
  const PatchBox& box = topNode.getBox();

  // Now find the patch it's in.
  float widthScalar = box.maxX - box.minX;
  float heightScalar = box.maxY - box.minY;

  float patchX = (x-box.minX) / widthScalar;
  float patchY = (y-box.minY) / heightScalar;

  patchX *= dim;
  patchY *= dim;

  int px = (int)patchX;
  int py = (int)patchY;

  patches[px + py*dim].getLightmapAt(x, y, texName, u, v);
}

// Passthrough functions for the patches and tessellators.
#define FOR_ALL_TESSELLATORS for (int tessCt=0; tessCt<tessellators.size(); tessCt++)
#define TESSELLATOR tessellators[tessCt]

#define FOR_ALL_PATCHES for (int patchCt=0; patchCt<patches.size(); patchCt++)
#define PATCH patches[patchCt]

void TerrainQuadtree::setGranularity( float newVal ) {
  FOR_ALL_TESSELLATORS
  {
    TESSELLATOR.setGranularity(newVal);
  }
}

float TerrainQuadtree::getGranularity() const {
  if (tessellators.size() == 0)
  {
    return 0;
  }
  return tessellators[0].getGranularity();
}

void TerrainQuadtree::setMode( GLenum newVal ) {
  FOR_ALL_TESSELLATORS
  {
    TESSELLATOR.setMode(newVal);
  }
}

GLenum TerrainQuadtree::getMode() const {
  if (tessellators.size() == 0)
  {
    return GL_FILL;
  }
  return tessellators[0].getMode();
}

void TerrainQuadtree::setFixCracks( bool newVal ) {
  FOR_ALL_TESSELLATORS
  {
    TESSELLATOR.setFixCracks(newVal);
  }
}

bool TerrainQuadtree::getFixCracks() const {
  if (tessellators.size() == 0)
  {
    return false;
  }
  return tessellators[0].getFixCracks();
}

void TerrainQuadtree::setMaxDepth( int newVal ) const {
  FOR_ALL_TESSELLATORS
  {
    TESSELLATOR.setMaxDepth(newVal);
  }
}

int TerrainQuadtree::getMaxDepth() const {
  if (tessellators.size() == 0)
  {
    return 1;
  }
  return tessellators[0].getMaxDepth();
}

void TerrainQuadtree::setUseLightmap(bool newVal) {
  FOR_ALL_PATCHES
  {
    PATCH.setUseLightmap(newVal);
  }
}

void TerrainQuadtree::setUseTexture(bool newVal) {
  FOR_ALL_PATCHES
  {
    PATCH.setUseTexture(newVal);
  }
}

bool TerrainQuadtree::getUseLightmap() const {
  if (patches.size() == 0)
  {
    return false;
  }
  return patches[0].getUseLightmap();
}

bool TerrainQuadtree::getUseTexture() const {
  if (patches.size() == 0)
  {
    return false;
  }
  return patches[0].getUseTexture();
}

void TerrainQuadtree::setTexture(int newVal) {
  FOR_ALL_PATCHES
  {
    PATCH.setTexture(newVal);
  }
}

int TerrainQuadtree::getTexture() const {
  if (patches.size() == 0)
  {
    return false;
  }
  return patches[0].getTexture();
}

#undef FOR_ALL_TESSELLATORS
#undef TESSELLATOR
#undef FOR_ALL_PATCHES
#undef PATCH