#include <fluid/Demos/FountainProtoDemo.h>

// Functors we use.
#include <fluid/ParticleSys/BoxBouncer.h>
#include <fluid/ParticleSys/AltitudeWind.h>
#include <fluid/ParticleSys/MoleculeAnimator.h>
#include <fluid/ParticleSys/ImplicitMover.h>
#include <fluid/ParticleSys/ParticleGenerator.h>
#include <fluid/ParticleSys/CompositeFunctor.h>
#include <fluid/ParticleSys/ConstantExerter.h>
#include <fluid/ParticleSys/PlaneCollider.h>
#include <fluid/ParticleSys/CylinderCollider.h>
#include <fluid/ParticleSys/HeightFieldCollider.h>
#include <fluid/calder/MultiHeightFieldCollider.h>

#include <fluid/calder/HeightField.h>
#include <fluid/calder/PolygonSampler.h>
#include <fluid/calder/WaveGenerator.h>

#include <harness/GlobalCamera.h>

#include <windows.h>
#include <mmsystem.h>

#include <iostream>

const int heightFieldDim = 40;
const float vertScalar = 7.0f;

#include <fluid/calder/basin1.cpp>
#include <fluid/calder/basin2.cpp>

FountainProtoDemo::FountainProtoDemo() : wavyPlane(DimRectangle(-vertScalar*4/3,-vertScalar*4/3,vertScalar*4/3,vertScalar*4/3), 1.15, 32)
{
  surfaceThreshold = 18.0f;

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
  anim->setAttractAcrossSurfaces(true);

  ConstantExerter* gravity = new ConstantExerter();
  gravity->setForce(Vector(0,0,-35.0f));

  AltitudeWind* basin1Shove = new AltitudeWind();
  basin1Shove->setForce(Vector(2,-0.5,0));

  // Hack for now, I'll get the mobile arm in and moving soon enough.
  BoxBouncer* bb = new BoxBouncer;
  bb->setBoundingBox(-1.19,100,-100,100,-100,100);

  CompositeFunctor* allForces = new CompositeFunctor();
  allForces->addFunctor(anim);
  allForces->addFunctor(gravity);
  allForces->addFunctor(basin1Shove);
  allForces->addFunctor(bb);
  force = allForces;

  loadBasin1();
  loadBasin2();

  basin1Shove->setBoundingBox(BoundingBox(1.088*vertScalar,-100,-0.1 + (mins.z+maxs.z)*0.5f, 100,100,100));
  
  MultiHeightFieldCollider* hfc = new MultiHeightFieldCollider(equilibrium*1.1, 200, 25, 1000);
  hfc->addHeightField(heightfield[0], Plane(Vector(1,0,0),-1.805*vertScalar));
  hfc->addHeightField(heightfield[1], Plane(Vector(-1,0,0),0.2));

  WaveGenerator* waveGen = new WaveGenerator(&wavyPlane);

  CompositeFunctor* allColliders = new CompositeFunctor();
  allColliders->addFunctor(hfc);
  allColliders->addFunctor(waveGen);
  collision = allColliders;

  ParticleGenerator* particleGen = new ParticleGenerator();

  particleGen->setBaseVelocity(Vector(0.0f,5.0f,0.0f));
  particleGen->setVelAngleVariance(0.25, 0.25);
  particleGen->setVelMagVariance(0.1);

  particleGen->setMinStartPoint(Vector(0.091*7, 0.815*7, 1.446*7));
  particleGen->setMaxStartPoint(Vector(0.091*7, 0.815*7, 1.446*7));

  particleGen->setMaxNumPoints(350);
  particleGen->setParticleKillHeight(1.1);

  particleGen->setParticleRate(0.025);
  generator = particleGen;

  ImplicitMover* mover = new ImplicitMover();
  mover->setMass(1.0f);
  integrator = mover;
}

void FountainProtoDemo::setGLCurrents()
{
  glColor4ub(192, 192, 192, 255);
  glDisable(GL_BLEND);
}

void FountainProtoDemo::start()
{
  wavyPlane.clearWaves();

  GlobalCamera::Instance()->getPosition().x = 21;
  GlobalCamera::Instance()->getPosition().y = -45;
  GlobalCamera::Instance()->getPosition().z = 39;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-21,45,-39));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));
}

