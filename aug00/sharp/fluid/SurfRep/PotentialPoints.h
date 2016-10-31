#ifndef POTENTIALPOINTS_H
#define POTENTIALPOINTS_H

//
// PotentialPoints
//
// PotentialPoints stores the basic surface representation, a bunch of PhysicalPoints.
// A PhysicalPoint has a position and strength, used in the implicit surface calculation,
// and physical properties like mass and velocity (used for animation.)
//
// PotentialPoints is dependency-free; it doesn't need to know how it's being used.  It is
// depended upon by all the ParticleSys objects, SurfaceSampler, and the high-level tessellation
// objects (derived classes of SurfaceTessellator.)
//

#include <math/Vector.h>
#include <vector>
#include <assert.h>
#include <fluid/SurfRep/BucketPartition.h>
#include <fluid/SurfRep/PhysicalPoint.h>

// This is the main class.
class PotentialPoints
{
public:
  PotentialPoints();
  PotentialPoints(int size);

  // Get the whole array.
  const PhysicalPoint* getPoints() const;

  __inline const PhysicalPoint& getPoint(int index) const;

  PhysicalPoint& getPoint(int index);

  // Add a point.
  void addPoint(const PhysicalPoint& newPoint);

  // Just takes one right off the end.
  void removePoint();

  // These are like the STL functions of the same names.
  void resize(int size, PhysicalPoint defaultValue = PhysicalPoint());
  void reserve(int capacity);
  void clear();
  int size() const;
  int capacity() const;

  // Called after mucking with the points' positions is done to make sure they're
  // in sorted order.  This is a bad thing to forget to do (I try to assert where
  // I can, though.)
  void finalizePointChanges();

  // Used by SurfaceSampler (nobody else uses it right now, it's only good for getting
  // points sorted along the x axis.  It's no different than getPoints() except that
  // it verifies that they're sorted by x coordinate first.
  const PhysicalPoint* getXSortedPoints() const;
  int getFirstPotentialXContributor(float locX, float radius) const;

  const BucketPartition* getBuckets() const;
  BucketPartition* getBuckets();

protected:
  // This mutable is only used in getXSortedPoints -- the sort order doesn't affect the points.
  mutable std::vector<PhysicalPoint> points;

  // This is maintained to allow us to defer sorting all the points together until the 
  // next time they're needed.
  mutable bool pointsDirty;

  // We use this to keep the points bucketed for fast lookups.
  BucketPartition buckets;
};

// Inline functions.
__inline const PhysicalPoint& PotentialPoints::getPoint(int index) const
{
  assert(pointsDirty == false);
  return points[index];
}

#endif //POTENTIALPOINTS_H