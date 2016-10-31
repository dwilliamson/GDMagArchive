#ifndef FOUNTAINDEMO_H
#define FOUNTAINDEMO_H

#include <fluid/Demos/FluidDemo.h>
#include <gl/glut.h>

class FountainDemo : public FluidDemo
{
public:
  FountainDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Golden Shower"); }

  virtual float getTessellationDensity()  { return 0.45f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/gold_greyplane.raw"); }

  virtual void start();

  virtual void setGLCurrents()            
  { 
    glColor3ub(255,255,255);
    glDisable(GL_BLEND);
  }

  virtual void update(float timePassed);

protected:
};

#endif //FOUNTAINDEMO_H