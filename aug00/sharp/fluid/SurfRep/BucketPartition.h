#ifndef BUCKETPARTITION_H
#define BUCKETPARTITION_H

//
// BucketPartition
//
// BucketPartition may look somewhat similar to IntHash, and that's because it's a (very) modified copy of
// IntHash.  The problem is that while they could share logic, they do different things -- IntHash is a
// hashtable that associates a three-vector of unsigned shorts with a specific object; BucketPartition is
// an object that associates a 3-vector of some precision (depending on the number and size of the buckets
// desired) with a linked list of PhysicalPoints.
//

#include <assert.h>

class PhysicalPoint;

// The hashtable class.
class BucketPartition
{
public:
  BucketPartition();
  ~BucketPartition();

  // Wipes the hash table of values.
  void clear();

  // Called once per frame to sort the nodes into their respective buckets.
  void buildTable(PhysicalPoint*, int numNodes, float pointStrength);

  // Utility function to tell you what cell you're in given a world space location.
  __inline void getCellLocation(float wx, float wy, float wz, unsigned short& cx, unsigned short& cy, unsigned short& cz) const;

  // Utility function to expand a neighborhood but clamp the bucket edges.
  __inline void expandNeighborhood(unsigned short&, unsigned short&, unsigned short&,
                                   unsigned short&, unsigned short&, unsigned short&) const;

  // Lookup to get the linked list of nodes in a given bucket.
  __inline const PhysicalPoint* find(unsigned short x, unsigned short y, unsigned short z) const;

  // Lookup to get the linked list of nodes in a given bucket.
  __inline PhysicalPoint* find(unsigned short x, unsigned short y, unsigned short z);

protected:
  // This is used to specify the bounding box in world coordinates that the buckets fill.  This is
  // so that we can sort the nodes into it and also so that we can implement the getCellLocation utility
  // function below.
  void setBoundingBox(float xMin, float xRange, float yMin, float yRange, float zMin, float zRange);

  // Sets the size of a bucket in world space.  It should be equal to POINT_STRENGTH, the maximum radius
  // at which a point has influence.  This way any location in a cubelet can only be contributed to by 
  // potential points in that cubelet and its 8 immediate neighbors.
  void setBucketWorldSize(float size);

  // Used to form the hash key.
  __inline int makeHashKey(unsigned short x, unsigned short y, unsigned short z) const;

  PhysicalPoint** table;
  int numHashBins; 

  // The range of our parameter space and the number of cells inside.
  float xMin, xRange, yMin, yRange, zMin, zRange;
  
  // These are all calculated from the setBitsPerComponent call, and stored for quick access.
  // So xBits is directly set from the function.  xCells = 1 << xBits, and xMask = xCells-1.
  int xCells, yCells, zCells;
  int xBits, yBits, zBits;
  int xMask, yMask, zMask;
};

// Inline functions.

// Utility function to tell you what cell you're in given a world space location.
__inline void BucketPartition::getCellLocation(float wx, float wy, float wz, unsigned short& cx, unsigned short& cy, unsigned short& cz) const
{
  assert(numHashBins > 0);
  cx = (unsigned short)(xCells * (wx - xMin) / xRange);
  cy = (unsigned short)(yCells * (wy - yMin) / yRange);
  cz = (unsigned short)(zCells * (wz - zMin) / zRange);
}

// Utility function to expand a neighborhood but clamp the bucket edges.
__inline void BucketPartition::expandNeighborhood(unsigned short& locMinX, unsigned short& locMinY, unsigned short& locMinZ, 
                                                  unsigned short& locMaxX, unsigned short& locMaxY, unsigned short& locMaxZ) const
{
  if (locMinX != 0) locMinX--;
  if (locMinY != 0) locMinY--;
  if (locMinZ != 0) locMinZ--;
  if (locMaxX < xCells-1) locMaxX++;
  if (locMaxY < yCells-1) locMaxY++;
  if (locMaxZ < zCells-1) locMaxZ++;
}

// Lookup to also get the value of a lookup back.  Returns null if not found.
__inline const PhysicalPoint* BucketPartition::find(unsigned short x, unsigned short y, unsigned short z) const
{
  assert(numHashBins > 0);
  int cell = makeHashKey(x,y,z);
  return table[cell];
}

__inline PhysicalPoint* BucketPartition::find(unsigned short x, unsigned short y, unsigned short z)
{
  assert(numHashBins > 0);
  int cell = makeHashKey(x,y,z);
  return table[cell];
}

// Used to form the hash key.
__inline int BucketPartition::makeHashKey(unsigned short x, unsigned short y, unsigned short z) const
{
  // We take a third the bits from each component.  We use the lower-order bits so it's like a mod-table
  // instead of clustering nearby cubelets into the same table location.
  int key;

  key = x & xMask;
  key <<= yBits;

  key |= y & yMask;
  key <<= zBits;

  key |= z & zMask;

  return key;
}

#endif //BUCKETPARTITION_H