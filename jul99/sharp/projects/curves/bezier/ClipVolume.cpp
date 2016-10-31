#include <curves/bezier/ClipVolume.h>
#include <curves/bezier/PatchBox.h>
#include <curves/GlobalCamera.h>
#include <math/Vector.h>
#include <gl/glut.h>

ClipVolume::ClipVolume(const GlobalCamera& cam)
{
  eyePt = cam.getPosition();

  // Get camera position information.
  const Position& pos = cam.getPosition();
  Vector eye(pos.x, pos.y, pos.z);

  // Get perspective information.
  float alpha = cam.getFovY() / 2.0;
  float beta = cam.getFovX() / 2.0;
  alpha = Vector::degreesToRadians(alpha);
  beta = Vector::degreesToRadians(beta);
  
  // Get vector stuff from the position.
  const Vector& nonOrthoTop = pos.getUpVector();
  const Vector& front = pos.getOrientation();
  Vector side;
  nonOrthoTop.cross(front, side);

  // Make sure our up vector is orthogonal.
  Vector top;
  front.cross(side, top);
  
  // Get our plane normals.
  Vector topNormal(-top);
  side.rotate(topNormal, -alpha);
  
  Vector bottomNormal(top);
  side.rotate(bottomNormal, alpha);
  
  Vector rightNormal(side);
  top.rotate(rightNormal, -beta);
  
  Vector leftNormal(-side);
  top.rotate(leftNormal, beta);
  
  Vector nearNormal(front);
  
  // Now calculate our plane offsets from the normals and the eye position.
  float topD    = eye.dot(-topNormal);
  float bottomD = eye.dot(-bottomNormal);
  float leftD   = eye.dot(-leftNormal);
  float rightD  = eye.dot(-rightNormal);
  float nearD   = eye.dot(-nearNormal);

  // For the far plane, find the point farDist away from the eye along the front vector.
  float farDist = cam.getFarPlane();
  Vector farPt(front);
  farPt.setMagnitude(farDist);
  farPt += eye;
  float farD    = farPt.dot(nearNormal);
  
  // Now generate the planes, and voila!
  topPlane    = Plane(topNormal   , topD   );
  bottomPlane = Plane(bottomNormal, bottomD);
  leftPlane   = Plane(leftNormal  , leftD  );
  rightPlane  = Plane(rightNormal , rightD );
  nearPlane   = Plane(nearNormal  , nearD  );
  farPlane    = Plane(-nearNormal , farD   );
}

bool boxIsOutside(const PatchBox& box, const Plane& plane)
{
  return !(plane.isInside(box.corners[0]) || plane.isInside(box.corners[1]) ||
           plane.isInside(box.corners[2]) || plane.isInside(box.corners[3]) ||
           plane.isInside(box.corners[4]) || plane.isInside(box.corners[5]) ||
           plane.isInside(box.corners[6]) || plane.isInside(box.corners[7]));
}

bool ClipVolume::contains(const PatchBox& box) const
{
  // If all of the box's corners are outside one plane, return true.  This is
  // sloppy but <Grim Fandango>good enough for government work</Grim Fandango>.
  if (boxIsOutside(box, leftPlane)) return false;
  if (boxIsOutside(box, rightPlane)) return false;
  if (boxIsOutside(box, nearPlane)) return false;
  if (boxIsOutside(box, farPlane)) return false;
  if (boxIsOutside(box, bottomPlane)) return false;
  if (boxIsOutside(box, topPlane)) return false;
  return true;
}