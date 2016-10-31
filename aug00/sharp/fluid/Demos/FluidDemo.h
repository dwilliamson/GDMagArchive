#ifndef FLUIDDEMO_H
#define FLUIDDEMO_H

//
// FluidDemo
//
// FluidDemo is the abstract base class for a Strategy of objects that initialize the fluid driver program
// with a set of functors to achieve some cool demo.  Like, a fountain FluidDemo might hand back a
// fountain-like generator, a ground plane for collision against, and a downwards force of 9.8 m/s^2, i.e.
// gravity.
//

#include <fluid/ParticleSys/ParticleFunctor.h>
#include <gl/glut.h>
#include <string>

class FluidDemo
{
public:
  FluidDemo() { generator = 0; force = 0; integrator = 0; collision = 0; surfaceThreshold = 3.0f; }

  // This is where the functors get deleted.
  virtual ~FluidDemo();

  // This is queried to fill in the dropdown menu for the list of demos.
  virtual std::string getName() { return std::string("Unnamed Demo"); }

  // All of these are queried for every frame by the main program and the functors are run on the
  // particle system.
  ParticleFunctor* getGenerator()  { return generator;  }
  ParticleFunctor* getForce()      { return force;      }
  ParticleFunctor* getIntegrator() { return integrator; }
  ParticleFunctor* getCollision()  { return collision;  }

  // This returns the isovalue for the surface; it cannot be overloaded, instead have the demo
  // set the surfaceThreshold variable in its constructor.  This is because most demos need this
  // value in their constructor, and so making it virtual would mean they couldn't call the function
  // and would need to duplicate the value.
  float getSurfaceThreshold()     { return surfaceThreshold;  }

  // The other thing a FluidDemo can do is specify some initial parameters for the demo.  Like the
  // tessellation density, the physics type (fixed/variable timestep) and the envmap texture.  We supply
  // defaults here for demos that don't care.
  virtual float getTessellationDensity()  { return 0.60f; }
  virtual bool useStablePhysics()         { return false; }
  virtual float getPhysicsTick()          { return 0.045; }
  virtual float getTimeScalar()           { return 1.0f; }  
  virtual std::string getEnvMapFilename() { return std::string("data/spheremap256.raw"); }

  // Used to let us set up the current stuff we want in OpenGL, like the current color, or anything
  // else (normals, texcoords, although a current color is generally more useful than those.)
  virtual void setGLCurrents()            { glColor3ub(255,255,255); }

  // This is called to let a FluidDemo know that it's being started or restarted, so it can reset any
  // internal counters or anything...
  virtual void start() {}

  // Also, in case a demo needs to do something per-frame, like inreasing particle flow,
  // or drawing a ground plane or something like that, this function gets called each frame
  // just before we draw the fluid itself.
  virtual void update(float timePassed) {}

protected:
  // These should be initialized by the implementation; that's all it needs to do, then it's done.
  // Well, other than the drawing stuff.
  ParticleFunctor* generator;
  ParticleFunctor* force;
  ParticleFunctor* integrator;
  ParticleFunctor* collision;

  float surfaceThreshold;
};

#endif //FLUIDDEMO_H