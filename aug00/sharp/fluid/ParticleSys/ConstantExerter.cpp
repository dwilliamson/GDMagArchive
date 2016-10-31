#include <fluid/ParticleSys/ConstantExerter.h>

void ConstantExerter::setForce(const Vector& forceIn)
{
  force = forceIn;
}

void ConstantExerter::update(PotentialPoints& points, float timePassed)
{
  // Accelerate all existing points
  for (int i=0; i<points.size(); i++)
  {
    points.getPoint(i).force += force;
  }
}