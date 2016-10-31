#ifndef SURFACESAMPLER_H
#define SURFACESAMPLER_H

// 
// SurfaceSampler
//
// The SurfaceSampler acts as a real-valued 3D function.  It uses a PotentialPoints set
// to determine for an argument location what the implicit function value is at that
// point.
//
// SurfaceSampler depends on PotentialPoints.
//

#include <math/Vector.h>
#include <fluid/SurfRep/PotentialPoints.h>

class SurfaceSampler
{
public:
  // This is also our default constructor; it's valid to have an 'uninitialized'
  // sampler (it just returns 0 or (0,0,0) for its two inquiries.)
  SurfaceSampler(const PotentialPoints* surfRepIn = 0);

  __inline float getValueAt(const Vector& loc) const;
  __inline Vector getNormalAt(const Vector& loc) const;

  void setPotentialPoints(const PotentialPoints* surfRepIn);

protected:
//  // Helper function to find the first point near (on the x axis) a point we care about.
//  int getFirstPotentialContributor(const Vector& loc) const;

  const PotentialPoints* surfRep;
};

// Inline functions.

#define SQUARE(a) ((a) * (a))

__inline float SurfaceSampler::getValueAt(const Vector& v0) const
{
  const float POINT_STRENGTH = 1.0f;
  float returnVal = 0;
  assert(surfRep != 0);
//  if (surfRep == 0)
//  {
//    return returnVal;
//  }

//  int index = surfRep->getFirstPotentialXContributor(v0.x, POINT_STRENGTH);
//  if (index == -1)
//  {
//    return 0;
//  }

  const BucketPartition* buckets = surfRep->getBuckets();
  const PhysicalPoint* node;
  unsigned short locMin[3];
  unsigned short locMax[3];
  buckets->getCellLocation(v0.x, v0.y, v0.z, locMin[0], locMin[1], locMin[2]);

  locMax[0] = locMin[0];
  locMax[1] = locMin[1];
  locMax[2] = locMin[2];

  buckets->expandNeighborhood(locMin[0],locMin[1],locMin[2], locMax[0],locMax[1],locMax[2]);

//  for (; SQUARE(surfRep->getPoint(index).loc.x-v0.x) <= POINT_STRENGTH && index < surfRep->size(); index++)
//  for (index=0; index < surfRep->size(); index++)
  for (int x=locMin[0]; x<=locMax[0]; x++)
  {
    for (int y=locMin[1]; y<=locMax[1]; y++)
    {
      for (int z=locMin[2]; z<=locMax[2]; z++)
      {
        for (node = buckets->find(x,y,z); node != 0; node = node->next)
        {
          Vector distanceVec = v0;
          distanceVec -= node->loc;
//          distanceVec -= surfRep->getPoint(index).loc;
          float distSquared = distanceVec.getMagnitudeSquared();

          if (distSquared < 1.0f)
          {
//            returnVal += (1.0f / (distSquared*distSquared)) - 1.0f;
            returnVal += (1.0f / distSquared) - 1.0f;
          }
//          returnVal += Math::maxOf(0, POINT_STRENGTH - distSquared);
        }
      }
    }
  }

  return returnVal;
}

__inline Vector SurfaceSampler::getNormalAt(const Vector& v0) const
{
  const float POINT_STRENGTH = 1.0f;
  Vector returnVal(0,0,0);
  assert(surfRep != 0);
//  if (surfRep == 0)
//  {
//    return returnVal;
//  }

  // The falloff function I'm using it's really differentiable, so I just had to make something
  // up for a tangent calculation that was really fast (this function alone was taking 40% of the
  // entire program time with a straightforward implementation) but looks reasonable.
//  int index = surfRep->getFirstPotentialXContributor(v0.x, POINT_STRENGTH);
//  if (index == -1)
//  {
//    return Vector(0,0,0);
//  }

  const BucketPartition* buckets = surfRep->getBuckets();
  const PhysicalPoint* node;
  unsigned short locMin[3];
  unsigned short locMax[3];
  buckets->getCellLocation(v0.x, v0.y, v0.z, locMin[0], locMin[1], locMin[2]);

  locMax[0] = locMin[0];
  locMax[1] = locMin[1];
  locMax[2] = locMin[2];

  buckets->expandNeighborhood(locMin[0],locMin[1],locMin[2], locMax[0],locMax[1],locMax[2]);

//  for (; SQUARE(surfRep->getPoint(index).loc.x-v0.x) <= POINT_STRENGTH && index < surfRep->size(); index++)
//  for (index=0; index < surfRep->size(); index++)
  int contribs = 0;

  for (int x=locMin[0]; x<=locMax[0]; x++)
  {
    for (int y=locMin[1]; y<=locMax[1]; y++)
    {
      for (int z=locMin[2]; z<=locMax[2]; z++)
      {
        for (node = buckets->find(x,y,z); node != 0; node = node->next)
        {
          Vector distanceVec = v0;
          distanceVec -= node->loc;
//          distanceVec -= surfRep->getPoint(index).loc;

          // This check prevents grid-artifacting appearing in the normal distribution.
          if (distanceVec.getMagnitudeSquared() <= POINT_STRENGTH)
          {
            // It should contribute less as it moves away...
            float invMagSquared = 1/distanceVec.getMagnitudeSquared();
            distanceVec *= invMagSquared;

            returnVal += distanceVec;
           
            contribs++;
          }
        }
      }
    }
  }

//  if (surfRep->size() > 0)
//  {
  // If the vertex interpolation left a vertex out of all points' influence, it'll end up with
  // a bogus normal, just find the closest point and use the vector from that.  Testing for equality
  // with 0 should be okay since the vector will have not been touched.
  if (contribs == 0)
  {
    // calculate it against all points in the 2-neighboring buckets, not just
    // the ones in the 1-neighborhood that are also near enough to contribute.
    buckets->expandNeighborhood(locMin[0],locMin[1],locMin[2], locMax[0],locMax[1],locMax[2]);

    for (int x=locMin[0]; x<=locMax[0]; x++)
    {
      for (int y=locMin[1]; y<=locMax[1]; y++)
      {
        for (int z=locMin[2]; z<=locMax[2]; z++)
        {
          for (node = buckets->find(x,y,z); node != 0; node = node->next)
          {
            Vector distanceVec = v0;
            distanceVec -= node->loc;

            // It should contribute less as it moves away...
            float invMagSquared = 1/distanceVec.getMagnitudeSquared();
            distanceVec *= invMagSquared;

            returnVal += distanceVec;

            contribs++;
          }
        }
      }
    }
    
    // This better not happen.
    assert(contribs != 0);
  }

  returnVal.fastNormalize();
//  }

  return returnVal;
}

#endif //SURFACESAMPLER_H