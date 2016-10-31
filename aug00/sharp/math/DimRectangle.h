#ifndef DIMRECTANGLE_H
#define DIMRECTANGLE_H

// This is a little helper class to store the dimensions of a rectangle.

class DimRectangle
{
public:
  DimRectangle(float minX = 0, float minY = 0, float maxX = 0, float maxY = 0)
  {
    this->minX = minX;
    this->minY = minY;
    this->sizeX = maxX-minX;
    this->sizeY = maxY-minY;
  }

  float minX, minY, sizeX, sizeY;
protected:
};

#endif //DIMRECTANGLE_H