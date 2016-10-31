#include <fluid/ParticleSys/BoxBouncer.h>

BoxBouncer::BoxBouncer()
{
  boundingBox[0][0] = 0;
  boundingBox[0][1] = 0;
  boundingBox[0][2] = 0;
  boundingBox[1][0] = 0;
  boundingBox[1][1] = 0;
  boundingBox[1][2] = 0;
}

void BoxBouncer::setBoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
  boundingBox[0][0] = minX;
  boundingBox[0][1] = minY;
  boundingBox[0][2] = minZ;
  boundingBox[1][0] = maxX;
  boundingBox[1][1] = maxY;
  boundingBox[1][2] = maxZ;
}

void BoxBouncer::update(PotentialPoints& pts, float timePassed)
{
  // For each point
  for (int i=0; i<pts.size(); i++)
  {
    PhysicalPoint& pt = pts.getPoint(i);

    // Bounce it off walls.
    if (pt.loc.x < boundingBox[0][0] || pt.loc.x > boundingBox[1][0])
    {
      pt.vel.x *= -1.0f;
    }
    if (pt.loc.y < boundingBox[0][1] || pt.loc.y > boundingBox[1][1])
    {                                           
      pt.vel.y *= -1.0f;
    }
    if (pt.loc.z < boundingBox[0][2] || pt.loc.z > boundingBox[1][2])
    {
      pt.vel.z *= -1.0f;
    }
  }
}
