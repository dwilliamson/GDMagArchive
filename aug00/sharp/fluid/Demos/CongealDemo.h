#ifndef CONGEALDEMO_H
#define CONGEALDEMO_H

//
// CongealDemo
//
// CongealDemo demonstrates that eerie T1000 sentient mercury effect by dropping molecules onto a plane
// and having them congeal across surfaces into one blob.  Kinda weird, but very cool.
//

#include <fluid/Demos/FluidDemo.h>
#include <gl/glut.h>

class CongealDemo : public FluidDemo
{
public:
  CongealDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Blood on Marble"); }

  virtual float getTessellationDensity()  { return 0.30f; }
  virtual bool useStablePhysics()         { return false; }
  virtual std::string getEnvMapFilename() { return std::string("data/silver_purpleplane2.raw"); }

  virtual void setGLCurrents();
  
  virtual void start();

  virtual void update(float timePassed);

protected:
  int quadTexture;
  
  float totalTimePassed;
};

#endif //CONGEALDEMO_H