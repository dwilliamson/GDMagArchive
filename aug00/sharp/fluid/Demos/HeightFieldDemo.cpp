#include <fluid/Demos/HeightFieldDemo.h>

// Functors we use.
#include <fluid/ParticleSys/BoxBouncer.h>
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>
#include <fluid/ParticleSys/CylinderCollider.h>
#include <fluid/ParticleSys/HeightFieldCollider.h>

#include <fluid/calder/HeightField.h>

#include <math/BoundingBox.h>

#include <harness/GlobalCamera.h>

#include <windows.h>
#include <mmsystem.h>

const int heightFieldDim = 32;

HeightFieldDemo::HeightFieldDemo()
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

  gravity = new ConstantExerter();
  gravity->setForce(Vector(0,0,-32.0f));

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(anim);
  allForces->addFunctor(gravity);
  force = allForces;

  heightfield.setRange(DimRectangle(-8,-8,8,8));

  float data[heightFieldDim*heightFieldDim];

  for(int x=0; x<heightFieldDim; x++)
  {
    for(int y=0; y<heightFieldDim; y++)
    {
      float xWorld = ((x * 16.0f) / (heightFieldDim-1)) - 8.0f;
      float yWorld = ((y * 16.0f) / (heightFieldDim-1)) - 8.0f;
      float rWorld = sqrt(xWorld*xWorld + yWorld*yWorld);

      // Various heightfield patterns.
      data[x+heightFieldDim*y] = 0.5*sin(-Math::Pi*0.5 + 0.4*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 0.5 * yWorld + 2;
//      data[x+heightFieldDim*y] = 0.8*sin(-Math::Pi*0.5 + 1.7*rWorld) + -0.7 * rWorld + 7;
//      data[x+heightFieldDim*y] = 1.5*sin(-Math::Pi*0.5 + 1.2*rWorld) + 2;
//      data[x+heightFieldDim*y] = 0.75*sin(-Math::Pi*0.5 + 1.2*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 2;
    }
  }
  heightfield.setHeightData(heightFieldDim, heightFieldDim, data);
  heightfield.setOutOfBoundsHeight(-20);
  HeightFieldCollider* hfc = new HeightFieldCollider(heightfield, equilibrium*1.1, 200, 10, 1000);

  CompositeFunctor* allColliders = new CompositeFunctor();
  allColliders->addFunctor(hfc);
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();
  particleGen->setBaseVelocity(Vector(0,0,0));
  particleGen->setVelAngleVariance(0.0, 0.0);
  particleGen->setVelMagVariance(0);
  particleGen->setMinStartPoint(Vector(-6.5, 4.5,7.5));
  particleGen->setMaxStartPoint(Vector( 6.5, 6.0,7.5));
//  particleGen->setMinStartPoint(Vector(-6.5,-6.5,7.5));
//  particleGen->setMaxStartPoint(Vector( 6.5, 6.5,7.5));
  particleGen->setMaxNumPoints(150);
  
  particleGen->setParticleBoundingBox(BoundingBox(-12,-12,-3, 12,12,13));
  particleGen->setParticleRate(0.2);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(1.0f);
  integrator = mover;
}

void HeightFieldDemo::start()
{
  GlobalCamera::Instance()->getPosition().x = 0;
  GlobalCamera::Instance()->getPosition().y = -18;
  GlobalCamera::Instance()->getPosition().z = 26;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(0,18,-26));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));
}

void HeightFieldDemo::update(float timePassed)
{
//  gravity->setForce(GlobalCamera::Instance()->getPosition().getUpVector() * -32);
  
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);

  glPointSize(3);

  glBegin(GL_TRIANGLES);

  for(int x=0; x<heightFieldDim-2; x++)
  {
    for(int y=0; y<heightFieldDim-2; y++)
    {
      if (x == 8 && y == 8)
      {
        int breakpoint = 0;
      }

      Vector p00;
      Vector v00;
      p00.x = (((x+0.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      p00.y = (((y+0.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      heightfield.getHeightAndNormalAt(p00.x, p00.y, p00.z, v00);

      Vector p10;
      Vector v10;
      p10.x = (((x+1.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      p10.y = (((y+0.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      heightfield.getHeightAndNormalAt(p10.x, p10.y, p10.z, v10);

      Vector p01;
      Vector v01;
      p01.x = (((x+0.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      p01.y = (((y+1.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      heightfield.getHeightAndNormalAt(p01.x, p01.y, p01.z, v01);

      Vector p11;
      Vector v11;
      p11.x = (((x+1.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      p11.y = (((y+1.0f) * 16.0f) / (heightFieldDim-1)) - 8.0f;
      heightfield.getHeightAndNormalAt(p11.x, p11.y, p11.z, v11);

      glColor3f(0,0,v00.z);
      glVertex3f(p00.x, p00.y, p00.z);
      glColor3f(0,0,v10.z);
      glVertex3f(p10.x, p10.y, p10.z);
      glColor3f(0,0,v11.z);
      glVertex3f(p11.x, p11.y, p11.z);

      glColor3f(0,0,v00.z);
      glVertex3f(p00.x, p00.y, p00.z);
      glColor3f(0,0,v11.z);
      glVertex3f(p11.x, p11.y, p11.z);
      glColor3f(0,0,v01.z);
      glVertex3f(p01.x, p01.y, p01.z);
    }
  }

  glEnd();

  glPopAttrib();
}
