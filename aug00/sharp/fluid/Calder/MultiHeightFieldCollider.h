#ifndef MULTIHEIGHTFIELDCOLLIDER_H
#define MULTIHEIGHTFIELDCOLLIDER_H

//
// MultiHeightFieldCollider
//
// MultiHeightFieldCollider is like HeightFieldCollider but has a semi-hacky support for multiple
// layered height fields where particles are tracked through the layers.  As they pass certain planes
// (the edge of each height field or something) they're moved to the next height field (presumably below
// the prior one.)
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <fluid/calder/HeightField.h>
#include <math/Plane.h>

class MultiHeightFieldCollider : public ParticleFunctor
{
public:
  // This constructor takes the plane equation as the vector (a,b,c) and the scalar d, where the plane
  // equation is ax+by+cz+d = 0.  Note that it's NOT ax+by+cz = d.
  //
  // The third argument is a threshold value, that is, the distance from the plane at which we start
  // repelling the points.  The fourth is the maximum force to exert upon the points (it's a spring, but
  // you need to cap it somewhere.)
  MultiHeightFieldCollider(float distThresh, float springCoeff, float dampCoeff, float maxRepel);

  // This chains a height field onto the end of the list, with the specified plane being the 
  // condition for switching from this height field to the next (or, if this is the last, ceasing to
  // collide against any of these height fields at all.)
  //
  // Note that we switch the point to the next layer when it is on the side of the plane with the normal
  // (i.e. the normal is "pointing at" the particle.)  If it seems to be dropping the points through a layer,
  // perhaps your normal needs flipping (and your d term as well, of course...)
  void addHeightField(const HeightField&, const Plane&);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return COLLIDER; }

protected:
  std::vector<HeightField> heightfields;
  std::vector<Plane> layerPlanes;

  float springScalar;
  float dampScalar;
  float maxDist;
  float maxForce;
};

#endif //MULTIHEIGHTFIELDCOLLIDER_H