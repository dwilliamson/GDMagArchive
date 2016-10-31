#pragma warning (disable: 4786)

#include <fluid/calder/WavePlane.h>
#include <harness/GlobalCamera.h>
#include <harness/TextureManager.h>
#include <math/Math.h>
#include <gl/glut.h>
#include <assert.h>

const float waveVelocity = 8.0f;
const float waveFrequency = 6.0f;
const float waveRadius = 3.5f;
const float waveAmplitudeScalar = 0.45f;
const float maxWaveAmplitude = 0.6f;
const float waveDuration = 5.0f;

const float waveVertexOffsetScalar = 0.5f;
const float waveNormalDirectionScalar = 2.0f;

const float waveSeparationTime = 0.1f;

void WaveImpulse::getHeightAndNormalAt(float locX, float locY, float& heightOut, Vector& normalOut)
{
  Vector distanceVec(locX-epiX, locY-epiY, 0);
  float distanceSquared = distanceVec.getMagnitudeSquared();

  float waveCenter = duration * waveVelocity;

  // Quick reject -- if it's not in the wave impulse exit.
  if ((distanceSquared < (waveCenter - waveRadius)*(waveCenter - waveRadius)) ||
      (distanceSquared > waveCenter*waveCenter))//(waveCenter + waveRadius)*(waveCenter + waveRadius)))
  {
    return;
  }

  float distance = sqrtf(distanceSquared);
  float phase = distance - waveCenter;
  float amplitude = 1.0f / distance;//(waveRadius - fabs(phase)) / (waveRadius * distance);  
  amplitude = Math::minOf(maxWaveAmplitude, amplitude);

  heightOut += waveVertexOffsetScalar * waveAmplitudeScalar * amplitude * sin(waveFrequency * (phase-Math::Pi*0.5f));

  // Build the normal contribution.
  distanceVec *= waveNormalDirectionScalar * waveAmplitudeScalar * amplitude * cos(waveFrequency * (phase-Math::Pi*0.5f)) / distance;
  distanceVec.z = 1.0f;
  normalOut += distanceVec;
}

void WaveImpulse::update(float timePassed)
{
  duration += timePassed;
}

WavePlane::WavePlane(DimRectangle area, float baseHeight, int dim)
{
  range = area;
  tessDim = dim;
  restHeight = baseHeight;

  verts = new float[4*tessDim*tessDim];
  texcs = new float[2*tessDim*tessDim];
  colrs = new unsigned char[4*tessDim*tessDim];
//  norms = new float[3*tessDim*tessDim];
  indis = new unsigned int[6*(tessDim-1)*(tessDim-1)];

  for (int x=0; x<tessDim; x++)
  {
    for (int y=0; y<tessDim; y++)
    {
      verts[4*(x+(tessDim*y)) + 0] = range.minX + (x / (float)(tessDim-1)) * range.sizeX;
      verts[4*(x+(tessDim*y)) + 1] = range.minY + (y / (float)(tessDim-1)) * range.sizeY;
      verts[4*(x+(tessDim*y)) + 2] = restHeight;

      // If it's an even row, offset by a half to get a hexagonal pattern instead.
      if (y % 2)
      {
        verts[4*(x+(tessDim*y)) + 0] += 0.5f * range.sizeX / (float)(tessDim-1);
      }
    }
  }

  for (x=0; x<tessDim-1; x++)
  {
    for (int y=0; y<tessDim-1; y++)
    {
      // Alternate diagonal direction to get a good hexagonal tessellation.
      if (y % 2 == 0)
      {
        indis[6*(x+(y*(tessDim-1))) + 0] = (x+0) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 1] = (x+1) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 2] = (x+0) + tessDim*(y+1);

        indis[6*(x+(y*(tessDim-1))) + 3] = (x+1) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 4] = (x+1) + tessDim*(y+1);
        indis[6*(x+(y*(tessDim-1))) + 5] = (x+0) + tessDim*(y+1);
      }
      else
      {
        indis[6*(x+(y*(tessDim-1))) + 0] = (x+0) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 1] = (x+1) + tessDim*(y+1);
        indis[6*(x+(y*(tessDim-1))) + 2] = (x+0) + tessDim*(y+1);

        indis[6*(x+(y*(tessDim-1))) + 3] = (x+0) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 4] = (x+1) + tessDim*(y+0);
        indis[6*(x+(y*(tessDim-1))) + 5] = (x+1) + tessDim*(y+1);
      }
    }
  }

  envmapTexture = TextureManager::instance()->addTexture(std::string("data/silver_sphere_colorlights.raw"), 256, 256, false);
}

WavePlane::~WavePlane()
{
  delete[] verts;
  verts = 0;
  delete[] texcs;
  texcs = 0;
  delete[] colrs;
  colrs = 0;
//  delete[] norms;
//  norms = 0;
  delete[] indis;
  indis = 0;
}

