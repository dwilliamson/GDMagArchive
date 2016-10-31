#include <fluid/ParticleSys/ImplicitMover.h>

ImplicitMover::ImplicitMover()
{
  mass = 1.0f;
}

void ImplicitMover::setMass(float val)
{
  mass = val;
}

void ImplicitMover::update(PotentialPoints& pts, float timePassed)
{
  // Precompute this to avoid the divide per particle since we're anal.
  float inverseMass = 1.0f / mass;

  // For each point
  for (int i=0; i<pts.size(); i++)
  {
    PhysicalPoint& pt = pts.getPoint(i);

    // Copy its current state into the pre-integration copy that the collision detection needs.
    pt.preIntegration.loc = pt.loc;
    pt.preIntegration.vel = pt.vel;
    pt.preIntegration.force = pt.force;

    // Move velocity forwards by the acceleration.
    Vector accMove = pt.force;
    accMove *= timePassed * inverseMass;
    pt.vel += accMove;

    // Move it by its velocity * time...
    Vector velMove = pt.vel;
    velMove *= timePassed;
    pt.loc += velMove; 

    pt.force = Vector(0,0,0);
  }

  // We've moved the points, let it resort them.
  pts.finalizePointChanges();
}
