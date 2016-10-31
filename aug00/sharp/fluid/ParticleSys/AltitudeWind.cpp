#include <fluid/ParticleSys/AltitudeWind.h>

AltitudeWind::AltitudeWind() : force(0,0,0), box(0,0,0,0,0,0)
{ 
}

void AltitudeWind::setForce(const Vector& forceIn)
{
  force = forceIn;
}

void AltitudeWind::setBoundingBox(const BoundingBox& applicationArea)
{
  box = applicationArea;
}

void AltitudeWind::update(PotentialPoints& points, float timePassed)
{
  // Accelerate all existing points
  for (int i=0; i<points.size(); i++)
  {
    const Vector& pt = points.getPoint(i).loc;

    if (pt.x >= box.minX && pt.y >= box.minY && pt.z >= box.minZ && 
        pt.x <= box.minX+box.sizeX && pt.y <= box.minY+box.sizeY && pt.z <= box.minZ+box.sizeZ)
    {
      points.getPoint(i).force += force;
    }
  }
}