#include <fluid/SurfRep/PotentialPoints.h>
#include <algorithm>
#include <assert.h>

PotentialPoints::PotentialPoints()
{
  pointsDirty = false;
}

PotentialPoints::PotentialPoints(int size)
{
  points.reserve(size);
}

// Get the whole array (these are the preferred access methods.)
const PhysicalPoint* PotentialPoints::getPoints() const
{
  // If they're not mucking with it, they want it in sorted order.  It SHOULD be there already,
  // don't try to cover up a failure to finalize.
  assert(pointsDirty == false);

  return &(points[0]);
}

PhysicalPoint& PotentialPoints::getPoint(int index)
{
  // We assume they're screwing with it.
  pointsDirty = true;

  return points[index];
}

// Add a point.
void PotentialPoints::addPoint(const PhysicalPoint& newPoint)
{
  points.push_back(newPoint);

  // Oops, need to sort again.
  pointsDirty = true;
}

// Just takes one right off the end.
void PotentialPoints::removePoint()
{
  points.resize(points.size()-1);

  pointsDirty = true;
}

// These are like the STL functions of the same names.
void PotentialPoints::resize(int size, PhysicalPoint defaultValue)
{
  points.resize(size, defaultValue);
  pointsDirty = true;
}

void PotentialPoints::reserve(int capacity)
{
  points.reserve(capacity);
}

void PotentialPoints::clear()
{
  points.clear();
  buckets.clear();

  // By lieu of there being no points, they are not dirty (both containers are emptied, there's
  // nothing to sort.)
  pointsDirty = false;
}

int PotentialPoints::size() const
{
  return points.size();
}

int PotentialPoints::capacity() const
{
  return points.capacity();
}

// This functor is used as a sort predicate by the next function, getXSortedPoints.
class XCoordLess
{
public:
  bool operator()(const PhysicalPoint& p0, const PhysicalPoint& p1)
  {
    return p0.loc.x < p1.loc.x;
  }
};

// Used by SurfaceSampler (nobody else uses it right now, it's only good for getting
// points sorted along the x axis.)  It's no different than getPoints() except that
// it verifies that they're sorted by x coordinate first.
const PhysicalPoint* PotentialPoints::getXSortedPoints() const
{                       
//  if (pointsDirty)
//  {
//    std::sort<std::vector<PhysicalPoint>::iterator, XCoordLess>(points.begin(), points.end(), XCoordLess());
//  }
//  pointsDirty = false;

  return &(points[0]);
}

// Called after mucking with the points' positions is done to make sure they're
// in sorted order.  This is a bad thing to forget to do (I try to assert where
// I can, though.)
void PotentialPoints::finalizePointChanges()
{
  const float POINT_STRENGTH = 1.0f;

//  if (pointsDirty)
  {
    buckets.buildTable(&(points[0]), points.size(), POINT_STRENGTH);
//    std::sort<std::vector<PhysicalPoint>::iterator, XCoordLess>(points.begin(), points.end(), XCoordLess());
  }
  pointsDirty = false;
}

const BucketPartition* PotentialPoints::getBuckets() const
{
  assert(pointsDirty == false);
  return &buckets;
}

BucketPartition* PotentialPoints::getBuckets()
{
  assert(pointsDirty == false);
  return &buckets;
}

// Helper function to find the first point near (on the x axis) a point we care about.
int PotentialPoints::getFirstPotentialXContributor(float locX, float radius) const
{
  // Get them sorted along the x axis so we can binary search for only elements we might care about.
  // Yes, it only narrows down one of three axes, but that gets us a speedup and we don't have
  // to accumulate sets of interesting points and intersect them.  Though that may be worth playing
  // with later.
  int numPoints = size();
  const PhysicalPoint* points = getXSortedPoints();

  // Find an interesting point.
  int lower = 0;
  int upper = numPoints;
  int current = numPoints/2;
  while (points[current].loc.x-locX > radius || points[current].loc.x-locX < -radius)
  {
    // Narrow our range down appropriately.
    if (points[current].loc.x < locX)
    {
      lower = current;
    }
    else
    {
      upper = current;
    }
      
    // If we aren't moving it then stop, we ain't gots nothin'.
    if (current == ((upper + lower) >> 1))
    {
      return -1;
    }

    current = (upper + lower) >> 1;
  }

  // Now, presumably we found something.  Back up until we found the first interesting point.
  while (current > 0 && fabs(points[current-1].loc.x-locX) <= radius)
  {
    current--;
  }

  return current;
}
