#include <curves/bezier/TerrainTreeNode.h>
#include <curves/bezier/ClipVolume.h>
#include <gl/glut.h>
#include <assert.h>

//#define min(x,y) ((x) < (y) ? (x) : (y))
//#define max(x,y) ((x) > (y) ? (x) : (y))

TerrainTreeNode::TerrainTreeNode() : boundingBox(0,0,0,0,0,0)
{
  maxXmaxY = 0;
  maxXminY = 0;
  minXmaxY = 0;
  minXminY = 0;
}

// We're responsible for deleting the data passed into us.
TerrainTreeNode::~TerrainTreeNode()
{
  delete maxXmaxY;
  delete maxXminY;
  delete minXmaxY;
  delete minXminY;
}

// This sets the branches of this node.  They're initially null, indicating that this node
// is a leaf node.  I put this all into one call to guarantee that either none of the
// branches are set, or all of them are.
void TerrainTreeNode::setBranches(TerrainTreeNode* maxXmaxY, TerrainTreeNode* maxXminY, TerrainTreeNode* minXmaxY, TerrainTreeNode* minXminY)
{
  this->maxXmaxY = maxXmaxY;
  this->maxXminY = maxXminY;
  this->minXmaxY = minXmaxY;
  this->minXminY = minXminY;

  // Now build our box out of theirs.
  float minX = minXmaxY->getBox().minX;
  float maxX = maxXminY->getBox().maxX;
  float minY = maxXminY->getBox().minY;
  float maxY = minXmaxY->getBox().maxY;

  float minZ = min( min(minXminY->getBox().minZ, minXmaxY->getBox().minZ), min(maxXminY->getBox().minZ, maxXmaxY->getBox().minZ) );
  float maxZ = max( max(minXminY->getBox().maxZ, minXmaxY->getBox().maxZ), max(maxXminY->getBox().maxZ, maxXmaxY->getBox().maxZ) );

  boundingBox = PatchBox(minX, maxX, minY, maxY, minZ, maxZ);
}

// This uses the supplied clipVolume to determine which leaf nodes of the quadtree are
// visible, and then stuffs those indices into the supplied vector.
void TerrainTreeNode::getVisibleLeaves(const ClipVolume& clipvol, int xMin, int xMax, int yMin, int yMax, std::vector< std::pair<int,int> >& out)
{
  // If we're visible at all...
  if (clipvol.contains(boundingBox))
  {
    // Now, if we're a leaf node...
    if (maxXmaxY == 0 || maxXminY == 0 || minXmaxY == 0 || minXminY == 0)
    {
      // First, some sanity checks.  If one was null, all better be null.
      assert(maxXmaxY == 0 && maxXminY == 0 && minXmaxY == 0 && minXminY == 0);

      // Also, xMin had better equal xMax+1 and yMin had better equal yMax+1
      assert((xMin+1 == xMax) && (yMin+1 == yMax));

      // Now, since we were inside, add our index (which is (xMin,yMin))
      out.push_back(std::pair<int,int>(xMin,yMin));
    }

    // Otherwise we're not a leaf node, so recurse.
    else
    {
      // Break up the mins and maxes.
      int xMid = (xMin + xMax) / 2;
      int yMid = (yMin + yMax) / 2;

      // Now pass those off to the child nodes.
      maxXmaxY->getVisibleLeaves(clipvol, xMid, xMax, yMid, yMax, out);
      maxXminY->getVisibleLeaves(clipvol, xMid, xMax, yMin, yMid, out);
      minXmaxY->getVisibleLeaves(clipvol, xMin, xMid, yMid, yMax, out);
      minXminY->getVisibleLeaves(clipvol, xMin, xMid, yMin, yMid, out);
    }
  }
}

// This is the bounding box for this patch.
const PatchBox& TerrainTreeNode::getBox() const
{
  return boundingBox;
}

// This should only be called for leaf nodes, as setBranches builds a box from the branches'
// boxes.
void TerrainTreeNode::setBox(const PatchBox& box)
{
  boundingBox = box;
}
