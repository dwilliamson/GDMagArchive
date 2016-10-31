#ifndef MOUNTAINEER_H
#define MOUNTAINEER_H

#include <curves/CurvePoint.h>

class Mountaineer
{
public:
  Mountaineer();
  ~Mountaineer();

  void generateFractalTerrain(CurvePoint* controlsArray, int controlCt, float granularity, float initRange);
protected:
  void subdivide(int x0, int x1, int y0, int y1);

  int numControls;
  CurvePoint* controls;
  bool* validities;
  float grain;
};

#endif //MOUNTAINEER_H