void FountainProtoDemo::update(float timePassed)
{
//  const float waveGenRate = 0.3f;
//  static float timeSinceLastWaveGen = waveGenRate;
//  if (timeSinceLastWaveGen > waveGenRate)
//  {
//    wavyPlane.createWaveImpulse(-0.5f + (rand()/(float)RAND_MAX),-0.5f + (rand()/(float)RAND_MAX));
//    timeSinceLastWaveGen -= waveGenRate;
//  }
//  timeSinceLastWaveGen += timePassed;

  wavyPlane.update(timePassed);
  wavyPlane.draw();

  fountain.draw();

//  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
//  glDisable(GL_TEXTURE_2D);
//  glDisable(GL_LIGHTING);
//  glDisable(GL_CULL_FACE);
//
//  glPointSize(3);
//
//  glBegin(GL_TRIANGLES);
//
//  for (int i=0; i<2; i++)
//  {
//    DimRectangle range = heightfield[i].getRange();
//
//    for(int x=0; x<heightFieldDim-2; x++)
//    {
//      for(int y=0; y<heightFieldDim-2; y++)
//      {
//        if (x == 8 && y == 8)
//        {
//          int breakpoint = 0;
//        }
//
//        Vector p00;
//        Vector v00;
//        p00.x = (((x+0.0f) * range.sizeX) / (heightFieldDim-1)) + range.minX;
//        p00.y = (((y+0.0f) * range.sizeY) / (heightFieldDim-1)) + range.minY;
//        heightfield[i].getHeightAndNormalAt(p00.x, p00.y, p00.z, v00);
//
//        Vector p10;
//        Vector v10;
//        p10.x = (((x+1.0f) * range.sizeX) / (heightFieldDim-1)) + range.minX;
//        p10.y = (((y+0.0f) * range.sizeY) / (heightFieldDim-1)) + range.minY;
//        heightfield[i].getHeightAndNormalAt(p10.x, p10.y, p10.z, v10);
//
//        Vector p01;
//        Vector v01;
//        p01.x = (((x+0.0f) * range.sizeX) / (heightFieldDim-1)) + range.minX;
//        p01.y = (((y+1.0f) * range.sizeY) / (heightFieldDim-1)) + range.minY;
//        heightfield[i].getHeightAndNormalAt(p01.x, p01.y, p01.z, v01);
//
//        Vector p11;
//        Vector v11;
//        p11.x = (((x+1.0f) * range.sizeX) / (heightFieldDim-1)) + range.minX;
//        p11.y = (((y+1.0f) * range.sizeY) / (heightFieldDim-1)) + range.minY;
//        heightfield[i].getHeightAndNormalAt(p11.x, p11.y, p11.z, v11);
//
//        glColor3f(0,0,v00.z);
//        glVertex3f(p00.x, p00.y, p00.z);
//        glColor3f(0,0,v10.z);
//        glVertex3f(p10.x, p10.y, p10.z);
//        glColor3f(0,0,v11.z);
//        glVertex3f(p11.x, p11.y, p11.z);
//
//        glColor3f(0,0,v00.z);
//        glVertex3f(p00.x, p00.y, p00.z);
//        glColor3f(0,0,v11.z);
//        glVertex3f(p11.x, p11.y, p11.z);
//        glColor3f(0,0,v01.z);
//        glVertex3f(p01.x, p01.y, p01.z);
//      }
//    }
//  }
//
//  glEnd();
//
//  glPopAttrib();
}

