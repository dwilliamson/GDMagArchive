#include <fluid/ParticleSys/PlaneCollider.h>

// This constructor takes the plane equation as the vector (a,b,c) and the scalar d, where the plane
// equation is ax+by+cz+d = 0.  Note that it's NOT ax+by+cz = d.
PlaneCollider::PlaneCollider(Vector abcIn, float dIn, float distThresh, float maxRepel)
{
  abc = abcIn;
  d = dIn;
  
  // Normalize the plane equation.
  d /= abc.getMagnitude();
  abc.normalize();
  
  maxDist = distThresh;
  maxForce = maxRepel;
}

void PlaneCollider::update(PotentialPoints& points, float timePassed)
{
  const float HYSTERESIS_ENVELOPE = 0.5f;

  // Find e ach of their distances to the plane, if they're within the threshold start pushing
  // on them.
  for (int i=0; i<points.size(); i++)
  {
    float distance = abc.dot(points.getPoint(i).loc) + d;

//    // We're doing things with standard impulse-based collisions instead of penalties because we can
//    // be really sloppy (don't have to subdivide the timestep to find the exact collision time, etc.)
//    // but I don't like the penalty springiness.
//    if (distance < 0)
//    {
//      PhysicalPoint& pt = points.getPoint(i);
//
//      // The point is inside the plane.  Move it to the plane's surface...
//      pt.loc -= abc * distance;
//
//      // Now hit its velocity with an impulse...
//      const float COEFFICIENT_OF_RESTITUTION = 0.0f;
//      float impulseMagnitude = -(1+COEFFICIENT_OF_RESTITUTION) * (abc.dot(pt.vel));
//      Vector impulse = abc;
//      impulse *= impulseMagnitude;
//      pt.vel += impulse;
//    }

    // Make a little envelope for hysteresis -- this is the zone where we won't exert force but we
    // will cancel other forces to keep the object from jittering right on the boundary.
    if (distance < maxDist * (1.0f + HYSTERESIS_ENVELOPE))
    {
      float velAlongNormal = abc.dot(points.getPoint(i).vel);

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
        float forceAlongNormal = abc.dot(points.getPoint(i).force);
        if (forceAlongNormal < 0)
        {
          Vector force(abc);
          force *= -forceAlongNormal;
          points.getPoint(i).force += force;
        }

        // Make sure it's not moving in the envelope of velocity away (it can end up moving away very
        // slowly from the plane...)
//        Vector velCancel(abc);
//        velCancel *= -velAlongNormal;
//        points.getPoint(i).vel += velCancel;
      }

      // Otherwise it's inside the plane or moving into the plane.
      else
      {
        float forceApply = 100.0f * log(1.0f + (maxDist - distance));
        forceApply = Math::minOf(forceApply, maxForce);

        Vector force(abc);
        force *= forceApply;
        points.getPoint(i).force += force;
      }
    }
  }
}
