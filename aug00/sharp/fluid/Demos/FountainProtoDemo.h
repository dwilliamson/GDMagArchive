#ifndef FOUNTAINPROTODEMO_H
#define FOUNTAINPROTODEMO_H

//
// FountainProtoDemo
//
// FountainProtoDemo is a test demo of fluid running down along a height field.
//

#include <fluid/Demos/FluidDemo.h>
#include <fluid/calder/HeightField.h>
#include <fluid/calder/WavePlane.h>
#include <fluid/calder/FountainVisuals.h>

class FountainProtoDemo : public FluidDemo
{
public:
  FountainProtoDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Mercury Fountain"); }

  virtual float getTessellationDensity()  { return 0.4f; }
  virtual bool useStablePhysics()         { return true; }
  virtual float getPhysicsTick()          { return 0.01; }
  virtual float getTimeScalar()           { return 0.35f; }  
  virtual std::string getEnvMapFilename() { return std::string("data/silver_sphere_colorlights.raw"); }

  virtual void setGLCurrents();
  virtual void start();

  virtual void update(float timePassed);

protected:
  HeightField heightfield[2];

  float xRange[2], yRange[2];

  Vector mins, maxs;

  WavePlane wavyPlane;
  FountainVisuals fountain;

  void loadBasin1();
  void loadBasin2();
};

#endif //FOUNTAINPROTODEMO_H