#include <fluid/ParticleSys/CylinderCollider.h>

// This takes the cylinder as a parameterized line, p(t) = linePt + lineSlope*t.  Then, the thickness
// is 'radius', the maximum distance away from the radius is distThresh, and maxRepel is the maximum
// correctional force it'll apply in the penalty methods.
CylinderCollider::CylinderCollider(Vector linePoint, Vector lineSlope, float radiusIn, float distThresh, float maxRepel)
{
  linePt = linePoint;
  
  lineDir = lineSlope;
  lineDir.normalize();

  radius = radiusIn;
  
  maxDist = distThresh;
  maxForce = maxRepel;
}

#define SQUARE(a) ((a) * (a))

void CylinderCollider::update(PotentialPoints& points, float timePassed)
{
  const float HYSTERESIS_ENVELOPE = 0.0f;//0.5f;

  // Find e ach of their distances to the plane, if they're within the threshold start pushing
  // on them.
  for (int i=0; i<points.size(); i++)
  {
    PhysicalPoint& pt = points.getPoint(i);
    Vector pointMinusPoint = pt.loc;
    pointMinusPoint -= linePt;

    // Since the closest point on the line will lie in the plane whose normal is the line direction and
    // that also contains the molecule, the closest point is linePt + (pmp dot dir) * dir;
    Vector closestPoint = lineDir;
    closestPoint *= lineDir.dot(pointMinusPoint);
    closestPoint += linePt;

    // Now find the vector between them, this is our normal.
    Vector normal = pt.loc;
    normal -= closestPoint;

    // We're doing things with standard impulse-based collisions instead of penalties because we can
    // be really sloppy (don't have to subdivide the timestep to find the exact collision time, etc.)
    // but I don't like the penalty springiness.
    float distSquared = normal.getMagnitudeSquared();
    if (distSquared < SQUARE(radius*(1.0f+HYSTERESIS_ENVELOPE)))
    {
      normal.fastNormalize();

      // Only move the point if it's intersecting; moving it can screw up the molecular interaction.
//      if (distSquared < SQUARE(radius))
//      {
        // Back the point's location up to be just touching the cylinder.
//        pt.loc = pt.preIntegration.loc;
      pt.loc = normal;
      pt.loc *= radius;
      pt.loc += closestPoint;
//      }

      // Now hit its velocity with an impulse...
      const float COEFFICIENT_OF_RESTITUTION = 0.0f;
      float impulseMagnitude = -(1+COEFFICIENT_OF_RESTITUTION) * (normal.dot(pt.vel));
      Vector impulse = normal;
      impulse *= impulseMagnitude;
      pt.vel += impulse;
    }

//    // Make a little envelope for hysteresis -- this is the zone where we won't exert force but we
//    // will cancel other forces to keep the object from jittering right on the boundary.
//    if ( normal.getMagnitudeSquared() < SQUARE(radius + maxDist * (1.0f + HYSTERESIS_ENVELOPE)) )
//    {
//      // Need to know this now, so soak the magnitude calculation (aiee, sqrt.)
//      float distance = normal.getMagnitude() - radius;
//      normal.fastNormalize();
//
//      float velAlongNormal = normal.dot(pt.vel);
//
//      // Include a little analytic contact -- if the particle is moving appreciably away from the plane,
//      // do nothing as long as it's outside the plane.
//      if (distance > 0 && velAlongNormal > 0.1f)
//      {
//        // no need to respond...
//      }
//
//      // If it's outside the plane and not moving into or out of the plane, just cancel out the force if
//      // it's accelerating into the plane.
//      else if (distance > 0 && velAlongNormal > 0.0f)
//      {
//        float forceAlongNormal = normal.dot(pt.force);
//        if (forceAlongNormal < 0)
//        {
//          Vector force(normal);
//          force *= -forceAlongNormal;
//          pt.force += force;
//        }
//
//        // Make sure it's not moving in the envelope of velocity away (it can end up moving away very
//        // slowly from the plane...)
////        Vector velCancel(normal);
////        velCancel *= -velAlongNormal;
////        pt.vel += velCancel;
//      }
//
//      // Otherwise it's inside the plane or moving into the plane.
//      else
//      {
//        float forceApply = 100.0f * log(1.0f + (maxDist - distance));
//        forceApply = Math::minOf(forceApply, maxForce);
//
//        Vector force(normal);
//        force *= forceApply;
//        pt.force += force;
//      }
//    }
  }
}
