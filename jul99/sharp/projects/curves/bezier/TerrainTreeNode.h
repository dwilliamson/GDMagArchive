#ifndef TERRAINTREENODE_H
#define TERRAINTREENODE_H

#include <curves/bezier/PatchBox.h>
#include <vector>

class ClipVolume;

class TerrainTreeNode
{
public:
  TerrainTreeNode();

  // We're responsible for deleting the data passed into us.
  ~TerrainTreeNode();

  // This sets the branches of this node.  They're initially null, indicating that this node
  // is a leaf node.  I put this all into one call to guarantee that either none of the
  // branches are set, or all of them are.
  void setBranches(TerrainTreeNode* maxXmaxY, TerrainTreeNode* maxXminY, TerrainTreeNode* minXmaxY, TerrainTreeNode* minXminY);

  // This uses the supplied clipVolume to determine which leaf nodes of the quadtree are
  // visible, and then stuffs those indices into the supplied vector.
  void getVisibleLeaves(const ClipVolume&, int xMin, int xMax, int yMin, int yMax, std::vector< std::pair<int,int> >& out);

  // This is the bounding box for this patch.
  const PatchBox& getBox() const;

  // This should only be called for leaf nodes, as setBranches builds a box from the branches'
  // boxes.
  void setBox(const PatchBox&);

protected:
  PatchBox boundingBox;

  TerrainTreeNode* maxXmaxY;
  TerrainTreeNode* maxXminY;
  TerrainTreeNode* minXmaxY;
  TerrainTreeNode* minXminY;
};

#endif //TERRAINTREENODE_H