#include <fluid/Tess/RecursiveTessellator.h>
#include <assert.h>

// Debugging only... this object doesn't need to draw anything except for debug purposes.
#include <gl/glut.h>

const float POINT_STRENGTH = 1.0f;

#define SQUARE(a) ((a) * (a))

void RecursiveTessellator::childTessellate(const PotentialPoints& points, float cubeletSize, float surfaceThreshold)
{
  // Setup our parameter space.
  int numCubelets = determineBoundingBox(points, cubeletSize);

  // Just do a really basic brute-force marching cubes.
  int loc[3] = {0,0,0};
  getCube()->setCurrentCube(0,0,0,numCubelets);
  recursiveTessellate(points, loc, numCubelets, surfaceThreshold);

  if (getDebugLevel() > 0)
  {
    for (int x=0; x<points.size(); x++)
    {
      const Vector& curPoint = points.getPoint(x).loc;

      glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);

      glPointSize(5);
      glColor3f(1,0,0);

      glBegin(GL_POINTS);
      glVertex3f(curPoint.x, curPoint.y, curPoint.z);
      glEnd();

      glPopAttrib();
    }
  }
}

// Recursive routine called by tessellate to recurse down the octree.
void RecursiveTessellator::recursiveTessellate(const PotentialPoints& points, int loc[3], int size, float surfaceThreshold)
{
  // Debugging: draw ourself if we're supposed to.
  if ((getDebugLevel() == 2 && size == 1) || (getDebugLevel() == 3))
  {
#define DRAW_CORNER(a) glVertex3fv(getCube()->getCorner(a))

    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    
    bool recurse = shouldRecurseOnCube(points, surfaceThreshold);
    if (recurse)
    {
      glColor3f(0,1,0);
      glLineWidth(1);
    }
    else
    {
      glColor3f(0.2,0.2,0.5);
      glLineWidth(1);
    }

    glBegin(GL_LINES);
    {
      DRAW_CORNER(0); DRAW_CORNER(1);
      DRAW_CORNER(1); DRAW_CORNER(2);
      DRAW_CORNER(2); DRAW_CORNER(3);
      DRAW_CORNER(3); DRAW_CORNER(0);
      DRAW_CORNER(4); DRAW_CORNER(5);
      DRAW_CORNER(5); DRAW_CORNER(6);
      DRAW_CORNER(6); DRAW_CORNER(7);
      DRAW_CORNER(7); DRAW_CORNER(4);
      DRAW_CORNER(0); DRAW_CORNER(4);
      DRAW_CORNER(1); DRAW_CORNER(5);
      DRAW_CORNER(2); DRAW_CORNER(6);
      DRAW_CORNER(3); DRAW_CORNER(7);
    }
    glEnd();

    glPopAttrib();

#undef DRAW_CORNER
  }

  // Figure out if we should even be here.
  if (!shouldRecurseOnCube(points))
  {
    return;
  }

  // If we're at the bottom, tessellate and exit.
  if (size == 1)
  {
    // We don't use neighbors, so just pass something in and then discard it.
    int neighbors;
    getPolygonizer()->polygonize(getCube(), surfaceThreshold, neighbors);
    return;
  }

  int downLoc[3];

  // We travel the octree in the same pattern that MarchingCube verts are stored in... see MarchingCube.h
//  static int octreeLookup[8][3] = { {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0},
//                                    {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} };

  downLoc[0] = loc[0];
  downLoc[1] = loc[1];
  downLoc[2] = loc[2];
  
  if (downLoc[0] == 8 && downLoc[1] == 8 && downLoc[2] == 8 && size == 2)
  {
    int putBreakpointHere = 0;
  }

  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[0] = loc[0] + (size>>1);
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[1] = loc[1] + (size>>1);
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[0] = loc[0];
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[1] = loc[1];
  downLoc[2] = loc[2] + (size>>1);
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[0] = loc[0] + (size>>1);
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[1] = loc[1] + (size>>1);
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);

  downLoc[0] = loc[0];
  getCube()->setCurrentCube(downLoc[0], downLoc[1], downLoc[2], size>>1);
  recursiveTessellate(points, downLoc, size>>1);
}

