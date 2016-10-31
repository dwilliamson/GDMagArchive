#ifndef BASICACCELERATOR_H
#define BASICACCELERATOR_H

// 
// BasicAccelerator
//
// This class generates points at a set starting point with a random amount of y velocity,
// and then accelerates them in the x direction until they hit a defined limit, at which point they're
// killed and a new point is spawned in at the starting point.  Ad naseum.
//

#include <fluid/ParticleSys/ParticleFunctor.h>

class BasicAccelerator : public ParticleFunctor
{
public:
  // Access methods used to set this guy up.  Call these in the order they're declared here.
  // (Some depend on others having been called.
  void setXForce(float force);
  void setXKillValue(float value);
  void setStartPoint(const Vector&);
  void setMaxNumPoints(int maxNum);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return FORCE_EXERTER; }

protected:

  // Calculated from the travel distance and max num points -- the frequency at which we should
  // generate new points.
  float newPointWaitTime;

  // Attributes from above functions.
  float xForce;
  float xKill;
  Vector startLoc;
  int maxNumPoints;
};

#endif //BASICACCELERATOR_H