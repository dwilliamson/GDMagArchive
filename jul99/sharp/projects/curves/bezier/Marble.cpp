#include <curves/bezier/Marble.h>
#include <curves/bezier/BezierPatch.h>
#include <curves/OpenGL.h>
#include <mmsystem.h>

Sphere* Marble::sphere = NULL;

// This is a marble that rolls around on a TerrainQuadtree.  You can create them, and then
// each frame, update and draw them.
Marble::Marble(float x, float y, float z, float mySize)
{
  position.x = x;
  position.y = y;
  position.z = z;

  velocity.x = 0;
  velocity.y = 0;
  velocity.z = 0;

  lastUpdateTime = timeGetTime();

  myRadius = mySize;
}

#define MARBLE_FALL_BOTTOM -1000.0f

bool Marble::update(TerrainQuadtree& terrain)
{
  long curTime = timeGetTime();
  float dt = (curTime - lastUpdateTime) * 0.001;

  // Figure out if we're on the terrain or not.  If we are, find the gradient to
  // accelerate along.
  float terrZ = MARBLE_FALL_BOTTOM;
  if (terrain.contains(position.x, position.y))
  {
    terrZ = terrain.getValueAt(position.x, position.y);
  }

  bool wasOnTerrain = false;
  if (position.z-myRadius <= terrZ)
  {
    wasOnTerrain = true;
    Vector grad = terrain.getGradientAt(position.x, position.y);

    Vector gradNeg = grad;
    gradNeg *= -1.0f;
    gradNeg.normalize();
    float gradLen = gradNeg.dot(Vector(0,0,-9.3f));
    gradNeg.setMagnitude(gradLen);

    // We have our gradient.  Accelerate along it.
    gradNeg *= dt;
    velocity += gradNeg;

    // Now make sure our velocity doesn't point down into the hill.  If it does, make it the gradient at this
    // point.
    Vector gradAlongVel = velocity;
    gradAlongVel.z = 0;
    gradAlongVel.normalize();
    BezierPatch::getVectorInGradientPlane(grad, gradAlongVel);

    // Now see which points "higher"
    float gradAlongVelR = sqrt(gradAlongVel.x*gradAlongVel.x + gradAlongVel.y*gradAlongVel.y);
    float gradAlongVelSlope = gradAlongVel.z / gradAlongVelR;

    float velR = sqrt(velocity.x*velocity.x + velocity.y*velocity.y);
    float velSlope = velocity.z / velR;

    if (gradAlongVelSlope > velSlope)
    {
      float velMag = velocity.getMagnitude();
      gradAlongVel.setMagnitude(velMag);
      velocity = gradAlongVel;
    }
  }

  // Otherwise, just accelerate down with gravity and move by the velocity,
  else
  {
    // This is gravity.
    Vector accel(0,0,-9.3f);
    accel *= dt;
    velocity += accel;
  }

  // Now move the point along the terrain by our velocity.
  Vector scaledVel(velocity);
  scaledVel *= dt;
  position += scaledVel;

  // Check to make sure we're still on the terrain.
  if (position.z-myRadius < MARBLE_FALL_BOTTOM)
  {
    return false;
  }

  // Update our lightmap and texture coordinate in the lightmap as well as our position
  // given our new location.
  float newTerrZ = MARBLE_FALL_BOTTOM;
  if (terrain.contains(position.x, position.y))
  {
    newTerrZ = terrain.getValueAt(position.x, position.y);
    terrain.getLightmapAt(position.x, position.y, lightmapName, u, v);
  }

  bool isOnTerrain = false;
  if (position.z-myRadius < newTerrZ)
  {
    isOnTerrain = true;
    position.z = newTerrZ+myRadius;
  }

  // If we just hit the ground...
  if (!wasOnTerrain && isOnTerrain)
  {
    if (terrain.contains(position.x, position.y))
    {
      Vector grad = terrain.getGradientAt(position.x, position.y);

      // If we're pointing straight down...
      if (fabs(velocity.x) + fabs(velocity.y) < 0.001)
      {
        grad *= -1.0f;
        grad.normalize();
        float velComponent = velocity.dot(grad);
        grad.setMagnitude(velComponent);
        velocity = grad;
      }

      // Otherwise, we just flew off the terrain for a second.
      else
      {
        Vector gradAlongVel = velocity;
        gradAlongVel.z = 0;
        gradAlongVel.normalize();
        BezierPatch::getVectorInGradientPlane(grad, gradAlongVel);
        gradAlongVel.normalize();
        float velComponent = velocity.dot(gradAlongVel);
        gradAlongVel.setMagnitude(velComponent);
        velocity = gradAlongVel;
      }
    }
  }

  // Faux friction.  If it's on the ground, slow it down a little.
  if (position.z-myRadius == newTerrZ)
  {
    Vector slowdown(velocity);
    slowdown *= 0.06f * dt;
    velocity -= slowdown;
  }

  lastUpdateTime = curTime;

  return true;
}

void Marble::draw()
{
  // Tell the sphere to draw.
  glColor3f(1,1,1);
  glPushMatrix();
  glTranslatef(position.x, position.y, position.z);

  // This shouldn't happen -- setTextureName should get called first.
  if (sphere == NULL)
  {
    sphere = new Sphere(20, 0);
  }

  sphere->draw(lightmapName, u, v);
  glPopMatrix();
}

void Marble::setTextureName(unsigned int texName)
{
  if (sphere == NULL)
  {
    sphere = new Sphere(20, texName);
  }
}