// Used by tessellate to set the cube to the right parameter space.
int RecursiveTessellator::determineBoundingBox(const PotentialPoints& points, float cubeletSize)
{
  // WARNING: This function assumes that all points' strengths are 1.0f
  if (points.size() == 0)
  {
    return 1;
  }

  const float POINT_RADIUS = POINT_STRENGTH + 0.1f;

  float minX = points.getPoint(0).loc.x - POINT_RADIUS;
  float minY = points.getPoint(0).loc.y - POINT_RADIUS;
  float minZ = points.getPoint(0).loc.z - POINT_RADIUS;
  float maxX = points.getPoint(0).loc.x + POINT_RADIUS;
  float maxY = points.getPoint(0).loc.y + POINT_RADIUS;
  float maxZ = points.getPoint(0).loc.z + POINT_RADIUS;

  for (int i=1; i<points.size(); i++)
  {
    minX = Math::minOf(points.getPoint(i).loc.x - POINT_RADIUS, minX);
    minY = Math::minOf(points.getPoint(i).loc.y - POINT_RADIUS, minY);
    minZ = Math::minOf(points.getPoint(i).loc.z - POINT_RADIUS, minZ);
    maxX = Math::maxOf(points.getPoint(i).loc.x + POINT_RADIUS, maxX);
    maxY = Math::maxOf(points.getPoint(i).loc.y + POINT_RADIUS, maxY);
    maxZ = Math::maxOf(points.getPoint(i).loc.z + POINT_RADIUS, maxZ);
  }

  // Make sure the cubelets don't jitter every frame -- align them in world space.
  minX = floor(minX / cubeletSize) * cubeletSize;
  minY = floor(minY / cubeletSize) * cubeletSize;
  minZ = floor(minZ / cubeletSize) * cubeletSize;
  maxX = ceil(maxX / cubeletSize) * cubeletSize;
  maxY = ceil(maxY / cubeletSize) * cubeletSize;
  maxZ = ceil(maxZ / cubeletSize) * cubeletSize;

  // We also need the cubelet space to be a power-of-two cube for the octree, so round up.
  int numCubeletsX = (maxX - minX) / cubeletSize;
  int numCubeletsY = (maxY - minY) / cubeletSize;
  int numCubeletsZ = (maxZ - minZ) / cubeletSize;

  int largestNumCubelets = Math::maxOf(numCubeletsX, Math::maxOf(numCubeletsY, numCubeletsZ));
  
  // Round up to next power of two.
  int logNextHighestPowerOfTwo = ceil(log((float)largestNumCubelets) / log(2.0f));
  largestNumCubelets = 1 << logNextHighestPowerOfTwo;

//  float modX = fmod(fabs((maxX - minX) / cubeletSize) + 0.00001, 1.0f);
//  float modY = fmod(fabs((maxY - minY) / cubeletSize) + 0.00001, 1.0f);
//  float modZ = fmod(fabs((maxZ - minZ) / cubeletSize) + 0.00001, 1.0f);
//  if (modX > 0.0001 || modY > 0.0001 || modZ > 0.0001)
//  {
//    int putBreakpointHere = 0;
//  }

  // Round up everyone.
  maxX = minX + largestNumCubelets * cubeletSize;
  maxY = minY + largestNumCubelets * cubeletSize;
  maxZ = minZ + largestNumCubelets * cubeletSize;

//  if (fabs(((maxX - minX) / (float)largestNumCubelets) - cubeletSize) > 0.0001 ||
//      fabs(((maxY - minY) / (float)largestNumCubelets) - cubeletSize) > 0.0001 ||
//      fabs(((maxZ - minZ) / (float)largestNumCubelets) - cubeletSize) > 0.0001)
//  {
//    int putBreakpointHere = 0;
//  }

  getCube()->setBoundingBox(minX, maxX-minX, minY, maxY-minY, minZ, maxZ-minZ);
  getCube()->setCellDensity(largestNumCubelets, largestNumCubelets, largestNumCubelets);

  return largestNumCubelets;
}