void FountainProtoDemo::loadBasin1()
{
  // Stuff the basin1Verts and basin1Indices into vectors.
  std::vector<Vector> vertVector1;
  std::vector<int> indexVector1;
  
  mins = Vector(basin1Verts[0]*vertScalar, basin1Verts[1]*vertScalar, basin1Verts[2]*vertScalar);
  maxs = Vector(basin1Verts[0]*vertScalar, basin1Verts[1]*vertScalar, basin1Verts[2]*vertScalar);

  for (int x=0; x<numBasin1Verts; x++)
  {
    vertVector1.push_back(Vector(basin1Verts[3*x+0]*vertScalar, basin1Verts[3*x+1]*vertScalar, basin1Verts[3*x+2]*vertScalar));

    // Track the range to center it later.
    if (basin1Verts[3*x+0]*vertScalar < mins.x) mins.x = basin1Verts[3*x+0]*vertScalar;
    if (basin1Verts[3*x+1]*vertScalar < mins.y) mins.y = basin1Verts[3*x+1]*vertScalar;
    if (basin1Verts[3*x+2]*vertScalar < mins.z) mins.z = basin1Verts[3*x+2]*vertScalar;
    if (basin1Verts[3*x+0]*vertScalar > maxs.x) maxs.x = basin1Verts[3*x+0]*vertScalar;
    if (basin1Verts[3*x+1]*vertScalar > maxs.y) maxs.y = basin1Verts[3*x+1]*vertScalar;
    if (basin1Verts[3*x+2]*vertScalar > maxs.z) maxs.z = basin1Verts[3*x+2]*vertScalar;
  }

  // Now center the verts.
  for (x=0; x<numBasin1Verts; x++)
  {
//    vertVector1[x] -= (maxs+mins)*0.5f;

    // Hardcode in a little slope in the x direction.
    vertVector1[x].z = Math::maxOf(vertVector1[x].z, -0.15 + (mins.z+maxs.z)*0.5f);
    vertVector1[x].z -= 0.0f;
//    float xRatio = 2 * (vertVector1[x].x-mins.x) / (maxs.x-mins.x);
//    vertVector1[x].z -= 0.20 * xRatio;
//    vertVector1[x].z = Math::minOf(vertVector1[x].z, 0.3);

  }

  xRange[0] = maxs.x-mins.x;
  yRange[0] = maxs.y-mins.y;
  
//  xRange[0] *= 0.95f;

  for (x=0; x<numBasin1Faces*3; x++)
  {
    indexVector1.push_back(basin1Indices[x]); 
  }

  PolygonSampler sampler;
  sampler.setPolygonMesh(vertVector1, indexVector1);
  sampler.setOutOfBoundsHeight(11.0);

  float data[heightFieldDim*heightFieldDim];

  for(x=0; x<heightFieldDim; x++)
  {
    for(int y=0; y<heightFieldDim; y++)
    {
      float xWorld = (mins.x+maxs.x)*0.5f + ((x * xRange[0]) / (heightFieldDim-1)) - xRange[0]*0.5f;
      float yWorld = (mins.y+maxs.y)*0.5f + ((y * yRange[0]) / (heightFieldDim-1)) - yRange[0]*0.5f;

      if ((x == 0 || x == 31) && (y == 0 || y == 31))
      {
        int breakpoint = 0;
      }

      // Various heightfield[0] patterns.
      data[x+heightFieldDim*y] = sampler.getHeightAt(xWorld, yWorld);
//      data[x+heightFieldDim*y] = 0.5*sin(-Math::Pi*0.5 + 0.4*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 0.5 * yWorld + 2;
//      data[x+heightFieldDim*y] = 0.8*sin(-Math::Pi*0.5 + 1.7*rWorld) + -0.7 * rWorld + 7;
//      data[x+heightFieldDim*y] = 1.5*sin(-Math::Pi*0.5 + 1.2*rWorld) + 2;
//      data[x+heightFieldDim*y] = 0.75*sin(-Math::Pi*0.5 + 1.2*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 2;
    }
  }
  heightfield[0].setHeightData(heightFieldDim, heightFieldDim, data);

  DimRectangle range(-xRange[0]*0.5f + (mins.x+maxs.x)*0.5f, -yRange[0]*0.5f + (mins.y+maxs.y)*0.5f,
                      xRange[0]*0.5f + (mins.x+maxs.x)*0.5f,  yRange[0]*0.5f + (mins.y+maxs.y)*0.5f);

  range.minX += range.sizeX*0.025;
  range.sizeX *= 0.95f;
  range.minY += range.sizeY*0.025;
  range.sizeY *= 0.95f;

  heightfield[0].setRange(range);

//  heightfield[0].setOutOfBoundsHeight(-20);
  heightfield[0].setOutOfBoundsHeight(11);
}

