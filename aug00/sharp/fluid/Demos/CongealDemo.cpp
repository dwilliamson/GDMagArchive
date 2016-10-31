#pragma warning (disable: 4786)

#include <fluid/Demos/CongealDemo.h>

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
#include <harness/OpenGL.h>
#include <harness/GlobalCamera.h>

#include <iostream>

// Control the initial and final (maximum) rate of the blood flow, as well as the time over which
// it ramps up.
const float STARTING_BLOOD_RATE = 1.5f;
const float FASTEST_BLOOD_RATE = 0.05f;
const float PARTICLE_RATE_RAMPUP_TIME = 8.0f;

CongealDemo::CongealDemo()
{
  float minConnectedDistance = sqrt(8.0f / (surfaceThreshold+2));

  // Note that we screw these scalars up intentionally to make it attract over a much
  // longer range than it really should.
  float equilibrium = 0.95f * minConnectedDistance;
  float stiffDist = 0.70f;
  float stiffTension = 20.0f;
//  float stiffTension = 1.0f;
  float maxStiffTension = 4*equilibrium;
  float repelTension = 1.0f;
  float attractRange = 1.5f;
  float attractTension = 0.5f;
  float maxRange = 1.9f;
  
  MoleculeAnimator* anim = new MoleculeAnimator(equilibrium, stiffDist, stiffTension, maxStiffTension, 
                                                repelTension, attractRange, attractTension, maxRange);
  anim->setAttractAcrossSurfaces(true);

  BoxBouncer* boundingBox = new BoxBouncer();
  boundingBox->setBoundingBox(-5.7,5.7,-5.7,5.7,-1,1);

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(anim);
  allForces->addFunctor(boundingBox);
  force = allForces;

  CompositeFunctor* allColliders = new CompositeFunctor();
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(0,0,0));
  particleGen->setVelAngleVariance(0.0, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(-2,-2,0));
  particleGen->setMaxStartPoint(Vector(2,2,0));
  particleGen->setMaxNumPoints(80);
  particleGen->setParticleRate(STARTING_BLOOD_RATE);
  particleGen->setParticleKillHeight(-5);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(5.0f);
  integrator = mover;

  // Load our texture.
  quadTexture = TextureManager::instance()->addTexture(std::string("data/marble.raw"), 512, 512, false);

  if (OpenGL::getSupportsAnisotropicTextures())
  {
    glBindTexture(GL_TEXTURE_2D, quadTexture);

    // Check this just for debugging purposes.
    float maxAnisotropy;
    glGetFloatv(OpenGL::GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    std::cout << "Card supports " << ((int)maxAnisotropy) << " : 1 anisotropic texture filtering." << std::endl;

    glTexParameterf(GL_TEXTURE_2D, OpenGL::GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
  }

  if (OpenGL::getSupportsTextureLodBias())
  {
    glBindTexture(GL_TEXTURE_2D, quadTexture);
    glTexEnvf(OpenGL::GL_TEXTURE_FILTER_CONTROL_EXT, OpenGL::GL_TEXTURE_LOD_BIAS_EXT, -0.5f);
  }

  totalTimePassed = 0;
}

void CongealDemo::setGLCurrents()
{
  glColor4ub(255,0,0,64); 
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
}

void CongealDemo::start()
{
  GlobalCamera::Instance()->getPosition().x = 4;
  GlobalCamera::Instance()->getPosition().y = 13;
  GlobalCamera::Instance()->getPosition().z = 17;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-4,-13,-17));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));

  // Restart this, it controls the particle rate rampup.
  totalTimePassed = 0;

  // Reset this guy; sometimes some leftover time (not *quite* enough for a new particle,
  // but close) will result in him creating a particle immediately (or at least too soon)
  // upon the demo start.
  ((ParticleGenerator*)generator)->start();
}

void CongealDemo::update(float timePassed)
{
  totalTimePassed += timePassed;

  // If it's still in the rampup time, it's a speedup in rate from 2.0f to 0.1f;
  if (totalTimePassed < PARTICLE_RATE_RAMPUP_TIME)
  {
    float t = (totalTimePassed / PARTICLE_RATE_RAMPUP_TIME);

    // Raise the power so it stays slow longer.
//    t *= t;

    float particleRate = STARTING_BLOOD_RATE - t*(STARTING_BLOOD_RATE - FASTEST_BLOOD_RATE);
    ((ParticleGenerator*)generator)->setParticleRate(particleRate);
  }

  glBindTexture(GL_TEXTURE_2D, quadTexture);

  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);

  glColor3f(1,1,1);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex3f( 6,-6,0);
  glTexCoord2f(0,1);
  glVertex3f( 6, 6,0);
  glTexCoord2f(1,1);
  glVertex3f(-6, 6,0);
  glTexCoord2f(1,0);
  glVertex3f(-6,-6,0);
  glEnd();

  glPopAttrib();
}