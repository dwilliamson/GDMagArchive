// This is a shell for applications to use in implementing their main loop; it just
// does some basic GL stuff, turns on a light, etc.
//
// BHS 11/18/99
#include <harness/main.h>
#include <harness/CameraMover.h>
#include <harness/FreeCamera.h>
#include <harness/PlatformSpecific.h>
#include <harness/GlobalCamera.h>
#include <harness/OpenGL.h>
#include <harness/PlatformSpecific.h>
#include <harness/TextureManager.h>

#include <math/Vector.h>

#include <iostream>
#include <gl/glut.h>
#include <assert.h>

#include <vector>
#include <sstream>

unsigned char cubeConfig = 0;
std::vector<std::vector<int> > triIndices;

int numTextures[12];

#pragma warning (disable: 4786)

GLenum polyMode = GL_FILL;

void outputTables();

const char* appWindowName()
{
  return "Unnamed Player";
}

void appCleanup()
{
}

void appInit(int argc, char** argv)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  glDisable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  
  float ones[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float zeros[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, ones);
  glLightfv(GL_LIGHT0, GL_SPECULAR, zeros);

  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);

  glColor3f(0.0f,0.2f,1.0f);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zeros);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

  // Setup our mover so we can have mouse-driven camera movement.
  CameraMover::initCallbacks();
  CameraMover::registerPersona(new FreeCamera());

  GlobalCamera::Instance()->setFovY(40);
  GlobalCamera::Instance()->getPosition().x = 5;
  GlobalCamera::Instance()->getPosition().y = 25;
  GlobalCamera::Instance()->getPosition().z = 5;
  GlobalCamera::Instance()->getPosition().setOrientation(Vector(-5,-25,-5));
  GlobalCamera::Instance()->getPosition().setUpVector(Vector(0,0,1));

  // Also, make the mouse invisible since we do mlook-style movement now!
  glutSetCursor(GLUT_CURSOR_NONE);

  // Rev up the extensions.
  OpenGL::initExtensions();

  triIndices.resize(128, std::vector<int>());

  for (int x=0; x<12; x++)
  {
    std::stringstream filenameStream;
    filenameStream << x << ".raw";
    numTextures[x] = TextureManager::instance()->addTexture(filenameStream.str(), 32,32,false,false);
  }

  glDisable(GL_TEXTURE_2D);
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
  case '0':
  case '9':
  case '8':
  case '7':
  case '6':
  case '5':
  case '4':
  case '3':
  case '2':
  case '1':
    // Add the appropriate edge.
    triIndices[cubeConfig].push_back(keyHit-'0');
    break;

  // We map edges 11 and 12 with the - and = keys.
  case '-':
    triIndices[cubeConfig].push_back(10);
    break;

  case '=':
    triIndices[cubeConfig].push_back(11);
    break;

  case 'p':
  case 'P':
    polyMode = (polyMode == GL_FILL) ? GL_LINE : GL_FILL;
    if (polyMode == GL_FILL)
    {
      glEnable(GL_CULL_FACE);
    }
    else
    {
      glDisable(GL_CULL_FACE);
    }
    break;

  // Clear this case.
  case 'c':
  case 'C':
    triIndices[cubeConfig].clear();
    break;

  // Backup one vertex.
  case 'b':
  case 'B':
    triIndices[cubeConfig].pop_back();
    break;

  // Backup as much as one tri...
  case 't':
  case 'T':
    if (triIndices[cubeConfig].size() < 3) triIndices[cubeConfig].clear();
    else triIndices[cubeConfig].resize(triIndices[cubeConfig].size() - ((triIndices[cubeConfig].size()-1) % 3) - 1);
    break;

  // Advance to the next config or backup to the previous one.
  case ']':
    if (cubeConfig < 127) cubeConfig++;
    break;
  case '[':
    if (cubeConfig > 0) cubeConfig--;
    break;

  case 'w':
  case 'W':
    outputTables();
    break;
  }
}

