#include <fluid/Demos/StreamDemo.h>

// Functors we use.
#include <fluid/ParticleSys/BoxBouncer.h>
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>
#include <fluid/ParticleSys/CylinderCollider.h>

#include <harness/GlobalCamera.h>

StreamDemo::StreamDemo()
{
  float minConnectedDistance = sqrt(8.0f / (surfaceThreshold+2));

  float equilibrium = 0.95f * minConnectedDistance;
  float stiffDist = 0.70f;
  float stiffTension = 60.0f;
  float maxStiffTension = 4*equilibrium;
  float repelTension = 1.0f;
  float attractRange = 1.5f;
  float attractTension = 0.5f;
  float maxRange = 1.9f;
  
  MoleculeAnimator* anim = new MoleculeAnimator(equilibrium, stiffDist, stiffTension, maxStiffTension, 
                                                repelTension, attractRange, attractTension, maxRange);

  ConstantExerter* gravity = new ConstantExerter();
//  gravity->setForce(Vector(0,0,-9.8));
  gravity->setForce(Vector(0,0,-32.0f));

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(anim);
  allForces->addFunctor(gravity);
  force = allForces;

  PlaneCollider* inclinedPlane0 = new PlaneCollider(Vector(1,0.5,1), 0, equilibrium*1, 1000);
  PlaneCollider* inclinedPlane1 = new PlaneCollider(Vector(-1,0.5,1), 0, equilibrium*1, 1000);

  CylinderCollider* pillar0 = new CylinderCollider(Vector(0,0,0), Vector(0,0,1), 1.0f, 0.4f, 100);

  CompositeFunctor* allColliders = new CompositeFunctor();
  allColliders->addFunctor(inclinedPlane0);
  allColliders->addFunctor(inclinedPlane1);
  allColliders->addFunctor(pillar0);
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(0,10,-3));
  particleGen->setVelAngleVariance(0.0, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(-1.5,-11,7));
  particleGen->setMaxStartPoint(Vector(1.5,-11,6));
  particleGen->setMaxNumPoints(300);
  particleGen->setParticleKillHeight(-8);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(1.0f);
  integrator = mover;

  cylinder = gluNewQuadric();
  gluQuadricDrawStyle(cylinder, GLU_FILL);
  gluQuadricNormals(cylinder, GLU_NONE);
}

void StreamDemo::start()
{
  GlobalCamera::Instance()->getPosition().x = 8;
  GlobalCamera::Instance()->getPosition().y = 24;
  GlobalCamera::Instance()->getPosition().z = 35;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-8,-24,-35));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));
}

void StreamDemo::update(float timePassed)
{
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

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

  glBegin(GL_QUADS);

  glColor3f(0.7,0.7,0.7);
  glVertex3f(0,-8,4);
  glVertex3f(8,-8,12);
  glVertex3f(8,16,0);
  glVertex3f(0,16,-8);

  glColor3f(0.3,0.3,0.3);
  glVertex3f(0,-8,4);
  glVertex3f(-8,-8,12);
  glVertex3f(-8,16,0);
  glVertex3f(0,16,-8);
//  glVertex3f(0,-8,4);
//  glVertex3f(0,-8,12);
//  glVertex3f(0,8,4);
//  glVertex3f(0,8,-4);

  glEnd();

  glColor3f(0.1,0.6,0.1);
  glPushMatrix();
  glTranslatef(0,0,-2);
  gluCylinder(cylinder, 0.4, 0.4, 10, 8, 8);
//  glTranslatef(-1.4,-5,0);
//  gluCylinder(cylinder, 1, 1, 10, 8, 8);
//  glTranslatef(6,11,0);
//  gluCylinder(cylinder, 1, 1, 10, 8, 8);
  glPopMatrix();

  glPopAttrib();
}
