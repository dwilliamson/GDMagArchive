#include <curves/Weather.h>
#include <curves/GlobalCamera.h>
#include <gl/glut.h>
#include <mmsystem.h>

Weather* Weather::singleton = NULL;

// This gets the weather system.
Weather* Weather::instance()
{
  if (!singleton)
  {
    singleton = new Weather();
  }
  return singleton;
}

// Well, since there's only snow right now, this is kinda moot.
void Weather::setWeatherType(WEATHER_TYPE newType)
{
  weatherType = newType;
}

// This determines how many particle per... some square somethings
// there are.  Tweak to taste.
void Weather::setThickness(float newThickness)
{
  thickness = newThickness;
}

// This determines what area the weather is created over.
void Weather::setArea(float newArea)
{
  area = newArea;
}

// This defines the range of the life of the weather particles.
void Weather::setStartZ(float newVal)
{
  highZ = newVal;
}

void Weather::setEndZ(float newVal)
{
  lowZ = newVal;
}

// Currently, I only support a constant velocity on weather systems.
// Maybe I can support acceleration later.
void Weather::setVelocity(const Vector& vel)
{
  velocity = vel;
}

// This is a diameter-size of the quad that is the particle.
void Weather::setSize(float newVal)
{
  diam = newVal;
}

// This is the texture used in drawing the weather particles.
void Weather::setTexture(int tex)
{
  textureName = tex;
}

// Update should be called each frame to move stuff.
#define RAND_NEG1_TO_POS1 ((2.0f * rand() / (float)RAND_MAX) - 1.0f)
void Weather::update()
{
  long curTime = timeGetTime();
  float dt = (curTime - lastTime) / 1000.0f;

  // Update particles and expire any old ones.
  Vector thisVelocity = velocity;
  thisVelocity *= dt;
  for (std::list<Vector>::iterator it = flakes.begin(); it != flakes.end();)
  {
    *it += thisVelocity;
    if (it->z < lowZ)
    {
      it = flakes.erase(it);
    }
    else
    {
      it++;
    }
  }

  // Add any particles that should have been created.
  float fractionalNewParticles = thickness * area * area * dt;
  int numNewParticles = ceil(fractionalNewParticles);

  for (int x=0; x<numNewParticles; x++)
  {
    Vector newParticle(RAND_NEG1_TO_POS1 * area, RAND_NEG1_TO_POS1 * area, highZ);
    flakes.push_back(newParticle);
  }

  lastTime = curTime;
}

// Draw draws the weather system.
void Weather::draw()
{
  // Get the "real" up vector (make sure it's perpendicular to the orientation vector).  Along the way,
  // get the left vector, too.
  const Position& pos = GlobalCamera::Instance()->getPosition();
  const Vector& ori = pos.getOrientation();
  Vector upV(pos.getUpVector());
  Vector left;
  upV.cross(ori, left);
  left.cross(ori, upV);

  // Now set them both to the right lengths.  These are what we use to make our billboarded quads.
  upV.setMagnitude(diam*0.5f);
  left.setMagnitude(diam*0.5f);

  float upX = upV.x;
  float upY = upV.y;
  float upZ = upV.z;

  float leftX = left.x;
  float leftY = left.y;
  float leftZ = left.z;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureName);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  float verts[12];
  float texCoords[8] = {0,1, 1,1, 1,0, 0,0};
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, verts);
  glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  for (std::list<Vector>::iterator it = flakes.begin(); it != flakes.end(); it++)
  {
    verts[0] = verts[3] = verts[6] = verts[9 ] = it->x;
    verts[1] = verts[4] = verts[7] = verts[10] = it->y;
    verts[2] = verts[5] = verts[8] = verts[11] = it->z;

    verts[0] += upX;
    verts[1] += upY;
    verts[2] += upZ;
    verts[0] += leftX;
    verts[1] += leftY;
    verts[2] += leftZ;

    verts[3] += upX;
    verts[4] += upY;
    verts[5] += upZ;
    verts[3] -= leftX;
    verts[4] -= leftY;
    verts[5] -= leftZ;

    verts[6] -= upX;
    verts[7] -= upY;
    verts[8] -= upZ;
    verts[6] -= leftX;
    verts[7] -= leftY;
    verts[8] -= leftZ;

    verts[9 ] -= upX;
    verts[10] -= upY;
    verts[11] -= upZ;
    verts[9 ] += leftX;
    verts[10] += leftY;
    verts[11] += leftZ;

    glDrawArrays(GL_QUADS, 0, 4);
  }
  glEnable(GL_CULL_FACE);
}

Weather::Weather()
{
  velocity = Vector(0,0,0);
  lastTime = timeGetTime();
  thickness = 0;
  area = 1;
  highZ = 0;
  lowZ = 0;
  textureName = 0;
  diam = 1;
}
