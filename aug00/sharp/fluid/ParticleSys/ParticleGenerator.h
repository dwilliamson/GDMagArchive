#ifndef PARTICLEGENERATOR_H
#define PARTICLEGENERATOR_H

//
// ParticleGenerator
//
// ParticleGenerator creates and adds potential points to the implicit system
// to maintain a desired number of particles.  It does this by assuming that any given particle will be
// removed from the system by a ParticleKiller in a finite amount of time.  Therefore, it first launches
// a single particle at the beginning of the simulation to time how long it takes for it to die, and then
// based on that timing releases particles periodically into the system to maintain a steady flow.  It uses
// subsequent particle counts as input to a feedback loop to keep the system balanced.
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <fluid/SurfRep/PotentialPoints.h>
#include <math/Vector.h>
#include <math/BoundingBox.h>

class ParticleGenerator : public ParticleFunctor
{
public:
  ParticleGenerator();

  // This function will wipe internal counters the generator uses (it accumulates the time passed
  // so at any point you can have just under one particle-rate's worth of time leftover for the
  // next frame, which can cause the premature creation of a particle when starting a new demo.
  void start();

  // These functions setup values for the new potential points; their distribution and velocity upon
  // creation.

  // Control the random velocity vector generation.  Velocity is generated pointing straight up and then
  // rotated to the base vector, so the angle variances form a cone around the base velocity vector.
  void setBaseVelocity(const Vector& vel);
  void setVelAngleVariance(float theta, float phi);

  // This is a scale, not a bias.  So putting a variance of 0.1 in will mean that the vector
  // will be as little as 0.9 the magnitude of the base velocity, or as much as 1.1.
  void setVelMagVariance(float mag);

  // The new points are distributed randomly through a box.  Set the min and max to the same point
  // to generate all new points at the same location.
  void setMinStartPoint(const Vector&);
  void setMaxStartPoint(const Vector&);

  // Used as the target for the feedback loop.
  void setMaxNumPoints(int maxNum);
  int getMaxNumPoints() const { return maxNumPoints; }

  // This is a placeholder; if I ever need more advanced particle-termination logic than this,
  // I'll pull this out and make a ParticleKiller its own logic.
  void setParticleKillHeight(float height);
  void setParticleBoundingBox(const BoundingBox& box);

  // This function is called every physics tick to generate any needed points.
  void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return GENERATOR; }

  // This controls the rate at which new particles are created (assuming we're not already at
  // our limit.)
  void setParticleRate(float rate);
  float getParticleRate() const;

protected:
  // Helper used to generate a new random point.
  void genPoint(PhysicalPoint& pt);

  // Used in generating the new points.
  Vector baseVel;
  float velThetaVary, velPhiVary;
  float velMagVary;

  Vector minStart, maxStart;

  float killHeight;
  BoundingBox bbox;
  bool useBBox;

  int maxNumPoints;
  float particleRate;

  float accruedTime;
};

#endif //PARTICLEGENERATOR_H