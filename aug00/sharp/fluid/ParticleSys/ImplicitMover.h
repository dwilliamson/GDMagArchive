#ifndef IMPLICITMOVER_H
#define IMPLICITMOVER_H

//
// ImplicitMover
//
// ImplicitMover is responsible for integrating the potential points forward by a specified timestep.
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <fluid/SurfRep/PotentialPoints.h>

class ImplicitMover : public ParticleFunctor
{
public:
  ImplicitMover();

  // We default to a mass of 1, but if you want heavier or lighter molecules (making the fluid look a
  // little more or less "viscous") change this around.
  void setMass(float mass);

  void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return INTEGRATOR; }

protected:
  float mass;
};

#endif //IMPLICITMOVER_H