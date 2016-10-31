#include <fluid/Demos/FluidDemo.h>

FluidDemo::~FluidDemo()
{
  delete generator;
  delete force;
  delete integrator;
  delete collision;
}