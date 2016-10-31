//
//  Curves and Curved Surfaces: Bezier patch terrain demo.
//
//      Brian Sharp, 9/28/98
//      brian_sharp@cognitoy.com
//
//  This file is the driver program for drawing a Bezier patch.  It supports different
//  kinds of tessellators, such as the (easiest, most straightforward) uniform tessellator
//  and more can easily be added.
//
//  Usage: 
//      - and + increase or decrease the tessellation granularity.
//      v toggles the display of just the patch the patch and its basis functions.
//      f toggles the size of the basis functions -- from an inset window to fullscreen.
//      c toggles the display of control points.
//      [ and ] decrease and increase the number of control points, respectively.
//      r regenerates random points.
//      p toggles between drawing the patch as a line strip or as points.
//

// The OpenGL enums and functions are used in here to some extent.  I include glut to shield myself
// from having to include windows.h
#include <gl/glut.h>

// stdlib.h is used by rand and srand.
#include <stdlib.h>

// This is the patch itself.
#include <curves/bezier/BezierPatch.h>
#include <curves/bezier/UniformPatchTessellator.h>
#include <curves/bezier/CentralPatchTessellator.h>
#include <curves/bezier/TerrainQuadtree.h>
#include <curves/bezier/CameraMover.h>
#include <curves/bezier/Mountaineer.h>
#include <curves/bezier/Marble.h>
#include <curves/OpenGL.h>
#include <curves/CurvePoint.h>
#include <curves/GlobalCamera.h>
#include <curves/Weather.h>
#include <curves/bezier/Marble.h>

// Declares the prototype for loadTexture that we need.
#include <curves/main.h>

#include <iostream>
#include <mmsystem.h>

// This value specifies the number of control points.  The actual number of points is this
// squared, as the patch is numPoints * numPoints in size.
int numPoints;

// This value is the number of steps in the tessellation.  It must be greater than 0.
int totalSteps = 20;

// This specifies whether the control points are drawn.
bool drawControls = true;

// This specifies whether or not to draw the basis functions for the patch.
bool verbose = false;

// This controls whether the basis functions are drawn in an inset window or over the whole screen.
bool basesFullscreen = false;

// This holds all our patches.
BezierPatch* bezierPatches = 0;

// This points to our tessellator used to render the patch.
BezierPatchTessellator* tessellator;

// This holds our array of control points.
CurvePoint* controls = 0;

// This is our terrain quadtree.
TerrainQuadtree terrain;

// This is the number of patches on a side (so if this number is 8, we have 8x8 patches).
// IT MUST BE A POWER OF TWO.
const int numPatches = 16;

// Prototypes: These functions are helper functions for the bezier algorithm.
void genPatch();

// These are our marbles that roll around on the terrain.
std::vector<Marble> marbles;
Marble cameraMarble;
bool useCamMarble = false;

// This, when turned on, sticks the camera to the terrain and makes it look along the terrain's
// tangent.
bool stickyCamera = false;

// Annoying but necessary: this eliminates the warnings about name-manglings that occur because of the
// use of STL containers.
#pragma warning (disable: 4786)

const char* appWindowName()
{
  return "Bezier Patch";
}

void appCleanup()
{
  delete tessellator;
}