// Used by tessellate to determine whether a given cube contains any surface info we care about.
bool RecursiveTessellator::shouldRecurseOnCube(const PotentialPoints& points, float surfaceThreshold)
{
  // The basic idea is to expand the box by the strength of a point (as that's the farthest
  // a point can ever contribute) and see if any are within that.
  
  // Min and max values, see MarchingCube.h
  float mins[3];
  float maxs[3];

  // WARNING: Note that we assume that all points' strengths are 1.0f.
  const float CULL_POINT_RADIUS = POINT_STRENGTH;

  mins[0] = getCube()->getCorner(0)[0];
  mins[1] = getCube()->getCorner(0)[1];
  mins[2] = getCube()->getCorner(0)[2];

  maxs[0] = getCube()->getCorner(6)[0];
  maxs[1] = getCube()->getCorner(6)[1];
  maxs[2] = getCube()->getCorner(6)[2];

  // Find only the points near enough to matter.
  float cubeCenX = (mins[0]+maxs[0])*0.5f;
  float maxDistX = (maxs[0]-mins[0])*0.5f + POINT_STRENGTH;
  int i = points.getFirstPotentialXContributor(cubeCenX, maxDistX);
  const PhysicalPoint* pts = points.getXSortedPoints();

  // See if any points are within this box.
  for (; fabs(pts[i].loc.x-cubeCenX) <= maxDistX && i<points.size(); i++)
  {
    const PhysicalPoint& pt = pts[i];
    float ptloc[3] = {pt.loc.x, pt.loc.y, pt.loc.z};

    // Basic idea -- if it's not inside a larger box, then the sphere is definitely not affecting this cube.
    // If it is, then it might be affecting the cube -- check to see if it's inside the cube (in which case
    // it is affecting it) or if it's not, only then check the more complex boundary (it's like a rounded
    // cube.)
    bool insideLargerCube = ptloc[0] > mins[0] - CULL_POINT_RADIUS && ptloc[0] < maxs[0] + CULL_POINT_RADIUS &&
                            ptloc[1] > mins[1] - CULL_POINT_RADIUS && ptloc[1] < maxs[1] + CULL_POINT_RADIUS &&
                            ptloc[2] > mins[2] - CULL_POINT_RADIUS && ptloc[2] < maxs[2] + CULL_POINT_RADIUS;

    if (insideLargerCube)
    {
      // If it's not obviously inside the cube, check the edge conditions.
      bool insideSmallerCube = ptloc[0] > mins[0] && ptloc[0] < maxs[0] &&
                               ptloc[1] > mins[1] && ptloc[1] < maxs[1] &&
                               ptloc[2] > mins[2] && ptloc[2] < maxs[2];

      bool isAffectingCube = insideSmallerCube;

      if (!insideSmallerCube)
      {
        // If the sphere is within range of the vertices, it's inside the box, say we need to keep going.
        float minDistSquared = Math::minOf((mins[0] - ptloc[0])*(mins[0] - ptloc[0]), (maxs[0] - ptloc[0])*(maxs[0] - ptloc[0])) + 
                               Math::minOf((mins[1] - ptloc[1])*(mins[1] - ptloc[1]), (maxs[1] - ptloc[1])*(maxs[1] - ptloc[1])) + 
                               Math::minOf((mins[2] - ptloc[2])*(mins[2] - ptloc[2]), (maxs[2] - ptloc[2])*(maxs[2] - ptloc[2]));
    
        
        // So this tests for the "rounded corners".
        if (minDistSquared < POINT_STRENGTH - surfaceThreshold)
        {
          isAffectingCube = true;
        }
        
        // Okay, if it wasn't inside the smaller cube or near enough to the corners, it could still close
        // enough to an edge.
        else
        {
          bool nearXSides = ptloc[0] > mins[0] - CULL_POINT_RADIUS && ptloc[0] < maxs[0] + CULL_POINT_RADIUS &&
                            ptloc[1] > mins[1] && ptloc[1] < maxs[1] &&
                            ptloc[2] > mins[2] && ptloc[2] < maxs[2];

          bool nearYSides = ptloc[0] > mins[0] && ptloc[0] < maxs[0] &&
                            ptloc[1] > mins[1] - CULL_POINT_RADIUS && ptloc[1] < maxs[1] + CULL_POINT_RADIUS &&
                            ptloc[2] > mins[2] && ptloc[2] < maxs[2];

          bool nearZSides = ptloc[0] > mins[0] && ptloc[0] < maxs[0] &&
                            ptloc[1] > mins[1] && ptloc[1] < maxs[1] &&
                            ptloc[2] > mins[2] - CULL_POINT_RADIUS && ptloc[2] < maxs[2] + CULL_POINT_RADIUS;

          if (nearXSides || nearYSides || nearZSides)
          {
            isAffectingCube = true;
          }
          
          // Alright, last god-damned test -- is it near any of the edges?  These are point/cylinder
          // intersection tests so they're nastier than the rest.
          else
          {
            // I'm just going to break down and do this in one if statement, so I get early exit.  Easier than
            // coding the early exit logic myself.  For the logic to these lookups, see the following
            // commented table of corner values, duplicated from MarchingCube.h (see that header for a more
            // in-depth description of what this means.)
            // 0: (0,0,0)
            // 1: (1,0,0)
            // 2: (1,1,0)
            // 3: (0,1,0)
            // 4: (0,0,1)
            // 5: (1,0,1)
            // 6: (1,1,1)
            // 7: (0,1,1)

            if (// Edge 0/1
                ((ptloc[0] > mins[0]) && (ptloc[0] < maxs[0]) && 
                (SQUARE(ptloc[1]-mins[1])+SQUARE(ptloc[2]-mins[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 1/2
                ((ptloc[1] > mins[1]) && (ptloc[1] < maxs[1]) && 
                (SQUARE(ptloc[0]-maxs[0])+SQUARE(ptloc[2]-mins[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 2/3
                ((ptloc[0] > mins[0]) && (ptloc[0] < maxs[0]) && 
                (SQUARE(ptloc[1]-maxs[1])+SQUARE(ptloc[2]-mins[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 3/0
                ((ptloc[1] > mins[1]) && (ptloc[1] < maxs[1]) && 
                (SQUARE(ptloc[0]-mins[0])+SQUARE(ptloc[2]-mins[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 4/5
                ((ptloc[0] > mins[0]) && (ptloc[0] < maxs[0]) && 
                (SQUARE(ptloc[1]-mins[1])+SQUARE(ptloc[2]-maxs[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 5/6
                ((ptloc[1] > mins[1]) && (ptloc[1] < maxs[1]) && 
                (SQUARE(ptloc[0]-maxs[0])+SQUARE(ptloc[2]-maxs[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 6/7
                ((ptloc[0] > mins[0]) && (ptloc[0] < maxs[0]) && 
                (SQUARE(ptloc[1]-maxs[1])+SQUARE(ptloc[2]-maxs[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 7/4
                ((ptloc[1] > mins[1]) && (ptloc[1] < maxs[1]) && 
                (SQUARE(ptloc[0]-mins[0])+SQUARE(ptloc[2]-maxs[2]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 0/4
                ((ptloc[2] > mins[2]) && (ptloc[2] < maxs[2]) && 
                (SQUARE(ptloc[0]-mins[0])+SQUARE(ptloc[1]-mins[1]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 1/5
                ((ptloc[2] > mins[2]) && (ptloc[2] < maxs[2]) && 
                (SQUARE(ptloc[0]-maxs[0])+SQUARE(ptloc[1]-mins[1]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 2/6
                ((ptloc[2] > mins[2]) && (ptloc[2] < maxs[2]) && 
                (SQUARE(ptloc[0]-maxs[0])+SQUARE(ptloc[1]-maxs[1]) < POINT_STRENGTH - surfaceThreshold)) ||
                // Edge 3/7
                ((ptloc[2] > mins[2]) && (ptloc[2] < maxs[2]) && 
                (SQUARE(ptloc[0]-mins[0])+SQUARE(ptloc[1]-maxs[1]) < POINT_STRENGTH - surfaceThreshold)))
            {
              // Phew!  At least one was true...
              isAffectingCube = true;
            }
          }
        }
      }

      // Is it within range of the cube?
      if (isAffectingCube)
      {
        // Careful, we don't want to recurse on every small cell that's entirely within a point cloud, so
        // check that before saying it's a go.  If the furthest corner is within a distance of 
        // sqrt(1-surfaceThreshold) then we're inside it.
        float maxDistSquared = Math::maxOf((mins[0] - ptloc[0])*(mins[0] - ptloc[0]), (maxs[0] - ptloc[0])*(maxs[0] - ptloc[0])) + 
                               Math::maxOf((mins[1] - ptloc[1])*(mins[1] - ptloc[1]), (maxs[1] - ptloc[1])*(maxs[1] - ptloc[1])) + 
                               Math::maxOf((mins[2] - ptloc[2])*(mins[2] - ptloc[2]), (maxs[2] - ptloc[2])*(maxs[2] - ptloc[2]));

        if (maxDistSquared > POINT_STRENGTH - surfaceThreshold)
        {
          return true;
        }
        else
        {
          int putBreakpointHere = 0;
        }
      }
    }
  }

  return false;
}
