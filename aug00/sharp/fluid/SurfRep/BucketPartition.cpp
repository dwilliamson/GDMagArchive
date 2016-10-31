#include <fluid/SurfRep/BucketPartition.h>
#include <fluid/SurfRep/PhysicalPoint.h>
#include <math.h>

BucketPartition::BucketPartition()
{
  table = 0;
  numHashBins = 0;
  xCells = yCells = zCells = 0;
  xBits = yBits = zBits = 0;
  xMask = yMask = zMask = 0;

  xMin = xRange = yMin = yRange = zMin = zRange = 0;
}

BucketPartition::~BucketPartition()
{
  // Empty us out, we're going away.
  delete[] table;
}

// Wipes the hash table of values.
void BucketPartition::clear()
{
  for (int x=0; x<numHashBins; x++)
  {
    table[x] = 0;
  }
}

// This is used to specify the bounding box in world coordinates that the buckets fill.  This is
// so that we can sort the nodes into it and also so that we can implement the getCellLocation utility
// function.
void BucketPartition::setBoundingBox(float xMinIn, float xRangeIn, float yMinIn, float yRangeIn, float zMinIn, float zRangeIn)
{
  xMin = xMinIn;
  xRange = xRangeIn;
  yMin = yMinIn;
  yRange = yRangeIn;
  zMin = zMinIn;
  zRange = zRangeIn;
}

// Sets the size of a bucket in world space.  It should be equal to POINT_STRENGTH, the maximum radius
// at which a point has influence.  This way any location in a cubelet can only be contributed to by 
// potential points in that cubelet and its 8 immediate neighbors.
void BucketPartition::setBucketWorldSize(float size)
{
  // We'll have to restart this guy anyway.
  delete[] table;

  // Given the world bounding box, figure out how many cubelets we need along each axis and round up to
  // the nearest power of two for each of them.
  float numBucketsX = xRange / size;
  float numBucketsY = yRange / size;
  float numBucketsZ = zRange / size;

  // Round up to next power of two.
  xBits = (int)ceil(log(numBucketsX) / log(2.0f));
  xCells = 1 << xBits;
  xMask = xCells-1;
  
  yBits = (int)ceil(log(numBucketsY) / log(2.0f));
  yCells = 1 << yBits;
  yMask = yCells-1;

  zBits = (int)ceil(log(numBucketsZ) / log(2.0f));
  zCells = 1 << zBits;
  zMask = zCells-1;

  // Adjust the bounding box accordingly.
  xRange = xCells * size;
  yRange = yCells * size;
  zRange = zCells * size;

  // Allocate the table space again.
  numHashBins = xCells*yCells*zCells;
  table = new PhysicalPoint*[numHashBins];
  clear();
}

// Called once per frame to sort the nodes into their respective buckets.
void BucketPartition::buildTable(PhysicalPoint* nodeArray, int numNodes, float pointStrength)
{
  if (numNodes == 0)
  {
    clear();
    return;
  }

  // Setup the tables.
  float min[3] = {nodeArray[0].loc.x-pointStrength, nodeArray[0].loc.y-pointStrength, nodeArray[0].loc.z-pointStrength};
  float max[3] = {nodeArray[0].loc.x+pointStrength, nodeArray[0].loc.y+pointStrength, nodeArray[0].loc.z+pointStrength};
  for (int x=1; x<numNodes; x++)
  {
    min[0] = Math::minOf(min[0], nodeArray[x].loc.x-pointStrength);
    min[1] = Math::minOf(min[1], nodeArray[x].loc.y-pointStrength);
    min[2] = Math::minOf(min[2], nodeArray[x].loc.z-pointStrength);

    max[0] = Math::maxOf(max[0], nodeArray[x].loc.x+pointStrength);
    max[1] = Math::maxOf(max[1], nodeArray[x].loc.y+pointStrength);
    max[2] = Math::maxOf(max[2], nodeArray[x].loc.z+pointStrength);
  }

  setBoundingBox(min[0], max[0]-min[0], min[1], max[1]-min[1], min[2], max[2]-min[2]);

  // A little slop to be safe (and reduce grid-artifacts.)
  setBucketWorldSize(pointStrength*1.1f);

  unsigned short cx, cy, cz;
  for (x=0; x<numNodes; x++)
  {
    getCellLocation(nodeArray[x].loc.x, nodeArray[x].loc.y, nodeArray[x].loc.z, cx, cy, cz);
    int cell = makeHashKey(cx, cy, cz);
    
    // Find the node we'll stick this guy onto.
    PhysicalPoint* lastNode = table[cell];

    // Special case: if the list is empty, make it the first guy.
    if (lastNode == 0)
    {
      table[cell] = &(nodeArray[x]);
      table[cell]->next = 0;
    }
    else
    {
      while (lastNode->next != 0) 
      {
        lastNode = lastNode->next;
      }

      // Stick it on there and set its next node to null.
      lastNode->next = &(nodeArray[x]);
      lastNode->next->next = 0;
    }
  }
}
