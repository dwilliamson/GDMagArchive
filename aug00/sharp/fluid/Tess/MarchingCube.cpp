#include <fluid/Tess/MarchingCube.h>

MarchingCube::MarchingCube()
{
  xMin = 0;
  xRange = 0;
  yMin = 0;
  yRange = 0;
  zMin = 0;
  zRange = 0;

  xStep = 0;
  yStep = 0;
  zStep = 0;
}

// Called per-tessellation to set the axis-aligned box of space we're parameterizing over.
void MarchingCube::setBoundingBox(float xMinIn, float xRangeIn, float yMinIn, float yRangeIn, float zMinIn, float zRangeIn)
{
  xMin = xMinIn;
  xRange = xRangeIn;
  yMin = yMinIn;
  yRange = yRangeIn;
  zMin = zMinIn;
  zRange = zRangeIn;
}

// Called per-tessellation to specify the density of smallest-cells in the space.
void MarchingCube::setCellDensity(int cellsX, int cellsY, int cellsZ)
{
  xStep = 1.0f / cellsX;
  yStep = 1.0f / cellsY;
  zStep = 1.0f / cellsZ;
}

// An alternate way to tell it the same thing.
void MarchingCube::setCellWorldSize(float cellSizeX, float cellSizeY, float cellSizeZ)
{
  xStep = cellSizeX / xRange;
  yStep = cellSizeY / yRange;
  zStep = cellSizeZ / zRange;
}

// Called during tessellation to set the size of the active cubelet of space. The size argument is 
// needed because the cubelet can span multiple cells if we're recursing and this cube is higher
// up in the octree, i.e. not a leaf node.  We only need to know size once for all 3 axes, since it
// will always be specifying a cube (in parameter space.)
void MarchingCube::setCurrentCube(int xLoc, int yLoc, int zLoc, int size)
{
  float curMin[3];
  float curSize[3];

  curMin[0] = xMin + xRange * xLoc * xStep;
  curMin[1] = yMin + yRange * yLoc * yStep;
  curMin[2] = zMin + zRange * zLoc * zStep;

  curSize[0] = xRange * size * xStep;
  curSize[1] = yRange * size * yStep;
  curSize[2] = zRange * size * zStep;

  // See comment at top of header file.
  static float cubeLookup[8][3] = { {0.0f,0.0f,0.0f}, {1.0f,0.0f,0.0f}, {1.0f,1.0f,0.0f}, {0.0f,1.0f,0.0f},
                                    {0.0f,0.0f,1.0f}, {1.0f,0.0f,1.0f}, {1.0f,1.0f,1.0f}, {0.0f,1.0f,1.0f} };

  for (int i=0; i<8; i++)
  {
    curCorners[i][0] = curMin[0] + cubeLookup[i][0] * curSize[0];
    curCorners[i][1] = curMin[1] + cubeLookup[i][1] * curSize[1];
    curCorners[i][2] = curMin[2] + cubeLookup[i][2] * curSize[2];
  }

  curLoc.cell[0] = xLoc;
  curLoc.cell[1] = yLoc;
  curLoc.cell[2] = zLoc;
}

// Access methods.
const float* MarchingCube::getCorner(int index) const
{
  return curCorners[index];
}

const IntVector& MarchingCube::getLocation() const
{
  return curLoc;
}
