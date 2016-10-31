#include <fluid/Demos/FillDemo.h>

// Functors we use.
#include <fluid/ParticleSys/BoxBouncer.h>
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>
#include <fluid/ParticleSys/CylinderCollider.h>

FillDemo::FillDemo()
{
  float minConnectedDistance = sqrt(8.0f / (surfaceThreshold+2));

  float equilibrium = 0.95f * minConnectedDistance;
  float stiffDist = 0.70f;
  float stiffTension = 60.0f;
  float maxStiffTension = 8*equilibrium;
  float repelTension = 3.0f;
  float attractRange = 1.5f;
  float attractTension = 0.5f;
  float maxRange = 1.9f;
  
  MoleculeAnimator* anim = new MoleculeAnimator(equilibrium, stiffDist, stiffTension, maxStiffTension, 
                                                repelTension, attractRange, attractTension, maxRange);

  ConstantExerter* gravity = new ConstantExerter();
//  gravity->setForce(Vector(0,0,-9.8));
  gravity->setForce(Vector(0,0,-9.0f));

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(anim);
  allForces->addFunctor(gravity);
  force = allForces;

  PlaneCollider* inclinedPlane0 = new PlaneCollider(Vector(1,1,1), 8, equilibrium*1, 1000);
  PlaneCollider* inclinedPlane1 = new PlaneCollider(Vector(-1,1,1), 8, equilibrium*1, 1000);
  PlaneCollider* inclinedPlane2 = new PlaneCollider(Vector(1,-1,1), 8, equilibrium*1, 1000);
  PlaneCollider* inclinedPlane3 = new PlaneCollider(Vector(-1,-1,1), 8, equilibrium*1, 1000);

  CompositeFunctor* allColliders = new CompositeFunctor();
  allColliders->addFunctor(inclinedPlane0);
  allColliders->addFunctor(inclinedPlane1);
  allColliders->addFunctor(inclinedPlane2);
  allColliders->addFunctor(inclinedPlane3);
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(0,0,0));
  particleGen->setVelAngleVariance(0.0, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(-5,-8.5,8));
  particleGen->setMaxStartPoint(Vector(-6,-9.5,9));
  particleGen->setMaxNumPoints(300);
  particleGen->setParticleKillHeight(-16);
  particleGen->setParticleRate(1);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(1.0f);
  integrator = mover;
}

void FillDemo::update(float timePassed)
{
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);

//  glBegin(GL_TRIANGLES);
//  glColor3f(0.4, 0.0, 0.4);
//  glVertex3f(0,0,0);
//  glVertex3f(8,8,4);
//  glVertex3f(-8,8,4);
//  
//  glColor3f(0.45,0.0,0.5);
//  glVertex3f(0,0,0);
//  glVertex3f(-8,-8,4);
//  glVertex3f(8,-8,4);
//
//  glColor3f(0.2,0,0.2);
//  glVertex3f(0,0,0);
//  glVertex3f(-8,8,4);
//  glVertex3f(-8,-8,4);
//
//  glColor3f(0.7,0,0.8);
//  glVertex3f(0,0,0);
//  glVertex3f(8,-8,4);
//  glVertex3f(8,8,4);
//
//  glEnd();

  glBegin(GL_TRIANGLES);

  glColor3f(0.3,0.3,0.3);
  glVertex3f(0,-16,8);
  glVertex3f(-16,0,8);
  glVertex3f(0,0,-8);

  glColor3f(0.7,0.7,0.7);
  glVertex3f(0,-16,8);
  glVertex3f(16,0,8);
  glVertex3f(0,0,-8);

//  PlaneCollider* inclinedPlane0 = new PlaneCollider(Vector(1,1,1), 0, equilibrium*1, 1000);
//  PlaneCollider* inclinedPlane1 = new PlaneCollider(Vector(-1,1,1), 0, equilibrium*1, 1000);
//  PlaneCollider* inclinedPlane2 = new PlaneCollider(Vector(1,-1,1), 0, equilibrium*1, 1000);
//  PlaneCollider* inclinedPlane3 = new PlaneCollider(Vector(-1,-1,1), 0, equilibrium*1, 1000);

  glColor3f(0.4,0.4,0.4);
  glVertex3f(0,16,8);
  glVertex3f(-16,0,8);
  glVertex3f(0,0,-8);

  glColor3f(0.8,0.8,0.8);
  glVertex3f(0,16,8);
  glVertex3f(16,0,8);
  glVertex3f(0,0,-8);

//  glVertex3f(0,-8,4);
//  glVertex3f(0,-8,12);
//  glVertex3f(0,8,4);
//  glVertex3f(0,8,-4);

  glEnd();

  glPopAttrib();
}