void FountainProtoDemo::loadBasin2()
{
  // Stuff the basin1Verts and basin1Indices into vectors.
  std::vector<Vector> vertVector2;
  std::vector<int> indexVector2;
  
  Vector mins, maxs;

  mins = Vector(basin2Verts[0]*vertScalar, basin2Verts[1]*vertScalar, basin2Verts[2]*vertScalar);
  maxs = Vector(basin2Verts[0]*vertScalar, basin2Verts[1]*vertScalar, basin2Verts[2]*vertScalar);

  for (int x=0; x<numBasin2Verts; x++)
  {
    vertVector2.push_back(Vector(basin2Verts[3*x+0]*vertScalar, basin2Verts[3*x+1]*vertScalar, basin2Verts[3*x+2]*vertScalar));

    // Track the range to center it later.
    if (basin2Verts[3*x+0]*vertScalar < mins.x) mins.x = basin2Verts[3*x+0]*vertScalar;
    if (basin2Verts[3*x+1]*vertScalar < mins.y) mins.y = basin2Verts[3*x+1]*vertScalar;
    if (basin2Verts[3*x+2]*vertScalar < mins.z) mins.z = basin2Verts[3*x+2]*vertScalar;
    if (basin2Verts[3*x+0]*vertScalar > maxs.x) maxs.x = basin2Verts[3*x+0]*vertScalar;
    if (basin2Verts[3*x+1]*vertScalar > maxs.y) maxs.y = basin2Verts[3*x+1]*vertScalar;
    if (basin2Verts[3*x+2]*vertScalar > maxs.z) maxs.z = basin2Verts[3*x+2]*vertScalar;
  }

  for (x=0; x<numBasin2Verts; x++)
  {
    // Hardcode in a little slope in the x direction.
    vertVector2[x].z -= 0.05f;
  }

  xRange[1] = maxs.x-mins.x;
  yRange[1] = maxs.y-mins.y;

  for (x=0; x<numBasin2Faces*3; x++)
  {
    indexVector2.push_back(basin2Indices[x]); 
  }

  PolygonSampler sampler;
  sampler.setPolygonMesh(vertVector2, indexVector2);
  sampler.setOutOfBoundsHeight(10.5);

  float data[heightFieldDim*heightFieldDim];

  for(x=0; x<heightFieldDim; x++)
  {
    for(int y=0; y<heightFieldDim; y++)
    {
      float xWorld = (mins.x+maxs.x)*0.5f + ((x * xRange[1]) / (heightFieldDim-1)) - xRange[1]*0.5f;
      float yWorld = (mins.y+maxs.y)*0.5f + ((y * yRange[1]) / (heightFieldDim-1)) - yRange[1]*0.5f;

      if ((x == 0 || x == 31) && (y == 0 || y == 31))
      {
        int breakpoint = 0;
      }

      // Various heightfield[1] patterns.
      data[x+heightFieldDim*y] = sampler.getHeightAt(xWorld, yWorld);
//      data[x+heightFieldDim*y] = 0.5*sin(-Math::Pi*0.5 + 0.4*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 0.5 * yWorld + 2;
//      data[x+heightFieldDim*y] = 0.8*sin(-Math::Pi*0.5 + 1.7*rWorld) + -0.7 * rWorld + 7;
//      data[x+heightFieldDim*y] = 1.5*sin(-Math::Pi*0.5 + 1.2*rWorld) + 2;
//      data[x+heightFieldDim*y] = 0.75*sin(-Math::Pi*0.5 + 1.2*xWorld) + 0.75*sin(-Math::Pi*0.5 + 1.5*yWorld) + 2;
    }
  }
  heightfield[1].setHeightData(heightFieldDim, heightFieldDim, data);
  heightfield[1].setRange(DimRectangle(-xRange[1]*0.5f + (mins.x+maxs.x)*0.5f, -yRange[1]*0.5f + (mins.y+maxs.y)*0.5f,
                                     xRange[1]*0.5f + (mins.x+maxs.x)*0.5f,  yRange[1]*0.5f + (mins.y+maxs.y)*0.5f));

//  heightfield[1].setOutOfBoundsHeight(-20);
  heightfield[1].setOutOfBoundsHeight(10.5);
}

