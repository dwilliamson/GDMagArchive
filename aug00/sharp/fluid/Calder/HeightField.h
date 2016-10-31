#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H

//
// HeightField
//
// HeightField is a representation of a 2D array of height values over some 2D area, and it can tell you 
// things like the interpolated height at any (u,v) within that area, as well as the gradient and
// normal there as well.
//

#include <math/DimRectangle.h>
#include <math/Vector.h>

class HeightField
{
public:
  HeightField();
  HeightField(const HeightField&);
  HeightField& operator=(const HeightField&);

  ~HeightField();

  void setHeightData(int dimX, int dimY, float* data);
  void setRange(DimRectangle dim);

  void getHeightData(int& dimX, int& dimY, float** data);
  DimRectangle getRange();

  void getHeightAndNormalAt(float x, float y, float& heightOut, Vector& normalOut);
  
  void setOutOfBoundsHeight(float val);

protected:
  float* heightTable;
  int dimX, dimY;
  float outOfBoundsHeight;

  DimRectangle range;
};

#endif //HEIGHTFIELD_H