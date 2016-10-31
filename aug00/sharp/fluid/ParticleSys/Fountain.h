#ifndef FOUNTAIN_H
#define FOUNTAIN_H

// 
// Fountain
//
// This class generates points at a set starting point spraying upwards in a cone-shape.
// and then accelerates them downwards until they hit the ground, at which point they're
// killed and a new point is spawned in at the starting point.  Ad naseum.
//

#include <fluid/ParticleSys/ParticleFunctor.h>

class Fountain : public ParticleFunctor
{
public:
  // Access methods used to set this guy up.  Call these in the order they're declared here.
  // (Some depend on others having been called.  Peak height is relative to the start point
  // height, so if you raise the start point the fountain will rise to a higher peak.  Floor,
  // on the other hand, is absolute... this seemed more intuitive, even if it means the two
  // are specified differently.
  void setMinPhi(float phi);
  void setMaxPhi(float phi);
  void setStartPoint(const Vector&);
  void setPeakHeight(float height);
  void setFloor(float height);

  // This, specifically, must be called last as it uses the variables set in the above to figure
  // out a particle rate to keep the fountain evenly distributed at the max number of particles.
  void setMaxNumPoints(int maxNum);
  int getMaxNumPoints() const { return maxNumPoints; }

  virtual void update(PotentialPoints&, float timePassed);
  virtual ParticleFunctorType getType() { return GENERATOR; }

protected:

  // Calculated from the travel distance and max num points -- the frequency at which we should
  // be creating or restarting points.
  float pointRefreshTime;
  
  // This is the upwards force needed to get the points to their specified max height.
  float zForce;

  // Attributes from above functions.
  float minPhi, maxPhi;
  float maxHeight, minHeight;
  Vector startLoc;
  int maxNumPoints;
};

#endif //FOUNTAIN_H