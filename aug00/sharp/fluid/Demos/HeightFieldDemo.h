#ifndef HEIGHTFIELDDEMO_H
#define HEIGHTFIELDDEMO_H

//
// HeightFieldDemo
//
// HeightFieldDemo is a test demo of fluid running down along a height field.
//

#include <fluid/Demos/FluidDemo.h>
#include <fluid/calder/HeightField.h>
#include <fluid/ParticleSys/ConstantExerter.h>

class HeightFieldDemo : public FluidDemo
{
public:
  HeightFieldDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Mercury on Heightfield"); }

  virtual float getTessellationDensity()  { return 0.50f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/silver_blueplane.raw"); }

  virtual void start();

  virtual void setGLCurrents()            
  { 
    glColor4ub(192, 192, 192, 255);
    glDisable(GL_BLEND);
  }

  virtual void update(float timePassed);

protected:
  HeightField heightfield;

  ConstantExerter* gravity;
};

#endif //HEIGHTFIELDDEMO_H