// Used to add a wave that propagates outwards for some amount of time (hardcoded since the fluid
// has one wave propagation speed and a finite x/y range.)
void WavePlane::createWaveImpulse(float xDisturb, float yDisturb)
{
  // To avoid thrashing and death if we have too many waves, reject any if another one is more recent than
  // some time threshold.
  if (waves.size() == 0 || waves.rbegin()->getDuration() > waveSeparationTime)
  {
    waves.push_back(WaveImpulse(xDisturb, yDisturb));
  }
}

// Tranquilizes the surface into a smooth, wave-free plane.
void WavePlane::clearWaves()
{
  waves.clear();
}

void WavePlane::update(float timePassed)
{
  std::list<WaveImpulse>::iterator waveIt;
  for (waveIt = waves.begin(); waveIt != waves.end(); waveIt++)
  {
    waveIt->update(timePassed);
    if (waveIt->getDuration() > waveDuration)
    {
      waveIt = waves.erase(waveIt);
    }
  }

  const Position& camPos = GlobalCamera::Instance()->getPosition();
  
  for (int x=0; x<tessDim*tessDim; x++)
  {
    Vector normal(0,0,0);
    verts[4*x+2] = restHeight;

    for (waveIt = waves.begin(); waveIt != waves.end(); waveIt++)
    {
      waveIt->getHeightAndNormalAt(verts[4*x+0], verts[4*x+1], verts[4*x+2], normal);
    }

    // If we were in no wave, we just have a normal of (0,0,1).
    if (normal.getMagnitudeSquared() < 0.0001)
    {
      normal = Vector(0,0,1);
    }
    else
    {
      normal.fastNormalize();
    }

//    norms[3*x+0] = normal.x;
//    norms[3*x+1] = normal.y;
//    norms[3*x+2] = normal.z;

    // Generate the lighting, just make it a directional or two.
    static Vector light0(-0.21821,-0.43643,-0.87287);
    static float light0Color[3] = {255, 255, 255};
    float light0Intensity = normal.dot(light0);
    light0Intensity = Math::maxOf(light0Intensity*-1, 0);
    colrs[4*x+0] = (unsigned char)(light0Intensity * light0Color[0]);
    colrs[4*x+1] = (unsigned char)(light0Intensity * light0Color[1]);
    colrs[4*x+2] = (unsigned char)(light0Intensity * light0Color[2]);
    colrs[4*x+3] = 255;

    // Generate the texcoords from that.
    Vector cameraLoc(-camPos);

    cameraLoc.x += verts[4*x + 0];
    cameraLoc.y += verts[4*x + 1];
    cameraLoc.z += verts[4*x + 2];
    
    cameraLoc.fastNormalize();

    // Got our camera vector, we know the normal is normalized, get the reflection vector and then do
    // the crezzy sphere_map thang.
    float normalDotCamera = normal.dot(cameraLoc);
    Vector reflection = cameraLoc;
    reflection.x -= 2 * normal.x * normalDotCamera;
    reflection.y -= 2 * normal.y * normalDotCamera;
    reflection.z -= 2 * normal.z * normalDotCamera;

    // Rotate around x to get our sphere map world-aligned just right.
    float holdZ = reflection.z;
    reflection.z = -reflection.y;
    reflection.y = holdZ;

    assert(fabs(reflection.getMagnitudeSquared()-1.0f) < 0.1);

    if (reflection.z > -1)
    {
      float invM = 0.5f*Math::fastInverseSqrt(2*reflection.z + 2);
      texcs[2*x+0] = 0.5f + reflection.x * invM;
      texcs[2*x+1] = 0.5f + reflection.y * invM;
    }
    else
    {
      // Put them  somewhere remotely intelligible.
      texcs[2*x+0] = 0.5f;
      texcs[2*x+1] = 0.5f;
    }
  }
}

void WavePlane::draw()
{
  glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glBindTexture(GL_TEXTURE_2D, envmapTexture);

  glVertexPointer(3,GL_FLOAT,16,verts);
  glTexCoordPointer(2,GL_FLOAT,8,texcs);
  glColorPointer(4,GL_UNSIGNED_BYTE,4,colrs);
  glDrawElements(GL_TRIANGLES, 6*(tessDim-1)*(tessDim-1), GL_UNSIGNED_INT, indis);

//  glDisable(GL_TEXTURE_2D);
//  glColor3f(1,1,0);
//  glBegin(GL_LINES);
//  for (int x=0; x<tessDim*tessDim; x++)
//  {
//    glVertex3f(verts[4*x+0], verts[4*x+1], verts[4*x+2]);
//    glVertex3f(verts[4*x+0] + norms[3*x+0], verts[4*x+1] + norms[3*x+1], verts[4*x+2] + norms[3*x+2]);
//  }
//  glEnd();

  glPopAttrib();
}
