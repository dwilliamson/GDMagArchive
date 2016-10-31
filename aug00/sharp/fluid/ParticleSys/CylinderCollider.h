#ifndef CYLINDERCOLLIDER_H
#define CYLINDERCOLLIDER_H

//
// CylinderCollider
//
// This will prevent (or try to prevent) points from penetrating an infinite cylinder specified as a line
// and a radius from that line.
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <math/Vector.h>

class CylinderCollider : public ParticleFunctor
{
public:
  // This takes the cylinder as a parameterized line, p(t) = linePt + lineDir*t.  Then, the thickness
  // is 'radius', the maximum distance away from the radius is distThresh, and maxRepel is the maximum
  // correctional force it'll apply in the penalty methods.
  CylinderCollider(Vector linePt, Vector lineDir, float radius, float distThresh, float maxRepel);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return COLLIDER; }

protected:

  Vector linePt;
  Vector lineDir;
  float radius;

  float maxDist;
  float maxForce;
};

#endif //CYLINDERCOLLIDER_H