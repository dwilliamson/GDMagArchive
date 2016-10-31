#include <fluid/ParticleSys/ParticleGenerator.h>
#include <math.h>

ParticleGenerator::ParticleGenerator()
{
  particleRate = 0.05f;
  accruedTime = 0;
  useBBox = false;
}

// This function will wipe internal counters the generator uses (it accumulates the time passed
// so at any point you can have just under one particle-rate's worth of time leftover for the
// next frame, which can cause the premature creation of a particle when starting a new demo.
void ParticleGenerator::start()
{
  // Start this guy over.
  accruedTime = 0;
}

// These functions setup values for the new potential points; their distribution and velocity upon
// creation.

// Control the random velocity vector generation.  Velocity is generated pointing straight up and then
// rotated to the base vector, so the angle variances form a cone around the base velocity vector.
void ParticleGenerator::setBaseVelocity(const Vector& vel)
{
  baseVel = vel;
}

void ParticleGenerator::setVelAngleVariance(float theta, float phi)
{
  velThetaVary = theta;
  velPhiVary = phi;
}

// This is a scale, not a bias.  So putting a variance of 0.1 in will mean that the vector
// will be as little as 0.9 the magnitude of the base velocity, or as much as 1.1.
void ParticleGenerator::setVelMagVariance(float mag)
{
  velMagVary = mag;
}

// The new points are distributed randomly through a box.  Set the min and max to the same point
// to generate all new points at the same location.
void ParticleGenerator::setMinStartPoint(const Vector& mins)
{
  minStart = mins;
}

void ParticleGenerator::setMaxStartPoint(const Vector& maxs)
{
  maxStart = maxs;
}

// This is a placeholder; if I ever need more advanced particle-termination logic than this,
// I'll pull this out and make a ParticleKiller its own logic.
void ParticleGenerator::setParticleKillHeight(float height)
{
  killHeight = height;
}

void ParticleGenerator::setParticleBoundingBox(const BoundingBox& box)
{
  bbox = box;
  useBBox = true;
}

// Used as the target for the feedback loop.
void ParticleGenerator::setMaxNumPoints(int maxNum)
{
  maxNumPoints = maxNum;
}

// This function is called every physics tick to generate any needed points.
void ParticleGenerator::update(PotentialPoints& points, float timePassed)
{
  // Did someone cut back on the points?  If we have too many, remove them...
  while (points.size() > maxNumPoints)
  {
    points.removePoint();
  }

  for (int i=0; i<points.size(); i++)
  {
    if (useBBox)
    {
      if (!bbox.isInside(points.getPoint(i).loc))
      {
        genPoint(points.getPoint(i));
      }
    }
    else
    {
      if (points.getPoint(i).loc.z < killHeight)
      {
        genPoint(points.getPoint(i));
      }
    }
  }

  accruedTime += timePassed;

  while (accruedTime > particleRate)
  {
    // Still need to add points?
    if (points.size() < maxNumPoints)
    {
      PhysicalPoint newPoint;
      genPoint(newPoint);
      points.addPoint(newPoint);
    }

    // Done adding points, just refreshing existing ones now.
//    else
//    {
//      // Track the lowest point, he's the one we'll use.
//      float lowestZ = points.getPoint(0).loc.z;
//      int lowestIndex = 0;
//
//      // Be tricky, search in reverse sometimes so we don't just
//      // start clearing the same exact points over and over.
//      if (rand() % 2)
//      {
//        for (int i=0; i<points.size(); i++)
//        {
//          if (points.getPoint(i).loc.z < lowestZ)
//          {
//            lowestZ = points.getPoint(i).loc.z;
//            lowestIndex = i;
//          }
//        }
//      }
//      else
//      {
//        for (int i=points.size()-1; i>=0; i--)
//        {
//          if (points.getPoint(i).loc.z < lowestZ)
//          {
//            lowestZ = points.getPoint(i).loc.z;
//            lowestIndex = i;
//          }
//        }
//      }
//
//      if (lowestZ < killHeight)
//      {
//        genPoint(points.getPoint(lowestIndex));
//      }
//    }

    accruedTime -= particleRate;
  }

  points.finalizePointChanges();
}

// This controls the rate at which new particles are created (assuming we're not already at
// our limit.)
void ParticleGenerator::setParticleRate(float rate)
{
  particleRate = rate;
}
float ParticleGenerator::getParticleRate() const
{
  return particleRate;
}

#define RAND_NEG1_TO_POS1 (2.0f * (rand() / (float)RAND_MAX) - 1.0f)
#define RAND_ZERO_TO_POS1 (rand() / (float)RAND_MAX)

// Helper used to generate a new random point.
void ParticleGenerator::genPoint(PhysicalPoint& pt)
{
  // Get a randomized location.
  pt.loc = minStart;
  pt.loc.x += RAND_ZERO_TO_POS1 * (maxStart.x - minStart.x);
  pt.loc.y += RAND_ZERO_TO_POS1 * (maxStart.y - minStart.y);
  pt.loc.z += RAND_ZERO_TO_POS1 * (maxStart.z - minStart.z);

  pt.heightFieldTier = 0;

  pt.vel = baseVel;
  if (baseVel.getMagnitudeSquared() > 0)
  {
    pt.vel *= (1.0f + RAND_NEG1_TO_POS1 * velMagVary);
    pt.vel.setPhi(pt.vel.getPhi() + RAND_NEG1_TO_POS1 * velPhiVary);
    pt.vel.setTheta(pt.vel.getTheta() + RAND_NEG1_TO_POS1 * velThetaVary);
  }
}