void appDraw()
{
  Vector corners[8] = {Vector(0,0,0), Vector(1,0,0), Vector(1,1,0), Vector(0,1,0), 
                       Vector(0,0,1), Vector(1,0,1), Vector(1,1,1), Vector(0,1,1)};

  Vector edges[12] = {(corners[0]+corners[1])*0.5f, (corners[1]+corners[2])*0.5f, 
                      (corners[2]+corners[3])*0.5f, (corners[3]+corners[0])*0.5f, 
                      (corners[4]+corners[5])*0.5f, (corners[5]+corners[6])*0.5f, 
                      (corners[6]+corners[7])*0.5f, (corners[7]+corners[4])*0.5f, 
                      (corners[0]+corners[4])*0.5f, (corners[1]+corners[5])*0.5f, 
                      (corners[2]+corners[6])*0.5f, (corners[3]+corners[7])*0.5f};

  int edgeEnds[24] = {0,1,  1,2,  2,3,  3,0, 
                      4,5,  5,6,  6,7,  7,4, 
                      0,4,  1,5,  2,6,  3,7};

  glClear(GL_DEPTH_BUFFER_BIT);

  // Update any mouse movement.
  CameraMover::update();

  // Re-init the camera.
  GlobalCamera::Instance()->makeCurrent();

  // Draw our surface.
  glPolygonMode(GL_FRONT_AND_BACK, polyMode);

  glColor3f(0.4f,0.4f,0.4f);
  glBegin(GL_LINES);

#define VERTEX(a) glVertex3f(corners[a].x, corners[a].y, corners[a].z)
  VERTEX(0); VERTEX(1);
  VERTEX(1); VERTEX(2);
  VERTEX(2); VERTEX(3);
  VERTEX(3); VERTEX(0);
  
  VERTEX(4); VERTEX(5);
  VERTEX(5); VERTEX(6);
  VERTEX(6); VERTEX(7);
  VERTEX(7); VERTEX(4);

  VERTEX(0); VERTEX(4);
  VERTEX(1); VERTEX(5);
  VERTEX(2); VERTEX(6);
  VERTEX(3); VERTEX(7);

//  VERTEX(1); VERTEX(4);
//  VERTEX(2); VERTEX(5);
//  VERTEX(2); VERTEX(7);
//  VERTEX(3); VERTEX(4);
//
//  VERTEX(1); VERTEX(3);
//  VERTEX(5); VERTEX(7);
//
//  VERTEX(3); VERTEX(5);

#undef VERTEX

  glEnd();

  // Now draw the vertices in color (green for 'on', red for 'off')
  glPointSize(5);
  glBegin(GL_POINTS);

  for (int x=0; x<8; x++)
  {
    if (cubeConfig & (1<<x))
    {
      glColor3f(0,1,0);
    }
    else
    {
      glColor3f(1,0,0);
    }
    glVertex3f(corners[x].x, corners[x].y, corners[x].z);
  }

  glEnd();

  // Now draw the current polygonization.  Full tris for every 3 verts, with the remaining ones
  // as points.
  glColor3f(0,0,1);
  glBegin(GL_TRIANGLES);
  for (x=0; x+2<triIndices[cubeConfig].size(); x+=3)
  {
    glVertex3f(edges[triIndices[cubeConfig][x+0]].x, edges[triIndices[cubeConfig][x+0]].y, edges[triIndices[cubeConfig][x+0]].z);
    glVertex3f(edges[triIndices[cubeConfig][x+1]].x, edges[triIndices[cubeConfig][x+1]].y, edges[triIndices[cubeConfig][x+1]].z);
    glVertex3f(edges[triIndices[cubeConfig][x+2]].x, edges[triIndices[cubeConfig][x+2]].y, edges[triIndices[cubeConfig][x+2]].z);
  }
  glEnd();

  glBegin(GL_POINTS);
  for (; x<triIndices[cubeConfig].size(); x++)
  {
    glVertex3f(edges[triIndices[cubeConfig][x]].x, edges[triIndices[cubeConfig][x]].y, edges[triIndices[cubeConfig][x]].z);
  }
  glEnd();

  // Draw each of the numbers.
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glColor3f(1,1,1);
  for (x=0; x<12; x++)
  {
    // Don't draw it if it's unnecessary for this triangulation case.
    if (((cubeConfig & (1<<edgeEnds[2*x+0])) ? 1:0) != ((cubeConfig & (1<<edgeEnds[2*x+1])) ? 1:0))
    {
      // Find the location.
      Vector numLoc = edges[x];
      numLoc -= Vector(0.5f,0.5f,0.5f);
      numLoc *= 1.2f;
      numLoc += Vector(0.5f,0.5f,0.5f);

      // Now find the offsets.
      Vector leftOffset, upOffset;
      upOffset = GlobalCamera::Instance()->getPosition().getUpVector();
    
      Vector forwardVec;
      forwardVec = GlobalCamera::Instance()->getPosition().getOrientation();
      forwardVec.cross(upOffset, leftOffset);

      upOffset.setMagnitude(0.1f);
      leftOffset.setMagnitude(0.1f);

      Vector quadCorners[4] = {numLoc + leftOffset - upOffset, numLoc - leftOffset - upOffset,
                               numLoc - leftOffset + upOffset, numLoc + leftOffset + upOffset};

      glBindTexture(GL_TEXTURE_2D, numTextures[x]);
      glBegin(GL_QUADS);
    
      glTexCoord2f(1,1);
      glVertex3f(quadCorners[0].x, quadCorners[0].y, quadCorners[0].z);

      glTexCoord2f(1,0);
      glVertex3f(quadCorners[3].x, quadCorners[3].y, quadCorners[3].z);

      glTexCoord2f(0,0);
      glVertex3f(quadCorners[2].x, quadCorners[2].y, quadCorners[2].z);

      glTexCoord2f(0,1);
      glVertex3f(quadCorners[1].x, quadCorners[1].y, quadCorners[1].z);

      glEnd();
    }
  }
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void outputTables()
{
  std::vector<int> edgesNeeded;
  edgesNeeded.resize(128);

  // Got the list of tris.  Now compute which edges are needed for each.
  for (unsigned char x=0; x<128; x++)
  {
    int neededEdgeBits = 0;

    for (int y=0; y<triIndices[x].size(); y++)
    {
      neededEdgeBits |= (1 << triIndices[x][y]);
    }

    edgesNeeded[x] = neededEdgeBits;
  }

  // Output them in a useful format...
  std::cout << "int edgeTable[256] = " << std::endl << "{" << std::endl;
  for (x=0; x<32; x++)
  {
    std::cout << "  ";
    for (int y=0; y<8; y++)
    {
      int index = ((x*8 + y) >= 128 ? ((unsigned char)(~(x*8+y))) : (x*8+y));

      std::cout << edgesNeeded[index] << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << "};" << std::endl << std::endl;

  std::cout << "int triTable[256][13] = " << std::endl << "{" << std::endl;
  for (x=0; x<256; x++)
  {
    int index = (x >= 128 ? ((unsigned char)(~x)) : x);

    std::cout << "  {";
    for (int y=0; y<12; y++)
    {
      assert(triIndices[index].size() <= 12);

      if (x == index)
      {
        // Output as usual for the manually-entered ones.
        if (triIndices[index].size() > y) std::cout << triIndices[index][y] << ", ";
        else std::cout << "-1, ";
      }
      else
      {
        // Output in reverse winding order for the complementary ones.
        if (triIndices[index].size() > y) std::cout << triIndices[index][triIndices[index].size()-1-y] << ", ";
        else std::cout << "-1, ";
      }
    }
    std::cout << "-1}," << std::endl;
  }
  std::cout << "};" << std::endl;
}