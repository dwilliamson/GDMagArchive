#ifndef DUMMYDEMO_H
#define DUMMYDEMO_H

#pragma warning (disable: 4786)

// This is a debug test that just draws a single sphere.

#include <fluid/Demos/FluidDemo.h>
#include <gl/glut.h>

class DummyDemo : public FluidDemo
{
public:
  DummyDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Dummy Test Demo"); }

  virtual float getTessellationDensity()  { return 0.15f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/silver_purpleplane2.raw"); }

  // This makes it look like crazy blood.
  virtual void setGLCurrents()            
  { 
    // This breaks the rest of the demo, so generally leave this 
    // demo out (it's really just for hacking around, anyway.)
    glColor4ub(128,128,128,255);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
  }

protected:
};

#endif //DUMMYDEMO_H