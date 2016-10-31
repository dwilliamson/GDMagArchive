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

#include <math/Vector.h>

#include <iostream>
#include <gl/glut.h>

#include <fluid/Render/ImplicitMesh.h>
#include <fluid/Tess/MarchingCube.h>
#include <fluid/Tess/CubePolygonizer.h>

ImplicitMesh mesh;
MarchingCube cube;
CubePolygonizer tess;

unsigned char cubeConfig = 0;

#pragma warning (disable: 4786)

GLenum polyMode = GL_FILL;

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

  mesh.setUseTexCoordArrays(false);
  mesh.setUseNormalArrays(false);
  mesh.setUseColorArrays(false);
  tess.setImplicitMesh(&mesh);
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
  case ')':
  case '0':
    break;

  case '(':
  case '9':
    break;

  case '-':
  case '_':
    cubeConfig--;
    break;

  case '=':
  case '+':
    cubeConfig++;
    break;

  case 'p':
  case 'P':
    polyMode = (polyMode == GL_FILL) ? GL_LINE : GL_FILL;
    break;

  case '.':
  case '>':
    break;

  case ',':
  case '<':
    break;

  case 'l':
  case 'L':
    {
      if (glIsEnabled(GL_LIGHTING))
      {
        glDisable(GL_LIGHTING);
      }
      else
      {
        glEnable(GL_LIGHTING);
      }
    }
    break;
  }
}

void appDraw()
{
  glClear(GL_DEPTH_BUFFER_BIT);

  // Update any mouse movement.
  CameraMover::update();

  // Re-init the camera.
  GlobalCamera::Instance()->makeCurrent();

  // Draw our surface.
  glPolygonMode(GL_FRONT_AND_BACK, polyMode);

  mesh.clear();
  tess.beginTessellation();

  cube.setBoundingBox(0,8,0,8,0,8);
  cube.setCellDensity(1,1,1);
  cube.setCurrentCube(0,0,0,1);

  int neighbors;
  
  float vals[8];

  if (cubeConfig == 15)
  {
    int breakpoint = 0;
  }
  for (int x=0; x<8; x++)
  {
    vals[x] = (cubeConfig & (1<<x)) ? 0.1 : 0.9;
  }

  tess.polygonize(&cube, 0.5, neighbors, vals, true);

  glColor3f(0,0,1);
  mesh.render();

  glColor3f(0,1,0);
  glBegin(GL_LINES);

#define VERTEX(x) glVertex3fv(cube.getCorner(x))
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

  VERTEX(1); VERTEX(4);
  VERTEX(2); VERTEX(5);
  VERTEX(2); VERTEX(7);
  VERTEX(3); VERTEX(4);

  VERTEX(1); VERTEX(3);
  VERTEX(5); VERTEX(7);

  VERTEX(3); VERTEX(5);

#undef VERTEX

  glEnd();
}
