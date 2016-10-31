#include <fluid/Tess/ContinuousTessellator.h>

// Needed for debug drawing.
#include <gl/glut.h>

const float POINT_STRENGTH = 1.0f;

void ContinuousTessellator::childTessellate(const PotentialPoints& points, float cubeletSize, float surfaceThreshold)
{
  // Loop variables.
  int i;

  // First step is to clear any per-frame cached info.
  cellsConsidered.clear();
  curSurfaceId = 1;

  // Initialize this to the proper size.
  pointsConsidered.resize(points.size());

  // Clear that vector.
  for (i=0; i<points.size(); i++)
  {
    pointsConsidered[i] = false;
  }

  // Now set up the parameter space.
  Vector minCorner = determineBoundingBox(points, cubeletSize);

  // Now get going... 
  for (i=0; i<points.size(); i++)
  {
    if (!pointsConsidered[i])
    {
      pointsConsidered[i] = true;
      
      // Finally!  Make some polygons.
      walkSurfaceFromPoint(points, i, minCorner, cubeletSize, surfaceThreshold);
    }
  }

  if (getDebugLevel() > 0)
  {
    for (int x=0; x<points.size(); x++)
    {
      const Vector& curPoint = points.getPoint(x).loc;

      glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
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

// This is the function that starts the polygonization from a potential point.
void ContinuousTessellator::walkSurfaceFromPoint(const PotentialPoints& points, int startingPoint, const Vector& minCorner, float cubeletSize, float surfaceThreshold)
{
  // Grab this just to cut down on typing! :)
  CubePolygonizer* poly = getPolygonizer();

  // Need to find the cubelet that contains the point's location.
  const Vector& point = points.getPoint(startingPoint).loc;
  unsigned short cubeLoc[3];

  // Find where this cubelet is in relation to the overall parameter space.
  assert(point.x > minCorner.x);
  assert(point.y > minCorner.y);
  assert(point.z > minCorner.z);
  cubeLoc[0] = floor((point.x-minCorner.x) / cubeletSize);
  cubeLoc[1] = floor((point.y-minCorner.y) / cubeletSize);
  cubeLoc[2] = floor((point.z-minCorner.z) / cubeletSize);
  assert((unsigned short)cubeLoc[0] == cubeLoc[0]);
  assert((unsigned short)cubeLoc[1] == cubeLoc[1]);
  assert((unsigned short)cubeLoc[2] == cubeLoc[2]);

  getCube()->setCurrentCube(cubeLoc[0], cubeLoc[1], cubeLoc[2], 1);

  // Now move it out in some arbitrary direction until we hit the surface.
  int neighbors;
  CubePolygonizer::CubeletLocation cubeType;
  while ((cubeType = poly->polygonize(getCube(), surfaceThreshold, neighbors, false)) == CubePolygonizer::INSIDE_SURFACE)
  {
    cubeLoc[2]++;
    getCube()->setCurrentCube(cubeLoc[0], cubeLoc[1], cubeLoc[2], 1);
  }
  
  // Okay, if we're on the surface we're good, if not something's broken (very very coarse
  // tessellation, perhaps?) so give up (it can happen at low tessellations so don't assert,
  // it'll just look ugly.)
  if (cubeType == CubePolygonizer::OUTSIDE_SURFACE)
  {
    return;
  }

  // Check to find our 'surface number' (an identifier for which surface we're under, used to tell if
  // two potential points are within the same surface volume.)  If we already have one this surface
  // has already been tessellated.
  int* cellNum = cellsConsidered.find(cubeLoc[0], cubeLoc[1], cubeLoc[2]);
  if (cellNum != 0)
  {
    points.getPoint(startingPoint).surfaceId = *cellNum;
    return;
  }

  // Okay, we haven't been tessellated yet... assign a new id.
  points.getPoint(startingPoint).surfaceId = curSurfaceId;

  // Okay, whee, we've got a cubelet on the surface and we're ready to go.
  // It's implied that they should use curSurfaceId as the surface id.
  recurseOverSurface(cubeLoc, surfaceThreshold);

  // We used one of these; increment it.
  curSurfaceId++;
}

// This is the function that recurses over the surface, polygonizing it cube by cube.
void ContinuousTessellator::recurseOverSurface(unsigned short cubeLoc[3], float surfaceThreshold)
{
  // I'm a little worried about stack space, so I don't allocate any more room on the stack
  // in here than I need to.
  static IntVector loc;

  // Make sure we should be here at all -- if we've already been considered, stop.
  loc.setLoc(cubeLoc);

  if (cellsConsidered.doesExist(cubeLoc[0], cubeLoc[1], cubeLoc[2]))
  {
    return;
  }

  // Tessellate this cube, mark it considered.
  getCube()->setCurrentCube(cubeLoc[0], cubeLoc[1], cubeLoc[2], 1);

//  // Hack -- see how much speed we can get out of culling against the ground plane.
//  if (getCube()->getCorner(6)[2] < 0)
//  {
//    return;
//  }

  int neighbors;
  
  CubePolygonizer::CubeletLocation cubeType = getPolygonizer()->polygonize(getCube(), surfaceThreshold, neighbors, true);
  assert(cubeType == CubePolygonizer::ON_SURFACE);

  cellsConsidered.insert(cubeLoc[0], cubeLoc[1], cubeLoc[2], curSurfaceId);

  // Draw it if we're supposed to.
  if (getDebugLevel() > 1)
  {
#define DRAW_CORNER(a) glVertex3fv(getCube()->getCorner(a))

    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    
    glColor3f(0,1,0);

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

  // Recurse on neighbors that haven't yet been considered.  Note that because cubeLoc
  // passed by pointer and not by value, we have to make sure to return it with the same
  // values that it came in with.
  if (neighbors & CubePolygonizer::NEIGHBOR_NEG_X)
  {
    cubeLoc[0]--; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[0]++;
  }
  if (neighbors & CubePolygonizer::NEIGHBOR_POS_X)
  {
    cubeLoc[0]++; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[0]--;
  }
  if (neighbors & CubePolygonizer::NEIGHBOR_NEG_Y)
  {
    cubeLoc[1]--; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[1]++;
  }
  if (neighbors & CubePolygonizer::NEIGHBOR_POS_Y)
  {
    cubeLoc[1]++; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[1]--;
  }
  if (neighbors & CubePolygonizer::NEIGHBOR_NEG_Z)
  {
    cubeLoc[2]--; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[2]++;
  }
  if (neighbors & CubePolygonizer::NEIGHBOR_POS_Z)
  {
    cubeLoc[2]++; recurseOverSurface(cubeLoc, surfaceThreshold); cubeLoc[2]--;
  }
}

// Used by tessellate to set the cube to the right parameter space.
Vector ContinuousTessellator::determineBoundingBox(const PotentialPoints& points, float cubeletSize)
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

  getCube()->setBoundingBox(minX, maxX-minX, minY, maxY-minY, minZ, maxZ-minZ);
  getCube()->setCellWorldSize(cubeletSize, cubeletSize, cubeletSize);

  return Vector(minX, minY, minZ);
}

// Optionally called at the end of the frame to draw debug info in an ortho projection.
void ContinuousTessellator::drawDebugStatistics()
{
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);

  cellsConsidered.drawDebugGraph(w-18-512-4,18,w-18,50);
  getPolygonizer()->drawDebugStatistics();
}
