#ifndef COMPOSITEFUNCTOR_H
#define COMPOSITEFUNCTOR_H

//
// CompositeFunctor
//
// This class is a Composite of functors.  It makes sense in terms of piling up a bunch of geometric 
// primitives to collide against and have this guy handle all of them, or tossing in gravity, too.
// It's just an easy way to aggregate a bunch of functor applications into one object.
//

#include <fluid/ParticleSys/ParticleFunctor.h>

class CompositeFunctor : public ParticleFunctor
{
public:
  virtual ~CompositeFunctor();

  // This takes an allocated functor, and WILL delete it upon program termination.  A reference-counted
  // pointer is what's really called for here, I suppose...
  void addFunctor(ParticleFunctor* functor);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() 
  { 
    if (functors.size() != 0) 
    {
      return functors[0]->getType();
    }
    else
    {
      return INVALID_TYPE;
    }
  }

protected:

  std::vector<ParticleFunctor*> functors;
};

#endif //COMPOSITEFUNCTOR_H