#include <fluid/ParticleSys/CompositeFunctor.h>
#include <assert.h>

CompositeFunctor::~CompositeFunctor()
{
  for (int x=0; x<functors.size(); x++)
  {
    delete functors[x];
  }
  functors.clear();
}

// This takes an allocated functor, and WILL delete it upon program termination.  A reference-counted
// pointer is what's really called for here, I suppose...
void CompositeFunctor::addFunctor(ParticleFunctor* functor)
{
  // Don't let them put different-typed functors into us, to make sure that the sorting mechanism isn't
  // preempted accidentally.
  if (functors.size() != 0 && functors[0]->getType() != functor->getType())
  {
    assert(false);
  }
  functors.push_back(functor);
}

void CompositeFunctor::update(PotentialPoints& points, float timePassed)
{
  for (std::vector<ParticleFunctor*>::iterator it = functors.begin(); it != functors.end(); it++)
  {
    (*it)->update(points, timePassed);
  }
}