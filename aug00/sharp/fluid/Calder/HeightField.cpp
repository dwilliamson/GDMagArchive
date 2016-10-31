#include <fluid/calder/HeightField.h>
#include <memory.h>

HeightField::HeightField()
{
  dimX = 0;
  dimY = 0;
  heightTable = 0;
}

HeightField::HeightField(const HeightField& hf)
{
  dimX = hf.dimX;
  dimY = hf.dimY;
  heightTable = new float[dimX*dimY];
  memcpy(heightTable, hf.heightTable, dimX*dimY*sizeof(float));
  outOfBoundsHeight = hf.outOfBoundsHeight;
  range = hf.range;
}

HeightField& HeightField::operator=(const HeightField& hf)
{
  dimX = hf.dimX;
  dimY = hf.dimY;
  delete[] heightTable;
  heightTable = new float[dimX*dimY];
  memcpy(heightTable, hf.heightTable, dimX*dimY*sizeof(float));
  outOfBoundsHeight = hf.outOfBoundsHeight;
  range = hf.range;

  return *this;
}

HeightField::~HeightField()
{
  dimX = -1;
  dimY = -1;
  delete[] heightTable;
  heightTable = 0;
}

void HeightField::setHeightData(int dimX, int dimY, float* data)
{
  this->dimX = dimX;
  this->dimY = dimY;
  delete[] heightTable;
  heightTable = new float[dimX*dimY];
  memcpy(heightTable, data, dimX*dimY*sizeof(float));
}

void HeightField::setRange(DimRectangle dim)
{
  range = dim;
}

void HeightField::getHeightData(int& dimX, int& dimY, float** data)
{
  dimX = this->dimX;
  dimY = this->dimY;
  *data = heightTable;
}

DimRectangle HeightField::getRange()
{
  return range;
}

void HeightField::getHeightAndNormalAt(float x, float y, float& heightOut, Vector& normalOut)
{
  x -= range.minX;
  y -= range.minY;

  x /= range.sizeX;
  y /= range.sizeY;

  if (x >= 1 || x < 0 || y >= 1 || y < 0)
  {
    // Out of range... just treat the heightfield as flat wherever it's not defined.
    heightOut = outOfBoundsHeight;

    // Always point towards the height field, to act as a wall.

    // Off a y edge?
    if (x >= 0 && x <= 1)
    {
      if (y <= 0)
      {
        normalOut = Vector(0,-y,0);
      }
      else
      {
        normalOut = Vector(0,1-y,0);
      }
    }

    // Off an x edge?
    else if (y >= 0 && y <= 1)
    {
      if (x <= 0)
      {
        normalOut = Vector(-x,0,0);
      }
      else
      {
        normalOut = Vector(1-x,0,0);
      }
    }

    // Okay, off a corner somewhere.
    else
    {
      normalOut = Vector(x > 0 ? (1-x) : (-x), y > 0 ? (1-y) : (-y), 0);
    }

    normalOut.fastNormalize();
    
    return;
  }

  x *= dimX-1;
  y *= dimY-1;

  // Now the fractional part of x and y are our u,v for the cell indicated by x and y's integral part.
  int cellX = (int)floor(x);
  int cellY = (int)floor(y);
  
  float u = x - cellX;
  float v = y - cellY;

  // Get the four points we're interpolating between.
  Vector p00((cellX+0) * range.sizeX / dimX + range.minX, (cellY+0) * range.sizeY / dimY + range.minY, heightTable[(cellX+0) + dimX*(cellY+0)]);
  Vector p10((cellX+1) * range.sizeX / dimX + range.minX, (cellY+0) * range.sizeY / dimY + range.minY, heightTable[(cellX+1) + dimX*(cellY+0)]);
  Vector p01((cellX+0) * range.sizeX / dimX + range.minX, (cellY+1) * range.sizeY / dimY + range.minY, heightTable[(cellX+0) + dimX*(cellY+1)]);
  Vector p11((cellX+1) * range.sizeX / dimX + range.minX, (cellY+1) * range.sizeY / dimY + range.minY, heightTable[(cellX+1) + dimX*(cellY+1)]);

  // Now bilerp (ala a bilinear bezier patch) between the four corner points.
  heightOut  = p11.z * u * v;
  heightOut += p01.z * (1-u) * v;
  heightOut += p10.z * u * (1-v);
  heightOut += p00.z * (1-u) * (1-v);

  // Now compute the tangents in either direction -- we just take the u or v derivative of the blending
  // factors above (so the u tangent's blending coefficient for the first point is d/du (u*v) == v
  Vector uTan = (p11 * v) + (p01 * -v) + (p10 * (1-v)) + (p00 * (v-1));
  Vector vTan = (p11 * u) + (p01 * (1-u)) + (p10 * -u) + (p00 * (u-1));
  uTan.cross(vTan, normalOut);
  normalOut.fastNormalize();
}

void HeightField::setOutOfBoundsHeight(float val)
{
  outOfBoundsHeight = val;
}