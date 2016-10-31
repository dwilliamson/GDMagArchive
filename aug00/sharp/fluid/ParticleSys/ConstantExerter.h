#ifndef CONSTANTEXERTER_H
#define CONSTANTEXERTER_H

//
// ConstantExerter
//
// This object is pretty simple -- it just applies a constant force to every particle every single
// physics tick.  Usually it'll be used to implement some global force like gravity or wind (or both.)
//

#include <math/Vector.h>
#include <fluid/ParticleSys/ParticleFunctor.h>

class ConstantExerter : public ParticleFunctor
{
public:
  // This is the force that will be applied non-stop to every point in the system.
  void setForce(const Vector& forceIn);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return FORCE_EXERTER; }

protected:

  Vector force;
};

#endif //CONSTANTEXERTER_H