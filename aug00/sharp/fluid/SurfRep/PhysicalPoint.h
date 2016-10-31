#ifndef PHYSICALPOINT_H
#define PHYSICALPOINT_H

//
// PhysicalPoint
//
// PhysicalPoint is the representation of a single particle moving through space, and stores all forces
// being applied upon it in the current physics frame, as well as its current velocity and position.  It
// also stores a shadow of itself, preIntegration.  This only makes sense to colliders, and is simply the
// state of the point before integration occurred (so where it was last frame.)  This is needed because
// the colliders will often have to "back up" a point if the integration step caused it to interpenetrate
// with a wall or something.
//

#include <math/Vector.h>

// This is the little mini-class used to store the point's pre-integration shadow.
class PointPack
{
public:
  PointPack() : loc(0,0,0), vel(0,0,0), force(0,0,0) {}

  Vector loc;
  Vector vel;
  Vector force;
};

// This is the class itself stored in PotentialPoints.
class PhysicalPoint
{
public:
  // Basic default constructor for initialization.
  PhysicalPoint() : loc(0,0,0), vel(0,0,0), force(0,0,0), preIntegration() { next = 0; surfaceId = 0; heightFieldTier = 0; }

  // Can be used to implement a velocity-based physics model involving these points.
  Vector loc;
  Vector vel;
  Vector force;

  // This is the past "shadow" of the point.  It only makes sense during the collision step.
  PointPack preIntegration;

  // This makes them easy to string together into linked lists.
  PhysicalPoint* next;

  // This is generated every frame during tessellation and indicates which surface the point
  // is under; there's no particular order to the numbers; what they're really useful for
  // is figuring out if two points' id tags are the same, in which case they're within the
  // same surface.
  mutable int surfaceId;

  // This is, admittedly, probably not the right place for this, but I can't figure out exactly what
  // the right place is, given that these points aren't stored in any given order in their container and
  // aren't guaranteed to maintain that order, either.  Anyway, what this is is a number used by the
  // MultiHeightFieldCollider to determine which level of the tiers of height fields this particle is on.
  // It needs to be persistent since there's not necessarily a good way to figure it out explicitly.
  mutable int heightFieldTier;
};

#endif //PHYSICALPOINT_H