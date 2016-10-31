#ifndef ALTITUDEWIND_H
#define ALTITUDEWIND_H

//
// AltitudeWind
//
// This object is pretty simple -- it just applies a constant force to every particle every single
// physics tick, PROVIDED that the particle is within a specified bounding box.
// It's an object I needed to tweak the calder mercury fountain (give the mercury in the top basin a
// little more "oomph") but who knows, maybe it'll come in handy elsewhere.
//

#include <math/Vector.h>
#include <math/BoundingBox.h>
#include <fluid/ParticleSys/ParticleFunctor.h>

class AltitudeWind : public ParticleFunctor
{
public:
  AltitudeWind();

  // This is the force that will be applied non-stop to every point in the system.
  void setForce(const Vector& forceIn);

  void setBoundingBox(const BoundingBox&);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return FORCE_EXERTER; }

protected:

  Vector force;
  BoundingBox box;
};

#endif //ALTITUDEWIND_H