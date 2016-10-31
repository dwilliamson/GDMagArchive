#ifndef STREAMDEMO_H
#define STREAMDEMO_H

//
// StreamDemo
//
// StreamDemo is a demo of water running down a channel formed by two intersecting planes.  There's also
// a cylinder blocking the center of the path just for fun.
//

#include <fluid/Demos/FluidDemo.h>
#include <gl/glut.h>

class StreamDemo : public FluidDemo
{
public:
  StreamDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Water Stream"); }

  virtual float getTessellationDensity()  { return 0.60f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/spheremap_blueball.raw"); }
  
  virtual void start();

  virtual void setGLCurrents()            
  {
    glColor4ub(100,120,255,128);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  virtual void update(float timePassed);

protected:
  // Our cylinder.
  GLUquadric* cylinder;
};

#endif //STREAMDEMO_H