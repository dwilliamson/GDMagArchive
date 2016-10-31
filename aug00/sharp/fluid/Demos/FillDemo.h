#ifndef FILLDEMO_H
#define FILLDEMO_H

//
// FillDemo
//
// FillDemo is a demo of water running down into a pool-like container and filling it.
//

#include <fluid/Demos/FluidDemo.h>
#include <gl/glut.h>

class FillDemo : public FluidDemo
{
public:
  FillDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Flood-Fill Demo"); }

  virtual float getTessellationDensity()  { return 0.60f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/spheremap_blueball.raw"); }

  virtual void setGLCurrents()            
  { 
    glColor4ub(100,120,255,128);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  virtual void update(float timePassed);

protected:
};

#endif //FILLDEMO_H