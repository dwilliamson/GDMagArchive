#include <fluid/ParticleSys/HeightFieldCollider.h>

// This constructor takes the plane equation as the vector (a,b,c) and the scalar d, where the plane
// equation is ax+by+cz+d = 0.  Note that it's NOT ax+by+cz = d.
HeightFieldCollider::HeightFieldCollider(HeightField hf, float distThresh, float springCoeff, float dampCoeff, float maxRepel)
{
  heightfield = hf;

  springScalar = springCoeff;
  dampScalar = dampCoeff;
  
  maxDist = distThresh;
  maxForce = maxRepel;
}

void HeightFieldCollider::update(PotentialPoints& points, float timePassed)
{
  // Find e ach of their distances to the heightfield, if they're within the threshold start pushing
  // on them.
  for (int i=0; i<points.size(); i++)
  {
    PhysicalPoint& pt = points.getPoint(i);

    float distance;
    Vector normal;
    heightfield.getHeightAndNormalAt(pt.loc.x, pt.loc.y, distance, normal);
    distance = pt.loc.z - distance;

    // Make a little envelope for hysteresis -- this is the zone where we won't exert force but we
    // will cancel other forces to keep the object from jittering right on the boundary.
    if (distance < maxDist)
    {
      float velAlongNormal = normal.dot(pt.vel);

      // Include a little analytic contact -- if the particle is moving appreciably away from the plane,
      // do nothing as long as it's outside the plane.
      if (distance > 0 && velAlongNormal > 0.1f)
      {
        // no need to respond...
      }

      // If it's outside the plane and not moving into or out of the plane, just cancel out the force if
      // it's accelerating into the plane.
      else if (distance > 0 && velAlongNormal > 0.0f)
      {
        float forceAlongNormal = normal.dot(pt.force);
        if (forceAlongNormal < 0)
        {
          Vector force(normal);
          force *= -forceAlongNormal;
          pt.force += force;
        }
      }

      // Otherwise it's inside the plane or moving into the plane.
      else
      {
        float forceApply = springScalar * (maxDist - distance) - dampScalar * (velAlongNormal);
        forceApply = Math::maxOf(Math::minOf(forceApply, maxForce), 0);

        Vector force(normal);
        force *= forceApply;
        pt.force += force;
      }
    }
  }
}
