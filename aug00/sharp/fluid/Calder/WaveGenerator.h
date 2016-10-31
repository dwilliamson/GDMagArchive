#ifndef WAVEGENERATOR_H
#define WAVEGENERATOR_H

//
// WaveGenerator
//
// WaveGenerator is used to create waves in a WavePlane whenever (and wherever) a particle passes downwards
// through it.
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <fluid/calder/WavePlane.h>
#include <math/Vector.h>

class WaveGenerator : public ParticleFunctor
{
public:
  WaveGenerator(WavePlane* plane);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return COLLIDER; }

protected:
  WavePlane* wavePlane;
};

#endif //WAVEGENERATOR_H