#ifndef MARBLE_H
#define MARBLE_H

#include <math/Vector.h>
#include <curves/bezier/TerrainQuadtree.h>
#include <curves/bezier/Sphere.h>

// This is a marble that rolls around on a TerrainQuadtree.  You can create them, and then
// each frame, update and draw them.
class Marble
{
public:
  Marble(float x=0, float y=0, float z=0, float mySize=0.3);
  
  // This returns false if the marble rolls off the terrain.
  bool update(TerrainQuadtree& terrain);

  void draw();

  static void setTextureName(unsigned int textureName);

  const Vector& getPosition() const { return position; }
  const Vector& getVelocity() const { return velocity; }

private:
  Vector velocity;
  Vector position;
  long lastUpdateTime;

  static Sphere* sphere;

  float u, v;
  unsigned int lightmapName;
  float myRadius;
};

#endif // MARBLE_H
