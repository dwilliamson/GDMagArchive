#include <fluid/calder/MultiHeightFieldCollider.h>

// This constructor takes the plane equation as the vector (a,b,c) and the scalar d, where the plane
// equation is ax+by+cz+d = 0.  Note that it's NOT ax+by+cz = d.
MultiHeightFieldCollider::MultiHeightFieldCollider(float distThresh, float springCoeff, float dampCoeff, float maxRepel)
{
  springScalar = springCoeff;
  dampScalar = dampCoeff;
  
  maxDist = distThresh;
  maxForce = maxRepel;
}

// This chains a height field onto the end of the list, with the specified plane being the 
// condition for switching from this height field to the next (or, if this is the last, ceasing to
// collide against any of these height fields at all.)
void MultiHeightFieldCollider::addHeightField(const HeightField& hf, const Plane& plane)
{
  heightfields.push_back(hf);
  layerPlanes.push_back(plane);
}

void MultiHeightFieldCollider::update(PotentialPoints& points, float timePassed)
{
  // Find each of their distances to the heightfield, if they're within the threshold start pushing
  // on them.
  for (int i=0; i<points.size(); i++)
  {
    PhysicalPoint& pt = points.getPoint(i);

    if (pt.heightFieldTier < heightfields.size())
    {
      // Does it need to be moved to the next heightfield?
      const Plane& curLayerPlane = layerPlanes[pt.heightFieldTier];
      
      Vector planeToPoint = curLayerPlane.getNormal() * (-curLayerPlane.getD() / (curLayerPlane.getNormal().getMagnitudeSquared()));
      planeToPoint = pt.loc - planeToPoint;
      float planeSide = planeToPoint.dot(curLayerPlane.getNormal());
      if (planeSide >= 0)
      {
        pt.heightFieldTier++;
      }

      if (pt.heightFieldTier < heightfields.size())
      {
        float distance;
        Vector normal;
        heightfields[pt.heightFieldTier].getHeightAndNormalAt(pt.loc.x, pt.loc.y, distance, normal);
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
  }
}
