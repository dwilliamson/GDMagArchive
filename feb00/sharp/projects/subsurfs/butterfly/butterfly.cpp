#include <harness/main.h>
#include <harness/CameraMover.h>
#include <harness/FreeCamera.h>
#include <harness/PlatformSpecific.h>
#include <harness/GlobalCamera.h>
#include <harness/OpenGL.h>
#include <math/Vector.h>
#include <gl/glut.h>
#include <subsurfs/butterfly/ButterflySurface.h>
#include <subsurfs/ControlNet.h>
#include <harness/PlatformSpecific.h>
#include <iostream>

#pragma warning (disable: 4786)

#include <vector>
#include <map>
using std::vector;

// This is our tessellator.
ButterflySurface surface;

// Goddamn 3dfx driver.
bool workaroundDone = false;

// This controls whether we draw the surface in wireframe or filled.
GLenum polyMode = GL_FILL;
GLenum shadeModel = GL_SMOOTH;
bool brightBG = true;

const char* appWindowName()
{
  return "Modified Butterfly Subdivision Surfaces.";
}

void appCleanup()
{
}

float rotAngle = 0.0f;
float spinRate = 0.0f;
float lastTime = timeGetTime() / 1000.0f;

void appInit(int argc, char** argv)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  
  float ones[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float zeros[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, ones);
  glLightfv(GL_LIGHT0, GL_SPECULAR, zeros);

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

  // Read the specified file.
  if (argc != 2)
  {
    std::cout << "ERROR: Must specify net file on command line!" << std::endl;
    exit(-1);
  }

  FILE* infile = fopen(argv[1], "rt");
  ControlNet loadNet;
  if ((infile == 0) || !loadNet.setData(infile))
  {
    std::cout << "ERROR reading from file " << argv[1] << std::endl;
    exit(-1);
  }

  loadNet.center();
  loadNet.scaleTo(6);
  loadNet.genTexCoords();

  surface.setControlNet(loadNet);
  surface.setRecursionDepth(4);
  workaroundDone = false;

  // Setup one light.
  glEnable(GL_LIGHT0);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);

  // Everything's blue.
  glColor3f(0.0f,0.2f,1.0f);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zeros);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

  glClearColor(0.8f, 0.9f, 1.0f, 1.0f);
  glLineWidth(2);
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch ( keyHit )
  {
  case ')':
  case '0':
    surface.setRecursionDepth(surface.getRecursionDepth()+1);    
    break;

  case '(':
  case '9':
    if (surface.getRecursionDepth() > 0)
    {
      surface.setRecursionDepth(surface.getRecursionDepth()-1);    
    }
    break;

  case '-':
  case '_':
    surface.setCurveThreshold(surface.getCurveThreshold() + 0.001);
    break;

  case '=':
  case '+':
    surface.setCurveThreshold(surface.getCurveThreshold() - 0.001);
    break;

  case 'p':
  case 'P':
    polyMode = (polyMode == GL_FILL) ? GL_LINE : GL_FILL;
    if (polyMode == GL_LINE)
    {
      glDisable(GL_CULL_FACE);
    }
    else
    {
      glEnable(GL_CULL_FACE);
    }
    break;

  case 's':
  case 'S':
    shadeModel = (shadeModel == GL_SMOOTH) ? GL_FLAT : GL_SMOOTH;
    glShadeModel(shadeModel);
    break;

  case 'r':
  case 'R':
    if (brightBG)
    {
      glClearColor(0,0,0,1);
    }
    else
    {
      glClearColor(0.8,0.9,1,1);
    }
    brightBG = !brightBG;
    break;

//  case '.':
//  case '>':
//    if (spinRate == 0.0f)
//    {
//      spinRate = 1.0f;
//    }
//    else spinRate *= 1.2f;
//    break;
//
//  case ',':
//  case '<':
//    if (spinRate <= 1.0f)
//    {
//      spinRate = 0.0f;
//    }
//    else spinRate *= 0.8f;
//    break;

  case 'l':
  case 'L':
    {
      int lightingOn;
      glGetIntegerv(GL_LIGHTING, &lightingOn);
      if (lightingOn == 1)
      {
        glDisable(GL_LIGHTING);
      }
      else
      {
        glEnable(GL_LIGHTING);
      }
    }
    break;

  case 'e':
  case 'E':
    {
      surface.setDrawEnvironmentMap(!surface.getDrawEnvironmentMap());
    }
    break;

  case 'T':
  case 't':
    {
      surface.setDrawTexture(!surface.getDrawTexture());
    }
    break;

  case 'G':
  case 'g':
    {
      surface.setDrawGlossMap(!surface.getDrawGlossMap());
    }
    break;
  }
}

void appDraw()
{
  glClear(GL_DEPTH_BUFFER_BIT);

  // Update any mouse movement.
  CameraMover::update();

  // Redo the matrices ourselves to be safe.
//  glMatrixMode(GL_PROJECTION);
//  glLoadIdentity();
//  glMatrixMode(GL_MODELVIEW);
//  glLoadIdentity();

  // Set up our projection matrix.  Note that because I'm
  // lazy, I'm putting the gluLookAt in the modelview matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(GlobalCamera::Instance()->getFovY(), 1.33f, 0.1f, 220.0f);

  // Clear our matrix, reposition the viewer.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  float eyeX = GlobalCamera::Instance()->getPosition().x;
  float eyeY = GlobalCamera::Instance()->getPosition().y;
  float eyeZ = GlobalCamera::Instance()->getPosition().z;
  float oriX = eyeX + GlobalCamera::Instance()->getPosition().getOrientation().x;
  float oriY = eyeY + GlobalCamera::Instance()->getPosition().getOrientation().y;
  float oriZ = eyeZ + GlobalCamera::Instance()->getPosition().getOrientation().z;
  float upvX = GlobalCamera::Instance()->getPosition().getUpVector().x;
  float upvY = GlobalCamera::Instance()->getPosition().getUpVector().y;
  float upvZ = GlobalCamera::Instance()->getPosition().getUpVector().z;

  // Position the light.
  float lightPos[4];
  lightPos[0] = 10; 
  lightPos[1] = 10; 
  lightPos[2] = 10; 
  lightPos[3] = 1.0f; 
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  gluLookAt(eyeX,eyeY,eyeZ, oriX,oriY,oriZ, upvX, upvY, upvZ);

  // Figure out how much to spin.
  float curTime = timeGetTime() / 1000.0f;
  float dt = curTime - lastTime;
  lastTime = curTime;
  float spinAmt = spinRate * dt;
  rotAngle += spinAmt;
  glRotatef(rotAngle, 0, 0, 1);

  // Draw our surface.
  glPolygonMode(GL_FRONT_AND_BACK, polyMode);
  surface.draw();

  if (!workaroundDone)
  {
    workaroundDone = true;
    surface.setRecursionDepth(0);
  }
}
