#include <fluid/ParticleSys/MoleculeAnimator.h>

// This is where you setup all the constants for the forces that this guy applies to particles.
// The first, equilibrium is the distance at which the points are not affecting each other.  When 
// they're closer than that distance, a repulsive force is applied that increases linearly as distance
// decreases; the second argument, repelTension, is a scalar on that force.  When the points are further
// than equilibrium apart, they will attract each other with a force that increases as they reach
// some other distance, equal to equilibrium*attractRange, away.  That force is scaled up by
// attractTension.  After a distance of equilibrium*attractRange, the attractive force falls off
// linearly to 0 by a distance of equilibrium*maxRange.
MoleculeAnimator::MoleculeAnimator(float equilibriumIn, float stiffDistIn, float stiffTensionIn,
                                   float maxStiffTension, float repelTensionIn, float attractRangeIn,
                                   float attractTensionIn, float maxRangeIn)
{
  // Compute more convenient values based off of these arguments.
  equilibrium = equilibriumIn;
  stiffDist = stiffDistIn*equilibrium;
  attractPeak = attractRangeIn*equilibrium;
  maxRange = maxRangeIn*equilibrium;

  stiffMax = maxStiffTension;
  stiffScalar = stiffTensionIn;
  repelScalar = repelTensionIn;
  attractScalar = attractTensionIn;

  // This scalar is completely determined by the distance between attractPeak and maxRange and the value
  // at attractPeak (at maxRange the value is 0.)
  attractFalloffScalar = -attractScalar * (attractPeak-equilibrium) / (maxRange - attractPeak);

  // Default to only attracting internally.
  attractAcrossSurfaces = false;
}

void MoleculeAnimator::setAttractAcrossSurfaces(bool val)
{
  attractAcrossSurfaces = val;
}

void MoleculeAnimator::update(PotentialPoints& points, float timePassed)
{
  // Loop variables.
  int i;

  // We use our partition to only check against our known neighbors.
  BucketPartition* buckets = points.getBuckets();

  PhysicalPoint* node;
  unsigned short locMin[3];
  unsigned short locMax[3];

  // First exert forces (change velocities) but don't change positions until all forces have
  // been calculated.
  for (i=0; i<points.size(); i++)
  {
    // Get the current point in consideration.
    PhysicalPoint& p0 = points.getPoint(i);
    int curId = p0.surfaceId;

    // Calc this guy's neighborhood.
    buckets->getCellLocation(p0.loc.x, p0.loc.y, p0.loc.z, locMin[0], locMin[1], locMin[2]);
    locMax[0] = locMin[0];
    locMax[1] = locMin[1];
    locMax[2] = locMin[2];
    buckets->expandNeighborhood(locMin[0],locMin[1],locMin[2], locMax[0],locMax[1],locMax[2]);
    buckets->expandNeighborhood(locMin[0],locMin[1],locMin[2], locMax[0],locMax[1],locMax[2]);

    for (int x=locMin[0]; x<=locMax[0]; x++)
    {
      for (int y=locMin[1]; y<=locMax[1]; y++)
      {
        for (int z=locMin[2]; z<=locMax[2]; z++)
        {
          for (node = buckets->find(x,y,z); node != 0; node = node->next)
          {
            // Cohesion means not touching points in different surfaces (unless we're going for that
            // T-1000 sentient liquid mercury effect...)
            if (attractAcrossSurfaces || (node->surfaceId == curId && node->surfaceId != 0))
            {
              // connect goes from p0 to p1.
              Vector connect = node->loc;
              connect -= p0.loc;

              float distSquared = connect.getMagnitudeSquared();

              // Make sure they're affecting each other at all.  I compare exactly to 0 because even the
              // slightest magnitude means that we have a vector between them to apply forces along.
              if (distSquared < maxRange*maxRange && distSquared > 0)
              {
                // We have to calculate this now (if we've made it this far we will need this.)
                float d = sqrt(distSquared);
  
                // This will determine the force applied to each.
                float forceMag;
  
                // Okay, break it into cases.
                if (d < stiffDist)
                {
                  forceMag = stiffScalar*(d-stiffDist) + repelScalar*(stiffDist-equilibrium);
                  forceMag = Math::maxOf(-stiffMax, forceMag);
                }
                else if (d < equilibrium)
                {
                  forceMag = repelScalar * (d - equilibrium);
                }
                else if (d < attractPeak)
                {
                  forceMag = attractScalar * (d - equilibrium);
                }
                else
                {
                  forceMag = attractScalar * (attractPeak-equilibrium) + attractFalloffScalar * (d - attractPeak);
                }
  
                // Apply that to one and inversely to the other.
                connect.fastNormalize();
                connect *= forceMag;
  
                p0.force += connect;
                node->force -= connect;
              }
            }
            // End consideration of a single neighbor.
          }
          // End bucket traversal
        }
      }
    }
    // End neighboring buckets traversal
  }
  // End applying all forces.

  // Now damp all points' velocities.
  for (i=0; i<points.size(); i++)
  {
    PhysicalPoint& p0 = points.getPoint(i);

    // Damp motion.
    Vector dampVel = p0.vel;
    dampVel *= 0.25*timePassed;
    p0.vel -= dampVel;
  }
}