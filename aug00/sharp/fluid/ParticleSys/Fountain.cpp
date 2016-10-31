#include <fluid/ParticleSys/Fountain.h>
#include <math.h>
#include <math/Math.h>

const float GRAVITY = -9.8f;

// Access methods used to set this guy up.  Call these in the order they're declared here.
// (Some depend on others having been called.  Peak height is relative to the start point
// height, so if you raise the start point the fountain will rise to a higher peak.
void Fountain::setMinPhi(float phi)
{
  minPhi = phi;
}

void Fountain::setMaxPhi(float phi)
{
  maxPhi = phi;
}

void Fountain::setStartPoint(const Vector& pt)
{
  startLoc = pt;
}

void Fountain::setPeakHeight(float height)
{
  maxHeight = height;

  // Calculate the necessary upwards force to get points to that peak.  From s = 0.5*a*t^2 + v0*t
  // we get v0 = sqrt(-2*a*s)
  zForce = sqrt(-2.0f * GRAVITY * height);
}

void Fountain::setFloor(float height)
{
  minHeight = height;
}

// This, specifically, must be called last as it uses the variables set in the above to figure
// out a particle rate to keep the fountain evenly distributed at the max number of particles.
void Fountain::setMaxNumPoints(int maxNum)
{
  const float PILEUP_POINTS_PORTION = 0.2f;

  // Figure out total travel time, given the upwards force and the floor.  Good old quadratic
  // formula.  In this case the quadratic eqn is 0.5*at^2 + v0*t + (s0-s) = 0
  float particleLifetime = (-zForce - sqrt(zForce*zForce - 2*GRAVITY*(startLoc.z-minHeight)))/GRAVITY;
  pointRefreshTime = particleLifetime / (maxNum * (1.0f-PILEUP_POINTS_PORTION));
  maxNumPoints = maxNum;
}

#define RAND_NEG1_TO_POS1 (2.0f * (rand() / (float)RAND_MAX) - 1.0f)
#define RAND_ZERO_TO_POS1 (rand() / (float)RAND_MAX)

void Fountain::update(PotentialPoints& points, float timePassed)
{
  // Track the time since we added a point.
  static float accruedTime = 0;

  // Accelerate all existing points (before adding new ones.)
  for (int i=0; i<points.size(); i++)
  {
    if (points.getPoint(i).loc.z < minHeight)
    {
      points.getPoint(i).loc.z = minHeight;
      points.getPoint(i).vel = Vector(0,0,0);
    }

//    // If it's passed the 'finish line', restart it (that is, if it's on the other side of xKill
//    // as the start point is.
//    if ( (points.getPoint(i).loc.x - xKill) * (startLoc.x - xKill) < 0 )
//    {
//      points.getPoint(i).loc = startLoc;
//      points.getPoint(i).vel = Vector(0, RAND_NEG1_TO_POS1*Y_VEL_SCALAR, 0);
//
//      // Run it for a random amount of time to avoid having
//      // it pile up.
//      float randTimePassed = RAND_NEG1_TO_POS1 * timePassed;
//      Vector randAddVel(xForce*randTimePassed,0,0);
//      points.getPoint(i).vel += randAddVel;
//      Vector randAddPos = points.getPoint(i).vel;
//      randAddPos *= randTimePassed;
//      points.getPoint(i).loc += randAddPos;
//    }
  }

  // If this is our first call, add a point.
//  if (lastTime == 0)
//  {
//    PhysicalPoint newPoint;
//    newPoint.loc = startLoc;
//
//    // Get this guy setup.
//    newPoint.vel = Vector(1,0,0);
//    newPoint.vel.setTheta(RAND_NEG1_TO_POS1*Math::Pi);
//    newPoint.vel.setPhi(RAND_ZERO_TO_POS1*(maxPhi-minPhi)+minPhi);
//
//    // Now scale it so it gets to the right max height.
//    newPoint.vel *= (zForce / newPoint.vel.z);
//
//    points.addPoint(newPoint);
//  }
//  
//  // Otherwise, has it been the necessary interval?
//  else

  // Did someone cut back on the points?  If we have too many, remove them...
  while (points.size() > maxNumPoints)
  {
    points.removePoint();
  }

  accruedTime += timePassed;

  while (accruedTime > pointRefreshTime)
  {
    // Still need to add points?
    if (points.size() < maxNumPoints)
    {
      PhysicalPoint newPoint;
      newPoint.loc = startLoc;

      // Get this guy setup.
      newPoint.vel = Vector(1,0,0);
      newPoint.vel.setTheta(RAND_NEG1_TO_POS1*Math::Pi);
      newPoint.vel.setPhi(RAND_ZERO_TO_POS1*(maxPhi-minPhi)+minPhi);

      // Now scale it so it gets to the right max height.
      newPoint.vel *= (zForce / newPoint.vel.z);

      points.addPoint(newPoint);
    }

    // Done adding points, just refreshing existing ones now.
    else
    {
      // Track the lowest point, he's the one we'll use.
      float lowestZ = points.getPoint(0).loc.z;
      int lowestIndex = 0;

      // Be tricky, search in reverse sometimes so we don't just
      // start clearing the same exact points over and over.
      if (rand() % 2)
      {
        for (int i=0; i<points.size(); i++)
        {
          if (points.getPoint(i).loc.z < lowestZ)
          {
            lowestZ = points.getPoint(i).loc.z;
            lowestIndex = i;
          }
        }
      }
      else
      {
        for (int i=points.size()-1; i>=0; i--)
        {
          if (points.getPoint(i).loc.z < lowestZ)
          {
            lowestZ = points.getPoint(i).loc.z;
            lowestIndex = i;
          }
        }
      }

      points.getPoint(lowestIndex).loc = startLoc;

      // Get this guy setup.
      points.getPoint(lowestIndex).vel = Vector(1,0,0);
      points.getPoint(lowestIndex).vel.setTheta(RAND_NEG1_TO_POS1*Math::Pi);
      points.getPoint(lowestIndex).vel.setPhi(RAND_ZERO_TO_POS1*(maxPhi-minPhi)+minPhi);

      // Now scale it so it gets to the right max height.
      points.getPoint(lowestIndex).vel *= (zForce / points.getPoint(lowestIndex).vel.z);
    }

    accruedTime -= pointRefreshTime;
  }
}