#include <fluid/Demos/FountainDemo.h>

// Functors we use.
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>

#include <harness/GlobalCamera.h>

FountainDemo::FountainDemo()
{
  float minConnectedDistance = sqrt(8.0f / (surfaceThreshold+2));

  ConstantExerter* gravity = new ConstantExerter();
  gravity->setForce(Vector(0,0,-9.8f));

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(gravity);
  force = allForces;

  CompositeFunctor* allColliders = new CompositeFunctor();
  collision = allColliders;

  // Set this guy to spray in a fountain shape (full theta variance from 0 to 2PI.)
  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(3,0,11));
  particleGen->setVelAngleVariance(3.1416, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(0,0,0.1));
  particleGen->setMaxStartPoint(Vector(0,0,0.1));
  particleGen->setMaxNumPoints(200);
  particleGen->setParticleRate(0.04);
  particleGen->setParticleKillHeight(0);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(1.0f);
  integrator = mover;
}

void FountainDemo::start()
{
  GlobalCamera::Instance()->getPosition().x = 10;
  GlobalCamera::Instance()->getPosition().y = 23;
  GlobalCamera::Instance()->getPosition().z = 15;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-10,-23,-15));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));
}

void FountainDemo::update(float timePassed)
{
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);

  glColor3f(0.4,0.4,0.4);
  glBegin(GL_QUADS);
  glVertex3f( 8,-8,0);
  glVertex3f( 8, 8,0);
  glVertex3f(-8, 8,0);
  glVertex3f(-8,-8,0);
  glEnd();

  glPopAttrib();
}