void appInit(int argc, char** argv)
{
  // Get the number of control points.  Default to a bicubic patch if none is given.
  numPoints = 4;
  
  // Now set up extensions we use (multitexturing).
  OpenGL::initExtensions();

  // Setup the patch.
  // We want numPatches x numPatches patches, which is (3*numPatches+1) control points on a side.
  controls = new CurvePoint[ (3*numPatches+1) * (3*numPatches+1) ];
  bezierPatches = new BezierPatch[ numPatches * numPatches ];
  genPatch();

  // Generate the marble's texture.
  unsigned int marbleTex = loadTexture("marble.raw");
  Marble::setTextureName(marbleTex);

  // Set the snowflake texture.
  unsigned int snowTex = loadAlphaTexture("snowflake.raw");
  Weather::instance()->setTexture(snowTex);
  Weather::instance()->setSize(1);

  // Generate a texture for it.
  int terrainTex = loadTexture("texture00.raw");
  terrain.setTexture(terrainTex);
  terrain.setGranularity(6.6f);

  // We use z-buffering, so turn it on.
  glEnable(GL_DEPTH_TEST);

  // Also, we'd like this off since you can see either side of the surface.
  glEnable(GL_CULL_FACE);

  // Setup our mover so we can have mouse-driven camera movement.
  CameraMover::initCallbacks();

  // Also, make the mouse invisible since we do mlook-style movement now!
  glutSetCursor(GLUT_CURSOR_NONE);

  // Testing: I'm playing with fog, let's see how this looks.
  // Good earthy brown: 0.35 0.17 0.00 1.00
//  float fogColor[4] = {0.35, 0.17, 0.00, 1.00};
//  float fogColor[4] = {0.94f, 0.95f, 1.00f, 1.0f};
  float fogColor[4] = {0.00f, 0.00f, 0.00f, 1.0f};
//  float fogColor[4] = {0.9f, 0.9f, 1.0f, 1.0f};

  // Setup the colors.
  glEnable(GL_FOG);
  glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
  glFogfv(GL_FOG_COLOR, fogColor);

  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogf(GL_FOG_DENSITY, 0.015f);

  glFogf(GL_FOG_START, GlobalCamera::Instance()->getFarPlane() * 0.7f);
  glFogf(GL_FOG_END, GlobalCamera::Instance()->getFarPlane());

  glHint(GL_FOG_HINT, GL_NICEST);

  // Also, set up our weather!  Snowflakes.
  Weather::instance()->setWeatherType(Weather::Snow);
  Weather::instance()->setArea(125.0f);
  Weather::instance()->setStartZ(150);
  Weather::instance()->setEndZ(-150);
  Weather::instance()->setThickness(0.005f);
//  Weather::instance()->setThickness(0.5f);
  Weather::instance()->setVelocity(Vector(0,0,-16.0f));
}

void appKey(unsigned char keyHit, int x, int y)
{
  // Note that we're case-insensitive here (- and _ do the same thing).
  switch (keyHit)
  {
    // Generate another random set of control points.
  case 'r':
  case 'R':
    genPatch();
    break;
    
    // Toggle between drawing the patch as filled, lines, or points.
  case 'p':
  case 'P':
    if (terrain.getMode() == GL_FILL)
    {
      terrain.setMode(GL_LINE);
    }
//    else if (terrain.getMode() == GL_LINE)
//    {
//      terrain.setMode(GL_POINT);
//    }
    else
    {
      terrain.setMode(GL_FILL);
    }
    break;

    // Turn up the tessellation.
  case '+':
  case '=':
    terrain.setGranularity(terrain.getGranularity() + 0.1f);
    break;

    // Turn down the tessellation.
  case '-':
  case '_':
    terrain.setGranularity(terrain.getGranularity() - 0.1f);
    break;

    // Toggle drawing of basis functions for the patch.
  case 'v':
  case 'V':
    verbose = !verbose;
    break;

  case 'b':
  case 'B':
    basesFullscreen = !basesFullscreen;
    break;

  case 'f':
  case 'F':
    terrain.setFixCracks(!terrain.getFixCracks());
    break;

  case '9':
  case '(':
    terrain.setMaxDepth(max(terrain.getMaxDepth() - 1, 0));
    break;

  case '0':
  case ')':
    terrain.setMaxDepth(terrain.getMaxDepth() + 1);
    break;

  case 'l':
  case 'L':
    terrain.setUseLightmap(!terrain.getUseLightmap());
    break;

  case 't':
  case 'T':
    terrain.setUseTexture(!terrain.getUseTexture());
    break;

  case '6':
  case '^':
    GlobalCamera::Instance()->setFovY(GlobalCamera::Instance()->getFovY() - 5);
    break;

  case '7':
  case '&':
    GlobalCamera::Instance()->setFovY(50);
    break;

  case '8':
  case '*':
    GlobalCamera::Instance()->setFovY(GlobalCamera::Instance()->getFovY() + 5);
    break;

  case 'z':
  case 'Z':
    terrain.setFixInterCracks(!terrain.getFixInterCracks());
    break;

  case 'm':
  case 'M':
    {
      const Position& pos = GlobalCamera::Instance()->getPosition();
      marbles.push_back(Marble(pos.x, pos.y, pos.z));
    }
    break;

  case 'n':
  case 'N':
    marbles.clear();
    break;

  case 'j':
  case 'J':
    {
      const Position& pos = GlobalCamera::Instance()->getPosition();
      cameraMarble = Marble(pos.x, pos.y, pos.z, 0.0f);
      useCamMarble = true;
    }
    break;

  case 'k':
  case 'K':
    useCamMarble = false;
    break;

  case 'h':
  case 'H':
    stickyCamera = !stickyCamera;
    break;
  }
}

