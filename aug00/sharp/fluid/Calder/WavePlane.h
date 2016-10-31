#ifndef WAVEPLANE_H
#define WAVEPLANE_H

//
// WavePlane
//
// WavePlane is the standard "jiggly plane" style of liquid rendering.  In this case I'm using it
// for the large pool of mercury in the bottom of the fountain since the wave propagation is just too
// hard to do right with the implicit surface itself, it's easier to just have a tessellated plane
// that does the wave impulses in closed form.
//

#include <math/DimRectangle.h>
#include <math/Vector.h>
#include <list>

class WaveImpulse
{
public:
  WaveImpulse(float x, float y) { epiX = x; epiY = y; duration = 0; }

  void getHeightAndNormalAt(float locX, float locY, float& heightOut, Vector& normalOut);
  void update(float timePassed);

  float getDuration() const { return duration; }

protected:
  float epiX, epiY;
  float duration;
};

class WavePlane
{
public:
  WavePlane(DimRectangle area, float baseHeight, int dim = 32);
  ~WavePlane();

  float getBaseHeight() const { return restHeight; }

  // Used to add a wave that propagates outwards for some amount of time (hardcoded since the fluid
  // has one wave propagation speed and a finite x/y range.)
  void createWaveImpulse(float xDisturb, float yDisturb);

  // Tranquilizes the surface into a smooth, wave-free plane.
  void clearWaves();

  void update(float timePassed);
  void draw();

protected:
  float* verts;
  float* texcs;
  unsigned char* colrs;
  unsigned int* indis;
//  float* norms;
  int tessDim;
  DimRectangle range;
  float restHeight;

  std::list<WaveImpulse> waves;

  unsigned int envmapTexture;
};

#endif //WAVEPLANE_H