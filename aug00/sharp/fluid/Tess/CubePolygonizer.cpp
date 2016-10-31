#include <fluid/Tess/CubePolygonizer.h>
#include <math/Math.h>
#include <assert.h>

#include <harness/GlobalCamera.h>
#include <gl/glut.h>

// Switch between these to use either marching cubes or marching tetrahedrons (but not both. :-P )
//#define MARCHING_TETRAHEDRONS
#define MARCHING_CUBES

#ifdef MARCHING_TETRAHEDRONS
#include <fluid/Tess/MarchingTetTable.h>
#else
#include <fluid/Tess/MarchingCubeTable.h>
#endif

CubePolygonizer::CubePolygonizer()
{
  sampler = 0;
  mesh = 0;
  currCube = 0;
}

// Used to hand this guy pointers to objects he needs to know about.
void CubePolygonizer::setSurfaceSampler(const SurfaceSampler* samplerIn)
{
  sampler = samplerIn;
}

void CubePolygonizer::setImplicitMesh(ImplicitMesh* meshIn)
{
  mesh = meshIn;
}

// Called to clear our cache (i.e. to tell us that we're starting a surface over.)
void CubePolygonizer::beginTessellation()
{
  cornerValueCache.clear();
  edgeVertexCache.clear();
}

// Polygon addition routine: this is the function that actually turns a cubelet into polygons.
// This code is aesthetically based off of Paul Bourke's tutorial code at:
//
//  http://www.swin.edu.au/astronomy/pbourke/modelling/polygonise/
//
// It's not like I copied it, but it is laid out similarly (especially the vertex interpolation part.)
CubePolygonizer::CubeletLocation CubePolygonizer::polygonize(const MarchingCube* cube, float surfaceThreshold, int& neighborsOut, bool addPolys)
{
  assert(mesh != 0);
  assert(sampler != 0);
  currCube = cube;
 
  // First, get the cubelet corner values.
  float cubeValues[8];
  evaluateCubelet(cubeValues);

  // Now aggregate the bitmask used to index into the lookup table.  One bit for each vertex, indicating
  // whether it's inside or outside the surface.
  int cornerBits = 0;
  for (int x=0; x<8; x++)
  {
    // If this one is outside the surface, set its bit to 1.  Otherwise set the bit to 0.
    cornerBits |= (cubeValues[x] < surfaceThreshold) ? (1<<x) : 0;
  }
  
  // Fast reject -- it's either wholly outside or inside the surface.  Determine which...
  if (edgesNeeded[cornerBits] == 0)
  {
    // If one was outside, they all were...
    if (cubeValues[0] < surfaceThreshold)
    {
      return OUTSIDE_SURFACE;
    }

    // ... and vice versa.
    else
    {
      return INSIDE_SURFACE;
    }
  }
  
  // Figure out which vertices we'll need, generate them.  Note that what we get back is an index,
  // as the interp function goes ahead and adds them to the mesh to hide from us whether it's 
  // reusing existing verts.  Also, the edges we need vertices along indicates which neighboring cells
  // will contain polygons, so build the neighbors flag during this stage.
  neighborsOut = 0;
  int vertexIndices[19];
  if (edgesNeeded[cornerBits] & (1<<0))
  {
    vertexInterp(0,1,cubeValues,surfaceThreshold, vertexIndices[0]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Y;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<1))
  {
    vertexInterp(1,2,cubeValues,surfaceThreshold, vertexIndices[1]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_X;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<2))
  {
    vertexInterp(2,3,cubeValues,surfaceThreshold, vertexIndices[2]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_Y;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<3))
  {
    vertexInterp(3,0,cubeValues,surfaceThreshold, vertexIndices[3]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_X;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<4))
  {
    vertexInterp(4,5,cubeValues,surfaceThreshold, vertexIndices[4]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Y;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<5))
  {              
    vertexInterp(5,6,cubeValues,surfaceThreshold, vertexIndices[5]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_X;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<6))
  {
    vertexInterp(6,7,cubeValues,surfaceThreshold, vertexIndices[6]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_Y;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<7))
  {
    vertexInterp(7,4,cubeValues,surfaceThreshold, vertexIndices[7]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_X;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<8))
  {
    vertexInterp(0,4,cubeValues,surfaceThreshold, vertexIndices[8]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_X;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Y;
  }
  if (edgesNeeded[cornerBits] & (1<<9))
  {
    vertexInterp(1,5,cubeValues,surfaceThreshold, vertexIndices[9]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_X;
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Y;
  }
  if (edgesNeeded[cornerBits] & (1<<10))
  {
    vertexInterp(2,6,cubeValues,surfaceThreshold, vertexIndices[10]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_X;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Y;
  }
  if (edgesNeeded[cornerBits] & (1<<11))
  {
    vertexInterp(3,7,cubeValues,surfaceThreshold, vertexIndices[11]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_X;
    neighborsOut = neighborsOut | NEIGHBOR_POS_Y;
  }

// Much as I hate ifdefs, I want this to be easy to flip between, but it's so time critical (this function
// gets called about a bazillion times during tessellation) that I don't want it built it if we're using
// marching cubes.  This is the code that calculates the cross-edges that you need for the tetrahedrons
// (since they split the cube up and introduce new edges in doing so) but not for the marching cubes approach.
#ifdef MARCHING_TETRAHEDRONS

  // Cross-edge ones added for the marching tetrahedron code.
  if (edgesNeeded[cornerBits] & (1<<12))
  {
    vertexInterp(1,4,cubeValues,surfaceThreshold, vertexIndices[12]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Y;
  }
  if (edgesNeeded[cornerBits] & (1<<13))
  {
    vertexInterp(2,5,cubeValues,surfaceThreshold, vertexIndices[13]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_X;
  }
  if (edgesNeeded[cornerBits] & (1<<14))
  {
    vertexInterp(2,7,cubeValues,surfaceThreshold, vertexIndices[14]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_Y;
  }
  if (edgesNeeded[cornerBits] & (1<<15))
  {
    vertexInterp(3,4,cubeValues,surfaceThreshold, vertexIndices[15]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_X;
  }
  if (edgesNeeded[cornerBits] & (1<<16))
  {
    vertexInterp(1,3,cubeValues,surfaceThreshold, vertexIndices[16]);
    neighborsOut = neighborsOut | NEIGHBOR_NEG_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<17))
  {
    vertexInterp(5,7,cubeValues,surfaceThreshold, vertexIndices[17]);
    neighborsOut = neighborsOut | NEIGHBOR_POS_Z;
  }
  if (edgesNeeded[cornerBits] & (1<<18))
  {
    vertexInterp(3,5,cubeValues,surfaceThreshold, vertexIndices[18]);
  }
#endif // MARCHING_TETRAHEDRONS
  
  // Loop over the table of indices and add triangles until we hit the end.  Since we don't necessarily 
  // know the length of each array (depending on whether we're using the marching cubes table, or the
  // marching tetrahedrons table), we pad a -1 at the end of every array, hence the termination condition.
  if (addPolys)
  {
    for (int i=0; newTriIndices[cornerBits][i] != -1; i += 3) 
    {
      mesh->addTri(vertexIndices[newTriIndices[cornerBits][i+0]], 
                   vertexIndices[newTriIndices[cornerBits][i+1]], 
                   vertexIndices[newTriIndices[cornerBits][i+2]]);
    }
  }

  // Say we found polygons.
  return ON_SURFACE;
}

// Cubelet-corner sampling function.
void CubePolygonizer::evaluateCubelet(float cubeValues[8])
{
  assert(sampler != 0);
  assert(currCube != 0);

  // Loop unrolled to avoid bizarre table lookup weirdness... it's more readable this way, honest!
  // Lookup table from MarchingCube.h:
  // 0: (0,0,0)
  // 1: (1,0,0)
  // 2: (1,1,0)
  // 3: (0,1,0)
  // 4: (0,0,1)
  // 5: (1,0,1)
  // 6: (1,1,1)
  // 7: (0,1,1)

  IntVector cornerVec(currCube->getLocation().cell);
  float* valueFind;
  Vector v0;

  // Corner 0
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(0)[0], currCube->getCorner(0)[1], currCube->getCorner(0)[2]);
    cubeValues[0] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[0]);
  }
  else
  {
    cubeValues[0] = *valueFind;
  }

  // Corner 1
  cornerVec.cell[0]++;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(1)[0], currCube->getCorner(1)[1], currCube->getCorner(1)[2]);
    cubeValues[1] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[1]);
  }
  else
  {
    cubeValues[1] = *valueFind;
  }

  // Corner 2
  cornerVec.cell[1]++;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(2)[0], currCube->getCorner(2)[1], currCube->getCorner(2)[2]);
    cubeValues[2] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[2]);
  }
  else
  {
    cubeValues[2] = *valueFind;
  }

  // Corner 3
  cornerVec.cell[0]--;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(3)[0], currCube->getCorner(3)[1], currCube->getCorner(3)[2]);
    cubeValues[3] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[3]);
  }
  else
  {
    cubeValues[3] = *valueFind;
  }

  // Corner 4
  cornerVec.cell[1]--;
  cornerVec.cell[2]++;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(4)[0], currCube->getCorner(4)[1], currCube->getCorner(4)[2]);
    cubeValues[4] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[4]);
  }
  else
  {
    cubeValues[4] = *valueFind;
  }

  // Corner 5
  cornerVec.cell[0]++;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(5)[0], currCube->getCorner(5)[1], currCube->getCorner(5)[2]);
    cubeValues[5] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[5]);
  }
  else
  {
    cubeValues[5] = *valueFind;
  }

  // Corner 6
  cornerVec.cell[1]++;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(6)[0], currCube->getCorner(6)[1], currCube->getCorner(6)[2]);
    cubeValues[6] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[6]);
  }
  else
  {
    cubeValues[6] = *valueFind;
  }

  // Corner 7
  cornerVec.cell[0]--;
  if ((valueFind = cornerValueCache.find(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2])) == 0)
  {
    v0 = Vector(currCube->getCorner(7)[0], currCube->getCorner(7)[1], currCube->getCorner(7)[2]);
    cubeValues[7] = sampler->getValueAt(v0);
    cornerValueCache.insert(cornerVec.cell[0], cornerVec.cell[1], cornerVec.cell[2], cubeValues[7]);
  }
  else
  {
    cubeValues[7] = *valueFind;
  }
}

