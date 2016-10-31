#include <fluid/calder/WaveGenerator.h>

WaveGenerator::WaveGenerator(WavePlane* plane)
{
  wavePlane = plane;
}

void WaveGenerator::update(PotentialPoints& points, float timePassed)
{
  for (int x=0; x<points.size(); x++)
  {
    PhysicalPoint& pt = points.getPoint(x);

    // If they're on different sides, it passed through.  If their product is negative
    // it means that one's on the top and one's on the bottom, and hence they're on different sides.
    if ((pt.preIntegration.loc.z - wavePlane->getBaseHeight()) * (pt.loc.z - wavePlane->getBaseHeight()) < 0)
    {
      wavePlane->createWaveImpulse(pt.loc.x, pt.loc.y);
    }
  }
}
