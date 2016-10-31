#include <fluid/SurfRep/SurfaceSampler.h>

SurfaceSampler::SurfaceSampler(const PotentialPoints* surfRepIn)
{
  surfRep = surfRepIn;
}

void SurfaceSampler::setPotentialPoints(const PotentialPoints* surfRepIn)
{
  surfRep = surfRepIn;
}

//// Helper function to find the first point near (on the x axis) a point we care about.
//int SurfaceSampler::getFirstPotentialContributor(const Vector& loc) const
//{
//  // Get them sorted along the x axis so we can binary search for only elements we might care about.
//  // Yes, it only narrows down one of three axes, but that gets us a speedup and we don't have
//  // to accumulate sets of interesting points and intersect them.  Though that may be worth playing
//  // with later.
//  int numPoints = surfRep->size();
//  const PhysicalPoint* points = surfRep->getXSortedPoints();
//
//  // Find an interesting point.
//  int lower = 0;
//  int upper = numPoints;
//  int current = numPoints/2;
//  while (SQUARE(points[current].loc.x-loc.x) > POINT_STRENGTH)
//  {
//    // Narrow our range down appropriately.
//    if (points[current].loc.x < loc.x)
//    {
//      lower = current;
//    }
//    else
//    {
//      upper = current;
//    }
//      
//    // If lower and upper are touching, there's no applicable points, return a failure.
//    if (upper - lower <= 1)
//    {
//      return -1;
//    }
//
//    current = (upper + lower) / 2;
//  }
//
//  // Now, presumably we found something.  Back up until we found the first interesting point.
//  while (current > 0 && SQUARE(points[current-1].loc.x-loc.x) <= POINT_STRENGTH)
//  {
//    current--;
//  }
//
//  return current;
//}