// Used to interpolate an edge of the current cubelet.
void CubePolygonizer::vertexInterp(int corner0, int corner1, float cubeValues[8], float surfaceThreshold, int& indexOut)
{
  // Just be safe...
  assert(corner0 >= 0 && corner0 < 8 && corner1 >= 0 && corner1 < 8);

  // Lookup table from MarchingCube.h
  int cubeCornerOffsets[8][3] = {{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0}, {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}};

  // The way we uniquely identify edges is a little tricky.  The idea is that every edge can
  // be uniquely identified by its midpoint.  Therefore, if we can store an IntVector that
  // identifies the corners, we just need to be able to express an offset by 0.5 cells.  So take
  // either of the corners, add or subtract 0.5 from whichever of the values (x,y, or z) we need
  // to to get to the edge's midpoint, and then to keep it an int, shift everything left by one bit.
  // Voila.  Note that this will yield the same result regardless of which of the two corners we
  // start with.

  // Get the base location of the cube.
  IntVector edgeLoc(currCube->getLocation().cell);

  // Translate to the first corner.
  edgeLoc.cell[0] += cubeCornerOffsets[corner0][0];
  edgeLoc.cell[1] += cubeCornerOffsets[corner0][1];
  edgeLoc.cell[2] += cubeCornerOffsets[corner0][2];

  // Now shift so we can fit the 0.5 in...
  edgeLoc.cell[0] <<= 1;
  edgeLoc.cell[1] <<= 1;
  edgeLoc.cell[2] <<= 1;

  // Now add or subtract 0.5 to get to the edge midpoint.
  edgeLoc.cell[0] += (cubeCornerOffsets[corner1][0] - cubeCornerOffsets[corner0][0]);
  edgeLoc.cell[1] += (cubeCornerOffsets[corner1][1] - cubeCornerOffsets[corner0][1]);
  edgeLoc.cell[2] += (cubeCornerOffsets[corner1][2] - cubeCornerOffsets[corner0][2]);

  // Voila: unique identifier.  Now look it up to see if we've already done this work.
  int* cacheIndex = edgeVertexCache.find(edgeLoc.cell[0], edgeLoc.cell[1], edgeLoc.cell[2]);

  // If we haven't found it yet, recalc.
  if (cacheIndex == 0)
  {
    Vector out;

    float ratio0 = (cubeValues[corner1] - surfaceThreshold) / (cubeValues[corner1] - cubeValues[corner0]);
    float ratio1 = 1.0f - ratio0;

    out.x = currCube->getCorner(corner0)[0] * ratio0 + currCube->getCorner(corner1)[0] * ratio1;
    out.y = currCube->getCorner(corner0)[1] * ratio0 + currCube->getCorner(corner1)[1] * ratio1;
    out.z = currCube->getCorner(corner0)[2] * ratio0 + currCube->getCorner(corner1)[2] * ratio1;

    int newV0 = mesh->addVertex(out);
    
    edgeVertexCache.insert(edgeLoc.cell[0], edgeLoc.cell[1], edgeLoc.cell[2], newV0);
    indexOut = newV0;
  }

  // If we've already found this, reuse it!
  else
  {
    indexOut = *cacheIndex;
  }
}

// Optionally called at the end of the frame to draw debug info in an ortho projection.
void CubePolygonizer::drawDebugStatistics()
{
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);

  cornerValueCache.drawDebugGraph(w-18-512-4,90,w-18,122);
  edgeVertexCache.drawDebugGraph(w-18-512-4,162,w-18,194);
}
