#ifndef BOXBOUNCER_H
#define BOXBOUNCER_H

//
// BoxBouncer moves implicit particles around much like those screensaves that bounce objects around
// the screen -- the boxbouncer simply moves each particle along by its velocity, reflecting it
// of the walls of the specified enclosing axis-aligned bounding box.
//

#include <fluid/ParticleSys/ParticleFunctor.h>

class BoxBouncer : public ParticleFunctor
{
public:
  BoxBouncer();

  void setBoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return FORCE_EXERTER; }

protected:

  float boundingBox[2][3];
};

#endif //BOXBOUNCER_H