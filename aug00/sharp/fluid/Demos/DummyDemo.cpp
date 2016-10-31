#include <fluid/Demos/DummyDemo.h>

// Functors we use.
#include <fluid/ParticleSys/BoxBouncer.h>
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>

// We need access to textures.
#include <harness/TextureManager.h>

DummyDemo::DummyDemo()
{
  CompositeFunctor* allForces = new CompositeFunctor();
  force = allForces;

  CompositeFunctor* allColliders = new CompositeFunctor();
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(0,0,0));
  particleGen->setVelAngleVariance(0.0, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(-1,-1,-1));
  particleGen->setMaxStartPoint(Vector(1,1,1));
  particleGen->setMaxNumPoints(9);
  particleGen->setParticleRate(0.1);
  particleGen->setParticleKillHeight(-5);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(5.0f);
  integrator = mover;
}
