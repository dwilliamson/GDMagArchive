#ifndef PARTICLEFUNCTOR_H
#define PARTICLEFUNCTOR_H

//
// ParticleFunctor
//
// ParticleFunctor is the abstract class for all Strategies that operate on the particles of an 
// implicit surface.  So you could have implementations that exert forces on points, implementations
// that generate or destroy points, implementations that move points around, or whatever.
//

#include <fluid/SurfRep/PotentialPoints.h>

class ParticleFunctor
{
public:
  virtual void update(PotentialPoints&, float timePassed) = 0;

  virtual ~ParticleFunctor() {}

  // Because a deep inheritance hierarchy would make this too rigid, I'm specifying "categories"
  // of ParticleFunctors with this enum; makes for easier maintenance and changing later.  Note that
  // the order they're defined here is the order they'll be applied in (so all Generators get to act
  // each frame before all Force Exerters, for example.)
  enum ParticleFunctorType { GENERATOR, FORCE_EXERTER, INTEGRATOR, COLLIDER, KILLER, INVALID_TYPE };

  // This can be used to query the type of a functor.  Each implementation defines this.
  virtual ParticleFunctorType getType() = 0;

protected:
};

#endif //PARTICLEFUNCTOR_H