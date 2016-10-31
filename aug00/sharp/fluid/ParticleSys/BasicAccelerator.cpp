#include <fluid/ParticleSys/BasicAccelerator.h>
#include <assert.h>

const float Y_VEL_SCALAR = 1.5f;

// Access methods used to set this guy up.
void BasicAccelerator::setXForce(float force)
{
  xForce = force;
}

void BasicAccelerator::setXKillValue(float value)
{
  xKill = value;
}

void BasicAccelerator::setStartPoint(const Vector& startPt)
{
  startLoc = startPt;
}

void BasicAccelerator::setMaxNumPoints(int maxNum)
{
  maxNumPoints = maxNum;

  // Calculate the interval at which a new point should be created from this.
  // Based on the simple d = 0.5*at^2 (there's no initial x velocity.)  Solve for t, you get
  // t = sqrt(2d/a)
  newPointWaitTime = sqrt(2 * fabs(xKill-startLoc.x) / fabs(xForce)) / (maxNumPoints-1);
  newPointWaitTime *= 1.2f;
}

#define RAND_NEG1_TO_POS1 (2.0f * (rand() / (float)RAND_MAX) - 1.0f)

void BasicAccelerator::update(PotentialPoints& points, float timePassed)
{
  // Track the time since we added a point.
  static float lastPointAddTime = 0;
  static float lastTime = 0;

  // Accelerate all existing points (before adding new ones.)
  for (int i=0; i<points.size(); i++)
  {
    // If it's passed the 'finish line', restart it (that is, if it's on the other side of xKill
    // as the start point is.
    if ( (points.getPoint(i).loc.x - xKill) * (startLoc.x - xKill) < 0 )
    {
      points.getPoint(i).loc = startLoc;
      points.getPoint(i).vel = Vector(0, RAND_NEG1_TO_POS1*Y_VEL_SCALAR, 0);

      // Run it for a random amount of time to avoid having
      // it pile up.
      float randTimePassed = RAND_NEG1_TO_POS1 * timePassed;
      Vector randAddVel(xForce*randTimePassed,0,0);
      points.getPoint(i).vel += randAddVel;
      Vector randAddPos = points.getPoint(i).vel;
      randAddPos *= randTimePassed;
      points.getPoint(i).loc += randAddPos;
    }
  }

  // If this is our first call, add a point.
  if (lastTime == 0)
  {
    PhysicalPoint newPoint;
    newPoint.loc = startLoc;
    newPoint.vel = Vector(0, RAND_NEG1_TO_POS1, 0);
    points.addPoint(newPoint);
  }
  
  // Otherwise, has it been the necessary interval?
  else
  {
    if (points.size() < maxNumPoints && (lastTime + timePassed - lastPointAddTime > newPointWaitTime))
    {
      PhysicalPoint newPoint;
      newPoint.loc = startLoc;
      newPoint.vel = Vector(0, RAND_NEG1_TO_POS1*Y_VEL_SCALAR, 0);
      points.addPoint(newPoint);

      lastPointAddTime = lastTime + timePassed;
    }
  }

  // Update the time.
  lastTime += timePassed;
}