#define RAND_NEG1_TO_POS1 ((2.0f * rand() / (float)RAND_MAX) - 1.0f)

void appDraw()
{
  // Update the camera.
  CameraMover::update();

  // If we're supposed to tie the camera to this rolling marble...
  if (useCamMarble)
  {
    cameraMarble.update(terrain);

    const Vector& camPt = cameraMarble.getPosition();

    // If we've fallen off the edge a reasonable distance, start us back up high again.
    if (camPt.z < -100.0f)
    {
      float randX = RAND_NEG1_TO_POS1 * 100;
      float randY = RAND_NEG1_TO_POS1 * 100;
      float newZ = 100.0f;
      cameraMarble = Marble(randX, randY, newZ, 0.0f);
      cameraMarble.update(terrain);
    }

    // Generate a look vector for the camera, in the marble's direction and along the terrain.
    Vector camLook(cameraMarble.getVelocity());

    Vector camUp(0,0,1);

    // If we're pointing down, make the up vector the negative surface normal at this point for good measure.
    if (fabs(camLook.x) + fabs(camLook.y) < 0.0001)
    {
      if (terrain.contains(camPt.x, camPt.y))
      {
        camUp = terrain.getNormalAt(camPt.x, camPt.y);
//        camUp *= -1.0f;
      }
    }
    else
    {
      // Don't want to go overboard with that, so damp it a little.
      camLook.normalize();
      camLook += Vector(camLook.x, camLook.y, 0);
    }

    Position& pos = GlobalCamera::Instance()->getPosition();
    pos.x = camPt.x;
    pos.y = camPt.y;
    pos.z = camPt.z + 4.0f;
    pos.setOrientation(camLook);
    pos.setUpVector(camUp);
  }

  else if (stickyCamera)
  {
    Position& camPos = GlobalCamera::Instance()->getPosition();

    if (terrain.contains(camPos.x, camPos.y))
    {
      camPos.z = terrain.getValueAt(camPos.x, camPos.y) + 4.0f;

      // Generate a look vector for the camera, in the camera's direction and along the terrain.
      Vector camLook(camPos.getOrientation());
      camLook.z = 0;
      camLook.normalize();
      Vector grad = terrain.getGradientAt(camPos.x, camPos.y);
      BezierPatch::getVectorInGradientPlane(grad, camLook);
      camPos.setOrientation(camLook);
      camPos.setUpVector(Vector(0,0,1));
    }
  }

  // Quick hack for aesthetics: make the camera at least 1 unit above the terrain.
  else if (terrain.contains(GlobalCamera::Instance()->getPosition().x, GlobalCamera::Instance()->getPosition().y))
  {
    float heightVal = terrain.getValueAt(GlobalCamera::Instance()->getPosition().x, GlobalCamera::Instance()->getPosition().y);
    if (GlobalCamera::Instance()->getPosition().z < heightVal + 4.0f)
    {
      GlobalCamera::Instance()->getPosition().z = heightVal+4.0f;
    }
  }

  // Unfortunately, I have to redo the matrices now to take the camera movement into account.
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

  gluLookAt(eyeX,eyeY,eyeZ, oriX,oriY,oriZ, upvX, upvY, upvZ);

  // Unlike some of these demos, we use the z-buffer, so we need to clear it.
  ::glClear(GL_DEPTH_BUFFER_BIT);

  terrain.draw();

  // Also, draw any marbles we have.
  for (std::vector<Marble>::iterator it = marbles.begin(); it!=marbles.end();)
  {
    // Delete them when they roll off the terrain.
    if (!it->update(terrain))
    {
      it = marbles.erase(it);
    }
    else
    {
      it->draw();
      it++;
    }
  }

  // Draw any specified weather system.
  Weather::instance()->update();
  Weather::instance()->draw();

  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
  {
    std::cout << gluErrorString(err) << std::endl;
  }
}

