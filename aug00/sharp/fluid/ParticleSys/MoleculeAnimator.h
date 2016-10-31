#ifndef MOLECULEANIMATOR_H
#define MOLECULEANIMATOR_H

//
// MoleculeAnimator
//
// MoleculeAnimator attempts to model molecular interactions between potential points in a totally hacked way
// that just kinda "looks good."  After all, who cares about physical accuracy if it doesn't look good?
//

#include <fluid/ParticleSys/ParticleFunctor.h>

class MoleculeAnimator : public ParticleFunctor
{
public:
  // This is where you setup all the constants for the forces that this guy applies to particles.
  // The first, equilibrium is the distance at which the points are not affecting each other.  When 
  // they're closer than that distance, a repulsive force is applied that increases linearly as distance
  // decreases; repelTension, is a scalar on that force.  When the points are further
  // than equilibrium apart, they will attract each other with a force that increases as they reach
  // some other distance, equal to equilibrium*attractRange, away.  That force is scaled up by
  // attractTension.  After a distance of equilibrium*attractRange, the attractive force falls off
  // linearly to 0 by a distance of equilibrium*maxRange.
  //
  // Update: I've added a couple fields, here: stiffDist*equilibrium is the distance at which the
  // repelTension jumps from repelTension to stiffTension, and maxStiffTension is the maximum force
  // it'll apply (to keep it from going totally nuts.)  stiffDist should be in the range [0,1].
  MoleculeAnimator(float equilibrium, float stiffDist, float stiffTension, float maxStiffTension,
                   float repelTension, float attractRange, float attractTension, float maxRange);

  // This allows you to override the animator's default behavior and tell it to attract points that are
  // in different surfaces.  The default behavior is not to.  If you turn this on (pass in 'true'),
  // you'll get a kind of T-1000 effect where separate surfaces will spontaneously congeal into one.
  // Eerie.  Coupled with a nice mercury environment map, you're set.
  void setAttractAcrossSurfaces(bool);

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return FORCE_EXERTER; }

protected:
  // All the constants for the force calculations are stored here.
  float stiffDist;
  float equilibrium;
  float attractPeak;
  float maxRange;

  // These are scalars for each of the linear functions (repulsive, attractive increase, attraction falloff.)
  float stiffMax;
  float stiffScalar;
  float repelScalar;
  float attractScalar;
  float attractFalloffScalar;

  // This determines whether we only attract points within the same surface or not.
  bool attractAcrossSurfaces;
};

#endif //MOLECULEANIMATOR_H