void genPatch()
{
  // Seed the random number generator to change the points each time we run.
  srand(timeGetTime());

  // Generate the points.  We have to be somewhat careful about how we do this to
  // preserve some kind of continuity between the patches.  Basically, each three corner
  // points on a patch need to be colinear, and since they're shared between patches, that
  // means that a five-point square cross centered on every corner point needs to be colinear.
  // So we generate three of those five points (two points and the center point) and then
  // derive the other two points from that.

  // Randomly generate all the points.  We'll replace some of them, but this is quick and concise.
  int numControls = numPatches * 3 + 1;

  for (int i = 0; i < numControls; i++)
  {
    for (int j = 0; j < numControls; j++)
    {
      // We want a reasonable distribution from around -125 to 125.
      float x = ((float)(i - (float)numControls * 0.5) / (float)numControls) * 250;
      float y = ((float)(j - (float)numControls * 0.5) / (float)numControls) * 250;
      controls[ i + (numControls*j) ] = CurvePoint(x, y, 0);
    }
  }

  // Now generate the height values for those patches.
  Mountaineer().generateFractalTerrain(controls, numControls, 0.3f, 0.5f);

  int patchI, j;

  // Now, the points in the i direction -- take the point to the left of the corner, and generate
  // the point to the right from the left and the corner.
  for (i = 3; i < numControls - 1; i += 3)
  {
    for (j = 0; j < numControls; j += 3)
    {
      // Because the points are evenly spaced in x and y, to make them colinear, the right point is
      // equal to the center minus the difference from the left point to the center, or
      // center - (left - center), or 2*center - left.
      controls[ (i+1) + (numControls*j) ] = controls[ i + (numControls*j) ];
      controls[ (i+1) + (numControls*j) ] *= 2;
      controls[ (i+1) + (numControls*j) ] -= controls[ (i-1) + (numControls*j) ];
    }
  }

  // Now do the same for the points in the j direction.
  for (j = 3; j < numControls - 1; j += 3)
  {
    for (i = 0; i < numControls; i += 3)
    {
      // Because the points are evenly spaced in x and y, to make them colinear, the low point is
      // equal to the center minus the difference from the high point to the center, or
      // center - (high - center), or 2*center - high.
      controls[ i + (numControls*(j+1)) ] = controls[ i + (numControls*j) ];
      controls[ i + (numControls*(j+1)) ] *= 2;
      controls[ i + (numControls*(j+1)) ] -= controls[ i + (numControls*(j-1)) ];
    }
  }

  // Okay, now we have to make sure the interior points are consistent.  This is a very limiting
  // continuity regime, but implementing the less limiting one looked rather like a nightmare, so
  // this one will do for now.
  for (patchI = 0; patchI < numPatches; patchI++)
  {
    // Calculate the starting i offset for this patch.
    int iOff = patchI * 3;

    for (int patchJ = 0; patchJ < numPatches; patchJ++)
    {
      // Calculate the starting j offset for this patch.
      int jOff = patchJ * 3;
      
      // Interior point from the upper-left corner.
      float cornerZ = controls[ (iOff) + ((jOff)*numControls) ].getZ();
      float horizZ = controls[ (iOff+1) + ((jOff)*numControls) ].getZ();
      float vertZ = controls[ (iOff) + ((jOff+1)*numControls) ].getZ();
      controls[ (iOff+1) + ((jOff+1)*numControls) ].setZ(vertZ + horizZ - cornerZ);

      // Interior point from the upper-right corner.
      cornerZ = controls[ (iOff+3) + ((jOff)*numControls) ].getZ();
      horizZ = controls[ (iOff+2) + ((jOff)*numControls) ].getZ();
      vertZ = controls[ (iOff+3) + ((jOff+1)*numControls) ].getZ();
      controls[ (iOff+2) + ((jOff+1)*numControls) ].setZ(vertZ + horizZ - cornerZ);

      // Interior point from the lower-right corner.
      cornerZ = controls[ (iOff+3) + ((jOff+3)*numControls) ].getZ();
      horizZ = controls[ (iOff+2) + ((jOff+3)*numControls) ].getZ();
      vertZ = controls[ (iOff+3) + ((jOff+2)*numControls) ].getZ();
      controls[ (iOff+2) + ((jOff+2)*numControls) ].setZ(vertZ + horizZ - cornerZ);

      // Interior point from the lower-left corner.
      cornerZ = controls[ (iOff) + ((jOff+3)*numControls) ].getZ();
      horizZ = controls[ (iOff+1) + ((jOff+3)*numControls) ].getZ();
      vertZ = controls[ (iOff) + ((jOff+2)*numControls) ].getZ();
      controls[ (iOff+1) + ((jOff+2)*numControls) ].setZ(vertZ + horizZ - cornerZ);
    }
  }

  // Okay!  Now pass those points off to our patches.
  std::vector< std::vector< CurvePoint > > pts;

  for (patchI = 0; patchI < numPatches; patchI++)
  {
    // Calculate the starting i offset for this patch.
    int iOff = patchI * 3;

    for (int patchJ = 0; patchJ < numPatches; patchJ++)
    {
      // Empty out the control point double-vector for each new patch.
      pts.clear();

      // Calculate the starting j offset for this patch.
      int jOff = patchJ * 3;
      
      for (i = 0; i < 4; i++)
      {
        // Every row, add a row to the double-vector.
        pts.push_back(std::vector< CurvePoint >());

        // Now, go through the controls on this row and add them.
        for (j = 0; j < 4; j++)
        {
          pts[ i ].push_back(controls[ (i+iOff) + (numControls*(j+jOff)) ]);
        }
      }

      // Use those controls points as the controls of the current patch.
      bezierPatches[ patchI + (numPatches*patchJ) ].setControlPoints(pts);

    }
  }

  // Now generate all the lightmaps.
  UniformPatchTessellator lightmapper;
  long startTime = timeGetTime();
  
  for (int x = 0; x < numPatches * numPatches; x++)
  {
    bezierPatches[x].generateLightmap(&lightmapper, 8);
  }
  
  long endTime = timeGetTime();
  std::cout << "Lightmapping took " << (endTime - startTime) << std::endl;

  // Save any textures the terrain had.
  int textureName = terrain.getTexture();

  // Now hand those to the TerrainQuadtree.
  terrain.setSize(numPatches);
  for (x=0; x<numPatches; x++)
  {
    for (int y=0; y<numPatches; y++)
    {
      terrain.setPatch(bezierPatches[x + y*numPatches], x, y);
    }
  }
  terrain.finalizePatches();

  // Reset the texture.
  terrain.setTexture(textureName);
}
